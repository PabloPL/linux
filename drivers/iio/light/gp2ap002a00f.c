// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2013 Samsung Electronics Co., Ltd.
 * Author: Jacek Anaszewski <j.anaszewski@samsung.com>
 *
 * IIO features supported by the driver:
 *
 * Read-only raw channels:
 *   - illuminance_clear [lux]
 *   - proximity
 *
 * Triggers:
 *   - proximity (rising and falling)
 *     - both falling and rising thresholds for the proximity events
 *       must be set to the values greater than 0.
 *
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irq_work.h>
#include <linux/of.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <linux/iio/consumer.h>
#include <linux/iio/events.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>

#define GP2A_I2C_NAME "gp2ap002a00f"

#define GP2A_ADDR_PROX 0x0
#define GP2A_ADDR_GAIN 0x1
#define GP2A_ADDR_HYS 0x2
#define GP2A_ADDR_CYCLE 0x3
#define	GP2A_ADDR_OPMOD 0x4
#define GP2A_ADDR_CON 0x6

struct gp2ap002a00f_data {
	struct i2c_client *client;
	struct regulator *vled_reg;
	struct iio_trigger *trig;
	struct regmap *regmap;
	struct gpio_desc *vout_gpiod;
	struct iio_channel *light_chan;
	int irq;
};

/* These are magic numbers from the vendor driver */
static const struct reg_sequence gp2a_reg_init_tab[] = {
	{
		.reg = GP2A_ADDR_GAIN,
		.def = 0x08,
	}, {
		.reg = GP2A_ADDR_HYS,
		.def = 0xc2,
	}, {
		.reg = GP2A_ADDR_CYCLE,
		.def = 0x04,
	}, {
		.reg = GP2A_ADDR_OPMOD,
		.def = 0x01,
	}
};

enum {
	GP2AP002A00F_SCAN_MODE_LIGHT,
	GP2AP002A00F_SCAN_MODE_PROXIMITY,
	GP2AP002A00F_CHAN_TIMESTAMP,
};

static bool gp2ap002a00f_is_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case GP2A_ADDR_PROX:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config gp2ap002a00f_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = GP2A_ADDR_CON,

	.volatile_reg = gp2ap002a00f_is_volatile_reg,
};

static int gp2ap002a00f_set_trigger_state(struct iio_trigger *trig, bool state)
{
	struct iio_dev *indio_dev = iio_trigger_get_drvdata(trig);
	struct gp2ap002a00f_data *data = iio_priv(indio_dev);

	if (state) {
		pm_runtime_get(&data->client->dev);
		enable_irq(data->irq);
	} else {
		disable_irq(data->irq);
		pm_runtime_put_autosuspend(&data->client->dev);
	}

	return 0;
}

static int gp2ap002a00f_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val, int *val2,
			   long mask)
{
	struct gp2ap002a00f_data *data = iio_priv(indio_dev);
	int err, adc;

	if (mask != IIO_CHAN_INFO_RAW)
		return -EINVAL;

	err = iio_device_claim_direct_mode(indio_dev);
	if (err)
		return err;

	pm_runtime_get(&data->client->dev);

	switch (chan->scan_index) {
	case GP2AP002A00F_SCAN_MODE_LIGHT:
		err = iio_read_channel_raw(data->light_chan, &adc);
		*val = adc;
		break;
	case GP2AP002A00F_SCAN_MODE_PROXIMITY:
		err = 0;
		*val = gpiod_get_value(data->vout_gpiod);
		break;
	default:
		err = -EINVAL;
	}

	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);

	iio_device_release_direct_mode(indio_dev);

	return err < 0 ? err : IIO_VAL_INT;
}

static const struct iio_chan_spec gp2ap002a00f_channels[] = {
	{
		.type = IIO_LIGHT,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.scan_type = {
			.sign = 'u',
			.realbits = 32,
			.shift = 0,
			.storagebits = 32,
			.endianness = IIO_LE,
		},
		.scan_index = GP2AP002A00F_SCAN_MODE_LIGHT,
	},
	{
		.type = IIO_PROXIMITY,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.scan_type = {
			.sign = 'u',
			.realbits = 16,
			.shift = 0,
			.storagebits = 16,
			.endianness = IIO_LE,
		},
		.scan_index = GP2AP002A00F_SCAN_MODE_PROXIMITY,
	},
	IIO_CHAN_SOFT_TIMESTAMP(GP2AP002A00F_CHAN_TIMESTAMP),
};

static const struct iio_info gp2ap002a00f_info = {
	.read_raw = &gp2ap002a00f_read_raw,
};

static const struct iio_trigger_ops gp2ap002a00f_trigger_ops = {
	.set_trigger_state = gp2ap002a00f_set_trigger_state,
};

static int gp2ap002a00f_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct gp2ap002a00f_data *data;
	struct iio_dev *indio_dev;
	struct regmap *regmap;
	int err;

	if (!client->dev.of_node) {
		dev_err(&client->dev, "Only DT initialization supported");
		return -EINVAL;
	}

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);

	data->light_chan = devm_iio_channel_get(&client->dev, "light");
	if (IS_ERR(data->light_chan)) {
		if (PTR_ERR(data->light_chan) != -EPROBE_DEFER)
			dev_err(&client->dev, "Failed to get ADC channel");
		return PTR_ERR(data->light_chan);
	}
	if (data->light_chan->channel->type != IIO_VOLTAGE) {
		dev_err(&client->dev, "Light channel type is not voltage");
		return -EINVAL;
	}

	data->vled_reg = devm_regulator_get(&client->dev, "vled");
	if (IS_ERR(data->vled_reg)) {
		dev_err(&client->dev, "Failed to get vled regulator");
		return PTR_ERR(data->vled_reg);
	}

	err = regulator_enable(data->vled_reg);
	if (err) {
		dev_err(&client->dev, "Failed to enable vled regulator");
		return err;
	}

	data->vout_gpiod = devm_gpiod_get(&client->dev, "vout", GPIOD_IN);
	if (IS_ERR(data->vout_gpiod)) {
		dev_err(&client->dev, "Failed to obtain vout GPIO.\n");
		err = PTR_ERR(data->vout_gpiod);
		goto error_regulator_disable;
	}

	regmap = devm_regmap_init_i2c(client, &gp2ap002a00f_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&client->dev, "Regmap initialization failed.\n");
		err = PTR_ERR(regmap);
		goto error_regulator_disable;
	}

	/* Initialize settings */
	err = regmap_multi_reg_write(regmap, gp2a_reg_init_tab,
				     ARRAY_SIZE(gp2a_reg_init_tab));
	if (err < 0) {
		dev_err(&client->dev, "Device initialization failed");
		goto error_regulator_disable;
	}

	i2c_set_clientdata(client, indio_dev);

	data->client = client;
	data->irq = client->irq;
	data->regmap = regmap;

	if (of_property_read_bool(client->dev.of_node, "wakeup-source"))
		device_set_wakeup_capable(&client->dev, true);

	indio_dev->dev.parent = &client->dev;
	indio_dev->channels = gp2ap002a00f_channels;
	indio_dev->num_channels = ARRAY_SIZE(gp2ap002a00f_channels);
	indio_dev->info = &gp2ap002a00f_info;
	indio_dev->name = id->name;
	indio_dev->modes = INDIO_DIRECT_MODE;

	/* Allocate trigger - note that it's only for proximity */
	data->trig = devm_iio_trigger_alloc(&client->dev, "%s-trigger",
							indio_dev->name);
	if (data->trig == NULL) {
		err = -ENOMEM;
		dev_err(&indio_dev->dev, "Failed to allocate iio trigger.\n");
		goto error_regulator_disable;
	}

	data->trig->ops = &gp2ap002a00f_trigger_ops;
	data->trig->dev.parent = &data->client->dev;

	err = devm_request_irq(&client->dev, client->irq,
			  iio_trigger_generic_data_rdy_poll,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			  "gp2a_irq",
			  data->trig);
	if (err < 0) {
		dev_err(&client->dev, "irq request failed\n");
		goto error_regulator_disable;
	}

	/* Disable IRQ for now - will be re-enabled by trigger when needed */
	disable_irq(data->irq);

	err = iio_trigger_register(data->trig);
	if (err < 0) {
		dev_err(&client->dev, "Failed to register iio trigger.\n");
		goto error_regulator_disable;
	}

	err = iio_device_register(indio_dev);
	if (err < 0)
		goto error_trigger_unregister;

	pm_runtime_set_autosuspend_delay(&client->dev, 1000);
	pm_runtime_use_autosuspend(&client->dev);

	err = pm_runtime_set_active(&client->dev);
	if (err)
		goto error_trigger_unregister;
	pm_runtime_enable(&client->dev);
	pm_runtime_idle(&client->dev);

	return 0;

error_trigger_unregister:
	iio_trigger_unregister(data->trig);
error_regulator_disable:
	regulator_disable(data->vled_reg);

	return err;
}

static int gp2ap002a00f_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);
	struct gp2ap002a00f_data *data = iio_priv(indio_dev);
	int err;

	err = regmap_write(data->regmap, GP2A_ADDR_OPMOD, 0x0);
	if (err < 0)
		dev_err(&client->dev, "Failed to power off the device.\n");

	regulator_disable(data->vled_reg);

	iio_trigger_unregister(data->trig);
	iio_device_unregister(indio_dev);

	pm_runtime_disable(&client->dev);
	pm_runtime_set_suspended(&client->dev);

	return 0;
}

static int __maybe_unused gp2ap002a00f_runtime_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct gp2ap002a00f_data *data = iio_priv(indio_dev);
	int err;

	pr_err("%s called", __func__);

	err = regmap_write(data->regmap, GP2A_ADDR_OPMOD, 0x0);
	if (err < 0)
		return err;

	return regulator_disable(data->vled_reg);
}

static int __maybe_unused gp2ap002a00f_runtime_resume(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct gp2ap002a00f_data *data = iio_priv(indio_dev);
	int err;

	err = regulator_enable(data->vled_reg);
	if (err < 0)
		return err;

	return regmap_multi_reg_write(data->regmap, gp2a_reg_init_tab,
				      ARRAY_SIZE(gp2a_reg_init_tab));
}

static int __maybe_unused gp2ap002a00f_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct gp2ap002a00f_data *data = iio_priv(indio_dev);

	if (!device_may_wakeup(dev))
		return gp2ap002a00f_runtime_suspend(dev);

	enable_irq_wake(data->irq);

	return 0;
}

static int __maybe_unused gp2ap002a00f_resume(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct gp2ap002a00f_data *data = iio_priv(indio_dev);

	if (!device_may_wakeup(dev))
		return gp2ap002a00f_runtime_resume(dev);

	disable_irq_wake(data->irq);

	return 0;
}

static const struct dev_pm_ops gp2ap002a00f_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(gp2ap002a00f_suspend,
				gp2ap002a00f_resume)
	SET_RUNTIME_PM_OPS(gp2ap002a00f_runtime_suspend,
			   gp2ap002a00f_runtime_resume, NULL)
};

static const struct i2c_device_id gp2ap002a00f_id[] = {
	{ GP2A_I2C_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, gp2ap002a00f_id);

#ifdef CONFIG_OF
static const struct of_device_id gp2ap002a00f_of_match[] = {
	{ .compatible = "sharp,gp2ap002a00f" },
	{ }
};
MODULE_DEVICE_TABLE(of, gp2ap002a00f_of_match);
#endif

static struct i2c_driver gp2ap002a00f_driver = {
	.driver = {
		.name	= GP2A_I2C_NAME,
		.pm	= &gp2ap002a00f_dev_pm_ops,
		.of_match_table = of_match_ptr(gp2ap002a00f_of_match),
	},
	.probe		= gp2ap002a00f_probe,
	.remove		= gp2ap002a00f_remove,
	.id_table	= gp2ap002a00f_id,
};

module_i2c_driver(gp2ap002a00f_driver);

MODULE_AUTHOR("Jonathan Bakker <xc-racer2@live.ca>");
MODULE_DESCRIPTION("Sharp GP2AP002A00F Proximity/ALS sensor driver");
MODULE_LICENSE("GPL v2");
