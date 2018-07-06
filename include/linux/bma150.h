/*
 * Copyright (c) 2011 Bosch Sensortec GmbH
 * Copyright (c) 2011 Unixphere
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _BMA150_H_
#define _BMA150_H_

#include <dt-bindings/input/bma150.h>

#define BMA150_DRIVER		"bma150"


struct bma150_cfg {
	bool any_motion_int;		/* Set to enable any-motion interrupt */
	bool hg_int;			/* Set to enable high-G interrupt */
	bool lg_int;			/* Set to enable low-G interrupt */
	u32 any_motion_dur;		/* Any-motion duration */
	u32 any_motion_thres;		/* Any-motion threshold */
	u32 hg_hyst;			/* High-G hysterisis */
	u32 hg_dur;			/* High-G duration */
	u32 hg_thres;			/* High-G threshold */
	u32 lg_hyst;			/* Low-G hysterisis */
	u32 lg_dur;			/* Low-G duration */
	u32 lg_thres;			/* Low-G threshold */
	u32 range;			/* one of BMA0150_RANGE_xxx */
	u32 bandwidth;			/* one of BMA0150_BW_xxx */
};

struct bma150_platform_data {
	struct bma150_cfg cfg;
	int (*irq_gpio_cfg)(void);
};

#endif /* _BMA150_H_ */
