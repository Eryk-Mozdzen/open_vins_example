#include <linux/fs.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define MPU6050_ADDR          0x68
#define MPU6050_REG_ACCEL_OUT 0x3B
#define MPU6050_SAMPLE_BYTES  14
#define FIFO_MAX_SAMPLES      1024

typedef struct {
    ktime_t timestamp;
    int16_t accel[3];
    int16_t temperature;
    int16_t gyro[3];
} mpu6050_sample_t;

typedef struct {
    struct i2c_client *client;
    struct gpio_desc *irq_gpiod;
    int irq;
    struct kfifo fifo;
    wait_queue_head_t readq;
    struct miscdevice miscdev;
    struct mutex lock;
} mpu6050_t;

static irqreturn_t mpu6050_interrupt(int irq, void *user) {
    (void)irq;

    mpu6050_t *mpu6050 = user;
    uint8_t data[MPU6050_SAMPLE_BYTES];

    mutex_lock(&mpu6050->lock);
    const int ret = i2c_smbus_read_i2c_block_data(mpu6050->client, MPU6050_REG_ACCEL_OUT,
                                                  MPU6050_SAMPLE_BYTES, data);
    mutex_unlock(&mpu6050->lock);

    if(ret < 0) {
        dev_err(&mpu6050->client->dev, "I2C read failed: %d\n", ret);
        return IRQ_HANDLED;
    }

    const mpu6050_sample_t sample = {
        .timestamp = ktime_get(),
        .accel[0] = (data[0] << 8) | data[1],
        .accel[1] = (data[2] << 8) | data[3],
        .accel[2] = (data[4] << 8) | data[5],
        .temperature = (data[6] << 8) | data[7],
        .gyro[0] = (data[8] << 8) | data[9],
        .gyro[1] = (data[10] << 8) | data[11],
        .gyro[2] = (data[12] << 8) | data[13],
    };

    if(!kfifo_is_full(&mpu6050->fifo)) {
        kfifo_in(&mpu6050->fifo, &sample, sizeof(sample));
        wake_up_interruptible(&mpu6050->readq);
    } else {
        dev_warn(&mpu6050->client->dev, "FIFO full, dropping sample\n");
    }

    return IRQ_HANDLED;
}

static irqreturn_t mpu_irq_top(int irq, void *user) {
    return IRQ_WAKE_THREAD;
}

static ssize_t mpu6050_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    mpu6050_t *mpu6050 = container_of(file->private_data, mpu6050_t, miscdev);
    size_t avail;
    size_t to_copy;
    int ret;

    if(count % sizeof(mpu6050_sample_t)) {
        return -EINVAL;
    }

    if(kfifo_is_empty(&mpu6050->fifo)) {
        if(file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }

        if(wait_event_interruptible(mpu6050->readq, !kfifo_is_empty(&mpu6050->fifo))) {
            return -ERESTARTSYS;
        }
    }

    avail = kfifo_len(&mpu6050->fifo);
    to_copy = min(avail, count);

    void *kbuf = kmalloc(to_copy, GFP_KERNEL);
    if(!kbuf) {
        return -ENOMEM;
    }

    ret = kfifo_out(&mpu6050->fifo, kbuf, to_copy);
    if(ret != (int)to_copy) {
        kfree(kbuf);
        return -EIO;
    }

    if(copy_to_user(buf, kbuf, to_copy)) {
        kfree(kbuf);
        return -EFAULT;
    }

    kfree(kbuf);

    return to_copy;
}

static unsigned int mpu6050_poll(struct file *file, poll_table *wait) {
    mpu6050_t *mpu6050 = container_of(file->private_data, mpu6050_t, miscdev);

    unsigned int mask = 0;
    poll_wait(file, &mpu6050->readq, wait);

    if(!kfifo_is_empty(&mpu6050->fifo)) {
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

static const struct file_operations mpu6050_fops = {
    .owner = THIS_MODULE,
    .read = mpu6050_read,
    .poll = mpu6050_poll,
};

static int mpu6050_probe(struct i2c_client *client) {
    dev_info(&client->dev, "probing MPU6050\n");

    int ret;
    mpu6050_t *mpu6050;

    mpu6050 = devm_kzalloc(&client->dev, sizeof(*mpu6050), GFP_KERNEL);
    if(!mpu6050) {
        return -ENOMEM;
    }

    i2c_set_clientdata(client, mpu6050);
    mpu6050->client = client;
    mutex_init(&mpu6050->lock);
    init_waitqueue_head(&mpu6050->readq);

    ret = kfifo_alloc(&mpu6050->fifo, FIFO_MAX_SAMPLES * sizeof(mpu6050_sample_t), GFP_KERNEL);
    if(ret) {
        dev_err(&client->dev, "kfifo_alloc failed\n");
        return -ENOMEM;
    }

    mpu6050->irq_gpiod = devm_gpiod_get_optional(&client->dev, "irq", GPIOD_IN);
    if(IS_ERR(&mpu6050->irq_gpiod)) {
        dev_err(&client->dev, "failed to get irq gpio\n");
        ret = PTR_ERR(mpu6050->irq_gpiod);
        goto err_fifo;
    }

    if(mpu6050->irq_gpiod) {
        mpu6050->irq = gpiod_to_irq(mpu6050->irq_gpiod);
        if(mpu6050->irq < 0) {
            dev_err(&client->dev, "gpiod_to_irq failed\n");
            ret = mpu6050->irq;
            goto err_fifo;
        }

        ret =
            devm_request_threaded_irq(&client->dev, mpu6050->irq, mpu_irq_top, mpu6050_interrupt,
                                      IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "MPU6050 IRQ", mpu6050);
        if(ret) {
            dev_err(&client->dev, "request_threaded_irq failed: %d\n", ret);
            goto err_fifo;
        }
    } else {
        dev_warn(&client->dev, "no irq gpio configured; falling back to polling\n");
    }

    mutex_lock(&mpu6050->lock);
    ret = i2c_smbus_write_byte_data(client, 0x6B, 0x00);
    mutex_unlock(&mpu6050->lock);
    if(ret) {
        dev_warn(&client->dev, "failed to wake MPU6050: %d\n", ret);
    }

    mpu6050->miscdev.minor = MISC_DYNAMIC_MINOR;
    mpu6050->miscdev.name = "mpu6050";
    mpu6050->miscdev.fops = &mpu6050_fops;
    mpu6050->miscdev.parent = &client->dev;
    ret = misc_register(&mpu6050->miscdev);
    if(ret) {
        dev_err(&client->dev, "misc_register failed: %d\n", ret);
        goto err_fifo;
    }

    dev_info(&client->dev, "MPU6050 driver loaded; char device /dev/%s\n", mpu6050->miscdev.name);

    return 0;

err_fifo:
    kfifo_free(&mpu6050->fifo);
    return ret;
}

static void mpu6050_remove(struct i2c_client *client) {
    mpu6050_t *mpu6050 = i2c_get_clientdata(client);

    misc_deregister(&mpu6050->miscdev);
    kfifo_free(&mpu6050->fifo);

    dev_info(&client->dev, "MPU6050 driver removed\n");
}

static const struct of_device_id mpu6050_match[] = {
    {
     .compatible = "invensense,mpu6050",
     },
    {},
};
MODULE_DEVICE_TABLE(of, mpu6050_match);

static const struct i2c_device_id mpu6050_id[] = {
    {"mpu6050", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, mpu6050_id);

static struct i2c_driver mpu6050_i2c_driver = {
    .driver =
        {
                 .name = "mpu6050",
                 .of_match_table = of_match_ptr(mpu6050_match),
                 },
    .probe = mpu6050_probe,
    .remove = mpu6050_remove,
    .id_table = mpu6050_id,
};

module_i2c_driver(mpu6050_i2c_driver);

MODULE_AUTHOR("Eryk Możdżeń");
MODULE_DESCRIPTION("MPU6050 I2C+IRQ driver");
MODULE_LICENSE("GPL v2");
