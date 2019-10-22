// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>

#define YAS_REG_CMDR		0x00	/* 000 << 5 */
#define YAS_REG_XOFFSETR	0x20	/* 001 << 5 */
#define YAS_REG_Y1OFFSETR	0x40	/* 010 << 5 */
#define YAS_REG_Y2OFFSETR	0x60	/* 011 << 5 */
#define YAS_REG_ICOILR		0x80	/* 100 << 5 */
#define YAS_REG_CAL		0xA0	/* 101 << 5 */
#define YAS_REG_CONFR		0xC0	/* 110 << 5 */
#define YAS_REG_DOUTR		0xE0	/* 111 << 5 */

struct yas529 {
	struct i2c_client *i2c;
	struct iio_mount_matrix orientation;
	struct gpio_desc *resetn_gpio;
	struct mutex lock;
	s64 cal_matrix[9];
};

static int yas529_zero_registers(struct yas529 *yas529)
{
	int ret;
	u8 dat;

	/* zero initialization coil register */
	dat = YAS_REG_ICOILR | 0x00;
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	/* zero config register */
	dat = YAS_REG_CONFR | 0x00;
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static int yas529_actuate_initcoil(struct yas529 *yas529)
{
	static const u8 initcoil_vals[16] = { 0x11, 0x01, 0x12, 0x02,
					      0x13, 0x03, 0x14, 0x04,
					      0x15, 0x05, 0x16, 0x06,
					      0x17, 0x07, 0x10, 0x00 };
	int ret, i;
	u8 dat;

	for (i = 0; i < ARRAY_SIZE(initcoil_vals); i++) {
		dat = YAS_REG_ICOILR | initcoil_vals[i];
		ret = i2c_master_send(yas529->i2c, &dat, 1);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int yas529_rough_offset_cfg(struct yas529 *yas529)
{
	int ret, i;
	s16 offset[3];
	u8 buf[6];
	u8 dat;

	/* Config register - measurements results */
	dat = YAS_REG_CONFR | 0x00;
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	/* Measurements command register - rough offset measurement */
	dat = YAS_REG_CMDR | 0x01;
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	/* wait at least 2ms */
	usleep_range(2000, 3000);

	ret = i2c_master_recv(yas529->i2c, buf, ARRAY_SIZE(buf));
	if (ret < 0)
		return ret;

	for (i = 0; i < 3; i++) {
		offset[i] = (s16)((u16)buf[5 - i * 2] +
			((u16)buf[4 - i * 2] & 0x7) * 256) - 5;
		if (offset[i] < 0)
			offset[i] = 0;
	}

	/* Set rough offset register values */
	dat = YAS_REG_XOFFSETR | offset[0];
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	dat = YAS_REG_Y2OFFSETR | offset[1];
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	dat = YAS_REG_Y2OFFSETR | offset[2];
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static int yas529_create_cal_matrix(struct yas529 *yas529)
{
	int ret;
	s16 tmp;
	u8 cal_data[9];
	u8 dat;

	/* Config register - CAL register read */
	dat = YAS_REG_CONFR | 0x08;
	ret = i2c_master_send(yas529->i2c, &dat, 1);
	if (ret < 0)
		return ret;

	/* wait at least 2ms */
	usleep_range(2000, 3000);

	/* CAL data read - first time is invalid */
	ret = i2c_master_recv(yas529->i2c, cal_data, ARRAY_SIZE(cal_data));
	if (ret < 0)
		return ret;

	ret = i2c_master_recv(yas529->i2c, cal_data, ARRAY_SIZE(cal_data));
	if (ret < 0)
		return ret;

	yas529->cal_matrix[0] = 100;

	tmp = (cal_data[0] & 0xFC) >> 2;
	yas529->cal_matrix[1] = tmp - 32;

	tmp = ((cal_data[0] & 0x03) << 2) | ((cal_data[1] & 0xC0) >> 6);
	yas529->cal_matrix[2] = tmp - 8;

	tmp = (cal_data[1] & 0x3F);
	yas529->cal_matrix[3] = tmp - 32;

	tmp = (cal_data[2] & 0xFC) >> 2;
	yas529->cal_matrix[4] = (tmp - 32) + 70;

	tmp = ((cal_data[2] & 0x03) << 4) | ((cal_data[3] & 0xF0) >> 4);
	yas529->cal_matrix[5] = tmp - 32;

	tmp = ((cal_data[3] & 0x0F) << 2) | ((cal_data[4] & 0xC0) >> 6);
	yas529->cal_matrix[6] = tmp - 32;

	tmp = (cal_data[4] & 0x3F);
	yas529->cal_matrix[7] = tmp - 32;

	tmp = (cal_data[5] & 0xFE) >> 1;
	yas529->cal_matrix[8] = (tmp - 64) + 130;

	return 0;
}

static int yas529_init(struct yas529 *yas529)
{
	int ret;

	ret = yas529_zero_registers(yas529);
	if (ret < 0)
		return ret;

	ret = yas529_actuate_initcoil(yas529);
	if (ret < 0)
		return ret;

	ret = yas529_rough_offset_cfg(yas529);
	if (ret < 0)
		return ret;

	ret = yas529_create_cal_matrix(yas529);
	if (ret < 0)
		return ret;

	return 0;
}

static void yas529_sensor_correction(struct yas529 *yas529, s16 raw[],
				     s16 fixed[])
{
	int i;
	s16 axis[3];

	axis[0] = -raw[0];
	axis[1] = raw[2] - raw[1];
	axis[2] = raw[2] + raw[1];

	for (i = 0; i < 3; i++) {
		fixed[i] = (s16)((yas529->cal_matrix[i * 3] * axis[0] +
				  yas529->cal_matrix[i * 3 + 1] * axis[1] +
				  yas529->cal_matrix[i * 3 + 2] * axis[2])
				  >> 7) * 41;
	}
}

static int yas529_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val, int *val2,
			   long mask)
{
	struct yas529 *yas529 = iio_priv(indio_dev);
	int ret = -EINVAL;
	int i;
	s16 raw[3], fixed[3];
	u8 hw_values[8];
	u8 cmd;

	mutex_lock(&yas529->lock);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (chan->address > 2) {
			dev_err(&yas529->i2c->dev, "faulty channel address\n");
			ret = -EIO;
			goto out_unlock;
		}

		cmd = YAS_REG_CONFR | 0x00;
		ret = i2c_master_send(yas529->i2c, &cmd, 1);
		if (ret < 0)
			return ret;

		cmd = YAS_REG_CMDR | 0x02;
		ret = i2c_master_send(yas529->i2c, &cmd, 1);
		if (ret < 0)
			return ret;

		for (i = 0; i < 13; i++) {
			usleep_range(1000, 1500);
			ret = i2c_master_recv(yas529->i2c, hw_values, 8);
			if (ret < 0) {
				pr_err("Failed to read data");
				return ret;
			}

			if (!(hw_values[0] & 0x80))
				break;
		}

		/* sensor isn't ready */
		if (hw_values[0] & 0x80) {
			dev_err(&yas529->i2c->dev, "Sensor isn't ready!");
			ret = -EBUSY;
			goto out_unlock;
		}

		/*
		 * we calculate all values, we'll discard those that we aren't
		 * using
		 */
		for (i = 0; i < 3; ++i)
			raw[2 - i] = ((hw_values[i << 1] & 0x7) << 8) +
				hw_values[(i << 1) | 1];

		yas529_sensor_correction(yas529, raw, fixed);

		*val = fixed[chan->address];

		ret = IIO_VAL_INT;
		break;
	case IIO_CHAN_INFO_SCALE:
		*val = 0;
		*val2 = 25;
		ret = IIO_VAL_INT_PLUS_MICRO;
		break;
	}

out_unlock:
	mutex_unlock(&yas529->lock);

	return ret;
}

static const struct iio_mount_matrix *
yas529_get_mount_matrix(const struct iio_dev *indio_dev,
			const struct iio_chan_spec *chan)
{
	struct yas529 *yas529 = iio_priv(indio_dev);

	return &yas529->orientation;
}

static const struct iio_chan_spec_ext_info yas529_ext_info[] = {
	IIO_MOUNT_MATRIX(IIO_SHARED_BY_DIR, yas529_get_mount_matrix),
	{ },
};

#define YAS529_AXIS_CHANNEL(axis, index)				\
	{								\
		.type = IIO_MAGN,					\
		.modified = 1,						\
		.channel2 = IIO_MOD_##axis,				\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		\
		.ext_info = yas529_ext_info,				\
		.address = index,					\
		.scan_index = index,					\
		.scan_type = {						\
			.sign = 's',					\
			.realbits = 16,					\
			.storagebits = 16,				\
			.endianness = IIO_LE				\
		},							\
	}

static const struct iio_chan_spec yas529_channels[] = {
	YAS529_AXIS_CHANNEL(X, 0),
	YAS529_AXIS_CHANNEL(Y, 1),
	YAS529_AXIS_CHANNEL(Z, 2),
	IIO_CHAN_SOFT_TIMESTAMP(3),
};

static const struct iio_info yas529_info = {
	.read_raw = &yas529_read_raw,
};

static int yas529_probe(struct i2c_client *i2c,
			const struct i2c_device_id *id)
{
	struct iio_dev *indio_dev;
	struct yas529 *yas529;
	int ret;

	/* Register with IIO */
	indio_dev = devm_iio_device_alloc(&i2c->dev, sizeof(*yas529));
	if (indio_dev == NULL)
		return -ENOMEM;

	yas529 = iio_priv(indio_dev);
	i2c_set_clientdata(i2c, indio_dev);
	yas529->i2c = i2c;
	mutex_init(&yas529->lock);

	ret = iio_read_mount_matrix(&i2c->dev, "mount-matrix",
				    &yas529->orientation);
	if (ret)
		return ret;

	yas529->resetn_gpio = devm_gpiod_get_optional(&i2c->dev, "resetn",
			GPIOD_OUT_HIGH);
	if (IS_ERR(yas529->resetn_gpio))
		return PTR_ERR(yas529->resetn_gpio);

	/* Reset device */
	gpiod_set_value(yas529->resetn_gpio, 0);
	usleep_range(2000, 3000);
	gpiod_set_value(yas529->resetn_gpio, 1);

	usleep_range(2000, 3000);

	ret = yas529_init(yas529);
	if (ret < 0) {
		dev_err(&i2c->dev, "Failed to initialize");
		return ret;
	}

	indio_dev->dev.parent = &i2c->dev;
	indio_dev->channels = yas529_channels;
	indio_dev->num_channels = ARRAY_SIZE(yas529_channels);
	indio_dev->info = &yas529_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->name = "yas529";

	ret = iio_device_register(indio_dev);
	if (ret)
		dev_err(&i2c->dev, "device register failed\n");

	return ret;
}

static int yas529_remove(struct i2c_client *i2c)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(i2c);

	iio_device_unregister(indio_dev);

	return 0;
}

static const struct i2c_device_id yas529_id[] = {
	{"yas529", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, yas529_id);

static const struct of_device_id yas529_of_match[] = {
	{ .compatible = "yamaha,yas529", },
	{}
};
MODULE_DEVICE_TABLE(of, yas529_of_match);

static struct i2c_driver yas529_driver = {
	.driver	 = {
		.name	= "yas529",
		.of_match_table = of_match_ptr(yas529_of_match),
	},
	.probe	  = yas529_probe,
	.remove	  = yas529_remove,
	.id_table = yas529_id,
};
module_i2c_driver(yas529_driver);

MODULE_DESCRIPTION("YAS529 3-axis magnetometer driver");
MODULE_AUTHOR("Jonathan Bakker");
MODULE_LICENSE("GPL v2");
