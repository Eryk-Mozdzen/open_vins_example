#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kfifo.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

#include "mpu6050.h"

#define GPIO_INT  586
#define FIFO_SIZE 1024

typedef struct {
    int64_t timestamp;
    int16_t gyro[3];
    int16_t accel[3];
    uint8_t _padding[4];
} sample_t;

typedef struct {
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int irq;
    struct kfifo fifo;
    spinlock_t lock;
    struct miscdevice miscdev;
} device_t;

static void reg_write(struct i2c_client *client, const uint8_t reg, const uint8_t value) {
    if(i2c_smbus_write_byte_data(client, reg, value) < 0) {
        dev_err(&client->dev, "mpu6050: failed to write 0x%02X to reg 0x%02X\n", value, reg);
    }
}

static irqreturn_t isr_drdy(int irq, void *dev_id) {
    (void)irq;

    device_t *device = dev_id;

    uint8_t buffer[14];
    int ret;

    ret = i2c_smbus_read_i2c_block_data(device->client, MPU6050_REG_ACCEL_XOUT_H, 14, buffer);
    if(ret < 0) {
        dev_err(&device->client->dev, "I2C read failed: %d\n", ret);
        return IRQ_HANDLED;
    }

    const sample_t sample = {
        .timestamp = ktime_to_ns(ktime_get()),
        .gyro[0] = (((int16_t)buffer[8]) << 8) | buffer[9],
        .gyro[1] = (((int16_t)buffer[10]) << 8) | buffer[11],
        .gyro[2] = (((int16_t)buffer[12]) << 8) | buffer[13],
        .accel[0] = (((int16_t)buffer[0]) << 8) | buffer[1],
        .accel[1] = (((int16_t)buffer[2]) << 8) | buffer[3],
        .accel[2] = (((int16_t)buffer[4]) << 8) | buffer[5],
    };

    unsigned long flags;
    spin_lock_irqsave(&device->lock, flags);
    if(!kfifo_is_full(&device->fifo)) {
        kfifo_in(&device->fifo, &sample, sizeof(sample));
    }
    spin_unlock_irqrestore(&device->lock, flags);

    return IRQ_HANDLED;
}

static int i2c_probe(struct i2c_client *client) {
    int ret;

    device_t *device = i2c_get_clientdata(client);

    ret = gpio_request(GPIO_INT, "mpu6050_irq");
    if(ret) {
        dev_err(&client->dev, "failed to request GPIO: %d\n", ret);
        return ret;
    }

    ret = gpio_direction_input(GPIO_INT);
    if(ret) {
        dev_err(&client->dev, "failed to set GPIO direction\n");
        gpio_free(GPIO_INT);
        return ret;
    }

    ret = gpio_to_irq(GPIO_INT);
    if(ret < 0) {
        dev_err(&client->dev, "failed to obtain IRQ: %d\n", ret);
        gpio_free(GPIO_INT);
        return ret;
    }
    device->irq = ret;

    ret = devm_request_threaded_irq(&client->dev, device->irq, NULL, isr_drdy,
                                    IRQF_TRIGGER_RISING | IRQF_ONESHOT, "mpu6050_irq", device);
    if(ret) {
        dev_err(&client->dev, "failed to request IRQ: %d\n", ret);
        gpio_free(GPIO_INT);
        return ret;
    }

    reg_write(device->client, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR_MGMT_1_DEVICE_RESET);
    mdelay(10);
    reg_write(device->client, MPU6050_REG_SIGNAL_PATH_RESET,
              MPU6050_SIGNAL_PATH_RESET_GYRO | MPU6050_SIGNAL_PATH_RESET_ACCEL |
                  MPU6050_SIGNAL_PATH_RESET_TEMP);
    mdelay(10);
    reg_write(device->client, MPU6050_REG_INT_ENABLE,
              MPU6050_INT_ENABLE_FIFO_OVERLOW_DISABLE | MPU6050_INT_ENABLE_I2C_MST_INT_DISABLE |
                  MPU6050_INT_ENABLE_DATA_RDY_ENABLE);
    reg_write(device->client, MPU6050_REG_INT_PIN_CFG,
              MPU6050_INT_PIN_CFG_LEVEL_ACTIVE_HIGH | MPU6050_INT_PIN_CFG_PUSH_PULL |
                  MPU6050_INT_PIN_CFG_PULSE | MPU6050_INT_PIN_CFG_STATUS_CLEAR_AFTER_ANY |
                  MPU6050_INT_PIN_CFG_FSYNC_DISABLE | MPU6050_INT_PIN_CFG_I2C_BYPASS_DISABLE);
    reg_write(device->client, MPU6050_REG_PWR_MGMT_1,
              MPU6050_PWR_MGMT_1_TEMP_DIS | MPU6050_PWR_MGMT_1_CLOCK_GYRO_X);
    reg_write(device->client, MPU6050_REG_CONFIG,
              MPU6050_CONFIG_EXT_SYNC_DISABLED | MPU6050_CONFIG_DLPF_SETTING_1);
    reg_write(device->client, MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_CONFIG_RANGE_4G);
    reg_write(device->client, MPU6050_REG_GYRO_CONFIG, MPU6050_GYRO_CONFIG_RANGE_500DPS);
    reg_write(device->client, MPU6050_REG_SMPLRT_DIV, 4);

    dev_info(&client->dev, "started\n");
    return 0;
}

static void i2c_remove(struct i2c_client *client) {
    (void)client;

    gpio_free(GPIO_INT);
}

static const struct i2c_device_id i2c_ids[] = {
    {"mpu6050", 0},
    {}
};

static struct i2c_driver i2c_driver = {
    .driver =
        {
                 .name = "mpu6050",
                 },
    .probe = i2c_probe,
    .remove = i2c_remove,
    .id_table = i2c_ids,
};

static device_t device;

static ssize_t fops_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    (void)file;
    (void)ppos;

    int ret;

    if(count < sizeof(sample_t)) {
        return -EINVAL;
    }

    sample_t sample;
    unsigned long flags;
    spin_lock_irqsave(&device.lock, flags);
    if(!kfifo_is_empty(&device.fifo)) {
        ret = kfifo_out(&device.fifo, &sample, sizeof(sample));
    } else {
        ret = 0;
    }
    spin_unlock_irqrestore(&device.lock, flags);

    if(ret == 0) {
        return 0;
    }

    if(copy_to_user(buf, &sample, sizeof(sample))) {
        return -EFAULT;
    }

    return sizeof(sample);
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = fops_read,
};

static int mod_init(void) {
    int ret;

    spin_lock_init(&device.lock);

    device.miscdev.fops = &fops;
    device.miscdev.minor = MISC_DYNAMIC_MINOR;
    device.miscdev.name = "mpu6050";
    ret = misc_register(&device.miscdev);
    if(ret) {
        pr_err("mpu6050: failed to register miscdev: %d\n", ret);
        return ret;
    }

    ret = kfifo_alloc(&device.fifo, FIFO_SIZE, GFP_KERNEL);
    if(ret) {
        pr_err("mpu6050: failed to allocate FIFO: %d\n", ret);
        return ret;
    }

    device.adapter = i2c_get_adapter(1);
    if(!device.adapter) {
        pr_err("mpu6050: failed to get I2C adapter\n");
        return -ENODEV;
    }

    struct i2c_board_info info = {
        I2C_BOARD_INFO("mpu6050", 0x68),
    };

    device.client = i2c_new_client_device(device.adapter, &info);
    if(IS_ERR(device.client)) {
        pr_err("mpu6050: failed to create client device\n");
        i2c_put_adapter(device.adapter);
        return PTR_ERR(device.client);
    }

    i2c_set_clientdata(device.client, &device);

    return i2c_add_driver(&i2c_driver);
}

static void mod_exit(void) {
    misc_deregister(&device.miscdev);
    kfifo_free(&device.fifo);

    i2c_del_driver(&i2c_driver);

    if(device.client) {
        i2c_unregister_device(device.client);
    }

    if(device.adapter) {
        i2c_put_adapter(device.adapter);
    }

    pr_info("mpu6050: exit\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_DEVICE_TABLE(i2c, i2c_ids);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MPU6050 device driver");
MODULE_AUTHOR("Eryk Możdżeń");
