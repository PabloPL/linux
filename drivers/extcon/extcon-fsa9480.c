/*
 * extcon-fsa9480.c - Fairchild Semiconductor FSA9480 extcon driver
 *
 * Copyright (c) 2014 Tomasz Figa <tomasz.figa@gmail.com>
 *
 * Loosely based on old fsa9480 misc-device driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/bitops.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/extcon-provider.h>
#include <linux/irqdomain.h>

/* FSA9480 I2C registers */
#define FSA9480_REG_DEVID		0x01
#define FSA9480_REG_CTRL		0x02
#define FSA9480_REG_INT1		0x03
#define FSA9480_REG_INT2		0x04
#define FSA9480_REG_INT1_MASK		0x05
#define FSA9480_REG_INT2_MASK		0x06
#define FSA9480_REG_ADC			0x07
#define FSA9480_REG_TIMING1		0x08
#define FSA9480_REG_TIMING2		0x09
#define FSA9480_REG_DEV_T1		0x0a
#define FSA9480_REG_DEV_T2		0x0b
#define FSA9480_REG_BTN1		0x0c
#define FSA9480_REG_BTN2		0x0d
#define FSA9480_REG_CK			0x0e
#define FSA9480_REG_CK_INT1		0x0f
#define FSA9480_REG_CK_INT2		0x10
#define FSA9480_REG_CK_INTMASK1		0x11
#define FSA9480_REG_CK_INTMASK2		0x12
#define FSA9480_REG_MANSW1		0x13
#define FSA9480_REG_MANSW2		0x14

/* Control */
#define CON_SWITCH_OPEN		(1 << 4)
#define CON_RAW_DATA		(1 << 3)
#define CON_MANUAL_SW		(1 << 2)
#define CON_WAIT		(1 << 1)
#define CON_INT_MASK		(1 << 0)
#define CON_MASK		(CON_SWITCH_OPEN | CON_RAW_DATA | \
				CON_MANUAL_SW | CON_WAIT)

/* Device Type 1 */
#define DEV_USB_OTG		7
#define DEV_DEDICATED_CHG	6
#define DEV_USB_CHG		5
#define DEV_CAR_KIT		4
#define DEV_UART		3
#define DEV_USB			2
#define DEV_AUDIO_2		1
#define DEV_AUDIO_1		0

/* Device Type 2 */
#define DEV_AV			14
#define DEV_TTY			13
#define DEV_PPD			12
#define DEV_JIG_UART_OFF	11
#define DEV_JIG_UART_ON		10
#define DEV_JIG_USB_OFF		9
#define DEV_JIG_USB_ON		8

/*
 * Manual Switch
 * D- [7:5] / D+ [4:2]
 * 000: Open all / 001: USB / 010: AUDIO / 011: UART / 100: V_AUDIO
 */
#define SW_VAUDIO		((4 << 5) | (4 << 2))
#define SW_UART			((3 << 5) | (3 << 2))
#define SW_AUDIO		((2 << 5) | (2 << 2))
#define SW_DHOST		((1 << 5) | (1 << 2))
#define SW_AUTO			((0 << 5) | (0 << 2))

/* Interrupt 1 */
#define INT1_MASK		(0xff << 0)
#define INT_DETACH		(1 << 1)
#define INT_ATTACH		(1 << 0)

/* Interrupt 2 mask */
#define INT2_MASK		(0x1f << 0)

/* Timing Set 1 */
#define TIMING1_ADC_500MS	(0x6 << 0)

struct fsa9480_usbsw {
	struct i2c_client *client;
	struct extcon_dev *edev;
	u16 dev;
};

static const unsigned int max8998_extcon_cable[] = {
	EXTCON_USB_HOST,
	EXTCON_USB,
	EXTCON_CHG_USB_SDP,
	EXTCON_CHG_USB_ACA,
	EXTCON_JACK_LINE_OUT,
	EXTCON_JACK_VIDEO_OUT,
	EXTCON_JIG,

	EXTCON_NONE,
};

static const u64 cable_types[] = {
	[DEV_USB_OTG] = BIT_ULL(EXTCON_USB_HOST),
	[DEV_DEDICATED_CHG] = BIT_ULL(EXTCON_USB) | BIT_ULL(EXTCON_CHG_USB_DCP),
	[DEV_USB_CHG] = BIT_ULL(EXTCON_USB) | BIT_ULL(EXTCON_CHG_USB_SDP),
	[DEV_CAR_KIT] = BIT_ULL(EXTCON_USB) | BIT_ULL(EXTCON_CHG_USB_SDP) | BIT_ULL(EXTCON_JACK_LINE_OUT),
	[DEV_UART] = BIT_ULL(EXTCON_JIG),
	[DEV_USB] = BIT_ULL(EXTCON_USB) | BIT_ULL(EXTCON_CHG_USB_SDP),
	[DEV_AUDIO_2] = BIT_ULL(EXTCON_JACK_LINE_OUT),
	[DEV_AUDIO_1] = BIT_ULL(EXTCON_JACK_LINE_OUT),
	[DEV_AV] = BIT_ULL(EXTCON_JACK_LINE_OUT) | BIT_ULL(EXTCON_JACK_VIDEO_OUT),
	[DEV_TTY] = BIT_ULL(EXTCON_JIG),
	[DEV_PPD] = BIT_ULL(EXTCON_JACK_LINE_OUT) | BIT_ULL(EXTCON_CHG_USB_ACA),
	[DEV_JIG_UART_OFF] = BIT_ULL(EXTCON_JIG),
	[DEV_JIG_UART_ON] = BIT_ULL(EXTCON_JIG),
	[DEV_JIG_USB_OFF] = BIT_ULL(EXTCON_USB) | BIT_ULL(EXTCON_JIG),
	[DEV_JIG_USB_ON] = BIT_ULL(EXTCON_USB) | BIT_ULL(EXTCON_JIG),
};

static int fsa9480_write_reg(struct i2c_client *client, int reg, int value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int fsa9480_read_reg(struct i2c_client *client, int reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int fsa9480_read_irq(struct i2c_client *client, int *value)
{
	u8 regs[2];
	int ret;

	ret = i2c_smbus_read_i2c_block_data(client, FSA9480_REG_INT1, 2, regs);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	*value = regs[1] << 8 | regs[0];
	return ret;
}

static void fsa9480_handle_change(struct fsa9480_usbsw *usbsw,
				  u16 mask, bool attached)
{
	while (mask) {
		int dev = fls64(mask) - 1;
		u64 cables = cable_types[dev];

		while (cables) {
			int cable = fls64(cables) - 1;

			extcon_set_state_sync(usbsw->edev, cable, attached);
			cables &= ~BIT_ULL(cable);
		}

		mask &= ~BIT_ULL(dev);
	}
}

static void fsa9480_detect_dev(struct fsa9480_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	int val1, val2;
	u16 val;

	val1 = fsa9480_read_reg(client, FSA9480_REG_DEV_T1);
	val2 = fsa9480_read_reg(client, FSA9480_REG_DEV_T2);
	if (val1 < 0 || val2 < 0) {
		dev_err(&client->dev, "%s: failed to read registers", __func__);
		return;
	}
	val = val2 << 8 | val1;

	dev_info(&client->dev, "dev1: 0x%x, dev2: 0x%x\n", val1, val2);

	/* handle detached cables first */
	fsa9480_handle_change(usbsw, usbsw->dev & ~val, false);

	/* then handle attached ones */
	fsa9480_handle_change(usbsw, val & ~usbsw->dev, true);

	usbsw->dev = val;
}

static irqreturn_t fsa9480_irq_handler(int irq, void *data)
{
	struct fsa9480_usbsw *usbsw = data;
	struct i2c_client *client = usbsw->client;
	int intr = 0;

	/* clear interrupt */
	fsa9480_read_irq(client, &intr);
	if (!intr)
		return IRQ_NONE;

	/* device detection */
	fsa9480_detect_dev(usbsw);

	return IRQ_HANDLED;
}

static int fsa9480_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct fsa9480_usbsw *info;
	int ret;

	if (!client->irq) {
		dev_err(&client->dev, "no interrupt provided\n");
		return -EINVAL;
	}

	info = devm_kzalloc(&client->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->client = client;

	i2c_set_clientdata(client, info);

	/* External connector */
	info->edev = devm_extcon_dev_allocate(&client->dev, max8998_extcon_cable);
	if (IS_ERR(info->edev)) {
		dev_err(&client->dev, "failed to allocate memory for extcon\n");
		ret = -ENOMEM;
		return ret;
	}

	ret = devm_extcon_dev_register(&client->dev, info->edev);
	if (ret) {
		dev_err(&client->dev, "failed to register extcon device\n");
		return ret;
	}

	/* ADC Detect Time: 500ms */
	fsa9480_write_reg(client, FSA9480_REG_TIMING1, TIMING1_ADC_500MS);

	/* configure automatic switching */
	fsa9480_write_reg(client, FSA9480_REG_CTRL, CON_MASK);

	/* unmask interrupt (attach/detach only) */
	fsa9480_write_reg(client, FSA9480_REG_INT1_MASK,
				INT1_MASK & ~(INT_ATTACH | INT_DETACH));
	fsa9480_write_reg(client, FSA9480_REG_INT2_MASK, INT2_MASK);

	ret = devm_request_threaded_irq(&client->dev, client->irq, NULL,
					fsa9480_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					"fsa9480", info);
	if (ret) {
		dev_err(&client->dev, "failed to request IRQ\n");
		return ret;
	}

	device_init_wakeup(&client->dev, true);
	fsa9480_detect_dev(info);

	return 0;
}

static int fsa9480_remove(struct i2c_client *client)
{
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int fsa9480_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	if (device_may_wakeup(&client->dev) && client->irq)
		enable_irq_wake(client->irq);

	return 0;
}

static int fsa9480_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	if (device_may_wakeup(&client->dev) && client->irq)
		disable_irq_wake(client->irq);

	return 0;
}
#endif

static const struct dev_pm_ops fsa9480_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(fsa9480_suspend, fsa9480_resume)
};

static const struct i2c_device_id fsa9480_id[] = {
	{"fsa9480", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, fsa9480_id);

static const struct of_device_id fsa9480_of_match[] = {
	{ .compatible = "fcs,fsa9480", },
	{ },
};
MODULE_DEVICE_TABLE(of, fsa9480_of_match);

static struct i2c_driver fsa9480_i2c_driver = {
	.driver = {
		.name = "fsa9480",
		.pm = &fsa9480_pm_ops,
		.of_match_table = fsa9480_of_match,
	},
	.probe = fsa9480_probe,
	.remove = fsa9480_remove,
	.id_table = fsa9480_id,
};

static int __init fsa9480_module_init(void)
{
	return i2c_add_driver(&fsa9480_i2c_driver);
}
subsys_initcall(fsa9480_module_init);

static void __exit fsa9480_module_exit(void)
{
	i2c_del_driver(&fsa9480_i2c_driver);
}
module_exit(fsa9480_module_exit);

MODULE_DESCRIPTION("Fairchild Semiconductor FSA9480 extcon driver");
MODULE_AUTHOR("Tomasz Figa <tomasz.figa@gmail.com>");
MODULE_LICENSE("GPL");
