/*
 * Copyright (C) 2011 Sony Ericsson Mobile Communications Inc.
 *
 * Author: Courtney Cavin <courtney.cavin@sonyericsson.com>
 * Prepared for up-stream by: Oskar Andero <oskar.andero@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/iio/consumer.h>
#include <linux/iio/iio.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/input/gp2ap002a00f.h>
#include <linux/of.h>
#include <linux/of_gpio.h>

struct gp2a_data {
	struct iio_channel *channel;
	struct input_dev *input;
	struct input_polled_dev *poll_dev;
	const struct gp2a_platform_data *pdata;
	struct i2c_client *i2c_client;
};

enum gp2a_addr {
	GP2A_ADDR_PROX	= 0x0,
	GP2A_ADDR_GAIN	= 0x1,
	GP2A_ADDR_HYS	= 0x2,
	GP2A_ADDR_CYCLE	= 0x3,
	GP2A_ADDR_OPMOD	= 0x4,
	GP2A_ADDR_CON	= 0x6
};

enum gp2a_controls {
	/* Software Shutdown control: 0 = shutdown, 1 = normal operation */
	GP2A_CTRL_SSD	= 0x01
};

static int gp2a_report(struct gp2a_data *dt)
{
	int vo = gpio_get_value(dt->pdata->vout_gpio);

	input_report_switch(dt->input, SW_FRONT_PROXIMITY, !vo);
	input_sync(dt->input);

	return 0;
}

static irqreturn_t gp2a_irq(int irq, void *handle)
{
	struct gp2a_data *dt = handle;

	gp2a_report(dt);

	return IRQ_HANDLED;
}

static void gp2a_poll(struct input_polled_dev *dev)
{
	struct gp2a_data *dt = dev->private;
	int ret, value;

	ret = iio_read_channel_processed(dt->channel, &value);
	if (ret < 0)
		dev_err(&dt->i2c_client->dev, "failed to read value!");

	input_report_abs(dev->input, ABS_MISC, value);
	input_sync(dev->input);
}

static int gp2a_enable(struct gp2a_data *dt)
{
	return i2c_smbus_write_byte_data(dt->i2c_client, GP2A_ADDR_OPMOD,
					 GP2A_CTRL_SSD);
}

static int gp2a_disable(struct gp2a_data *dt)
{
	return i2c_smbus_write_byte_data(dt->i2c_client, GP2A_ADDR_OPMOD,
					 0x00);
}

static int gp2a_device_open(struct input_dev *dev)
{
	struct gp2a_data *dt = input_get_drvdata(dev);
	int error;

	error = gp2a_enable(dt);
	if (error < 0) {
		dev_err(&dt->i2c_client->dev,
			"unable to activate, err %d\n", error);
		return error;
	}

	gp2a_report(dt);

	return 0;
}

static void gp2a_device_close(struct input_dev *dev)
{
	struct gp2a_data *dt = input_get_drvdata(dev);
	int error;

	error = gp2a_disable(dt);
	if (error < 0)
		dev_err(&dt->i2c_client->dev,
			"unable to deactivate, err %d\n", error);
}

static int gp2a_initialize(struct gp2a_data *dt)
{
	int error;

	error = i2c_smbus_write_byte_data(dt->i2c_client, GP2A_ADDR_GAIN,
					  0x08);
	if (error < 0)
		return error;

	error = i2c_smbus_write_byte_data(dt->i2c_client, GP2A_ADDR_HYS,
					  0xc2);
	if (error < 0)
		return error;

	error = i2c_smbus_write_byte_data(dt->i2c_client, GP2A_ADDR_CYCLE,
					  0x04);
	if (error < 0)
		return error;

	error = gp2a_disable(dt);

	return error;
}

static struct gp2a_platform_data *gp2a_parse_dt_pdata(struct device *dev)
{
	struct gp2a_platform_data *pdata;
	int ret;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);

	pdata->wakeup = of_property_read_bool(dev->of_node, "wakeup");

	pdata->vout_gpio = of_get_named_gpio(dev->of_node, "vout-gpio", 0);
	if (pdata->vout_gpio < 0) {
		dev_err(dev, "failed to find vout-gpio");
		return ERR_PTR(-EINVAL);
	}

	ret = device_property_read_u32(dev, "light-adc-max",
				       &pdata->light_adc_max);
	if (ret)
		pdata->light_adc_max = 4096;
	ret = device_property_read_u32(dev, "light-adc-fuzz",
				       &pdata->light_adc_fuzz);
	if (ret)
		pdata->light_adc_fuzz = 80;

	return pdata;
}

static int gp2a_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	const struct gp2a_platform_data *pdata = dev_get_platdata(&client->dev);
	struct gp2a_data *dt;
	int error, value;

	if (IS_ENABLED(CONFIG_OF) && client->dev.of_node) {
		pdata = gp2a_parse_dt_pdata(&client->dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}

	if (!pdata)
		return -EINVAL;

	if (pdata->hw_setup) {
		error = pdata->hw_setup(client);
		if (error < 0)
			return error;
	}

	error = devm_gpio_request_one(&client->dev, pdata->vout_gpio,
				      GPIOF_IN, GP2A_I2C_NAME);
	if (error)
		goto err_hw_shutdown;

	dt = devm_kzalloc(&client->dev, sizeof(struct gp2a_data), GFP_KERNEL);
	if (!dt) {
		error = -ENOMEM;
		goto err_hw_shutdown;
	}

	dt->pdata = pdata;
	dt->i2c_client = client;

	dt->channel = devm_iio_channel_get(&client->dev, "light");
	if (!IS_ERR(dt->channel)) {
		if (!dt->channel->indio_dev) {
			error = -ENXIO;
			goto err_hw_shutdown;
		}

		if (dt->pdata->light_adc_max <= 0 ||
			dt->pdata->light_adc_fuzz <= 0) {
			error = -EINVAL;
			goto err_hw_shutdown;
		}

		dt->poll_dev = devm_input_allocate_polled_device(&client->dev);
		if (!dt->poll_dev) {
			dev_err(&client->dev,
				"failed to allocate polled input device");
			error = -ENOMEM;
			goto err_hw_shutdown;
		}

		if (!device_property_read_u32(&client->dev, "poll-interval",
					      &value))
			dt->poll_dev->poll_interval = value;

		dt->poll_dev->poll = gp2a_poll;
		dt->poll_dev->private = dt;

		dt->poll_dev->input->name = GP2A_I2C_NAME;

		input_set_capability(dt->poll_dev->input, EV_ABS, ABS_MISC);
		input_set_abs_params(dt->poll_dev->input, ABS_MISC, 0,
				     dt->pdata->light_adc_max,
				     dt->pdata->light_adc_fuzz, 0);

		error = input_register_polled_device(dt->poll_dev);
		if (error)
			goto err_hw_shutdown;
	} else if (PTR_ERR(dt->channel) == -EPROBE_DEFER) {
		error = -EPROBE_DEFER;
		goto err_hw_shutdown;
	}

	error = gp2a_initialize(dt);
	if (error < 0)
		goto err_hw_shutdown;

	dt->input = devm_input_allocate_device(&client->dev);
	if (!dt->input) {
		error = -ENOMEM;
		goto err_hw_shutdown;
	}

	input_set_drvdata(dt->input, dt);

	dt->input->open = gp2a_device_open;
	dt->input->close = gp2a_device_close;
	dt->input->name = GP2A_I2C_NAME;
	dt->input->id.bustype = BUS_I2C;
	dt->input->dev.parent = &client->dev;

	input_set_capability(dt->input, EV_SW, SW_FRONT_PROXIMITY);

	error = devm_request_threaded_irq(&client->dev, client->irq, NULL,
			gp2a_irq, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
			IRQF_ONESHOT, GP2A_I2C_NAME, dt);
	if (error) {
		dev_err(&client->dev, "irq request failed\n");
		goto err_hw_shutdown;
	}

	error = input_register_device(dt->input);
	if (error) {
		dev_err(&client->dev, "device registration failed\n");
		goto err_hw_shutdown;
	}

	device_init_wakeup(&client->dev, pdata->wakeup);
	i2c_set_clientdata(client, dt);

	return 0;

err_hw_shutdown:
	if (pdata->hw_shutdown)
		pdata->hw_shutdown(client);
	return error;
}

static int gp2a_remove(struct i2c_client *client)
{
	struct gp2a_data *dt = i2c_get_clientdata(client);
	const struct gp2a_platform_data *pdata = dt->pdata;

	input_unregister_device(dt->input);
	if (dt->poll_dev)
		input_unregister_polled_device(dt->poll_dev);

	if (pdata->hw_shutdown)
		pdata->hw_shutdown(client);

	return 0;
}

static int __maybe_unused gp2a_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct gp2a_data *dt = i2c_get_clientdata(client);
	int retval = 0;

	if (device_may_wakeup(&client->dev)) {
		enable_irq_wake(client->irq);
	} else {
		mutex_lock(&dt->input->mutex);
		if (dt->input->users)
			retval = gp2a_disable(dt);
		mutex_unlock(&dt->input->mutex);
	}

	return retval;
}

static int __maybe_unused gp2a_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct gp2a_data *dt = i2c_get_clientdata(client);
	int retval = 0;

	if (device_may_wakeup(&client->dev)) {
		disable_irq_wake(client->irq);
	} else {
		mutex_lock(&dt->input->mutex);
		if (dt->input->users)
			retval = gp2a_enable(dt);
		mutex_unlock(&dt->input->mutex);
	}

	if (dt->poll_dev) {
		/* Out of range value so real value goes through next */
		input_abs_set_val(dt->poll_dev->input, ABS_MISC,
				  -dt->pdata->light_adc_max);
	}

	return retval;
}

static SIMPLE_DEV_PM_OPS(gp2a_pm, gp2a_suspend, gp2a_resume);

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id gp2a_of_match[] = {
	{ .compatible = "sharp,gp2ap002a00f" },
	{ },
};
MODULE_DEVICE_TABLE(of, gp2a_of_match);
#endif

static const struct i2c_device_id gp2a_i2c_id[] = {
	{ GP2A_I2C_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, gp2a_i2c_id);

static struct i2c_driver gp2a_i2c_driver = {
	.driver = {
		.name	= GP2A_I2C_NAME,
		.of_match_table = of_match_ptr(gp2a_of_match),
		.pm	= &gp2a_pm,
	},
	.probe		= gp2a_probe,
	.remove		= gp2a_remove,
	.id_table	= gp2a_i2c_id,
};

module_i2c_driver(gp2a_i2c_driver);

MODULE_AUTHOR("Courtney Cavin <courtney.cavin@sonyericsson.com>");
MODULE_DESCRIPTION("Sharp GP2AP002A00F I2C Proximity/Opto sensor driver");
MODULE_LICENSE("GPL v2");
