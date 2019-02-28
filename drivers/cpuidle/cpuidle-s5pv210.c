/*
 * CPU idle support for Samsung s5pv210 SoC.
 *
 * Copyright (C) 2019 Paweł Chmiel <pawel.mikolaj.chmiel@gmail.com>
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#include <linux/cpuidle.h>
#include <asm/cpuidle.h>

#define S5P_PWR_CFG_OFFSET              0x4000
#define S5P_PWR_CFG_STANDBYWFI_SHIFT    8
#define S5P_CFG_STANDBYWFI_IDLE                 0x0
#define S5P_CFG_STANDBYWFI_DEEPIDLE             0x1
#define S5P_CFG_STANDBYWFI_STOP                 0x2
#define S5P_CFG_STANDBYWFI_SLEEP                0x3

#define S5P_IDLE_CFG_OFFSET                             0x4020
#define S5P_IDLE_CFG_DIDLE_SHIFT                0
#define S5P_IDLE_CFG_TOP_MEMORY_SHIFT   28
#define S5P_IDLE_CFG_TOP_MEMORY_ENABLE  0x2
#define S5P_IDLE_CFG_TOP_LOGIC_SHIFT    30
#define S5P_IDLE_CFG_TOP_LOGIC_ENABLE   0x2

static struct regmap *map;

/* Actual code that puts the SoC in different idle states */
static int s5pv210_enter_idle(struct cpuidle_device *dev,
			      struct cpuidle_driver *drv,
			      int index)
{
	static u32 value;
	static u32 mask;

	mask = (3 << S5P_IDLE_CFG_TOP_LOGIC_SHIFT)
	       | (3 << S5P_IDLE_CFG_TOP_MEMORY_SHIFT)
	       | (3 << S5P_IDLE_CFG_DIDLE_SHIFT);

	value = (S5P_IDLE_CFG_TOP_LOGIC_ENABLE << S5P_IDLE_CFG_TOP_LOGIC_SHIFT)
		| (S5P_IDLE_CFG_TOP_MEMORY_ENABLE << S5P_IDLE_CFG_TOP_MEMORY_SHIFT);

	regmap_update_bits(map, S5P_IDLE_CFG_OFFSET, mask, value);

	mask = (3 << S5P_PWR_CFG_STANDBYWFI_SHIFT);
	value = (S5P_CFG_STANDBYWFI_IDLE << S5P_PWR_CFG_STANDBYWFI_SHIFT);

	regmap_update_bits(map, S5P_PWR_CFG_OFFSET, mask, value);

	cpu_do_idle();

	return index;
}

static struct cpuidle_driver s5pv210_idle_driver = {
	.name				= "s5pv210_idle",
	.owner				= THIS_MODULE,
	.states[0] = {
		.enter			= s5pv210_enter_idle,
		.exit_latency		= 1,
		.target_residency	= 10000,
		.name			= "WFI",
		.desc			= "ARM WFI",
	},
	.state_count			= 1,
};

static int s5pv210_idle_probe(struct platform_device *pdev)
{
	map = syscon_regmap_lookup_by_phandle(pdev->dev.of_node, "regmap");
	if (IS_ERR(map)) {
		dev_err(&pdev->dev, "unable to get syscon");
		return PTR_ERR(map);
	}

	return cpuidle_register(&s5pv210_idle_driver, NULL);
}

static int s5pv210_idle_remove(struct platform_device *pdev)
{
	cpuidle_unregister(&s5pv210_idle_driver);

	return 0;
}

static const struct of_device_id s5pv210_idle_of_match[] = {
	{ .compatible = "samsung,s5pv210-cpuidle" },
	{},
};

static struct platform_driver s5pv210_cpuidle_driver = {
	.probe			= s5pv210_idle_probe,
	.remove			= s5pv210_idle_remove,
	.driver			= {
		.name		= "s5pv210-cpuidle",
		.of_match_table = s5pv210_idle_of_match,
	},
};

static int __init s5pv210_cpuidle_init(void)
{
	return platform_driver_register(&s5pv210_cpuidle_driver);
}

device_initcall(s5pv210_cpuidle_init);