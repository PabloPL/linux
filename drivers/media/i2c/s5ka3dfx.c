// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Samsung S5KA3DFX UXGA 1/4" 2.0Mp CMOS Image Sensor SoC with
 * an Embedded Image Processor
 *
 * Copyright (C) 2019 Jonathan Bakker <xc-racer2@live.ca>
 *
 * Based on a driver authored by Sylwester Nawrocki, <s.nawrocki@samsung.com>
 * Copyright (C) 2010 - 2011 Samsung Electronics Co., Ltd.
 * Contact: Sylwester Nawrocki, <s.nawrocki@samsung.com>
 *
 * Initial register configuration based on a driver
 * Copyright (C) 2009, Jinsung Yang <jsgood.yang@samsung.com>.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/videodev2.h>
#include <linux/module.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>

static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable module debug trace. Set to 1 to enable.");

#define MODULE_NAME		"S5KA3DFX"

/* The token to mark an array end */
#define REG_END		0xFFFF

/* I2C retry attempts */
#define I2C_RETRY_COUNT		(5)

struct s5ka3dfx_format {
	u32 code;
	enum v4l2_colorspace colorspace;
};

enum frame_size {
	VGA = 0,
	QVGA,
	QCIF,
};

enum {
	S5KA_COLORFX_NONE = 0,
	S5KA_COLORFX_BW,
	S5KA_COLORFX_SEPIA,
	S5KA_COLORFX_NEGATIVE,
	S5KA_COLORFX_AQUA,
};

enum {
	S5KA_WB_AUTO = 0,
	S5KA_WB_INCANDESCENT,
	S5KA_WB_FLUORESCENT,
	S5KA_WB_DAYLIGHT,
	S5KA_WB_CLOUDY,
};

struct s5ka3dfx_frmsize {
	u16 width;
	u16 height;
	enum frame_size frs;
};

static const char * const s5ka3dfx_supply_name[] = {
	"vddio", "isp_sys", "dvdd", "isp_host"
};

#define S5KA3DFX_NUM_SUPPLIES ARRAY_SIZE(s5ka3dfx_supply_name)

struct s5ka3dfx_info {
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_ctrl_handler hdl;
	struct regulator_bulk_data supply[S5KA3DFX_NUM_SUPPLIES];
	struct gpio_desc *gpio_nreset;
	struct gpio_desc *gpio_nstby;
	struct clk *mclk;

	/* Protects the struct members below */
	struct mutex lock;

	const struct s5ka3dfx_format *curr_fmt;
	const struct s5ka3dfx_frmsize *curr_win;
	unsigned int streaming:1;
	unsigned int hflip:1;
	unsigned int vflip:1;
	unsigned int power:1;
};

struct i2c_regval {
	u16 addr;
	u16 val;
};

/* Supported resolutions. */
static const struct s5ka3dfx_frmsize s5ka3dfx_sizes[] = {
	{
		.width		= 640,
		.height		= 480,
		.frs		= VGA,
	}, {
		.width		= 320,
		.height		= 240,
		.frs		= QVGA,
	}, {
		.width		= 176,
		.height		= 144,
		.frs		= QCIF,
	},
};

/* Supported pixel formats. */
static const struct s5ka3dfx_format s5ka3dfx_formats[] = {
	{
		.code		= MEDIA_BUS_FMT_YUYV8_2X8,
		.colorspace	= V4L2_COLORSPACE_JPEG,
	},
};

static const struct i2c_regval s5ka3dfx_base_regs[] = {
	/*
	 * These register values were taken from the vendor
	 * driver and their meaning is unclear.
	 * Sets have been grouped according to the vendor
	 * driver with relevant comments left in.
	 */
	{ 0xef, 0x02 }, { 0x13, 0xa0 },
	{ 0x23, 0x53 }, { 0x26, 0x24 },
	{ 0x2c, 0x05 }, { 0x05, 0x00 },
	{ 0x03, 0x58 }, { 0x24, 0x0a },
	{ 0x0b, 0x84 }, { 0x1e, 0xb7 },
	{ 0x56, 0x05 }, { 0x28, 0x96 },
	{ 0x67, 0x3c },

	{ 0xef, 0x03 }, { 0x50, 0xd2 },
	{ 0x0f, 0x31 }, { 0xef, 0x03 },
	{ 0x70, 0x00 }, /* un-mirrored */
	{ 0x5F, 0x03 }, { 0x60, 0x02 },
	{ 0x61, 0x0F }, { 0x62, 0x0C },
	{ 0x63, 0x01 }, { 0x64, 0xE7 },
	{ 0x65, 0x01 }, { 0x66, 0xE7 },

	{ 0x6d, 0x56 }, { 0x6e, 0xC0 },
	{ 0x6f, 0xC0 },

	{ 0x4c, 0x00 }, { 0x4d, 0x9e },

	{ 0xef, 0x03 }, { 0x00, 0x07 },
	{ 0x01, 0x80 }, { 0x02, 0x7f },
	{ 0x2b, 0x41 }, { 0x31, 0x00 },
	{ 0x32, 0x09 },

	{ 0x33, 0x80 }, { 0x34, 0x79 },

	{ 0x36, 0x3A },	/*39, 3a, N.L. ST */
	{ 0x37, 0x38 },

	{ 0x6a, 0x00 }, { 0x7b, 0x05 },
	{ 0x38, 0x05 }, { 0x39, 0x03 },

	{ 0x2d, 0x08 }, { 0x2e, 0x20 },
	{ 0x2f, 0x30 }, { 0x30, 0xff },
	{ 0x7c, 0x06 }, { 0x7d, 0x20 },
	{ 0x7e, 0x0c }, { 0x7f, 0x20 },

	{ 0x28, 0x02 }, { 0x29, 0x9f },
	{ 0x2a, 0x00 },

	{ 0x13, 0x00 }, { 0x14, 0xa0 },

	{ 0x1a, 0x5d }, { 0x1b, 0x58 },
	{ 0x1c, 0x60 }, { 0x1d, 0x4f },

	{ 0x1e, 0x68 },
	{ 0x1f, 0x42 }, /* 44, Indoor Rgain Min */
	{ 0x20, 0x7A }, /* 75 82, 8a, Indoor Bgain Max */
	{ 0x21, 0x4D },	/* 4Indoor Bgain Min */

	{ 0x3a, 0x13 }, { 0x3b, 0x3c },
	{ 0x3c, 0x00 }, { 0x3d, 0x18 },

	{ 0x23, 0x80 },

	{ 0x15, 0x0b }, { 0x16, 0xd2 },
	{ 0x17, 0x64 }, { 0x18, 0x78 },

	{ 0xef, 0x00 }, { 0xde, 0x00 },
	{ 0xdf, 0x1f }, { 0xe0, 0x00 },
	{ 0xe1, 0x37 }, { 0xe2, 0x08 },
	{ 0xe3, 0x42 }, { 0xe4, 0x00 },
	{ 0xe5, 0x12 }, { 0xe6, 0x9e },
	{ 0xe9, 0x00 }, { 0xe7, 0x01 },
	{ 0xe8, 0x13 }, { 0xe9, 0x01 },
	{ 0xe7, 0x01 }, { 0xe8, 0x06 },
	{ 0xe9, 0x02 }, { 0xe7, 0x00 },
	{ 0xe8, 0xef }, { 0xe9, 0x03 },
	{ 0xe7, 0x00 }, { 0xe8, 0xe0 },
	{ 0xe9, 0x04 }, { 0xe7, 0x00 },
	{ 0xe8, 0xc3 }, { 0xe9, 0x05 },
	{ 0xe7, 0x00 }, { 0xe8, 0xab },
	{ 0xe9, 0x06 }, { 0xe7, 0x00 },
	{ 0xe8, 0x91 }, { 0xe9, 0x07 },
	{ 0xe7, 0x00 }, { 0xe8, 0xbd },
	{ 0xe9, 0x08 }, { 0xe7, 0x00 },
	{ 0xe8, 0xab }, { 0xe9, 0x09 },
	{ 0xe7, 0x00 }, { 0xe8, 0x9a },
	{ 0xe9, 0x0a }, { 0xe7, 0x00 },
	{ 0xe8, 0x8f }, { 0xe9, 0x0b },
	{ 0xe7, 0x00 }, { 0xe8, 0x78 },
	{ 0xe9, 0x0c }, { 0xe7, 0x00 },
	{ 0xe8, 0x69 }, { 0xe9, 0x0d },
	{ 0xe7, 0x00 }, { 0xe8, 0x55 },
	{ 0xe9, 0x0e }, { 0xe7, 0x00 },
	{ 0xe8, 0x4c }, { 0xe9, 0x0f },
	{ 0xe7, 0x00 }, { 0xe8, 0x4d },
	{ 0xe9, 0x10 }, { 0xe7, 0x00 },
	{ 0xe8, 0x43 }, { 0xe9, 0x11 },
	{ 0xe7, 0x00 }, { 0xe8, 0x39 },
	{ 0xe9, 0x12 }, { 0xe7, 0x00 },
	{ 0xe8, 0x26 }, { 0xe9, 0x13 },
	{ 0xe7, 0x00 }, { 0xe8, 0x1e },
	{ 0xe9, 0x14 }, { 0xe7, 0x00 },
	{ 0xe8, 0x0d }, { 0xe9, 0x15 },
	{ 0xe7, 0x07 }, { 0xe8, 0xd8 },
	{ 0xe9, 0x16 }, { 0xe7, 0x07 },
	{ 0xe8, 0xd8 }, { 0xe9, 0x17 },
	{ 0xe7, 0x07 }, { 0xe8, 0xe1 },
	{ 0xe9, 0x18 }, { 0xe7, 0x07 },
	{ 0xe8, 0xdc }, { 0xe9, 0x19 },
	{ 0xe7, 0x07 }, { 0xe8, 0xd3 },
	{ 0xe9, 0x1a }, { 0xe7, 0x07 },
	{ 0xe8, 0xcb }, { 0xe9, 0x1b },
	{ 0xe7, 0x07 }, { 0xe8, 0xbe },
	{ 0xe9, 0x1c }, { 0xe7, 0x07 },
	{ 0xe8, 0x62 }, { 0xe9, 0x1d },
	{ 0xe7, 0x07 }, { 0xe8, 0x66 },
	{ 0xe9, 0x1e }, { 0xe7, 0x07 },
	{ 0xe8, 0x71 }, { 0xe9, 0x1f },
	{ 0xe7, 0x07 }, { 0xe8, 0x80 },
	{ 0xe9, 0x20 }, { 0xe7, 0x07 },
	{ 0xe8, 0x75 }, { 0xe9, 0x21 },
	{ 0xe7, 0x07 }, { 0xe8, 0x67 },
	{ 0xe9, 0x22 }, { 0xe7, 0x07 },
	{ 0xe8, 0x85 }, { 0xe9, 0x40 },


	{ 0xd1, 0x08 }, { 0xdd, 0x03 },

	{ 0x23, 0x17 }, { 0x24, 0x17 },
	{ 0x25, 0x17 }, { 0x27, 0x18 },
	{ 0x29, 0x60 }, { 0x2a, 0x22 },

	{ 0x2f, 0x01 },

	{ 0x36, 0x01 }, { 0x37, 0xc2 },
	{ 0x38, 0xa8 }, { 0x39, 0x98 },
	{ 0x3a, 0x00 }, { 0x3b, 0xf0 },
	{ 0x3c, 0x01 }, { 0x3d, 0x5e },
	{ 0xb9, 0x02 }, { 0xbb, 0xb0 },
	{ 0xbc, 0x18 }, { 0xbd, 0x30 },
	{ 0xbf, 0x38 }, { 0xc1, 0x88 },
	{ 0xc8, 0x11 }, { 0xeb, 0x81 },
	{ 0xed, 0x05 }, { 0xb1, 0x00 },
	{ 0xb2, 0x62 }, { 0xb3, 0x00 },
	{ 0xb4, 0x00 }, { 0xb5, 0x01 },
	{ 0xb6, 0xa3 }, { 0xb7, 0x02 },
	{ 0xb8, 0x80 }, { 0x77, 0x00 },
	{ 0x78, 0x00 }, { 0xef, 0x00 },
	{ 0x93, 0x40 }, { 0x94, 0x80 },
	{ 0x95, 0xc0 }, { 0x96, 0xc0 },
	{ 0x97, 0x20 }, { 0x98, 0x20 },
	{ 0x99, 0x30 }, { 0xA0, 0x00 },
	{ 0xA1, 0x00 }, { 0xA2, 0x1c },
	{ 0xA3, 0x16 }, { 0xA4, 0x03 },
	{ 0xA5, 0x07 }, { 0xA6, 0x00 },
	{ 0xef, 0x00 }, { 0xad, 0xd0 },
	{ 0xaf, 0x10 }, { 0xef, 0x00 },
	{ 0x42, 0x65 }, { 0x44, 0x61 },

	{ 0x57, 0x00 },

	{ 0xef, 0x03 }, { 0x01, 0x3C },
	{ 0x02, 0x05 }, { 0x03, 0x21 },
	{ 0x04, 0x60 }, { 0x06, 0x1c },
	{ 0x07, 0x01 }, { 0x08, 0x01 },
	{ 0x0b, 0x01 }, { 0x51, 0x10 },
	{ 0x52, 0x00 }, { 0x53, 0x00 },
	{ 0x54, 0x00 }, { 0x55, 0x22 },
	{ 0x56, 0x01 }, { 0x57, 0x61 },
	{ 0x58, 0x25 }, { 0x67, 0xcf },
	{ 0x69, 0x17 }, { 0xef, 0x00 },
	{ 0x58, 0x00 }, { 0x59, 0x00 },
	{ 0x5a, 0x02 }, { 0x5b, 0x73 },
	{ 0x5c, 0x00 }, { 0x5d, 0x00 },
	{ 0x5e, 0x01 }, { 0x5f, 0xe0 },
	{ 0x60, 0x00 }, { 0x61, 0xEA },
	{ 0x62, 0x01 }, { 0x63, 0x80 },
	{ 0x64, 0x00 }, { 0x65, 0xAF },
	{ 0x66, 0x01 }, { 0x67, 0x2D },
	{ 0xef, 0x00 }, { 0x6a, 0x01 },
	{ 0x6b, 0xe0 }, { 0x6c, 0x05 },
	{ 0x6d, 0x00 }, { 0x6e, 0x0e },
	{ 0x6f, 0x00 }, { 0x70, 0x10 },
	{ 0xef, 0x03 }, { 0x22, 0x24 },
	{ 0x3e, 0x23 }, { 0x3f, 0x23 },
	{ 0x40, 0x00 }, { 0x41, 0x09 },
	{ 0x4a, 0x09 }, { 0x4b, 0x04 },
	{ 0x5b, 0x20 }, { 0x5d, 0x35 },
	{ 0x5e, 0x13 }, { 0x78, 0x0f },
	{ 0xef, 0x00 }, { 0x4c, 0x80 },
	{ 0x4d, 0xbb }, { 0x4e, 0x84 },
	{ 0x4f, 0x91 }, { 0x50, 0x64 },
	{ 0x51, 0x93 }, { 0x52, 0x03 },
	{ 0x53, 0xc7 }, { 0x54, 0x83 },
	{ 0xef, 0x03 }, { 0x6e, 0x40 },
	{ 0x6f, 0x50 }, /* dgain for shutter 700lux*/

	{ 0xef, 0x00 }, { 0x48, 0x00 },
	{ 0x49, 0x00 }, { 0x4A, 0x03 },
	{ 0x48, 0x01 }, { 0x49, 0x00 },
	{ 0x4A, 0x06 }, { 0x48, 0x02 },
	{ 0x49, 0x00 }, { 0x4A, 0x24 },
	{ 0x48, 0x03 }, { 0x49, 0x00 },
	{ 0x4A, 0x8a }, { 0x48, 0x04 },
	{ 0x49, 0x01 }, { 0x4A, 0x20 },
	{ 0x48, 0x05 }, { 0x49, 0x01 },
	{ 0x4A, 0xB4 }, { 0x48, 0x06 },
	{ 0x49, 0x02 }, { 0x4A, 0x23 },
	{ 0x48, 0x07 }, { 0x49, 0x02 },
	{ 0x4A, 0x72 }, { 0x48, 0x08 },
	{ 0x49, 0x02 }, { 0x4A, 0xBE },
	{ 0x48, 0x09 }, { 0x49, 0x02 },
	{ 0x4A, 0xFA }, { 0x48, 0x0A },
	{ 0x49, 0x03 }, { 0x4A, 0x27 },
	{ 0x48, 0x0B }, { 0x49, 0x03 },
	{ 0x4A, 0x55 }, { 0x48, 0x0C },
	{ 0x49, 0x03 }, { 0x4A, 0x81 },
	{ 0x48, 0x0D }, { 0x49, 0x03 },
	{ 0x4A, 0xA2 }, { 0x48, 0x0E },
	{ 0x49, 0x03 }, { 0x4A, 0xBC },
	{ 0x48, 0x0F }, { 0x49, 0x03 },
	{ 0x4A, 0xD4 }, { 0x48, 0x10 },
	{ 0x49, 0x03 }, { 0x4A, 0xE8 },
	{ 0x48, 0x11 }, { 0x49, 0x03 },
	{ 0x4A, 0xF4 }, { 0x48, 0x12 },
	{ 0x49, 0x03 }, { 0x4A, 0xFF },
	{ 0x48, 0x20 }, { 0x49, 0x00 },
	{ 0x4A, 0x03 }, { 0x48, 0x21 },
	{ 0x49, 0x00 }, { 0x4A, 0x06 },
	{ 0x48, 0x22 }, { 0x49, 0x00 },
	{ 0x4A, 0x24 }, { 0x48, 0x23 },
	{ 0x49, 0x00 }, { 0x4A, 0x8a },
	{ 0x48, 0x24 }, { 0x49, 0x01 },
	{ 0x4A, 0x20 }, { 0x48, 0x25 },
	{ 0x49, 0x01 }, { 0x4A, 0xB4 },
	{ 0x48, 0x26 }, { 0x49, 0x02 },
	{ 0x4A, 0x23 }, { 0x48, 0x27 },
	{ 0x49, 0x02 }, { 0x4A, 0x72 },
	{ 0x48, 0x28 }, { 0x49, 0x02 },
	{ 0x4A, 0xBE }, { 0x48, 0x29 },
	{ 0x49, 0x02 }, { 0x4A, 0xFA },
	{ 0x48, 0x2A }, { 0x49, 0x03 },
	{ 0x4A, 0x27 }, { 0x48, 0x2B },
	{ 0x49, 0x03 }, { 0x4A, 0x55 },
	{ 0x48, 0x2C }, { 0x49, 0x03 },
	{ 0x4A, 0x81 }, { 0x48, 0x2D },
	{ 0x49, 0x03 }, { 0x4A, 0xA2 },
	{ 0x48, 0x2E }, { 0x49, 0x03 },
	{ 0x4A, 0xBC }, { 0x48, 0x2F },
	{ 0x49, 0x03 }, { 0x4A, 0xD4 },
	{ 0x48, 0x30 }, { 0x49, 0x03 },
	{ 0x4A, 0xE8 }, { 0x48, 0x31 },
	{ 0x49, 0x03 }, { 0x4A, 0xF4 },
	{ 0x48, 0x32 }, { 0x49, 0x03 },
	{ 0x4A, 0xFF }, { 0x48, 0x40 },
	{ 0x49, 0x00 }, { 0x4A, 0x03 },
	{ 0x48, 0x41 }, { 0x49, 0x00 },
	{ 0x4A, 0x06 }, { 0x48, 0x42 },
	{ 0x49, 0x00 }, { 0x4A, 0x24 },
	{ 0x48, 0x43 }, { 0x49, 0x00 },
	{ 0x4A, 0x8a }, { 0x48, 0x44 },
	{ 0x49, 0x01 }, { 0x4A, 0x20 },
	{ 0x48, 0x45 }, { 0x49, 0x01 },
	{ 0x4A, 0xB4 }, { 0x48, 0x46 },
	{ 0x49, 0x02 }, { 0x4A, 0x23 },
	{ 0x48, 0x47 }, { 0x49, 0x02 },
	{ 0x4A, 0x72 }, { 0x48, 0x48 },
	{ 0x49, 0x02 }, { 0x4A, 0xBE },
	{ 0x48, 0x49 }, { 0x49, 0x02 },
	{ 0x4A, 0xFA }, { 0x48, 0x4A },
	{ 0x49, 0x03 }, { 0x4A, 0x27 },
	{ 0x48, 0x4B }, { 0x49, 0x03 },
	{ 0x4A, 0x55 }, { 0x48, 0x4C },
	{ 0x49, 0x03 }, { 0x4A, 0x81 },
	{ 0x48, 0x4D }, { 0x49, 0x03 },
	{ 0x4A, 0xA2 }, { 0x48, 0x4E },
	{ 0x49, 0x03 }, { 0x4A, 0xBC },
	{ 0x48, 0x4F }, { 0x49, 0x03 },
	{ 0x4A, 0xD4 }, { 0x48, 0x50 },
	{ 0x49, 0x03 }, { 0x4A, 0xE8 },
	{ 0x48, 0x51 }, { 0x49, 0x03 },
	{ 0x4A, 0xF4 }, { 0x48, 0x52 },
	{ 0x49, 0x03 }, { 0x4A, 0xFF },
	{ 0xEF, 0x03 }, { 0x00, 0x03 },

	{ REG_END, 0 },
};

static const struct i2c_regval s5ka3dfx_frame_sizes[][30] = {
	[VGA] = {
		{ 0xef, 0x00 }, { 0x7a, 0x00 },
		{ 0x11, 0x00 }, { 0x12, 0x00 },
		{ 0x15, 0x02 }, { 0x16, 0x90 },
		{ 0x13, 0x01 }, { 0x14, 0xF0 },
		{ 0x31, 0x04 }, { 0x30, 0x06 },
		{ 0x34, 0x02 }, { 0x35, 0x88 },
		{ 0x32, 0x01 }, { 0x33, 0xE8 },
		{ 0x7d, 0x02 }, { 0x7e, 0x88 },
		{ 0x7b, 0x01 }, { 0x7C, 0xe8 },
		{ 0x81, 0x02 }, { 0x82, 0x01 },
		{ 0x7f, 0x01 }, { 0x80, 0xe8 },
		{ 0xc3, 0x04 }, { 0xc2, 0x04 },
		{ 0xc6, 0x02 }, { 0xc7, 0x80 },
		{ 0xc4, 0x01 }, { 0xc5, 0xe0 },
		{ 0x7a, 0x01 },
		{ REG_END, 0 },
	},
	[QVGA] = {
		{ 0xef, 0x00 }, { 0x7a, 0x00 },
		{ 0x11, 0x00 }, { 0x12, 0x00 },
		{ 0x15, 0x02 }, { 0x16, 0x90 },
		{ 0x13, 0x01 }, { 0x14, 0xF0 },
		{ 0x31, 0x04 }, { 0x30, 0x06 },
		{ 0x34, 0x02 }, { 0x35, 0x88 },
		{ 0x32, 0x01 }, { 0x33, 0xE8 },
		{ 0x7d, 0x02 }, { 0x7e, 0x88 },
		{ 0x7b, 0x01 }, { 0x7c, 0xe8 },
		{ 0x81, 0x01 }, { 0x82, 0x48 },
		{ 0x7f, 0x00 }, { 0x80, 0xf8 },
		{ 0xc3, 0x04 }, { 0xc2, 0x04 },
		{ 0xc6, 0x01 }, { 0xc7, 0x40 },
		{ 0xc4, 0x00 }, { 0xc5, 0xf0 },
		{ 0x7a, 0x03 },
		{ REG_END, 0 },
	},
	[QCIF] = {
		{ 0xef, 0x00 }, { 0x7a, 0x00 },
		{ 0x11, 0x00 }, { 0x12, 0x00 },
		{ 0x15, 0x02 }, { 0x16, 0x90 },
		{ 0x13, 0x01 }, { 0x14, 0xF0 },
		{ 0x31, 0x04 }, { 0x30, 0x06 },
		{ 0x34, 0x02 }, { 0x35, 0x88 },
		{ 0x32, 0x01 }, { 0x33, 0xE8 },
		{ 0x7d, 0x02 }, { 0x7e, 0x88 },
		{ 0x7b, 0x01 }, { 0x7c, 0xe8 },
		{ 0x81, 0x00 }, { 0x82, 0xc0 },
		{ 0x7f, 0x00 }, { 0x80, 0x98 },
		{ 0xc3, 0x08 }, { 0xc2, 0x04 },
		{ 0xc6, 0x00 }, { 0xc7, 0xb0 },
		{ 0xc4, 0x00 }, { 0xc5, 0x90 },
		{ 0x7a, 0x03 },
		{ REG_END, 0 },
	},
};

static const struct i2c_regval s5ka3dfx_wbs[][7] = {
	[S5KA_WB_AUTO] = {
		{ 0xef, 0x03 }, { 0x00, 0x87 },
		{ 0xef, 0x00 }, { 0x42, 0x6f },
		{ 0x43, 0x40 }, { 0x44, 0x5a },
		{ REG_END, 0 },
	},
	[S5KA_WB_INCANDESCENT] = {
		{ 0xef, 0x03 }, { 0x00, 0x85 },
		{ 0xef, 0x00 }, { 0x42, 0x48 },
		{ 0x43, 0x43 }, { 0x44, 0x7e },
		{ REG_END, 0 },
	},
	[S5KA_WB_FLUORESCENT] = {
		{ 0xef, 0x03 }, { 0x00, 0x85 },
		{ 0xef, 0x00 }, { 0x42, 0x5c },
		{ 0x43, 0x40 }, { 0x44, 0x6d },
		{ REG_END, 0 },
	},
	[S5KA_WB_DAYLIGHT] = {
		{ 0xef, 0x03 }, { 0x00, 0x85 },
		{ 0xef, 0x00 }, { 0x42, 0x67 },
		{ 0x43, 0x40 }, { 0x44, 0x4c },
		{ REG_END, 0 },
	},
	[S5KA_WB_CLOUDY] = {
		{ 0xef, 0x03 }, { 0x00, 0x85 },
		{ 0xef, 0x00 }, { 0x42, 0x75 },
		{ 0x43, 0x3d }, { 0x44, 0x42 },
		{ REG_END, 0 },
	},
};

/* Exposure value is index - 5 */
static const struct i2c_regval s5ka3dfx_exposure_values[][4] = {
	{
		{ 0xef, 0x03 }, { 0x31, 0xc0 }, { 0x32, 0x98 },
		{ REG_END, 0 }, /* -5 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0xA5 }, { 0x32, 0x90 },
		{ REG_END, 0 }, /* -4 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x9E }, { 0x32, 0x88 },
		{ REG_END, 0 }, /* -3 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x90 }, { 0x32, 0x00 },
		{ REG_END, 0 }, /* - 2 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x8A }, { 0x32, 0x08 },
		{ REG_END, 0 }, /* -1 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x00 }, { 0x32, 0x09 },
		{ REG_END, 0 }, /* 0 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x0A }, { 0x32, 0x20 },
		{ REG_END, 0 }, /* +1 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x14 }, { 0x32, 0x30 },
		{ REG_END, 0 }, /* +2 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x1E }, { 0x32, 0x38 },
		{ REG_END, 0 }, /* +3 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x28 }, { 0x32, 0x40 },
		{ REG_END, 0 }, /* +4 */
	}, {
		{ 0xef, 0x03 }, { 0x31, 0x30 }, { 0x32, 0x48 },
		{ REG_END, 0 }, /* +5 */
	},
};

static const struct i2c_regval s5ka3dfx_fx_modes[][6] = {
	[S5KA_COLORFX_NONE] = {
		{ 0xef, 0x00 }, { 0xd3, 0x00 },
		{ 0xd4, 0x00 }, { 0xd5, 0x01 },
		{ 0xd6, 0xa3 },
		{ REG_END, 0 },
	},
	[S5KA_COLORFX_BW] = {
		{ 0xef, 0x00 }, { 0xd3, 0x00 },
		{ 0xd4, 0x03 }, { 0xd5, 0x80 },
		{ 0xd6, 0x80 },
		{ REG_END, 0 },
	},
	[S5KA_COLORFX_SEPIA] = {
		{ 0xef, 0x00 }, { 0xd3, 0x00 },
		{ 0xd4, 0x03 }, { 0xd5, 0x60 },
		{ 0xd6, 0x8c },
		{ REG_END, 0 },
	},
	[S5KA_COLORFX_NEGATIVE] = {
		{ 0xef, 0x00 }, { 0xd3, 0x01 },
		{ 0xd4, 0x00 }, { 0xd5, 0x2c },
		{ 0xd6, 0x81 },
		{ REG_END, 0 },
	},
	[S5KA_COLORFX_AQUA] = {
		{ 0xef, 0x00 }, { 0xd3, 0x00 },
		{ 0xd4, 0x03 }, { 0xd5, 0xdc },
		{ 0xd6, 0x00 },
		{ REG_END, 0 },
	},
};

static inline struct s5ka3dfx_info *to_s5ka3dfx(struct v4l2_subdev *sd)
{
	return container_of(sd, struct s5ka3dfx_info, sd);
}

static inline struct v4l2_subdev *to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct s5ka3dfx_info, hdl)->sd;
}

static inline int s5ka3dfx_bulk_write_reg(struct v4l2_subdev *sd,
					 const struct i2c_regval *msg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg i2c_msg;
	unsigned char buf[2];
	int retry, ret = 0;

	i2c_msg.addr = client->addr;
	i2c_msg.len = 2;
	i2c_msg.buf = buf;

	while (msg->addr != REG_END) {
		buf[0] = msg->addr;
		buf[1] = msg->val;

		for (retry = 0; retry < I2C_RETRY_COUNT; retry++) {
			ret = i2c_transfer(client->adapter, &i2c_msg, 1);
			if (ret == 1)
				break;

			msleep_interruptible(10);
		}

		if (ret < 0) {
			dev_err(&client->dev, "i2c transfer failed: %d", ret);
			return -EIO;
		}
		msg++;
	}

	return 0;
}


/* Called with struct s5ka3dfx_info.lock mutex held */
static int s5ka3dfx_set_exposure(struct v4l2_subdev *sd, int val)
{
	/* exposure ranges from -5 to +5 */
	val += 5;

	if (val < 0 || val > ARRAY_SIZE(s5ka3dfx_exposure_values) - 1)
		return -EINVAL;

	return s5ka3dfx_bulk_write_reg(sd, s5ka3dfx_exposure_values[val]);
}

/* Called with struct s5ka3dfx_info.lock mutex held */
static int s5ka3dfx_set_wb(struct v4l2_subdev *sd, int val)
{
	static const unsigned short wb[][2] = {
		{ V4L2_WHITE_BALANCE_INCANDESCENT,  S5KA_WB_INCANDESCENT },
		{ V4L2_WHITE_BALANCE_FLUORESCENT,   S5KA_WB_FLUORESCENT },
		{ V4L2_WHITE_BALANCE_CLOUDY,        S5KA_WB_CLOUDY },
		{ V4L2_WHITE_BALANCE_DAYLIGHT,      S5KA_WB_DAYLIGHT },
		{ V4L2_WHITE_BALANCE_AUTO,          S5KA_WB_AUTO },
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(wb); i++) {
		if (wb[i][0] != val)
			continue;

		return s5ka3dfx_bulk_write_reg(sd, s5ka3dfx_wbs[wb[i][1]]);
	}

	return -EINVAL;
}

/* Called with struct s5ka3dfx_info.lock mutex held */
static int s5ka3dfx_set_colorfx(struct v4l2_subdev *sd, int val)
{
	static const unsigned short fx[][2] = {
		{ V4L2_COLORFX_NONE,     S5KA_COLORFX_NONE },
		{ V4L2_COLORFX_BW,       S5KA_COLORFX_BW },
		{ V4L2_COLORFX_SEPIA,    S5KA_COLORFX_SEPIA },
		{ V4L2_COLORFX_NEGATIVE, S5KA_COLORFX_NEGATIVE },
		{ V4L2_COLORFX_AQUA,     S5KA_COLORFX_AQUA},
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(fx); i++) {
		if (fx[i][0] != val)
			continue;

		return s5ka3dfx_bulk_write_reg(sd, s5ka3dfx_wbs[fx[i][1]]);
	}

	return -EINVAL;
}

/* Called with struct s5ka3dfx_info.lock mutex held */
static int s5ka3dfx_set_flip(struct v4l2_subdev *sd, int hflip, int vflip)
{
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);
	struct i2c_regval regval[3];
	int ret;

	regval[0].addr = 0xef;
	regval[0].val = 0x3;
	regval[1].addr = 0x70;
	regval[1].val = 0x0;
	regval[2].addr = REG_END;

	if (hflip)
		regval[1].val |= 0x01;
	if (vflip)
		regval[1].val |= 0x02;

	ret = s5ka3dfx_bulk_write_reg(sd, regval);
	if (!ret) {
		info->hflip = hflip;
		info->vflip = vflip;
	}
	return ret;
}

/* Find nearest matching image pixel size. */
static int s5ka3dfx_try_frame_size(struct v4l2_mbus_framefmt *mf,
				  const struct s5ka3dfx_frmsize **size)
{
	unsigned int min_err = ~0;
	int i = ARRAY_SIZE(s5ka3dfx_sizes);
	const struct s5ka3dfx_frmsize *fsize = &s5ka3dfx_sizes[0],
		*match = NULL;

	while (i--) {
		int err = abs(fsize->width - mf->width)
				+ abs(fsize->height - mf->height);

		if (err < min_err) {
			min_err = err;
			match = fsize;
		}
		fsize++;
	}
	if (match) {
		mf->width  = match->width;
		mf->height = match->height;
		if (size)
			*size = match;
		return 0;
	}
	return -EINVAL;
}

/* Called with info.lock mutex held */
static int power_enable(struct s5ka3dfx_info *info)
{
	int ret;

	if (info->power) {
		v4l2_info(&info->sd, "%s: sensor is already on\n", __func__);
		return 0;
	}

	gpiod_set_value_cansleep(info->gpio_nstby, 0);
	gpiod_set_value_cansleep(info->gpio_nreset, 0);

	ret = regulator_bulk_enable(S5KA3DFX_NUM_SUPPLIES, info->supply);
	if (ret)
		return ret;

	mdelay(1);

	gpiod_set_value_cansleep(info->gpio_nstby, 1);

	mdelay(5);

	ret = clk_prepare_enable(info->mclk);
	if (ret)
		return ret;

	mdelay(5);

	gpiod_set_value_cansleep(info->gpio_nreset, 1);

	mdelay(5);

	info->power = 1;

	v4l2_dbg(1, debug, &info->sd,  "%s: sensor is on\n", __func__);
	return 0;
}

/* Called with info.lock mutex held */
static int power_disable(struct s5ka3dfx_info *info)
{
	int ret;

	if (!info->power) {
		v4l2_info(&info->sd, "%s: sensor is already off\n", __func__);
		return 0;
	}

	gpiod_set_value_cansleep(info->gpio_nreset, 0);
	gpiod_set_value_cansleep(info->gpio_nstby, 0);

	clk_disable_unprepare(info->mclk);

	ret = regulator_bulk_disable(S5KA3DFX_NUM_SUPPLIES, info->supply);
	if (ret)
		return ret;

	info->power = 0;

	v4l2_dbg(1, debug, &info->sd,  "%s: sensor is off\n", __func__);

	return 0;
}

static int s5ka3dfx_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = to_sd(ctrl);
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);
	int ret = 0;

	v4l2_dbg(1, debug, sd, "%s: ctrl_id: %d, value: %d\n",
		 __func__, ctrl->id, ctrl->val);

	mutex_lock(&info->lock);
	/*
	 * If the device is not powered up by the host driver do
	 * not apply any controls to H/W at this time. Instead
	 * the controls will be restored right after power-up.
	 */
	if (!info->power)
		goto unlock;

	switch (ctrl->id) {
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		ret = s5ka3dfx_set_wb(sd, ctrl->val);
		break;
	case V4L2_CID_EXPOSURE:
		ret = s5ka3dfx_set_exposure(sd, ctrl->val);
		break;
	case V4L2_CID_HFLIP:
		ret = s5ka3dfx_set_flip(sd, ctrl->val, info->vflip);
		break;
	case V4L2_CID_VFLIP:
		ret = s5ka3dfx_set_flip(sd, info->hflip, ctrl->val);
		break;
	case V4L2_CID_COLORFX:
		ret = s5ka3dfx_set_colorfx(sd, ctrl->val);
		break;
	default:
		ret = -EINVAL;
	}
unlock:
	mutex_unlock(&info->lock);
	return ret;
}

static int s5ka3dfx_enum_mbus_code(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index >= ARRAY_SIZE(s5ka3dfx_formats))
		return -EINVAL;

	code->code = s5ka3dfx_formats[code->index].code;
	return 0;
}

static int s5ka3dfx_get_fmt(struct v4l2_subdev *sd,
			   struct v4l2_subdev_pad_config *cfg,
			   struct v4l2_subdev_format *fmt)
{
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);
	struct v4l2_mbus_framefmt *mf;

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
		if (cfg) {
			mf = v4l2_subdev_get_try_format(sd, cfg, 0);
			fmt->format = *mf;
		}
		return 0;
	}
	mf = &fmt->format;

	mutex_lock(&info->lock);
	mf->width = info->curr_win->width;
	mf->height = info->curr_win->height;
	mf->code = info->curr_fmt->code;
	mf->colorspace = info->curr_fmt->colorspace;
	mf->field = V4L2_FIELD_NONE;

	mutex_unlock(&info->lock);
	return 0;
}

/* Return nearest media bus frame format. */
static const struct s5ka3dfx_format *s5ka3dfx_try_fmt(struct v4l2_subdev *sd,
					    struct v4l2_mbus_framefmt *mf)
{
	int i = ARRAY_SIZE(s5ka3dfx_formats);

	while (--i)
		if (mf->code == s5ka3dfx_formats[i].code)
			break;
	mf->code = s5ka3dfx_formats[i].code;

	return &s5ka3dfx_formats[i];
}

static int s5ka3dfx_set_fmt(struct v4l2_subdev *sd,
			    struct v4l2_subdev_pad_config *cfg,
			    struct v4l2_subdev_format *fmt)
{
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);
	const struct s5ka3dfx_frmsize *size = NULL;
	const struct s5ka3dfx_format *nf;
	struct v4l2_mbus_framefmt *mf;
	int ret = 0;

	nf = s5ka3dfx_try_fmt(sd, &fmt->format);
	s5ka3dfx_try_frame_size(&fmt->format, &size);
	fmt->format.colorspace = V4L2_COLORSPACE_JPEG;
	fmt->format.field = V4L2_FIELD_NONE;

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
		if (cfg) {
			mf = v4l2_subdev_get_try_format(sd, cfg, 0);
			*mf = fmt->format;
		}
		return 0;
	}
	mutex_lock(&info->lock);
	if (!info->streaming) {
		info->curr_fmt = nf;
		info->curr_win = size;
	} else {
		ret = -EBUSY;
	}
	mutex_unlock(&info->lock);
	return ret;
}

/* Called with struct s5ka3dfx_info.lock mutex held */
static int s5ka3dfx_start_preview(struct v4l2_subdev *sd)
{
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);

	return s5ka3dfx_bulk_write_reg(sd,
			s5ka3dfx_frame_sizes[info->curr_win->frs]);
}

static int s5ka3dfx_s_power(struct v4l2_subdev *sd, int on)
{
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);
	int ret;

	mutex_lock(&info->lock);
	if (on) {
		ret = power_enable(info);
		if (!ret)
			ret = s5ka3dfx_bulk_write_reg(sd, s5ka3dfx_base_regs);
	} else {
		ret = power_disable(info);
	}
	mutex_unlock(&info->lock);

	/* Restore the controls state */
	if (!ret && on)
		ret = v4l2_ctrl_handler_setup(&info->hdl);

	return ret;
}

static int s5ka3dfx_s_stream(struct v4l2_subdev *sd, int on)
{
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);
	int ret = 0;

	mutex_lock(&info->lock);
	if (on) {
		ret = s5ka3dfx_start_preview(sd);
	} else {
		/* No known way of turning streaming off,
		 * so simply reset the chip and prepare it
		 * again
		 */
		ret = power_disable(info);
		if (ret)
			return ret;

		ret = power_enable(info);
		if (!ret)
			ret = s5ka3dfx_bulk_write_reg(sd, s5ka3dfx_base_regs);
	}

	if (!ret)
		info->streaming = on;

	mutex_unlock(&info->lock);
	return ret;
}

static int s5ka3dfx_log_status(struct v4l2_subdev *sd)
{
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);

	v4l2_ctrl_handler_log_status(&info->hdl, sd->name);
	return 0;
}

static int s5ka3dfx_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct v4l2_mbus_framefmt *mf =
			v4l2_subdev_get_try_format(sd, fh->pad, 0);

	mf->width = s5ka3dfx_sizes[0].width;
	mf->height = s5ka3dfx_sizes[0].height;
	mf->code = s5ka3dfx_formats[0].code;
	mf->colorspace = V4L2_COLORSPACE_JPEG;
	mf->field = V4L2_FIELD_NONE;
	return 0;
}

static const struct v4l2_subdev_internal_ops s5ka3dfx_subdev_internal_ops = {
	.open = s5ka3dfx_open,
};

static const struct v4l2_ctrl_ops s5ka3dfx_ctrl_ops = {
	.s_ctrl = s5ka3dfx_s_ctrl,
};

static const struct v4l2_subdev_core_ops s5ka3dfx_core_ops = {
	.s_power	= s5ka3dfx_s_power,
	.log_status	= s5ka3dfx_log_status,
};

static const struct v4l2_subdev_pad_ops s5ka3dfx_pad_ops = {
	.enum_mbus_code		= s5ka3dfx_enum_mbus_code,
	.get_fmt		= s5ka3dfx_get_fmt,
	.set_fmt		= s5ka3dfx_set_fmt,
};

static const struct v4l2_subdev_video_ops s5ka3dfx_video_ops = {
	.s_stream		= s5ka3dfx_s_stream,
};

static const struct v4l2_subdev_ops s5ka3dfx_ops = {
	.core	= &s5ka3dfx_core_ops,
	.pad	= &s5ka3dfx_pad_ops,
	.video	= &s5ka3dfx_video_ops,
};

static int s5ka3dfx_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct s5ka3dfx_info *info;
	struct v4l2_subdev *sd;
	int ret;
	int i;

	info = devm_kzalloc(&client->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	mutex_init(&info->lock);
	sd = &info->sd;
	v4l2_i2c_subdev_init(sd, client, &s5ka3dfx_ops);

	sd->internal_ops = &s5ka3dfx_subdev_internal_ops;
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

	v4l2_ctrl_handler_init(&info->hdl, 5);

	v4l2_ctrl_new_std(&info->hdl, &s5ka3dfx_ctrl_ops,
			  V4L2_CID_EXPOSURE, -5, 5, 1, 0);

	v4l2_ctrl_new_std(&info->hdl, &s5ka3dfx_ctrl_ops,
			  V4L2_CID_HFLIP, 0, 1, 1, 0);

	v4l2_ctrl_new_std(&info->hdl, &s5ka3dfx_ctrl_ops,
			  V4L2_CID_VFLIP, 0, 1, 1, 0);

	/* Supports V4L2_COLORFX_NONE, V4L2_COLORFX_BW, V4L2_COLORFX_SEPIA,
	 * V4L2_COLORFX_NEGATIVE, V4L2_COLORFX_AQUA
	 */
	v4l2_ctrl_new_std_menu(&info->hdl, &s5ka3dfx_ctrl_ops,
			  V4L2_CID_COLORFX, V4L2_COLORFX_AQUA, ~0x40f,
			  V4L2_COLORFX_NONE);

	/*
	 * Supports V4L2_WHITE_BALANCE_AUTO, V4L2_WHITE_BALANCE_INCANDESCENT,
	 * V4L2_WHITE_BALANCE_FLUORESCENT, V4L2_WHITE_BALANCE_DAYLIGHT,
	 * V4L2_WHITE_BALANCE_CLOUDY
	 */
	v4l2_ctrl_new_std_menu(&info->hdl, &s5ka3dfx_ctrl_ops,
			  V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE,
			  V4L2_WHITE_BALANCE_CLOUDY, ~0x14e,
			  V4L2_WHITE_BALANCE_AUTO);

	sd->ctrl_handler = &info->hdl;

	ret = info->hdl.error;
	if (ret)
		goto np_err;

	info->gpio_nreset = devm_gpiod_get(&client->dev,
			"nreset", GPIOD_OUT_HIGH);
	if (IS_ERR(info->gpio_nreset)) {
		ret = PTR_ERR(info->gpio_nreset);
		dev_err(&client->dev, "nreset gpio request fail: %d\n", ret);
		goto np_err;
	}

	info->gpio_nstby = devm_gpiod_get(&client->dev,
			"nstandby", GPIOD_OUT_HIGH);
	if (IS_ERR(info->gpio_nstby)) {
		ret = PTR_ERR(info->gpio_nstby);
		dev_err(&client->dev, "nstandby gpiorequest fail: %d\n", ret);
		goto np_err;
	}

	info->curr_fmt = &s5ka3dfx_formats[0];
	info->curr_win = &s5ka3dfx_sizes[0];

	info->mclk = devm_clk_get(&client->dev, "mclk");
	if (IS_ERR(info->mclk)) {
		ret = PTR_ERR(info->mclk);
		goto np_err;
	}

	ret = clk_set_rate(info->mclk, 24000000);
	if (ret < 0) {
		dev_err(&client->dev, "failed to set mclk to 24000000");
		goto np_err;
	}

	for (i = 0; i < S5KA3DFX_NUM_SUPPLIES; i++)
		info->supply[i].supply = s5ka3dfx_supply_name[i];

	ret = devm_regulator_bulk_get(&client->dev, S5KA3DFX_NUM_SUPPLIES,
				 info->supply);
	if (ret)
		goto np_err;

	info->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&sd->entity, 1, &info->pad);
	if (ret < 0)
		goto np_err;

	ret = v4l2_async_register_subdev(sd);
	if (ret < 0)
		goto np_err;

	dev_info(&client->dev, "s5ka3dfx: successfully probed");

	return 0;

np_err:
	v4l2_ctrl_handler_free(&info->hdl);
	media_entity_cleanup(&sd->entity);
	return ret;
}

static int s5ka3dfx_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct s5ka3dfx_info *info = to_s5ka3dfx(sd);

	v4l2_async_unregister_subdev(sd);
	v4l2_ctrl_handler_free(&info->hdl);
	media_entity_cleanup(&sd->entity);

	return 0;
}

static const struct i2c_device_id s5ka3dfx_id[] = {
	{ MODULE_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, s5ka3dfx_id);

#ifdef CONFIG_OF
static const struct of_device_id s5k6a3_of_match[] = {
	{ .compatible = "samsung,s5ka3dfx" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, s5k6a3_of_match);
#endif

static struct i2c_driver s5ka3dfx_i2c_driver = {
	.driver = {
		.of_match_table	= of_match_ptr(s5k6a3_of_match),
		.name = MODULE_NAME
	},
	.probe		= s5ka3dfx_probe,
	.remove		= s5ka3dfx_remove,
	.id_table	= s5ka3dfx_id,
};

module_i2c_driver(s5ka3dfx_i2c_driver);

MODULE_DESCRIPTION("Samsung S5KA3DFX camera driver");
MODULE_AUTHOR("Jonathan Bakker <xc-racer2@live.ca>");
MODULE_LICENSE("GPL");
