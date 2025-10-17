#include <linux/circ_buf.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

#define MPU6050_ADDR_ADO_0                          0x68
#define MPU6050_ADDR_ADO_1                          0x69
#define MPU6050_REG_SELF_TEST_X                     0x0D
#define MPU6050_REG_SELF_TEST_Y                     0x0E
#define MPU6050_REG_SELF_TEST_Z                     0x0F
#define MPU6050_REG_SELF_TEST_A                     0x10
#define MPU6050_REG_SMPLRT_DIV                      0x19
#define MPU6050_REG_CONFIG                          0x1A
#define MPU6050_REG_GYRO_CONFIG                     0x1B
#define MPU6050_REG_ACCEL_CONFIG                    0x1C
#define MPU6050_REG_FIFO_EN                         0x23
#define MPU6050_REG_I2C_MST_CTRL                    0x24
#define MPU6050_REG_I2C_SLV0_ADDR                   0x25
#define MPU6050_REG_I2C_SLV0_REG                    0x26
#define MPU6050_REG_I2C_SLV0_CTRL                   0x27
#define MPU6050_REG_I2C_SLV1_ADDR                   0x28
#define MPU6050_REG_I2C_SLV1_REG                    0x29
#define MPU6050_REG_I2C_SLV1_CTRL                   0x2A
#define MPU6050_REG_I2C_SLV2_ADDR                   0x2B
#define MPU6050_REG_I2C_SLV2_REG                    0x2C
#define MPU6050_REG_I2C_SLV2_CTRL                   0x2D
#define MPU6050_REG_I2C_SLV3_ADDR                   0x2E
#define MPU6050_REG_I2C_SLV3_REG                    0x2F
#define MPU6050_REG_I2C_SLV3_CTRL                   0x30
#define MPU6050_REG_I2C_SLV4_ADDR                   0x31
#define MPU6050_REG_I2C_SLV4_REG                    0x32
#define MPU6050_REG_I2C_SLV4_DO                     0x33
#define MPU6050_REG_I2C_SLV4_CTRL                   0x34
#define MPU6050_REG_I2C_SLV4_DI                     0x35
#define MPU6050_REG_I2C_MST_STATUS                  0x36
#define MPU6050_REG_INT_PIN_CFG                     0x37
#define MPU6050_REG_INT_ENABLE                      0x38
#define MPU6050_REG_INT_STATUS                      0x3A
#define MPU6050_REG_ACCEL_XOUT_H                    0x3B
#define MPU6050_REG_ACCEL_XOUT_L                    0x3C
#define MPU6050_REG_ACCEL_YOUT_H                    0x3D
#define MPU6050_REG_ACCEL_YOUT_L                    0x3E
#define MPU6050_REG_ACCEL_ZOUT_H                    0x3F
#define MPU6050_REG_ACCEL_ZOUT_L                    0x40
#define MPU6050_REG_TEMP_OUT_H                      0x41
#define MPU6050_REG_TEMP_OUT_L                      0x42
#define MPU6050_REG_GYRO_XOUT_H                     0x43
#define MPU6050_REG_GYRO_XOUT_L                     0x44
#define MPU6050_REG_GYRO_YOUT_H                     0x45
#define MPU6050_REG_GYRO_YOUT_L                     0x46
#define MPU6050_REG_GYRO_ZOUT_H                     0x47
#define MPU6050_REG_GYRO_ZOUT_L                     0x48
#define MPU6050_REG_EXT_SENS_DATA_00                0x49
#define MPU6050_REG_EXT_SENS_DATA_01                0x4A
#define MPU6050_REG_EXT_SENS_DATA_02                0x4B
#define MPU6050_REG_EXT_SENS_DATA_03                0x4C
#define MPU6050_REG_EXT_SENS_DATA_04                0x4D
#define MPU6050_REG_EXT_SENS_DATA_05                0x4E
#define MPU6050_REG_EXT_SENS_DATA_06                0x4F
#define MPU6050_REG_EXT_SENS_DATA_07                0x50
#define MPU6050_REG_EXT_SENS_DATA_08                0x51
#define MPU6050_REG_EXT_SENS_DATA_09                0x52
#define MPU6050_REG_EXT_SENS_DATA_10                0x53
#define MPU6050_REG_EXT_SENS_DATA_11                0x54
#define MPU6050_REG_EXT_SENS_DATA_12                0x55
#define MPU6050_REG_EXT_SENS_DATA_13                0x56
#define MPU6050_REG_EXT_SENS_DATA_14                0x57
#define MPU6050_REG_EXT_SENS_DATA_15                0x58
#define MPU6050_REG_EXT_SENS_DATA_16                0x59
#define MPU6050_REG_EXT_SENS_DATA_17                0x5A
#define MPU6050_REG_EXT_SENS_DATA_18                0x5B
#define MPU6050_REG_EXT_SENS_DATA_19                0x5C
#define MPU6050_REG_EXT_SENS_DATA_20                0x5D
#define MPU6050_REG_EXT_SENS_DATA_21                0x5E
#define MPU6050_REG_EXT_SENS_DATA_22                0x5F
#define MPU6050_REG_EXT_SENS_DATA_23                0x60
#define MPU6050_REG_I2C_SLV0_DO                     0x63
#define MPU6050_REG_I2C_SLV1_DO                     0x64
#define MPU6050_REG_I2C_SLV2_DO                     0x65
#define MPU6050_REG_I2C_SLV3_DO                     0x66
#define MPU6050_REG_I2C_MST_DELAY_CTRL              0x67
#define MPU6050_REG_SIGNAL_PATH_RESET               0x68
#define MPU6050_REG_USER_CTRL                       0x6A
#define MPU6050_REG_PWR_MGMT_1                      0x6B
#define MPU6050_REG_PWR_MGMT_2                      0x6C
#define MPU6050_REG_FIFO_COUNTH                     0x72
#define MPU6050_REG_FIFO_COUNTL                     0x73
#define MPU6050_REG_FIFO_R_W                        0x74
#define MPU6050_REG_WHO_AM_I                        0x75
#define MPU6050_CONFIG_EXT_SYNC_DISABLED            (0x00 << 3)
#define MPU6050_CONFIG_EXT_SYNC_TEMP_OUT_L          (0x01 << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_XOUT_L         (0x02 << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_YOUT_L         (0x03 << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_ZOUT_L         (0x04 << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_XOUT_L        (0x05 << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_YOUT_L        (0x06 << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_ZOUT_L        (0x07 << 3)
#define MPU6050_CONFIG_DLPF_SETTING_0               (0x00 << 0)
#define MPU6050_CONFIG_DLPF_SETTING_1               (0x01 << 0)
#define MPU6050_CONFIG_DLPF_SETTING_2               (0x02 << 0)
#define MPU6050_CONFIG_DLPF_SETTING_3               (0x03 << 0)
#define MPU6050_CONFIG_DLPF_SETTING_4               (0x04 << 0)
#define MPU6050_CONFIG_DLPF_SETTING_5               (0x05 << 0)
#define MPU6050_CONFIG_DLPF_SETTING_6               (0x06 << 0)
#define MPU6050_GYRO_CONFIG_X_SELF_TEST             (0x01 << 7)
#define MPU6050_GYRO_CONFIG_Y_SELF_TEST             (0x01 << 6)
#define MPU6050_GYRO_CONFIG_Z_SELF_TEST             (0x01 << 5)
#define MPU6050_GYRO_CONFIG_RANGE_250DPS            (0x00 << 3)
#define MPU6050_GYRO_CONFIG_RANGE_500DPS            (0x01 << 3)
#define MPU6050_GYRO_CONFIG_RANGE_1000DPS           (0x02 << 3)
#define MPU6050_GYRO_CONFIG_RANGE_2000DPS           (0x03 << 3)
#define MPU6050_ACCEL_CONFIG_X_SELF_TEST            (0x01 << 7)
#define MPU6050_ACCEL_CONFIG_Y_SELF_TEST            (0x01 << 6)
#define MPU6050_ACCEL_CONFIG_Z_SELF_TEST            (0x01 << 5)
#define MPU6050_ACCEL_CONFIG_RANGE_2G               (0x00 << 3)
#define MPU6050_ACCEL_CONFIG_RANGE_4G               (0x01 << 3)
#define MPU6050_ACCEL_CONFIG_RANGE_8G               (0x02 << 3)
#define MPU6050_ACCEL_CONFIG_RANGE_16G              (0x03 << 3)
#define MPU6050_FIFO_EN_TEMP                        (0x01 << 7)
#define MPU6050_FIFO_EN_XG                          (0x01 << 6)
#define MPU6050_FIFO_EN_YG                          (0x01 << 5)
#define MPU6050_FIFO_EN_ZG                          (0x01 << 4)
#define MPU6050_FIFO_EN_ACCEL                       (0x01 << 3)
#define MPU6050_FIFO_EN_SLV2                        (0x01 << 2)
#define MPU6050_FIFO_EN_SLV1                        (0x01 << 1)
#define MPU6050_FIFO_EN_SLV0                        (0x01 << 0)
#define MPU6050_INT_PIN_CFG_LEVEL_ACTIVE_HIGH       (0x00 << 7)
#define MPU6050_INT_PIN_CFG_LEVEL_ACTIVE_LOW        (0x01 << 7)
#define MPU6050_INT_PIN_CFG_PUSH_PULL               (0x00 << 6)
#define MPU6050_INT_PIN_CFG_OPEN_DRAIN              (0x01 << 6)
#define MPU6050_INT_PIN_CFG_PULSE                   (0X00 << 5)
#define MPU6050_INT_PIN_CFG_LATCH_PIN               (0X01 << 5)
#define MPU6050_INT_PIN_CFG_STATUS_CLEAR_AFTER_READ (0X00 << 4)
#define MPU6050_INT_PIN_CFG_STATUS_CLEAR_AFTER_ANY  (0X01 << 4)
#define MPU6050_INT_PIN_CFG_FSYNC_LEVEL_ACTIVE_HIGH (0X00 << 3)
#define MPU6050_INT_PIN_CFG_FSYNC_LEVEL_ACTIVE_LOW  (0X01 << 3)
#define MPU6050_INT_PIN_CFG_FSYNC_DISABLE           (0X00 << 2)
#define MPU6050_INT_PIN_CFG_FSYNC_ENABLE            (0X01 << 2)
#define MPU6050_INT_PIN_CFG_I2C_BYPASS_DISABLE      (0X00 << 1)
#define MPU6050_INT_PIN_CFG_I2C_BYPASS_ENABLE       (0X01 << 1)
#define MPU6050_INT_ENABLE_FIFO_OVERLOW_DISABLE     (0x00 << 4)
#define MPU6050_INT_ENABLE_FIFO_OVERLOW_ENABLE      (0x01 << 4)
#define MPU6050_INT_ENABLE_I2C_MST_INT_DISABLE      (0x00 << 3)
#define MPU6050_INT_ENABLE_I2C_MST_INT_ENABLE       (0x01 << 3)
#define MPU6050_INT_ENABLE_DATA_RDY_DISABLE         (0x00 << 0)
#define MPU6050_INT_ENABLE_DATA_RDY_ENABLE          (0x01 << 0)
#define MPU6050_SIGNAL_PATH_RESET_GYRO              (0x01 << 2)
#define MPU6050_SIGNAL_PATH_RESET_ACCEL             (0x01 << 1)
#define MPU6050_SIGNAL_PATH_RESET_TEMP              (0x01 << 0)
#define MPU6050_USER_CTRL_FIFO_EN                   (0x01 << 6)
#define MPU6050_USER_CTRL_I2C_MST_EN                (0x01 << 5)
#define MPU6050_USER_CTRL_I2C_IF_DIS                (0x01 << 4)
#define MPU6050_USER_CTRL_FIFO_RESET                (0x01 << 2)
#define MPU6050_USER_CTRL_I2C_MST_RESET             (0x01 << 1)
#define MPU6050_USER_CTRL_SIG_COND_RESET            (0x01 << 0)
#define MPU6050_PWR_MGMT_1_DEVICE_RESET             (0x01 << 7)
#define MPU6050_PWR_MGMT_1_SLEEP                    (0x01 << 6)
#define MPU6050_PWR_MGMT_1_CYCLE                    (0x01 << 5)
#define MPU6050_PWR_MGMT_1_TEMP_DIS                 (0x01 << 3)
#define MPU6050_PWR_MGMT_1_CLOCK_INTERNAL           (0x00 << 0)
#define MPU6050_PWR_MGMT_1_CLOCK_GYRO_X             (0x01 << 0)
#define MPU6050_PWR_MGMT_1_CLOCK_GYRO_Y             (0x02 << 0)
#define MPU6050_PWR_MGMT_1_CLOCK_GYRO_Z             (0x03 << 0)
#define MPU6050_PWR_MGMT_1_CLOCK_EXTERNAL_32_768KHZ (0x04 << 0)
#define MPU6050_PWR_MGMT_1_CLOCK_EXTERNAL_19_2MHZ   (0x05 << 0)
#define MPU6050_PWR_MGMT_1_CLOCK_STOP               (0x07 << 0)
#define MPU6050_PWR_MGMT_2_LP_WAKE_1_25HZ           (0x00 << 6)
#define MPU6050_PWR_MGMT_2_LP_WAKE_5HZ              (0x01 << 6)
#define MPU6050_PWR_MGMT_2_LP_WAKE_20HZ             (0x02 << 6)
#define MPU6050_PWR_MGMT_2_LP_WAKE_40HZ             (0x03 << 6)
#define MPU6050_PWR_MGMT_2_STBY_XA                  (0x01 << 5)
#define MPU6050_PWR_MGMT_2_STBY_YA                  (0x01 << 4)
#define MPU6050_PWR_MGMT_2_STBY_ZA                  (0x01 << 3)
#define MPU6050_PWR_MGMT_2_STBY_XG                  (0x01 << 2)
#define MPU6050_PWR_MGMT_2_STBY_YG                  (0x01 << 1)
#define MPU6050_PWR_MGMT_2_STBY_ZG                  (0x01 << 0)
#define MPU6050_WHO_AM_I_VALUE                      (0x68 << 0)

#define GPIO_INT 586
#define BUF_MASK 0x0000000F
#define BUF_SIZE 16

typedef struct {
    int64_t timestamp;
    int16_t gyro[3];
    int16_t accel[3];
} sample_t;

typedef struct {
    sample_t buf[BUF_SIZE];
    unsigned int head;
    unsigned int tail;
} circbuf_t;

typedef struct {
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int irq;
    circbuf_t circbuf;
    spinlock_t lock;
    struct miscdevice miscdev;
} device_t;

static void reg_write(struct i2c_client *client, const uint8_t reg, const uint8_t value) {
    const int ret = i2c_smbus_write_byte_data(client, reg, value);

    if(ret < 0) {
        dev_err(&client->dev, "mpu6050: failed to write 0x%02X to reg 0x%02X: %d\n", value, reg,
                ret);
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
    device->circbuf.buf[device->circbuf.head] = sample;
    device->circbuf.head++;
    device->circbuf.head %= BUF_MASK;
    if(CIRC_CNT(device->circbuf.head, device->circbuf.tail, BUF_SIZE) >= BUF_SIZE) {
        device->circbuf.tail = device->circbuf.head - BUF_SIZE + 1;
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
    if(CIRC_CNT(device.circbuf.head, device.circbuf.tail, BUF_SIZE) >= 1) {
        sample = device.circbuf.buf[device.circbuf.tail];
        device.circbuf.tail++;
        device.circbuf.tail %= BUF_MASK;
        ret = 1;
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

    device.circbuf.head = 0;
    device.circbuf.tail = 0;

    device.miscdev.fops = &fops;
    device.miscdev.minor = MISC_DYNAMIC_MINOR;
    device.miscdev.name = "mpu6050";
    device.miscdev.mode = 0666;
    ret = misc_register(&device.miscdev);
    if(ret) {
        pr_err("mpu6050: failed to register miscdev: %d\n", ret);
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
