#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>

// I2C Settings
#define MPR121_I2C_ADDRESS 0x5A
#define MPR121_TOUCH_THRESHOLD 12
#define MPR121_RELEASE_THRESHOLD 6

// Registers
#define MPR121_TOUCHSTATUS_L 0x00
#define MPR121_TOUCHSTATUS_H 0x01
#define MPR121_FILTDATA_0L 0x04
#define MPR121_FILTDATA_0H 0x05
#define MPR121_BASELINE_0 0x1E
#define MPR121_MHDR 0x2B
#define MPR121_NHDR 0x2C
#define MPR121_NCLR 0x2D
#define MPR121_FDLR 0x2E
#define MPR121_MHDF 0x2F
#define MPR121_NHDF 0x30
#define MPR121_NCLF 0x31
#define MPR121_FDLF 0x32
#define MPR121_NHDT 0x33
#define MPR121_NCLT 0x34
#define MPR121_FDLT 0x35

#define MPR121_TOUCHTH_0 0x41
#define MPR121_RELEASETH_0 0x42
#define MPR121_DEBOUNCE 0x5B
#define MPR121_CONFIG1 0x5C
#define MPR121_CONFIG2 0x5D
#define MPR121_ECR 0x5E
#define MPR121_CHARGECURR_0 0x5F
#define MPR121_CHARGETIME_1 0x6C
#define MPR121_AUTOCONFIG0 0x7B
#define MPR121_AUTOCONFIG1 0x7C
#define MPR121_UPLIMIT 0x7D
#define MPR121_LOWLIMIT 0x7E
#define MPR121_TARGETLIMIT 0x7F

#define MPR121_GPIODIR 0x76
#define MPR121_GPIOEN 0x77
#define MPR121_GPIOSET 0x78
#define MPR121_GPIOCLR 0x79
#define MPR121_GPIOTOGGLE 0x7A

#define MPR121_SOFTRESET 0x80

#define INPUT_KEY_SLIDE_RIGHT 0x2F0  // スライド右方向
#define INPUT_KEY_SLIDE_LEFT  0x2F1  // スライド左方向

struct mpr121_data
{
    uint16_t touch_status;
    // uint8_t btn_cache;
    // bool in_int;
    const struct device *dev;
    // struct gpio_callback gpio_cb;
    struct k_work work;
};

typedef int (*mpr121_seq_read_t)(const struct device *dev, const uint8_t addr, uint8_t *buf,
                                 const uint8_t len);
typedef int (*mpr121_write_t)(const struct device *dev, const uint8_t addr, const uint8_t val);

struct mpr121_config
{
    union
    {
        struct i2c_dt_spec i2c;
    } bus;

    mpr121_seq_read_t seq_read;
    mpr121_write_t write;
};

int mpr121_set_sleep(const struct device *dev, bool enabled);