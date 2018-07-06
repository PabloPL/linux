/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides bindings for the BMA150 accelerometer
 */
#ifndef _DT_BINDINGS_INPUT_BMA150_H
#define _DT_BINDINGS_INPUT_BMA150_H

/* Range */
#define BMA150_RANGE_2G		0
#define BMA150_RANGE_4G		1
#define BMA150_RANGE_8G		2

/* Refresh rate */
#define BMA150_BW_25HZ		0
#define BMA150_BW_50HZ		1
#define BMA150_BW_100HZ		2
#define BMA150_BW_190HZ		3
#define BMA150_BW_375HZ		4
#define BMA150_BW_750HZ		5
#define BMA150_BW_1500HZ	6

#endif /* _DT_BINDINGS_INPUT_BMA150_H */
