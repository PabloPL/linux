// SPDX-License-Identifier: GPL-2.0
//
// S5PV210 Generic power domain support.
//
// Copyright (c) 2010 Samsung Electronics Co., Ltd.
//		http://www.samsung.com
//
// Implementation of S5PV210 specific power domain control which is used in
// conjunction with runtime-pm.

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pm_domain.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <dt-bindings/power/s5pv210-power.h>

#define S5PV210_PD_AUDIO_SHIFT        (1 << 7)
#define S5PV210_PD_CAM_SHIFT          (1 << 5)
#define S5PV210_PD_TV_SHIFT           (1 << 4)
#define S5PV210_PD_LCD_SHIFT          (1 << 3)
#define S5PV210_PD_G3D_SHIFT          (1 << 2)
#define S5PV210_PD_MFC_SHIFT          (1 << 1)

/*
 * S5PV210 specific wrapper around the generic power domain
 */
struct s5pv210_pm_domain {
	void __iomem *base;
	void __iomem *stat;
	struct generic_pm_domain genpd;
	u32 ctrlbit;
	char name[30];
};

static unsigned int ctrlbits[S5PV210_POWER_DOMAIN_COUNT] = {
	S5PV210_PD_AUDIO_SHIFT,
	S5PV210_PD_CAM_SHIFT,
	S5PV210_PD_TV_SHIFT,
	S5PV210_PD_LCD_SHIFT,
	S5PV210_PD_G3D_SHIFT,
	S5PV210_PD_MFC_SHIFT,
};

static spinlock_t pd_lock;

static int s5pv210_pd_pwr_done(struct s5pv210_pm_domain *pd)
{
	unsigned int cnt;
	cnt = 1000;

	do {
		if (readl_relaxed(pd->stat) & pd->ctrlbit)
			return 0;
		udelay(1);
	} while (cnt-- > 0);

	return -ETIME;
}

static int s5pv210_pd_pwr_off(struct s5pv210_pm_domain *pd)
{
	unsigned int cnt;
	cnt = 1000;

	do {
		if (!(readl_relaxed(pd->stat) & pd->ctrlbit))
			return 0;
		udelay(1);
	} while (cnt-- > 0);

	return -ETIME;
}

static bool s5pv210_pd_is_enabled(void __iomem *stat, u32 ctrlbit)
{
	return (readl_relaxed(stat) & ctrlbit) ? true : false;
}

static int s5pv210_pd_power(struct generic_pm_domain *domain, bool enable)
{
	struct s5pv210_pm_domain *pd;
	void __iomem *base;
	u32 ctrlbit, pd_reg;
	bool enabled;

	pd = container_of(domain, struct s5pv210_pm_domain, genpd);
	base = pd->base;
	ctrlbit = pd->ctrlbit;

	enabled = s5pv210_pd_is_enabled(pd->stat, ctrlbit) ? true : false;

	if (enable == enabled)
		return 0;

#if 1
	if (ctrlbit == S5PV210_PD_AUDIO_SHIFT) {
		pr_err("Audio PD is broken, leaving enabled");
		return 0;
	}
#endif

	spin_lock(&pd_lock);
	pd_reg = readl_relaxed(base);
	if (enable) {
		writel_relaxed((pd_reg | ctrlbit), base);
		if (s5pv210_pd_pwr_done(pd))
			goto out;
	} else {
		writel_relaxed((pd_reg & ~(ctrlbit)), base);
		if (s5pv210_pd_pwr_off(pd))
			goto out;
	}
	spin_unlock(&pd_lock);
	return 0;
out:
	spin_unlock(&pd_lock);
	return -ETIME;
}

static int s5pv210_pd_power_on(struct generic_pm_domain *domain)
{
	return s5pv210_pd_power(domain, true);
}

static int s5pv210_pd_power_off(struct generic_pm_domain *domain)
{
	return s5pv210_pd_power(domain, false);
}

static int s5pv210_pm_domain_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct s5pv210_pm_domain *s5pv210_pd;
	struct genpd_onecell_data *s5pv210_pd_data;
	struct generic_pm_domain **domains;
	void __iomem *base;
	void __iomem *stat;
	int i;

	if (!np) {
		dev_err(dev, "device tree node not found\n");
		return -ENODEV;
	}

	base = of_iomap(np, 0);
	if (!base) {
		pr_err("%s: failed to map base register\n", __func__);
		return -EFAULT;
	}

	stat = of_iomap(np, 1);
	if (!stat) {
		pr_err("%s: failed to map status register\n", __func__);
		return -EFAULT;
	}

	s5pv210_pd = devm_kcalloc(dev, S5PV210_POWER_DOMAIN_COUNT,
					sizeof(*s5pv210_pd), GFP_KERNEL);
	if (!s5pv210_pd)
		return -ENOMEM;

	s5pv210_pd_data = devm_kzalloc(dev, sizeof(*s5pv210_pd_data),
					GFP_KERNEL);
	if (!s5pv210_pd_data)
		return -ENOMEM;

	domains = devm_kcalloc(dev, S5PV210_POWER_DOMAIN_COUNT,
				sizeof(*domains), GFP_KERNEL);
	if (!domains)
		return -ENOMEM;

	spin_lock_init(&pd_lock);

	for (i = 0; i < ARRAY_SIZE(ctrlbits); i++, s5pv210_pd++) {
		domains[i] = &s5pv210_pd->genpd;

		s5pv210_pd->ctrlbit = ctrlbits[i];
		s5pv210_pd->base = base;
		s5pv210_pd->stat = stat;
		sprintf(s5pv210_pd->name, "%s.%d", np->name, i);
		s5pv210_pd->genpd.name = s5pv210_pd->name;
		s5pv210_pd->genpd.power_off = s5pv210_pd_power_off;
		s5pv210_pd->genpd.power_on = s5pv210_pd_power_on;

		/*
		 * Treat all power domains as off at boot.
		 *
		 * Some domains may actually be on, but keep it this way
		 * for reference counting purpose.
		 */
		pm_genpd_init(&s5pv210_pd->genpd, NULL, false);
	}

	s5pv210_pd_data->domains = domains;
	s5pv210_pd_data->num_domains = S5PV210_POWER_DOMAIN_COUNT;

	of_genpd_add_provider_onecell(np, s5pv210_pd_data);

	return 0;
}

static const struct of_device_id s5pv210_power_domain_ids[] = {
	{ .compatible = "samsung,s5pv210-pd", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, s5pv210_power_domain_ids);

static struct platform_driver s5pv210_power_domain_driver = {
	.driver	= {
		.name = "s5pv210-pd",
		.of_match_table = s5pv210_power_domain_ids,
	},
	.probe = s5pv210_pm_domain_probe,
};
module_platform_driver(s5pv210_power_domain_driver);