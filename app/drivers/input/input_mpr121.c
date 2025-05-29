#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/init.h>
#include <zephyr/input/input.h>
#include <zephyr/pm/device.h>

#include <zephyr/logging/log.h>

#include "input_mpr121.h"

LOG_MODULE_REGISTER(mpr121, CONFIG_INPUT_LOG_LEVEL);

static int mpr121_seq_read(const struct device *dev, const uint8_t addr, uint8_t *buf,
                           const uint8_t len)
{
    const struct mpr121_config *config = dev->config;
    return config->seq_read(dev, addr, buf, len);
}
static int mpr121_write(const struct device *dev, const uint8_t addr, const uint8_t val)
{
    const struct mpr121_config *config = dev->config;
    return config->write(dev, addr, val);
}

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

static int mpr121_i2c_seq_read(const struct device *dev, const uint8_t addr, uint8_t *buf,
                               const uint8_t len)
{
    const struct mpr121_config *config = dev->config;
    return i2c_burst_read_dt(&config->bus.i2c, MPR121_I2C_ADDRESS | addr, buf, len);
}

static int mpr121_i2c_write(const struct device *dev, const uint8_t addr, const uint8_t val)
{
    const struct mpr121_config *config = dev->config;
    return i2c_reg_write_byte_dt(&config->bus.i2c, MPR121_I2C_ADDRESS | addr, val);
}

#endif // DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

/**
 *  @brief      Read the touch status of all 13 channels as bit values in a 12
 * bit integer.
 *  @returns    a 12 bit integer with each bit corresponding to the touch status
 *              of a sensor. For example, if bit 0 is set then channel 0 of the
 * device is currently deemed to be touched.
 */
uint16_t mpr1212_get_touched(const struct device *dev)
{
    int ret;
    uint8_t t[2];

    ret = mpr121_seq_read(dev, MPR121_TOUCHSTATUS_L, t, 2);
    if (ret < 0)
    {
        LOG_ERR("mpr1212_get_touched: read status: %d", ret);
        return 0;
    }

    uint16_t ui16 = t[1] | (t[0] << 8);

    return ui16 & 0x0FFF;
}

static void mpr121_report_data(const struct device *dev)
{
    // const struct mpr121_config *config = dev->config;
    struct mpr121_data *data = dev->data;

    uint16_t touched = mpr1212_get_touched(dev);
    data->touch_status = touched;

    LOG_HEXDUMP_DBG(&touched, 1, "MPR121 Touch Status");

    // input_report_rel(dev, INPUT_REL_X, dx, false, K_FOREVER);
    // input_report_rel(dev, INPUT_REL_Y, dy, true, K_FOREVER);

    return;
}

static void mpr121_work_cb(struct k_work *work)
{
    LOG_INF("Running mpr121_work_cb");
    struct mpr121_data *data = CONTAINER_OF(work, struct mpr121_data, work);
    mpr121_report_data(data->dev);
}

int mpr121_get_config(const struct device *dev, uint8_t config1or2)
{
    int ret = 0;
    int regAddr = MPR121_CONFIG1;

    switch (config1or2)
    {
    case 1:
        regAddr = MPR121_CONFIG1;
        break;
    case 2:
        regAddr = MPR121_CONFIG2;
        break;
    default:
        LOG_ERR("mpr121_get_config: config1or2 can only have a value of 1 or 2");
        return -1;
    }

    uint8_t configValue[1];
    ret = mpr121_seq_read(dev, regAddr, configValue, 1);

    if (ret >= 0)
    {
        LOG_DBG("Default config%d: %d", config1or2, configValue[0]);
    }

    return configValue[0];
}

int mpr121_set_stop(const struct device *dev)
{
    int ret = 0;

    ret = mpr121_write(dev, MPR121_ECR, 0x00); // Stop mode
    if (ret < 0)
    {
        LOG_ERR("can't set stop mode %d", ret);
    }

    return ret;
}

int mpr121_set_run(const struct device *dev)
{
    // enable X electrodes and start MPR121
    uint8_t ECR_SETTING = 0b10000000 + 12; // 5 bits for baseline tracking & proximity disabled + X
                                           // amount of electrodes running (12)
    int ret = 0;

    ret = mpr121_write(dev, MPR121_ECR, ECR_SETTING); // Running mode
    if (ret < 0)
    {
        LOG_ERR("can't set run mode %d", ret);
    }

    return ret;
}

int mpr121_set_thresholds(const struct device *dev, uint8_t touch, uint8_t release)
{
    int ret = 0;

    // set all thresholds (the same)
    for (uint8_t i = 0; i < 12; i++)
    {

        ret = mpr121_write(dev, MPR121_TOUCHTH_0 + 2 * i, touch); // Set electrode touch threshold
        if (ret < 0)
            break;
        ret = mpr121_write(dev, MPR121_RELEASETH_0 + 2 * i, release); // Set electrode release threshold
        if (ret < 0)
            break;
    }

    if (ret < 0)
    {
        LOG_ERR("failed to set touch / release thresholds %d", ret);
    }

    return ret;
}

static int mpr121_init(const struct device *dev)
{
    struct mpr121_data *data = dev->data;
    // const struct mpr121_config *config = dev->config;
    int ret;

    LOG_INF("Running MPR121 Init");

    // data->in_int = false;
    k_msleep(10);
    ret = mpr121_write(dev, MPR121_SOFTRESET, 0x63); // Soft Reset
    if (ret < 0)
    {
        LOG_ERR("can't soft reset %d", ret);
        return ret;
    }
    k_msleep(1);

    ret = mpr121_set_stop(dev);
    if (ret < 0)
    {
        return ret;
    }

    int config1 = mpr121_get_config(dev, 1);
    if (config1 != 0x24)
    {
        LOG_ERR("config1 is not 0x24, it is %d", config1);
        return -1;
    }

    ret = mpr121_set_thresholds(dev, MPR121_TOUCH_THRESHOLD, MPR121_RELEASE_THRESHOLD);
    if (ret < 0)
    {
        return ret;
    }

    mpr121_write(dev, MPR121_MHDR, 0x01);
    mpr121_write(dev, MPR121_NHDR, 0x01);
    mpr121_write(dev, MPR121_NCLR, 0x0E);
    mpr121_write(dev, MPR121_FDLR, 0x00);

    mpr121_write(dev, MPR121_MHDF, 0x01);
    mpr121_write(dev, MPR121_NHDF, 0x05);
    mpr121_write(dev, MPR121_NCLF, 0x01);
    mpr121_write(dev, MPR121_FDLF, 0x00);

    mpr121_write(dev, MPR121_NHDT, 0x00);
    mpr121_write(dev, MPR121_NCLT, 0x00);
    mpr121_write(dev, MPR121_FDLT, 0x00);

    mpr121_write(dev, MPR121_DEBOUNCE, 0);
    mpr121_write(dev, MPR121_CONFIG1, 0x10); // default, 16uA charge current
    mpr121_write(dev, MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period                                      // IS_ENABLED(CONFIG_MPR121_AUTOCONFIG)

    mpr121_set_run(dev); // Start MPR121

    data->dev = dev;

    k_work_init(&data->work, mpr121_work_cb);


    return 0;
}

#if IS_ENABLED(CONFIG_PM_DEVICE)

static int mpr121_pm_action(const struct device *dev, enum pm_device_action action)
{
    // switch (action)
    // {
    // case PM_DEVICE_ACTION_SUSPEND:
    //     return set_int(dev, false);
    // case PM_DEVICE_ACTION_RESUME:
    //     return set_int(dev, true);
    // default:
    //     return -ENOTSUP;
    // }

    return -ENOTSUP;
}

#endif // IS_ENABLED(CONFIG_PM_DEVICE)

#define DT_DRV_COMPAT nxp_mpr121
#define INST_DT_NXP_MPR121(inst) DT_INST(inst, nxp_mpr121)

#define MPR121_INST(n)                                                                \
    static struct mpr121_data mpr121_data_##n;                                        \
    static const struct mpr121_config mpr121_config_##n = {                           \
        .bus = {.i2c = I2C_DT_SPEC_INST_GET(n)},                                      \
        .seq_read = mpr121_i2c_seq_read,                                              \
        .write = mpr121_i2c_write,                                                    \
    };                                                                                \
    PM_DEVICE_DT_INST_DEFINE(n, mpr121_pm_action);                                    \
    DEVICE_DT_INST_DEFINE(n, mpr121_init, PM_DEVICE_DT_INST_GET(n), &mpr121_data_##n, \
                          &mpr121_config_##n, POST_KERNEL, CONFIG_INPUT_MPR121_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(MPR121_INST)