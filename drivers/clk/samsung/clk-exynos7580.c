#include <linux/clk-provider.h>
#include <linux/of.h>

#include "clk.h"
#include <dt-bindings/clock/exynos7580-clk.h>

/*
 * TODO
 * - input clocks for clock controller
 */

/* Register Offset definitions for CMU_APL (0x10800000) */
#define APL_PLL_LOCK				0x0000
#define APL_PLL_CON0				0x0100
#define APL_PLL_CON1				0x0104
#define APL_PLL_FREQ_DET			0x0108
#define MUX_SEL_APL_0				0x0200
#define MUX_SEL_APL_1				0x0204
#define MUX_SEL_APL_2				0x0208
#define MUX_EN_APL_0				0x0300
#define MUX_EN_APL_1				0x0304
#define MUX_EN_APL_2				0x0308
#define MUX_STAT_APL_0				0x0400
#define MUX_STAT_APL_1				0x0404
#define MUX_STAT_APL_2				0x0408
#define DIV_APL_0					0x0600
#define DIV_APL_1					0x0604
#define DIV_STAT_APL_0				0x0700
#define DIV_STAT_APL_1				0x0704
#define EN_ACLK_APL					0x0800
#define EN_PCLK_APL					0x0900
#define EN_SCLK_APL					0x0A00
#define CLKOUT_CMU_APL				0x0C00
#define CLKOUT_CMU_APL_DIV_STAT		0x0C04
#define EN_PDN_APL					0x0D00
#define APL_ARMCLK_STOPCTRL			0x1000
#define APL_PWR_CTRL				0x1020
#define APL_PWR_CTRL2				0x1024
#define APL_PWR_CTRL3				0x1028
#define APL_INTR_SPREAD_EN			0x1080
#define APL_INTR_SPREAD_STANDBYWFI	0x1084

#define MUX_GATE GATE

PNAME(mout_apl_p) = {"apl_pll", "mout_bus_pll_apl_user"};

static const unsigned long apl_clk_regs[] __initconst = {
};

static const struct samsung_mux_clock apl_mux_clks[] __initconst = {
	MUX(0, "mout_apl", mout_apl_p, MUX_SEL_APL_2, 0, 1),
};

static const struct samsung_gate_clock apl_gate_clks[] __initconst = {
	MUX_GATE(0, "mout_bus_pll_apl_user", "bus_pll", MUX_SEL_APL_1, 0, CLK_IGNORE_UNUSED, 0),
};

struct samsung_pll_rate_table table_apl[] = {
	/* rate		p,  m,  s,  k */
	{1900000000U,	4, 292, 0, 0},
	{1800000000U,	4, 276, 0, 0},
	{1700000000U,	4, 262, 0, 0},
	{1600000000U,	4, 246, 0, 0},
	{1500000000U,	4, 230, 0, 0},
	{1400000000U,	4, 216, 0, 0},
	{1300000000U,	4, 200, 0, 0},
	{1200000000U,	4, 368, 1, 0},
	{1100000000U,	4, 340, 1, 0},
	{1000000000U,	4, 308, 1, 0},
	{900000000U,	4, 276, 1, 0},
	{800000000U,	4, 248, 1, 0},
	{700000000U,	4, 216, 1, 0},
	{600000000U,	4, 368, 2, 0},
	{500000000U,	4, 312, 2, 0},
	{400000000U,	4, 248, 2, 0},
	{300000000U,	4, 368, 3, 0},
	{200000000U,	4, 240, 3, 0},
	{100000000U,	4, 256, 4, 0},
	{0,		0, 0, 0, 0},
};

static const struct samsung_pll_clock apl_pll_clks[] __initconst = {
	PLL(pll_2555x, CLK_APL_PLL, "apl_pll", "fin_pll", APL_PLL_LOCK, APL_PLL_CON0, table_apl),
};

static const struct samsung_cmu_info apl_cmu_info __initconst = {
	.pll_clks		= apl_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(apl_pll_clks),
	.mux_clks		= apl_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(apl_mux_clks),
	.gate_clks		= apl_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(apl_gate_clks),
	.nr_clk_ids		= APL_NR_CLK,
	.clk_regs		= apl_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(apl_clk_regs),
};

static void __init exynos7580_cmu_apl_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &apl_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_apl, "samsung,exynos7580-cmu-apl",
		exynos7580_cmu_apl_init);

/* Register Offset definitions for CMU_AUD (0x110C0000) */
#define MUX_SEL_AUD0		0x0200
#define MUX_SEL_AUD1		0x0204
#define MUX_EN_AUD0			0x0300
#define MUX_EN_AUD1			0x0304
#define MUX_STAT_AUD0		0x0400
#define DIV_AUD0			0x0600
#define DIV_AUD1			0x0604
#define DIV_STAT_AUD0		0x0700
#define DIV_STAT_AUD1		0x0704
#define EN_ACLK_AUD			0x0800
#define EN_PCLK_AUD			0x0900
#define EN_SCLK_AUD			0x0A00

static const struct samsung_fixed_rate_clock aud_fixed_rate_clks[] __initconst = {
	FRATE(0, "ioclk_audi2s0cdclk", NULL, 0, 100000000),
	FRATE(0, "ioclk_audi2s0sclk", NULL, 0, 50000000),
	FRATE(CLK_IOCLK_AUDIOCDCLK1, "ioclk_audiocdclk1", NULL, 0, 100000000),
	FRATE(0, "ioclk_audmix_bclk", NULL, 0, 12288000),
};

PNAME(mout_sclk_mi2s_pcm_aud_p) = {"mout_aud_pll_user", "ioclk_audi2s0cdclk"};

static const unsigned long aud_clk_regs[] __initconst = {
};

static const struct samsung_mux_clock aud_mux_clks[] __initconst = {
	MUX(CLK_MOUT_SCLK_MI2S, "mout_sclk_mi2s_aud", mout_sclk_mi2s_pcm_aud_p, MUX_SEL_AUD1, 0, 1),
	MUX(CLK_MOUT_SCLK_PCM, "mout_sclk_pcm_aud", mout_sclk_mi2s_pcm_aud_p, MUX_SEL_AUD1, 8, 1),
};

static const struct samsung_div_clock aud_div_clks[] __initconst = {
	DIV(CLK_DOUT_ACLK_133, "dout_aclk_aud_133", "mout_aud_pll_user", DIV_AUD0, 0, 4),
	DIV(CLK_DOUT_SCLK_MI2S, "dout_sclk_mi2s_aud", "mout_sclk_mi2s_aud", DIV_AUD1, 0, 4),
	DIV(CLK_DOUT_SCLK_PCM, "dout_sclk_pcm_aud", "mout_sclk_pcm_aud", DIV_AUD1, 4, 8),
	DIV(CLK_DOUT_SCLK_AUD_UART, "dout_sclk_uart_aud", "mout_aud_pll_user", DIV_AUD1, 12, 4),
	DIV(CLK_DOUT_SCLK_AUDMIXER, "dout_sclk_audmixer_aud", "mout_aud_pll_user", DIV_AUD1, 20, 4),
};

static const struct samsung_gate_clock aud_gate_clks[] __initconst = {
	GATE(CLK_LPASS_DMAC, "aclk_lpass_dmac", "dout_aclk_aud_133", EN_ACLK_AUD, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_LPASS_MEM, "aclk_lpass_mem", "dout_aclk_aud_133", EN_ACLK_AUD, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_sasync_xiu_aud_to_mif", "dout_aclk_aud_133", EN_ACLK_AUD, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_upsizer_aud_64to128", "dout_aclk_aud_133", EN_ACLK_AUD, 7, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_PCLK_MI2S, "pclk_mi2s_aud", "dout_aclk_aud_133", EN_PCLK_AUD, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_pcm_aud", "dout_aclk_aud_133", EN_PCLK_AUD, 1, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_PUART3, "pclk_uart_aud", "dout_aclk_aud_133", EN_PCLK_AUD, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_lpass_sfr", "dout_aclk_aud_133", EN_PCLK_AUD, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_gpio_aud", "dout_aclk_aud_133", EN_PCLK_AUD, 7, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_MI2S, "sclk_mi2s_aud_i2scodclki", "dout_sclk_mi2s_aud", EN_SCLK_AUD, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_PCM, "sclk_pcm_aud", "dout_sclk_pcm_aud", EN_SCLK_AUD, 2, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SUART3, "sclk_uart_aud", "dout_sclk_uart_aud", EN_SCLK_AUD, 3, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_AUDMIXER_SYSCLK, "sclk_audmixer", "dout_sclk_audmixer_aud", EN_SCLK_AUD, 7, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_AUDMIXER_BCLK0, "sclk_audmixer_bclk0", "ioclk_audmix_bclk", EN_SCLK_AUD, 8, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_AUDMIXER_BCLK1, "sclk_audmixer_bclk1", "ioclk_audmix_bclk", EN_SCLK_AUD, 9, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_AUDMIXER_BCLK2, "sclk_audmixer_bclk2", "ioclk_audmix_bclk", EN_SCLK_AUD, 10, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_MI2S_AUD_BCLK, "sclk_mi2s_aud_i2sbclki", "ioclk_audi2s0sclk", EN_SCLK_AUD, 1, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_AUD_PLL_USER, "mout_aud_pll_user", "aud_pll", MUX_SEL_AUD0, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info aud_cmu_info __initconst = {
	.fixed_clks		= aud_fixed_rate_clks,
	.nr_fixed_clks		= ARRAY_SIZE(aud_fixed_rate_clks),
	.mux_clks		= aud_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(aud_mux_clks),
	.div_clks		= aud_div_clks,
	.nr_div_clks		= ARRAY_SIZE(aud_div_clks),
	.gate_clks		= aud_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(aud_gate_clks),
	.nr_clk_ids		= AUD_NR_CLK,
	.clk_regs		= aud_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(aud_clk_regs),
};

static void __init exynos7580_cmu_aud_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &aud_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_aud, "samsung,exynos7580-cmu-aud",
		exynos7580_cmu_aud_init);

/* Register Offset definitions for CMU_BUS0 (0x11800000) */
#define DIV_BUS0			0x0600
#define DIV_STAT_BUS0		0x0700
#define EN_ACLK_BUS0		0x0800
#define EN_PCLK_BUS0		0x0900

static const unsigned long bus0_clk_regs[] __initconst = {
	DIV_BUS0,
};

static const struct samsung_div_clock bus0_div_clks[] __initconst = {
	DIV(0, "dout_pclk_bus0_100", "dout_aclk_bus0_400", DIV_BUS0, 0, 3),
};

static const struct samsung_cmu_info bus0_cmu_info __initconst = {
	.div_clks		= bus0_div_clks,
	.nr_div_clks		= ARRAY_SIZE(bus0_div_clks),
	.nr_clk_ids		= BUS0_NR_CLK,
	.clk_regs		= bus0_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(bus0_clk_regs),
};

static void __init exynos7580_cmu_bus0_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &bus0_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_bus0, "samsung,exynos7580-cmu-bus0",
		exynos7580_cmu_bus0_init);

/* Register Offset definitions for CMU_BUS1 (0x12400000) */
#define DIV_BUS1			0x0600
#define DIV_STAT_BUS1		0x0700
#define EN_ACLK_BUS1		0x0800
#define EN_PCLK_BUS1		0x0900

static const unsigned long bus1_clk_regs[] __initconst = {
	DIV_BUS1,
};

static const struct samsung_div_clock bus1_div_clks[] __initconst = {
	DIV(0, "dout_pclk_bus1_100", "dout_aclk_bus1_400", DIV_BUS1, 0, 3),
};

static const struct samsung_cmu_info bus1_cmu_info __initconst = {
	.div_clks		= bus1_div_clks,
	.nr_div_clks		= ARRAY_SIZE(bus1_div_clks),
	.nr_clk_ids		= BUS1_NR_CLK,
	.clk_regs		= bus1_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(bus1_clk_regs),
};

static void __init exynos7580_cmu_bus1_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &bus1_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_bus1, "samsung,exynos7580-cmu-bus1",
		exynos7580_cmu_bus1_init);

/* Register Offset definitions for CMU_BUS2 (0x13000000) */
#define DIV_BUS2			0x0600
#define DIV_STAT_BUS2		0x0700
#define EN_ACLK_BUS2		0x0800
#define EN_PCLK_BUS2		0x0900

static const unsigned long bus2_clk_regs[] __initconst = {
	DIV_BUS2,
};

static const struct samsung_div_clock bus2_div_clks[] __initconst = {
	DIV(0, "dout_pclk_bus2_100", "dout_aclk_bus2_400", DIV_BUS2, 0, 3),
};

static const struct samsung_cmu_info bus2_cmu_info __initconst = {
	.div_clks		= bus2_div_clks,
	.nr_div_clks		= ARRAY_SIZE(bus2_div_clks),
	.nr_clk_ids		= BUS2_NR_CLK,
	.clk_regs		= bus2_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(bus2_clk_regs),
};

static void __init exynos7580_cmu_bus2_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &bus2_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_bus2, "samsung,exynos7580-cmu-bus2",
		exynos7580_cmu_bus2_init);

/* Register Offset definitions for CMU_CPU (0x10900000) */
#define CPU_PLL_LOCK				0x0000
#define CPU_PLL_CON0				0x0100
#define CPU_PLL_CON1				0x0104
#define CPU_PLL_FREQ_DET			0x0108
#define MUX_SEL_CPU_0				0x0200
#define MUX_SEL_CPU_1				0x0204
#define MUX_SEL_CPU_2				0x0208
#define MUX_EN_CPU_0				0x0300
#define MUX_EN_CPU_1				0x0304
#define MUX_EN_CPU_2				0x0308
#define MUX_STAT_CPU_0				0x0400
#define MUX_STAT_CPU_1				0x0404
#define MUX_STAT_CPU_2				0x0408
#define DIV_CPU_0					0x0600
#define DIV_CPU_1					0x0604
#define DIV_STAT_CPU_0				0x0700
#define DIV_STAT_CPU_1				0x0704
#define EN_ACLK_CPU					0x0800
#define EN_PCLK_CPU					0x0900
#define EN_SCLK_CPU					0x0A00
#define CLKOUT_CMU_CPU				0x0C00
#define CLKOUT_CMU_CPU_DIV_STAT		0x0C04
#define EN_PDN_CPU					0x0D00
#define CPU_ARMCLK_STOPCTRL			0x1000
#define CPU_PWR_CTRL				0x1020
#define CPU_PWR_CTRL2				0x1024
#define CPU_PWR_CTRL3				0x1028
#define CPU_INTR_SPREAD_EN			0x1080
#define CPU_INTR_SPREAD_STANBYWFI	0x1084

PNAME(mout_cpu_p) = {"cpu_pll", "mout_bus_pll_cpu_user"};

static const unsigned long cpu_clk_regs[] __initconst = {
};

static const struct samsung_mux_clock cpu_mux_clks[] __initconst = {
	MUX(0, "mout_cpu", mout_cpu_p, MUX_SEL_CPU_2, 0, 1),
};

static const struct samsung_gate_clock cpu_gate_clks[] __initconst = {
	MUX_GATE(0, "mout_bus_pll_cpu_user", "bus_pll", MUX_SEL_CPU_1, 0, CLK_IGNORE_UNUSED, 0),
};

struct samsung_pll_rate_table table_cpu[] = {
	/* rate		p,  m,  s,  k */
	{1900000000U,	4, 292, 0, 0},
	{1800000000U,	4, 276, 0, 0},
	{1700000000U,	4, 262, 0, 0},
	{1600000000U,	4, 246, 0, 0},
	{1500000000U,	4, 230, 0, 0},
	{1400000000U,	4, 216, 0, 0},
	{1300000000U,	4, 200, 0, 0},
	{1200000000U,	4, 368, 1, 0},
	{1100000000U,	4, 340, 1, 0},
	{1000000000U,	4, 308, 1, 0},
	{900000000U,	4, 276, 1, 0},
	{800000000U,	4, 248, 1, 0},
	{700000000U,	4, 216, 1, 0},
	{600000000U,	4, 368, 2, 0},
	{500000000U,	4, 312, 2, 0},
	{400000000U,	4, 248, 2, 0},
	{300000000U,	4, 368, 3, 0},
	{200000000U,	4, 240, 3, 0},
	{100000000U,	4, 256, 4, 0},
	{0,		0, 0, 0, 0},
};

static const struct samsung_pll_clock cpu_pll_clks[] __initconst = {
	PLL(pll_2555x, CLK_CPU_PLL, "cpu_pll", "fin_pll", CPU_PLL_LOCK, CPU_PLL_CON0, table_cpu),
};

static const struct samsung_cmu_info cpu_cmu_info __initconst = {
	.pll_clks		= cpu_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(cpu_pll_clks),
	.mux_clks		= cpu_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(cpu_mux_clks),
	.gate_clks		= cpu_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(cpu_gate_clks),
	.nr_clk_ids		= CPU_NR_CLK,
	.clk_regs		= cpu_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(cpu_clk_regs),
};

static void __init exynos7580_cmu_cpu_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &cpu_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_cpu, "samsung,exynos7580-cmu-cpu",
		exynos7580_cmu_cpu_init);

/* Register Offset definitions for CMU_DISP (0x14870000) */
#define DISP_PLL_LOCK			0x0000
#define DISP_PLL_CON0			0x0100
#define DISP_PLL_CON1			0x0104
#define DISP_PLL_FREQ_DET		0x010C
#define MUX_SEL_DISP0			0x0200
#define MUX_SEL_DISP1			0x0204
#define MUX_SEL_DISP2			0x0208
#define MUX_SEL_DISP4			0x0210
#define MUX_EN_DISP0			0x0300
#define MUX_EN_DISP1			0x0304
#define MUX_EN_DISP2			0x0308
#define MUX_EN_DISP4			0x0310
#define MUX_STAT_DISP0			0x0400
#define MUX_STAT_DISP1			0x0404
#define MUX_STAT_DISP2			0x0408
#define MUX_STAT_DISP4			0x0410
#define MUX_IGNORE_DISP2		0x0508
#define DIV_DISP				0x0600
#define DIV_STAT_DISP			0x0700
#define EN_ACLK_DISP0			0x0800
#define EN_ACLK_DISP1			0x0804
#define EN_PCLK_DISP			0x0900
#define EN_SCLK_DISP			0x0A00
#define CLKOUT_CMU_DISP			0x0C00
#define CLKOUT_CMU_DISP_DIV_STAT	0x0C04

static const struct samsung_fixed_rate_clock disp_fixed_rate_clks[] __initconst = {
	FRATE(0, "phyclk_txbyteclkhs_m4s4", NULL, 0, 188000000),
	FRATE(0, "phyclk_rxclkesc0_m4s4", NULL, 0, 20000000),
	FRATE(0, "sclk_decon_int_rgb_vclk", NULL, 0, 166000000),
};

PNAME(mout_sclk_decon_int_eclk_p) = {"disp_pll", "mout_sclk_decon_int_eclk_user"};
PNAME(mout_sclk_decon_int_vclk_p) = {"disp_pll", "mout_sclk_decon_int_vclk_user"};

static const unsigned long disp_clk_regs[] __initconst = {
};

static const struct samsung_mux_clock disp_mux_clks[] __initconst = {
	MUX(CLK_MUX_SCLK_DECON_INT_ECLK, "mout_sclk_decon_int_eclk", mout_sclk_decon_int_eclk_p, MUX_SEL_DISP4, 0, 1),
	MUX(CLK_MUX_SCLK_DECON_INT_VCLK, "mout_sclk_decon_int_vclk", mout_sclk_decon_int_vclk_p, MUX_SEL_DISP4, 4, 1),
};

static const struct samsung_div_clock disp_div_clks[] __initconst = {
	DIV(CLK_DIV_PCLK_DISP_100, "dout_pclk_disp_100", "mout_aclk_disp_200_user", DIV_DISP, 0, 2),
	DIV(CLK_DIV_SCLK_DECON_INT_ECLK, "dout_sclk_decon_int_eclk", "mout_sclk_decon_int_eclk", DIV_DISP, 4, 3),
	DIV(CLK_DIV_SCLK_DECON_INT_VCLK, "dout_sclk_decon_int_vclk", "mout_sclk_decon_int_vclk", DIV_DISP, 8, 3),
};

struct samsung_pll_rate_table table_disp[] = {
	/* rate		p,  m,  s,  k */
	{333000000U,	13, 333, 1, 0},
	{284000000U,	13, 284, 1, 0},
	{276000000U,	13, 276, 1, 0},
	{273000000U,	4, 336, 3, 0},
	{166000000U,	13, 332, 2, 0},
	{142000000U,	13, 284, 2, 0},
  	{140250000U,	13, 561, 3, 0},
	{136000000U,	13, 272, 2, 0},
	{74250000U, 	13, 297, 3, 0},
	{64500000U, 	13, 129, 2, 0},
	{63000000U, 	13, 63, 1, 0},
	{58000000U,	13, 116, 2, 0},
	{0,		0, 0, 0, 0},
};

static const struct samsung_gate_clock disp_gate_clks[] __initconst = {
	GATE(CLK_ACLK_DECON0, "aclk_decon_int", "mout_aclk_disp_200_user", EN_ACLK_DISP0, 0, 0, 0),
	GATE(CLK_ACLK_XIU_DISP1, "aclk_xiu_disp1", "mout_aclk_disp_200_user", EN_ACLK_DISP1, 0, 0, 0),
	GATE(CLK_ACLK_SMMU_DISP_MMU, "aclk_smmu_disp_mmu", "mout_aclk_disp_200_user", EN_ACLK_DISP1, 9, 0, 0),
	GATE(CLK_PCLK_DECON_INT, "pclk_decon_int", "dout_pclk_disp_100", EN_PCLK_DISP, 0, 0, 0),
	GATE(CLK_PCLK_DSI_LINK0, "pclk_dsi_link0", "dout_pclk_disp_100", EN_PCLK_DISP, 4, 0, 0),
	GATE(CLK_PCLK_SMMU_DISP_MMU, "pclk_smmu_disp_mmu", "dout_pclk_disp_100", EN_PCLK_DISP, 11, 0, 0),
	GATE(CLK_SCLK_DECON_INT_ECLK, "sclk_decon_int_eclk", "dout_sclk_decon_int_eclk", EN_SCLK_DISP, 0, 0, 0),
	GATE(CLK_SCLK_DECON_INT_VCLK, "sclk_decon_int_vclk", "dout_sclk_decon_int_vclk", EN_SCLK_DISP, 1, 0, 0),
	GATE(CLK_SCLK_DSI_LINK0_I_RGB_VCLK, "sclk_dsi_link0_i_rgb_vclk", "sclk_decon_int_rgb_vclk", EN_SCLK_DISP, 5, 0, 0),
	GATE(CLK_PHYCLK_BITCLKDIV8, "phyclk_bitclkdiv8", "mout_phyclk_bitclkdiv8_user", EN_SCLK_DISP, 9, 0, 0),
	GATE(CLK_PHYCLK_RXCLKESC0, "phyclk_rxclkesc0", "mout_phyclk_rxclkesc0_user", EN_SCLK_DISP, 10, 0, 0),
	MUX_GATE(CLK_MUX_ACLK_DISP_200_USER, "mout_aclk_disp_200_user", "aclk_disp_200" , MUX_SEL_DISP1, 0, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MUX_SCLK_DECON_INT_ECLK_USER, "mout_sclk_decon_int_eclk_user", "sclk_disp_decon_int_eclk", MUX_SEL_DISP1, 8, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MUX_SCLK_DECON_INT_VCLK_USER, "mout_sclk_decon_int_vclk_user", "sclk_disp_decon_int_vclk", MUX_SEL_DISP1, 12, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MUX_PHYCLK_BITCLKDIV8_USER, "mout_phyclk_bitclkdiv8_user", "phyclk_txbyteclkhs_m4s4", MUX_SEL_DISP2, 16, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MUX_PHYCLK_RXCLKESC0_USER, "mout_phyclk_rxclkesc0_user", "phyclk_rxclkesc0_m4s4", MUX_SEL_DISP2, 20, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_pll_clock disp_pll_clks[] __initconst = {
	PLL(pll_2551x, CLK_DISP_PLL, "disp_pll", "fin_pll", DISP_PLL_LOCK, DISP_PLL_CON0, table_disp),
};

static const struct samsung_cmu_info disp_cmu_info __initconst = {
	.fixed_clks		= disp_fixed_rate_clks,
	.nr_fixed_clks		= ARRAY_SIZE(disp_fixed_rate_clks),
	.pll_clks		= disp_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(disp_pll_clks),
	.mux_clks		= disp_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(disp_mux_clks),
	.div_clks		= disp_div_clks,
	.nr_div_clks		= ARRAY_SIZE(disp_div_clks),
	.gate_clks		= disp_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(disp_gate_clks),
	.nr_clk_ids		= DISP_NR_CLK,
	.clk_regs		= disp_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(disp_clk_regs),
};

static void __init exynos7580_cmu_disp_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &disp_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_disp, "samsung,exynos7580-cmu-disp",
		exynos7580_cmu_disp_init);

/* Register Offset definitions for CMU_FSYS (0x13730000) */
#define USB_PLL_LOCK			0x0000
#define USB_PLL_CON0			0x0100
#define USB_PLL_CON1			0x0104
#define USB_PLL_FREQ_DET		0x010C
#define MUX_SEL_FSYS0			0x0200
#define MUX_SEL_FSYS1			0x0204
#define MUX_SEL_FSYS2			0x0208
#define MUX_EN_FSYS0			0x0300
#define MUX_EN_FSYS1			0x0304
#define MUX_EN_FSYS2			0x0308
#define MUX_STAT_FSYS0			0x0400
#define MUX_STAT_FSYS1			0x0404
#define MUX_STAT_FSYS2			0x0408
#define MUX_IGNORE_FSYS0		0x0504
#define MUX_IGNORE_FSYS1		0x0508
#define EN_ACLK_FSYS0			0x0800
#define EN_ACLK_FSYS1			0x0804
#define EN_ACLK_FSYS2			0x0808
#define EN_PCLK_FSYS			0x0900
#define EN_SCLK_FSYS			0x0A00
#define CLKOUT_CMU_FSYS			0x0C00
#define CLKOUT_CMU_FSYS_DIV_STAT	0x0C04

static const struct samsung_fixed_rate_clock fsys_fixed_rate_clks[] __initconst = {
	FRATE(0, "phyclk_usbhost20_freeclk", NULL, 0, 60000000),
	FRATE(0, "phyclk_usbhost20_phyclock", NULL, 0, 60000000),
	FRATE(0, "phyclk_usbhost20_clk48mohcl", NULL, 0, 48000000),
	FRATE(0, "phyclk_usbotg20", NULL, 0, 60000000),
};

static const unsigned long fsys_clk_regs[] __initconst = {
	EN_SCLK_FSYS,
	EN_PCLK_FSYS,
	EN_ACLK_FSYS0,
	EN_ACLK_FSYS1,
	EN_ACLK_FSYS2,
	MUX_SEL_FSYS0,
	USB_PLL_CON0,
};

static const struct samsung_gate_clock fsys_gate_clks[] __initconst = {
	GATE(CLK_SCLK_MMC0, "sclk_mmc0_sdclkin", "sclk_fsys_mmc0_sdclkin", EN_SCLK_FSYS, 5, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_MMC1, "sclk_mmc1_sdclkin", "sclk_fsys_mmc1_sdclkin", EN_SCLK_FSYS, 6, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_MMC2, "sclk_mmc2_sdclkin", "sclk_fsys_mmc2_sdclkin", EN_SCLK_FSYS, 7, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_USB_PLL, "sclk_usb20_phy_clkcore", "usb_pll", EN_SCLK_FSYS, 1, 0, 0),
	GATE(0, "phyclk_usbhot20_freeclk", "mout_phyclk_usbhost20_freeclk_user", EN_SCLK_FSYS, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "phyclk_usbhost20_usb20_phyclock", "mout_phyclk_usbhost20_phyclock_user", EN_SCLK_FSYS, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "phyclk_usbhost20_clk48mohci", "mout_phyclk_usbhost20_clk48mohcl_user", EN_SCLK_FSYS, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "phyclk_usbotg20_otg20_phyclock", "mout_phyclk_usbotg20", EN_SCLK_FSYS, 22, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_PDMA0, "aclk_dma_fsys0", "aclk_upsizer_dma_fsys0", EN_ACLK_FSYS0, 0, 0, 0),
	GATE(CLK_PDMA1, "aclk_dma_fsys1", "aclk_upsizer_dma_fsys1", EN_ACLK_FSYS0, 1, 0, 0),
	GATE(CLK_USBHOST20, "aclk_usbhost20", "aclk_fsys_200", EN_ACLK_FSYS0, 3, 0, 0),
	GATE(0, "aclk_sromc_hclk", "aclk_fsys_200", EN_ACLK_FSYS0, 6, 0, 0),
	GATE(CLK_ACLK_MMC0, "aclk_mmc0", "aclk_fsys_200", EN_ACLK_FSYS0, 7, 0, 0),
	GATE(CLK_ACLK_MMC1, "aclk_mmc1", "aclk_fsys_200", EN_ACLK_FSYS0, 8, 0, 0),
	GATE(CLK_ACLK_MMC2, "aclk_mmc2", "aclk_fsys_200", EN_ACLK_FSYS0, 9, 0, 0),
	GATE(CLK_OTG_ACLK, "aclk_usbotg20", "aclk_fsys_200", EN_ACLK_FSYS0, 16, 0, 0),
	GATE(CLK_OTG_HCLK, "aclk_ahb2axi_usbotg", "aclk_fsys_200", EN_ACLK_FSYS0, 17, 0, 0),
	GATE(CLK_UPSIZER_OTG, "aclk_upsizer_usbotg", "aclk_fsys_200", EN_ACLK_FSYS0, 18, 0, 0),
	GATE(CLK_XIU_D_FSYS1, "aclk_xiu_d_fsys1", "aclk_fsys_200", EN_ACLK_FSYS1, 1, 0, 0),
	GATE(CLK_AHB_USBHS, "aclk_ahb_usbhs", "aclk_fsys_200", EN_ACLK_FSYS1, 8, 0, 0),
	GATE(CLK_AHB2AXI_USBHS, "aclk_ahb2axi_usbhs", "aclk_fsys_200", EN_ACLK_FSYS1, 17, 0, 0),
	GATE(CLK_UPSIZER_FSYS1, "aclk_upsizer_fsys1_to_fsys0", "aclk_fsys_200", EN_ACLK_FSYS2, 4, 0, 0),
	GATE(0, "aclk_upsizer_dma_fsys0", "aclk_fsys_200", EN_ACLK_FSYS2, 6, 0, 0),
	GATE(0, "aclk_upsizer_dma_fsys1", "aclk_fsys_200", EN_ACLK_FSYS2, 7, 0, 0),
	GATE(CLK_UPSIZER_AHB_USBHS, "aclk_upsizer_ahb_usbhs", "aclk_fsys_200", EN_ACLK_FSYS2, 10, 0, 0),
	GATE(0, "aclk_ppmu_fsys", "aclk_fsys_200", EN_ACLK_FSYS2, 11, 0, 0),
	GATE(0, "pclk_ppmu_fsys", "aclk_fsys_200", EN_PCLK_FSYS, 18, 0, 0),
	MUX_GATE(CLK_FREECLK, "mout_phyclk_usbhost20_freeclk_user", "phyclk_usbhost20_freeclk",  MUX_SEL_FSYS1, 8, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_PHYCLK, "mout_phyclk_usbhost20_phyclock_user", "phyclk_usbhost20_phyclock", MUX_SEL_FSYS1, 12, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_CLK48MOHCI, "mout_phyclk_usbhost20_clk48mohcl_user", "phyclk_usbhost20_clk48mohcl", MUX_SEL_FSYS1, 16, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_PHY_OTG, "mout_phyclk_usbotg20", "phyclk_usbotg20", MUX_SEL_FSYS2, 12, CLK_IGNORE_UNUSED, 0),
};

struct samsung_pll_rate_table table_usb[] = {
	/* rate		p,  m,  s,  k */
	{50000000U,	13, 400, 4, 0},
	{24000000U,	13, 384, 5, 0},
	{0,		0, 0, 0, 0},
};

static const struct samsung_pll_clock fsys_pll_clks[] __initconst = {
	PLL(pll_2551x, 0, "usb_pll", "fin_pll", USB_PLL_LOCK, USB_PLL_CON0, table_usb),
};

static const struct samsung_cmu_info fsys_cmu_info __initconst = {
	.fixed_clks		= fsys_fixed_rate_clks,
	.nr_fixed_clks		= ARRAY_SIZE(fsys_fixed_rate_clks),
	.pll_clks		= fsys_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(fsys_pll_clks),
	.gate_clks		= fsys_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(fsys_gate_clks),
	.nr_clk_ids		= FSYS_NR_CLK,
	.clk_regs		= fsys_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(fsys_clk_regs),
};

static void __init exynos7580_cmu_fsys_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &fsys_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_fsys, "samsung,exynos7580-cmu-fsys",
		exynos7580_cmu_fsys_init);

/* Register Offset definitions for CMU_G3D (0x11460000) */
#define G3D_PLL_LOCK			0x0000
#define G3D_PLL_CON0			0x0100
#define G3D_PLL_CON1			0x0104
#define G3D_PLL_FREQ_DET		0x010C
#define MUX_SEL_G3D0			0x0200
#define MUX_SEL_G3D1			0x0204
#define MUX_SEL_G3D2			0x0208
#define MUX_EN_G3D0				0x0300
#define MUX_EN_G3D1				0x0304
#define MUX_EN_G3D2				0x0308
#define MUX_STAT_G3D0			0x0400
#define MUX_STAT_G3D1			0x0404
#define MUX_STAT_G3D2			0x0408
#define DIV_G3D					0x0600
#define DIV_STAT_G3D			0x0700
#define EN_ACLK_G3D				0x0800
#define EN_PCLK_G3D				0x0900
#define CLKOUT_CMU_G3D			0x0C00
#define CLKOUT_CMU_G3D_DIV_STAT	0x0C04
#define EN_PDN_G3D				0x0D00

PNAME(mout_g3d_p) = {"g3d_pll", "aclk_g3d_400"};

static const unsigned long g3d_clk_regs[] __initconst = {
};

static const struct samsung_mux_clock g3d_mux_clks[] __initconst = {
	MUX(CLK_MOUT_G3D, "mout_g3d", mout_g3d_p, MUX_SEL_G3D2, 0, 1),
};

static const struct samsung_div_clock g3d_div_clks[] __initconst = {
	DIV(CLK_DOUT_ACLK_G3D, "dout_aclk_g3d_600", "mout_g3d", DIV_G3D, 0, 3),
	DIV(0, "dout_pclk_g3d_150", "dout_aclk_g3d_600", DIV_G3D, 4, 3),
};

static const struct samsung_gate_clock g3d_gate_clks[] __initconst = {
	GATE(CLK_G3D, "aclk_g3d", "dout_aclk_g3d_600", EN_ACLK_G3D, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_bus_d_g3d", "dout_aclk_g3d_600", EN_ACLK_G3D, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_async_ahb2apb_g3d", "dout_aclk_g3d_600", EN_ACLK_G3D, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_qe_g3d", "dout_aclk_g3d_600", EN_ACLK_G3D, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_ppmu_g3d", "dout_aclk_g3d_600", EN_ACLK_G3D, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_async_ahb2apb_g3d", "dout_pclk_g3d_150", EN_PCLK_G3D, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_pmu_g3d", "dout_pclk_g3d_150", EN_PCLK_G3D, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_sysreg_g3d", "dout_pclk_g3d_150", EN_PCLK_G3D, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_qe_g3d", "dout_pclk_g3d_150", EN_PCLK_G3D, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_ppmu_g3d", "dout_pclk_g3d_150", EN_PCLK_G3D, 7, CLK_IGNORE_UNUSED, 0),
};

struct samsung_pll_rate_table table_g3d[] = {
	/* rate		p,  m,  s,  k */
	{800000000U,    13, 400, 0, 0},
	{734000000U,    13, 367, 0, 0},
	{668000000U,	13, 334, 0, 0},
	{534000000U,	13, 267, 0, 0},
	{440000000U,	13, 440, 1, 0},
	{350000000U,    13, 350, 1, 0},
	{266000000U,	13, 266, 1, 0},
	{160000000U,	13, 320, 2, 0},
	{0,		0, 0, 0, 0},
};

static const struct samsung_pll_clock g3d_pll_clks[] __initconst = {
	PLL(pll_2551x, CLK_G3D_PLL, "g3d_pll", "fin_pll", G3D_PLL_LOCK, G3D_PLL_CON0, table_g3d),
};

static const struct samsung_cmu_info g3d_cmu_info __initconst = {
	.pll_clks		= g3d_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(g3d_pll_clks),
	.mux_clks		= g3d_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(g3d_mux_clks),
	.div_clks		= g3d_div_clks,
	.nr_div_clks		= ARRAY_SIZE(g3d_div_clks),
	.gate_clks		= g3d_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(g3d_gate_clks),
	.nr_clk_ids		= G3D_NR_CLK,
	.clk_regs		= g3d_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(g3d_clk_regs),
};

static void __init exynos7580_cmu_g3d_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &g3d_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_g3d, "samsung,exynos7580-cmu-g3d",
		exynos7580_cmu_g3d_init);

/* Register Offset definitions for CMU_IMEM (0x10C60000) */
#define EN_ACLK_IMEM				0x0800
#define EN_ACLK_IMEM_SEC_IRAMC_TOP	0x0804
#define EN_ACLK_IMEM_SEC_SSS		0x0808
#define EN_ACLK_IMEM_SEC_RTIC		0x080C
#define EN_PCLK_IMEM				0x0900
#define EN_PCLK_IMEM_SEC_SSS		0x0904
#define EN_PCLK_IMEM_SEC_RTIC		0x0908

static const unsigned long imem_clk_regs[] __initconst = {
	EN_ACLK_IMEM,
	EN_ACLK_IMEM_SEC_IRAMC_TOP,
	EN_ACLK_IMEM_SEC_SSS,
	EN_ACLK_IMEM_SEC_RTIC,
	EN_PCLK_IMEM,
	EN_PCLK_IMEM_SEC_SSS,
	EN_PCLK_IMEM_SEC_RTIC,
};

static const struct samsung_gate_clock imem_gate_clks[] __initconst = {
	GATE(0, "aclk_intc_cpu", "aclk_imem_200", EN_ACLK_IMEM, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_iramc_top", "aclk_imem_200", EN_ACLK_IMEM_SEC_IRAMC_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_downsizer_gic", "aclk_imem_200", EN_ACLK_IMEM, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_masync_xiu_d_to_p_imem", "aclk_imem_200", EN_ACLK_IMEM, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_sss", "aclk_imem_266", EN_ACLK_IMEM_SEC_SSS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_rtic", "aclk_imem_266", EN_ACLK_IMEM_SEC_RTIC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_xiu_d_imem", "aclk_imem_266", EN_ACLK_IMEM, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_bus_d_imem", "aclk_imem_266", EN_ACLK_IMEM, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_asyncahbm_sss_atlas", "aclk_imem_266", EN_ACLK_IMEM, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_ppmu_imem", "aclk_imem_266", EN_ACLK_IMEM, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_sasync_xiu_d_to_p_imem", "aclk_imem_266", EN_ACLK_IMEM, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_downsizer_async", "aclk_imem_266", EN_ACLK_IMEM, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_pmu_imem", "aclk_imem_200", EN_PCLK_IMEM, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_sysreg_imem", "aclk_imem_200", EN_PCLK_IMEM, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_sss", "aclk_imem_200", EN_PCLK_IMEM_SEC_SSS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_rtic", "aclk_imem_200", EN_PCLK_IMEM_SEC_RTIC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_ppmu_imem", "aclk_imem_200", EN_PCLK_IMEM, 2, 0, 0),
};

static const struct samsung_cmu_info imem_cmu_info __initconst = {
	.gate_clks		= imem_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(imem_gate_clks),
	.nr_clk_ids		= IMEM_NR_CLK,
	.clk_regs		= imem_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(imem_clk_regs),
};

static void __init exynos7580_cmu_imem_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &imem_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_imem, "samsung,exynos7580-cmu-imem",
		exynos7580_cmu_imem_init);

/* Register Offset definitions for CMU_ISP (0x14720000) */
#define ISP_PLL_LOCK			0x0000
#define ISP_PLL_CON0			0x0100
#define ISP_PLL_CON1			0x0104
#define ISP_PLL_FREQ_DET		0x010C
#define MUX_SEL_ISP0			0x0200
#define MUX_SEL_ISP1			0x0204
#define MUX_SEL_ISP2			0x0208
#define MUX_SEL_ISP3			0x020C
#define MUX_SEL_ISP4			0x0210
#define MUX_SEL_ISP5			0x0214
#define MUX_EN_ISP0				0x0300
#define MUX_EN_ISP1				0x0304
#define MUX_EN_ISP2				0x0308
#define MUX_EN_ISP3				0x030C
#define MUX_EN_ISP4				0x0310
#define MUX_EN_ISP5				0x0314
#define MUX_STAT_ISP0			0x0400
#define MUX_STAT_ISP1			0x0404
#define MUX_STAT_ISP2			0x0408
#define MUX_STAT_ISP3			0x040C
#define MUX_STAT_ISP4			0x0410
#define MUX_STAT_ISP5			0x0414
#define MUX_IGNORE_ISP2			0x0508
#define DIV_ISP0				0x0600
#define DIV_ISP1				0x0604
#define DIV_ISP2				0x0608
#define DIV_ISP3				0x060C
#define DIV_ISP4				0x0610
#define DIV_ISP5				0x0614
#define DIV_STAT_ISP0			0x0700
#define DIV_STAT_ISP1			0x0704
#define DIV_STAT_ISP2			0x0708
#define DIV_STAT_ISP3			0x070C
#define DIV_STAT_ISP4			0x0710
#define DIV_STAT_ISP5			0x0714
#define EN_ACLK_ISP_0			0x0800
#define EN_ACLK_ISP_1			0x0804
#define EN_ACLK_ISP_2			0x0808
#define EN_PCLK_ISP_0			0x0900
#define EN_PCLK_ISP_1			0x0904
#define EN_SCLK_ISP				0x0A00
#define CLKOUT_CMU_ISP			0x0C00
#define CLKOUT_CMU_ISP_DIV_STAT	0x0C04

static const struct samsung_fixed_rate_clock isp_fixed_rate_clks[] __initconst = {
	FRATE(0, "phyclk_csi_phy0_rxbyteclkhs0", NULL, 0, 188000000),
	FRATE(0, "phyclk_csi_phy1_rxbyteclkhs0", NULL, 0, 188000000),
};

PNAME(mout_sclk_cpu_isp_clkin_a_p) = {"dout_isp_pll_div2", "mout_aclk_isp_400_user"};
PNAME(mout_sclk_cpu_isp_clkin_b_p) = {"mout_sclk_cpu_isp_clkin_a", "mout_aclk_isp_333_user"};
PNAME(mout_aclk_csi_link1_75_p) = {"dout_isp_pll_div3", "mout_aclk_isp_333_user"};
PNAME(mout_aclk_csi_link1_75_b_p) = {"mout_aclk_csi_link1_75", "mout_aclk_isp_400_user"};
PNAME(mout_aclk_fimc_isp_450_a_p) = {"dout_isp_pll_div2", "mout_aclk_isp_400_user"};
PNAME(mout_aclk_fimc_isp_450_b_p) = {"mout_aclk_fimc_isp_450_a", "mout_aclk_isp_333_user"};
PNAME(mout_aclk_fimc_isp_450_c_p) = {"mout_aclk_fimc_isp_450_b", "dout_isp_pll_div3"};
PNAME(mout_aclk_fimc_isp_450_d_p) = {"mout_aclk_fimc_isp_450_c", "mout_aclk_isp_266_user"};
PNAME(mout_aclk_fimc_fd_300_p) = {"dout_isp_pll_div3", "mout_aclk_isp_266_user"};
PNAME(mout_aclk_link_data_a_p) = {"dout_isp_pll_div2", "mout_aclk_isp_400_user"};
PNAME(mout_aclk_link_data_b_p) = {"mout_aclk_link_data_a", "mout_aclk_isp_333_user"};
PNAME(mout_aclk_link_data_c_p) = {"mout_aclk_link_data_b", "mout_aclk_isp_266_user"};

static const unsigned long isp_clk_regs[] __initconst = {
};

static const struct samsung_mux_clock isp_mux_clks[] __initconst = {
	MUX(CLK_MOUT_SCLK_CPU_ISP_CLKIN_A, "mout_sclk_cpu_isp_clkin_a", mout_sclk_cpu_isp_clkin_a_p, MUX_SEL_ISP3, 0, 1),
	MUX(CLK_MOUT_SCLK_CPU_ISP_CLKIN_B, "mout_sclk_cpu_isp_clkin_b", mout_sclk_cpu_isp_clkin_b_p, MUX_SEL_ISP3, 4, 1),
	MUX(CLK_MOUT_ACLK_CSI_LINK1_75, "mout_aclk_csi_link1_75", mout_aclk_csi_link1_75_p, MUX_SEL_ISP3, 8, 1),
	MUX(CLK_MOUT_ACLK_CSI_LINK1_75_B, "mout_aclk_csi_link1_75_b", mout_aclk_csi_link1_75_b_p, MUX_SEL_ISP3, 12, 1),
	MUX(CLK_MOUT_ACLK_FIMC_ISP_450_A, "mout_aclk_fimc_isp_450_a", mout_aclk_fimc_isp_450_a_p, MUX_SEL_ISP4, 0, 1),
	MUX(CLK_MOUT_ACLK_FIMC_ISP_450_B, "mout_aclk_fimc_isp_450_b", mout_aclk_fimc_isp_450_b_p, MUX_SEL_ISP4, 4, 1),
	MUX(CLK_MOUT_ACLK_FIMC_ISP_450_C, "mout_aclk_fimc_isp_450_c", mout_aclk_fimc_isp_450_c_p, MUX_SEL_ISP4, 8, 1),
	MUX(CLK_MOUT_ACLK_FIMC_ISP_450_D, "mout_aclk_fimc_isp_450_d", mout_aclk_fimc_isp_450_d_p, MUX_SEL_ISP4, 12, 1),
	MUX(CLK_MOUT_ACLK_FIMC_FD_300, "mout_aclk_fimc_fd_300", mout_aclk_fimc_fd_300_p, MUX_SEL_ISP4, 16, 1),
	MUX(CLK_MOUT_ACLK_LINK_DATA_A, "mout_aclk_link_data_a", mout_aclk_link_data_a_p, MUX_SEL_ISP5, 0, 1),
	MUX(CLK_MOUT_ACLK_LINK_DATA_B, "mout_aclk_link_data_b", mout_aclk_link_data_b_p, MUX_SEL_ISP5, 4, 1),
	MUX(CLK_MOUT_ACLK_LINK_DATA_C, "mout_aclk_link_data_c", mout_aclk_link_data_c_p, MUX_SEL_ISP5, 8, 1),
};

static const struct samsung_div_clock isp_div_clks[] __initconst = {
	DIV(CLK_DOUT_ISP_PLL_DIV2, "dout_isp_pll_div2", "isp_pll", DIV_ISP0, 0, 2),
	DIV(CLK_DOUT_ISP_PLL_DIV3, "dout_isp_pll_div3", "isp_pll", DIV_ISP0, 4, 2),
	DIV(CLK_DOUT_SCLK_CPU_ISP_CLKIN, "dout_sclk_cpu_isp_clkin", "mout_sclk_cpu_isp_clkin_b", DIV_ISP1, 0, 3),
	DIV(CLK_DOUT_SCLK_CPU_ISP_ATCLKIN, "dout_sclk_cpu_isp_atclkin", "dout_sclk_cpu_isp_clkin", DIV_ISP1, 4, 3),
	DIV(CLK_DOUT_SCLK_CPU_ISP_PCLKDBG, "dout_sclk_cpu_isp_pclkdbg", "dout_sclk_cpu_isp_clkin", DIV_ISP1, 8, 3),
	DIV(CLK_DOUT_PCLK_CSI_LINK0_225, "dout_pclk_csi_link0_225", "dout_isp_pll_div2", DIV_ISP2, 0, 2),
	DIV(CLK_DOUT_ACLK_CSI_LINK1_75, "dout_aclk_csi_link1_75", "mout_aclk_csi_link1_75_b", DIV_ISP2, 4, 3),
	DIV(CLK_DOUT_PCLK_CSI_LINK1_37, "dout_pclk_csi_link1_37", "dout_aclk_csi_link1_75", DIV_ISP2, 8, 2),
	DIV(CLK_DOUT_ACLK_FIMC_ISP_450, "dout_aclk_fimc_isp_450", "mout_aclk_fimc_isp_450_d", DIV_ISP3, 0, 3),
	DIV(CLK_DOUT_PCLK_FIMC_ISP_225, "dout_pclk_fimc_isp_225", "dout_aclk_fimc_isp_450", DIV_ISP3, 4, 3),
	DIV(CLK_DOUT_ACLK_FIMC_FD_300, "dout_aclk_fimc_fd_300", "mout_aclk_fimc_fd_300", DIV_ISP3, 8, 3),
	DIV(CLK_DOUT_PCLK_FIMC_FD_150, "dout_pclk_fimc_fd_150", "dout_aclk_fimc_fd_300", DIV_ISP3, 12, 2),
	DIV(CLK_DOUT_ACLK_ISP_266, "dout_aclk_isp_266", "mout_aclk_isp_266_user", DIV_ISP4, 0, 3),
	DIV(CLK_DOUT_ACLK_ISP_133, "dout_aclk_isp_133", "dout_aclk_isp_266", DIV_ISP4, 4, 2),
	DIV(CLK_DOUT_ACLK_ISP_67, "dout_aclk_isp_67", "dout_aclk_isp_266", DIV_ISP4, 8, 2),
	DIV(CLK_DOUT_ACLK_LINK_DATA, "dout_aclk_link_data", "mout_aclk_link_data_c", DIV_ISP5, 0, 3),
};

static const struct samsung_gate_clock isp_gate_clks[] __initconst = {
	GATE(CLK_ACLK_CSI_LINK0_1_WRAP_CLK, "aclk_csi_link0_1_wrap_clk", "dout_isp_pll_div2", EN_ACLK_ISP_0, 0, 0, 0),
	GATE(CLK_ACLK_FIMC_BNS_ACLK, "aclk_fimc_bns_aclk", "dout_isp_pll_div2", EN_ACLK_ISP_0, 3, 0, 0),
	GATE(CLK_ACLK_CSI_LINK1_1_WRAP_CLK, "aclk_csi_link1_1_wrap_clk", "dout_aclk_csi_link1_75", EN_ACLK_ISP_0, 1, 0, 0),
	GATE(CLK_ACLK_FIMC_BNS_L_ACLK, "aclk_fimc_bns_l_aclk", "dout_aclk_csi_link1_75", EN_ACLK_ISP_0, 4, 0, 0),
	GATE(CLK_ACLK_ISP_1, "aclk_isp_1", "dout_aclk_fimc_isp_450", EN_ACLK_ISP_1, 26, 0, 0),
	GATE(CLK_ACLK_SASYNC_FIMC_BNS_M_TO_BUS, "aclk_sasync_fimc_bns_m_to_bus", "dout_isp_pll_div2", EN_ACLK_ISP_1, 27, 0, 0),
	GATE(CLK_ACLK_MASYNC_FIMC_BNS_M_TO_BUS, "aclk_masync_fimc_bns_m_to_bus", "dout_aclk_link_data", EN_ACLK_ISP_1, 28, 0, 0),
	GATE(CLK_ACLK_SASYNC_FIMC_BNS_L_TO_BUS, "aclk_sasync_fimc_bns_l_to_bus", "dout_aclk_csi_link1_75", EN_ACLK_ISP_1, 9, 0, 0),
	GATE(CLK_ACLK_MASYNC_FIMC_BNS_L_TO_BUS, "aclk_masync_fimc_bns_l_to_bus", "dout_aclk_link_data", EN_ACLK_ISP_1, 2, 0, 0),
	MUX_GATE(CLK_MOUT_ACLK_ISP_400_USER, "mout_aclk_isp_400_user", "aclk_isp_400", MUX_SEL_ISP1, 0, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_ACLK_ISP_333_USER, "mout_aclk_isp_333_user", "aclk_isp_333", MUX_SEL_ISP1, 4, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_ACLK_ISP_266_USER, "mout_aclk_isp_266_user", "aclk_isp_266", MUX_SEL_ISP1, 8, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_SCLK_SPI0_ISP_EXT_CLK_USER, "mout_sclk_spi0_isp_ext_clk_user", "sclk_isp_spi0_ext_clk", MUX_SEL_ISP1, 12, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_SCLK_SPI1_ISP_EXT_CLK_USER, "mout_sclk_spi1_isp_ext_clk_user", "sclk_isp_spi1_ext_clk", MUX_SEL_ISP1, 16, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_SCLK_UART_ISP_EXT_CLK_USER, "mout_sclk_uart_isp_ext_clk_user", "sclk_isp_uart_ext_clk", MUX_SEL_ISP1, 20, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_PHYCLK_CSI_LINK0_RX_USER, "mout_phyclk_csi_link0_rx_user", "phyclk_csi_phy0_rxbyteclkhs0", MUX_SEL_ISP2, 0, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_PHYCLK_CSI_LINK1_RX_USER, "mout_phyclk_csi_link1_rx_user", "phyclk_csi_phy1_rxbyteclkhs0", MUX_SEL_ISP2, 4, CLK_IGNORE_UNUSED, 0),
};

struct samsung_pll_rate_table table_isp[] = {
	/* rate		p,  m,  s,  k */
	{1060000000U,	13, 530, 0, 0},
	{860000000U,	13, 430, 0, 0},
	{800000000U,	13, 400, 0, 0},
	{400000000U,	13, 400, 1, 0},
	{0,		0, 0, 0, 0},
};

static const struct samsung_pll_clock isp_pll_clks[] __initconst = {
	PLL(pll_2551x, CLK_ISP_PLL, "isp_pll", "fin_pll", ISP_PLL_LOCK, ISP_PLL_CON0, table_isp),
};

static const struct samsung_cmu_info isp_cmu_info __initconst = {
	.fixed_clks		= isp_fixed_rate_clks,
	.nr_fixed_clks		= ARRAY_SIZE(isp_fixed_rate_clks),
	.pll_clks		= isp_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(isp_pll_clks),
	.mux_clks		= isp_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(isp_mux_clks),
	.gate_clks		= isp_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(isp_gate_clks),
	.div_clks		= isp_div_clks,
	.nr_div_clks		= ARRAY_SIZE(isp_div_clks),
	.nr_clk_ids		= ISP_NR_CLK,
	.clk_regs		= isp_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(isp_clk_regs),
};

static void __init exynos7580_cmu_isp_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &isp_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_isp, "samsung,exynos7580-cmu-isp",
		exynos7580_cmu_isp_init);

/* Register Offset definitions for CMU_MFCMSCL (0x12CB0000) */
#define MUX_SEL_MFCMSCL		0x0200
#define MUX_EN_MFCMSCL		0x0300
#define MUX_STAT_MFCMSCL	0x0400
#define DIV_MFCMSCL			0x0600
#define DIV_STAT_MFCMSCL	0x0700
#define EN_ACLK_MFCMSCL		0x0800
#define EN_PCLK_MFCMSCL		0x0900
#define EN_PDN_MFCMSCL		0x0D00


static const unsigned long mfcmscl_clk_regs[] __initconst = {
};

static const struct samsung_div_clock  mfcmscl_div_clks[] __initconst = {
	DIV(0, "dout_pclk_mfcmscl_100", "mout_aclk_mscl_400_user", DIV_MFCMSCL, 0, 2),
};

static const struct samsung_gate_clock mfcmscl_gate_clks[] __initconst = {
	GATE(CLK_GATE_ACLK_M2M_SCALER0, "aclk_m2m_scaler0", "aclk_xiu_d_mscl", EN_ACLK_MFCMSCL, 0, 0, 0),
	GATE(CLK_GATE_ACLK_M2M_SCALER1, "aclk_m2m_scaler1", "aclk_xiu_d_mscl", EN_ACLK_MFCMSCL, 1, 0, 0),
	GATE(CLK_ACLK_JPEG, "aclk_jpeg", "aclk_masync_ahb_jpeg", EN_ACLK_MFCMSCL, 2, 0, 0),
	GATE(0, "aclk_xiu_d_mscl", "aclk_bus_d_mscl", EN_ACLK_MFCMSCL, 3, 0, 0),
	GATE(0, "aclk_bus_d_mscl", "mout_aclk_mscl_400_user", EN_ACLK_MFCMSCL, 4, 0, 0),
	GATE(0, "aclk_masync_ahb_jpeg", "aclk_upsizer_xiu_d_mscl", EN_ACLK_MFCMSCL, 5, 0, 0),
	GATE(0, "aclk_upsizer_xiu_d_mscl", "aclk_xiu_d_mscl", EN_ACLK_MFCMSCL, 6, 0, 0),
	GATE(CLK_ACLK_SMMU_MSCL_MMU, "aclk_smmu_mscl_mmu", "mout_aclk_mscl_400_user", EN_ACLK_MFCMSCL, 7, 0, 0),
	GATE(0, "aclk_ppmu_mscl", "mout_aclk_mscl_400_user", EN_ACLK_MFCMSCL, 8, 0, 0),
	GATE(CLK_ACLK_AHB2APB_MFC, "aclk_ahb2apb_mfc", "dout_pclk_mfcmscl_100", EN_ACLK_MFCMSCL, 9, 0, 0),
	GATE(0, "aclk_sasync_ahb_jpeg", "dout_pclk_mfcmscl_100", EN_ACLK_MFCMSCL, 12, 0, 0),
	GATE(CLK_PCLK_AHB_JPEG, "pclk_ahb_jpeg", "aclk_sasync_ahb_jpeg", EN_PCLK_MFCMSCL, 0, 0, 0),
	GATE(CLK_GATE_PCLK_M2M_SCALER0, "pclk_m2m_scaler0", "dout_pclk_mfcmscl_100", EN_PCLK_MFCMSCL, 1, 0, 0),
	GATE(CLK_GATE_PCLK_M2M_SCALER1, "pclk_m2m_scaler1", "dout_pclk_mfcmscl_100", EN_PCLK_MFCMSCL, 2, 0, 0),
	GATE(CLK_PCLK_SMMU_MSCL_MMU, "pclk_smmu_mscl_mmu", "dout_pclk_mfcmscl_100", EN_PCLK_MFCMSCL, 5, 0, 0),
	GATE(0, "pclk_ppmu_mscl", "dout_pclk_mfcmscl_100", EN_PCLK_MFCMSCL, 6, 0, 0),
	GATE(CLK_PCLK_SMMU_MFC_MMU, "pclk_smmu_mfc_mmu", "dout_pclk_mfcmscl_100", EN_PCLK_MFCMSCL, 17, 0, 0),
	GATE(0, "pclk_ppmu_mfc", "dout_pclk_mfcmscl_100", EN_PCLK_MFCMSCL, 18, 0, 0),
	GATE(CLK_PCLK_SASYNC_AHB2APB_MFC, "pclk_sasync_ahb2apb_mfc", "dout_pclk_mfcmscl_100", EN_PCLK_MFCMSCL, 19, 0, 0),
	GATE(CLK_PCLK_MFC, "pclk_mfc", "mout_aclk_mfc_266_user", EN_PCLK_MFCMSCL, 16, 0, 0),
	GATE(CLK_ACLK_MFC, "aclk_mfc", "mout_aclk_mfc_266_user", EN_ACLK_MFCMSCL, 16, 0, 0),
	GATE(CLK_ACLK_BUS_D_MFC, "aclk_bus_d_mfc", "mout_aclk_mfc_266_user", EN_ACLK_MFCMSCL, 17, 0, 0),
	GATE(CLK_ACLK_UPSIZER_MFC, "aclk_upsizer_mfc", "mout_aclk_mfc_266_user", EN_ACLK_MFCMSCL, 18, 0, 0),
	GATE(CLK_ACLK_SMMU_MFC_MMU, "aclk_smmu_mfc_mmu", "mout_aclk_mfc_266_user", EN_ACLK_MFCMSCL, 19, 0, 0),
	GATE(0, "aclk_ppmu_mfc", "mout_aclk_mfc_266_user", EN_ACLK_MFCMSCL, 20, 0, 0),
	GATE(CLK_ACLK_SASYNC_AHB2APB_MFC, "aclk_sasync_ahb2apb_mfc", "mout_aclk_mfc_266_user", EN_ACLK_MFCMSCL, 13, 0, 0),
	MUX_GATE(CLK_MOUT_ACLK_MSCL_400_USER, "mout_aclk_mscl_400_user", "aclk_mfcmscl_400", MUX_SEL_MFCMSCL, 4, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MOUT_ACLK_MFC_266_USER, "mout_aclk_mfc_266_user", "aclk_mfcmscl_266", MUX_SEL_MFCMSCL, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info mfcmscl_cmu_info __initconst = {
	.gate_clks		= mfcmscl_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(mfcmscl_gate_clks),
	.div_clks		= mfcmscl_div_clks,
	.nr_div_clks		= ARRAY_SIZE(mfcmscl_div_clks),
	.nr_clk_ids		= MFCMSCL_NR_CLK,
	.clk_regs		= mfcmscl_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(mfcmscl_clk_regs),
};

static void __init exynos7580_cmu_mfcmscl_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &mfcmscl_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_mfcmscl, "samsung,exynos7580-cmu-mfcmscl",
		exynos7580_cmu_mfcmscl_init);

/* Register Offset definitions for CMU_MIF (0x10430000) */
#define MEM0_PLL_LOCK			0x0000
#define MEDIA_PLL_LOCK			0x0008
#define BUS_PLL_LOCK			0x000C
#define MEM0_PLL_CON0			0x0100
#define MEM0_PLL_CON1			0x0104
#define MEM0_PLL_FREQ_DET		0x010C
#define MEDIA_PLL_CON0			0x0120
#define MEDIA_PLL_CON1			0x0124
#define MEDIA_PLL_FREQ_DET		0x012C
#define BUS_PLL_CON0			0x0130
#define BUS_PLL_CON1			0x0134
#define BUS_PLL_FREQ_DET		0x013C
#define MUX_SEL_MIF0			0x0200
#define MUX_SEL_MIF1			0x0204
#define MUX_SEL_MIF2			0x0208
#define MUX_SEL_MIF3			0x020C
#define MUX_SEL_MIF4			0x0210
#define MUX_SEL_MIF5			0x0214
#define MUX_SEL_TOP_DISP		0x0218
#define MUX_EN_MIF0				0x0300
#define MUX_EN_MIF1				0x0304
#define MUX_EN_MIF2				0x0308
#define MUX_EN_MIF3				0x030C
#define MUX_EN_MIF4				0x0310
#define MUX_EN_MIF5				0x0314
#define MUX_EN_TOP_DISP			0x0318
#define MUX_STAT_MIF0			0x0400
#define MUX_STAT_MIF1			0x0404
#define MUX_STAT_MIF2			0x0408
#define MUX_STAT_MIF3			0x040C
#define MUX_STAT_MIF4			0x0410
#define MUX_STAT_MIF5			0x0414
#define MUX_STAT_TOP_DISP		0x0418
#define MUX_STAT_CPMUX			0x0420
#define DIV_MIF0				0x0600
#define DIV_MIF1				0x0604
#define DIV_TOP_DISP			0x0610
#define DIV_STAT_MIF0			0x0700
#define DIV_STAT_MIF1			0x0704
#define DIV_STAT_TOP_DISP		0x0710
#define EN_ACLK_MIF0			0x0800
#define EN_ACLK_MIF1			0x0804
#define EN_ACLK_MIF2			0x0808
#define EN_ACLK_TOP_DISP		0x0810
#define EN_PCLK_MIF				0x0900
#define EN_PCLK_MIF_SEC_DREX0_TZ	0x0904
#define EN_SCLK_MIF0			0x0A00
#define EN_SCLK_TOP				0x0A10
#define EN_SCLK_TOP_DISP		0x0A14
#define CLKOUT_CMU_MIF			0x0C00
#define CLKOUT_CMU_MIF_DIV_STAT	0x0C04
#define CLK_EN_PDN_MIF			0x0D00
#define AUTO_ROOT_CLK_GATE_MIF	0x0E00
#define DREX_FREQ_CTRL0			0x1000
#define DREX_FREQ_CTRL1			0x1004
#define PAUSE					0x1008
#define DDRPHY_LOCK_CTRL		0x100C
#define AGC_EN					0x1010

PNAME(mout_mem0_pll_div2_p) = {"mem0_pll", "ffac_mif_mem0_pll_div2"};
PNAME(mout_media_pll_div2_p) = {"media_pll", "ffac_mif_media_pll_div2"};
PNAME(mout_mem0_media_pll_div2_p) = {"mout_mem0_pll_div2", "mout_media_pll_div2"};
PNAME(mout_bus_media_pll_p) = {"bus_pll", "mout_media_pll_div2"};
PNAME(mout_sclk_disp_decon_int_eclk_a_p) = {"fin_pll", "bus_pll"};
PNAME(mout_sclk_disp_decon_int_eclk_b_p) = {"mout_sclk_disp_decon_int_eclk_a",
					    "mout_media_pll_div2"};
PNAME(mout_sclk_disp_decon_int_vclk_a_p) = {"fin_pll", "mout_media_pll_div2"};
PNAME(mout_sclk_disp_decon_int_vclk_b_p) = {"mout_sclk_disp_decon_int_vclk_a",
					    "bus_pll"};

static const unsigned long mif_clk_regs[] __initconst = {
	DIV_MIF0,
	DIV_MIF1,
	MUX_SEL_MIF3,
	MUX_SEL_MIF4,
	MUX_SEL_MIF5,
	DIV_TOP_DISP,
	MUX_SEL_TOP_DISP,
	EN_SCLK_TOP,
	MEDIA_PLL_CON0,
	BUS_PLL_CON0,
};

static const struct samsung_mux_clock mif_mux_clks[] __initconst = {
	MUX(0, "mout_mem0_pll_div2", mout_mem0_pll_div2_p, MUX_SEL_MIF3, 16, 1),
	MUX(0, "mout_media_pll_div2", mout_media_pll_div2_p, MUX_SEL_MIF3, 24, 1),
	MUX(0, "mout_clkm_phy_b", mout_mem0_media_pll_div2_p, MUX_SEL_MIF4, 4, 1),
	MUX(0, "mout_clk2x_phy_b", mout_mem0_media_pll_div2_p, MUX_SEL_MIF4, 20, 1),
	MUX(0, "mout_aclk_mif_400", mout_bus_media_pll_p, MUX_SEL_MIF5, 0, 1),
	MUX(0, "mout_aclk_mif_100", mout_bus_media_pll_p, MUX_SEL_MIF5, 4, 1),
	MUX(0, "mout_aclk_mif_fix_100", mout_bus_media_pll_p, MUX_SEL_MIF5, 8, 1),
	MUX(CLK_MUX_ACLK_DISP_200, "mout_aclk_disp_200", mout_bus_media_pll_p, MUX_SEL_TOP_DISP, 0, 1),
	MUX(CLK_MUX_SCLK_DISP_DECON_INT_ECLK_A, "mout_sclk_disp_decon_int_eclk_a", mout_sclk_disp_decon_int_eclk_a_p, MUX_SEL_TOP_DISP, 8, 1),
	MUX(CLK_MUX_SCLK_DISP_DECON_INT_ECLK_B, "mout_sclk_disp_decon_int_eclk_b", mout_sclk_disp_decon_int_eclk_b_p, MUX_SEL_TOP_DISP, 12, 1),
	MUX(CLK_MUX_SCLK_DISP_DECON_INT_VCLK_A, "mout_sclk_disp_decon_int_vclk_a", mout_sclk_disp_decon_int_vclk_a_p, MUX_SEL_TOP_DISP, 16, 1),
	MUX(CLK_MUX_SCLK_DISP_DECON_INT_VCLK_B, "mout_sclk_disp_decon_int_vclk_b", mout_sclk_disp_decon_int_vclk_b_p, MUX_SEL_TOP_DISP, 20, 1),
};

static const struct samsung_div_clock mif_div_clks[] __initconst = {
	DIV(0, "dout_clkm_phy", "mout_clkm_phy_b", DIV_MIF0, 0, 4),
	DIV(0, "dout_clk2x_phy", "mout_clk2x_phy_b", DIV_MIF0, 4, 4),
	DIV(0, "dout_aclk_mif_400", "mout_aclk_mif_400", DIV_MIF1, 0, 3),
	DIV(0, "dout_aclk_mif_200", "dout_aclk_mif_400", DIV_MIF1, 4, 2),
	DIV(0, "dout_aclk_mif_100", "mout_aclk_mif_100", DIV_MIF1, 8, 4),
	DIV(0, "dout_aclk_mif_fix_100", "mout_aclk_mif_fix_100", DIV_MIF1, 12, 3),
	DIV(CLK_DIV_ACLK_DISP_200, "dout_aclk_disp_200", "mout_aclk_disp_200", DIV_TOP_DISP, 0, 4),
	DIV(CLK_DIV_SCLK_DISP_DECON_INT_ECLK, "dout_sclk_disp_decon_int_eclk", "mout_sclk_disp_decon_int_eclk_b", DIV_TOP_DISP, 16, 4),
	DIV(CLK_DIV_SCLK_DISP_DECON_INT_VCLK, "dout_sclk_disp_decon_int_vclk", "mout_sclk_disp_decon_int_vclk_b", DIV_TOP_DISP, 20, 4),
};

static const struct samsung_gate_clock mif_gate_clks[] __initconst = {
	GATE(CLK_SCLK_MEDIA_PLL_TOP, "sclk_media_pll_top", "mout_media_pll_div2", EN_SCLK_TOP, 3, 0, 0),
	GATE(CLK_ACLK_DISP_200, "aclk_disp_200", "dout_aclk_disp_200", EN_ACLK_TOP_DISP, 0, 0, 0),
	GATE(CLK_SCLK_DISP_DECON_INT_ECLK, "sclk_disp_decon_int_eclk", "dout_sclk_disp_decon_int_eclk", EN_SCLK_TOP_DISP, 0, 0, 0),
	GATE(CLK_SCLK_DISP_DECON_INT_VCLK, "sclk_disp_decon_int_vclk", "dout_sclk_disp_decon_int_vclk", EN_SCLK_TOP_DISP, 1, 0, 0),
	GATE(CLK_HSI2C3, "pclk_hsi2c3", "dout_aclk_mif_fix_100", EN_PCLK_MIF, 27, CLK_IGNORE_UNUSED, 0),
};

struct samsung_pll_rate_table table_bus[] = {
	/* rate		p,  m,  s,  k */
	{800000000U,	13, 400, 0, 0},
	{0,		0, 0, 0, 0},
};

struct samsung_pll_rate_table table_media[] = {
	/* rate		p,  m,  s,  k */
	{1650000000U,	13, 825, 0, 0},
	{1334000000U,	13, 667, 0, 0},
	{910000000U,	4, 280, 1, 0},
	{832000000U,	4, 256, 1, 0},
	{825000000U,	13, 825, 1, 0},
	{741000000U,	4, 228, 1, 0},
	{728000000U,	4, 224, 1, 0},
	{715000000U,	4, 220, 1, 0},
	{689000000U,	4, 212, 1, 0},
	{676000000U,	4, 208, 1, 0},
	{667000000U,	13, 667, 1, 0},
	{663000000U,	4, 204, 1, 0},
	{559000000U,	4, 344, 2, 0},
	{546000000U,	4, 336, 2, 0},
	{416000000U,	4, 256, 2, 0},
	{338000000U,	4, 208, 2, 0},
	{325000000U,	4, 200, 2, 0},
	{273000000U,	4, 336, 3, 0},
	{247000000U,	4, 304, 3, 0},
	{200000000U,	13, 800, 3, 0},
	{0,		0, 0, 0, 0},
};

struct samsung_pll_rate_table table_mem0[] = {
	/* rate		p,  m,  s,  k */
	{1650000000U,	13, 825, 0, 0},
	{1334000000U,	13, 667, 0, 0},
	{910000000U,	4, 280, 1, 0},
	{832000000U,	4, 256, 1, 0},
	{825000000U,	13, 825, 1, 0},
	{741000000U,	4, 228, 1, 0},
	{728000000U,	4, 224, 1, 0},
	{715000000U,	4, 220, 1, 0},
	{689000000U,	4, 212, 1, 0},
	{676000000U,	4, 208, 1, 0},
	{667000000U,	13, 667, 1, 0},
	{663000000U,	4, 204, 1, 0},
	{559000000U,	4, 344, 2, 0},
	{546000000U,	4, 336, 2, 0},
	{416000000U,	4, 256, 2, 0},
	{338000000U,	4, 208, 2, 0},
	{325000000U,	4, 200, 2, 0},
	{273000000U,	4, 336, 3, 0},
	{247000000U,	4, 304, 3, 0},
	{200000000U,	13, 800, 3, 0},
	{0,		0, 0, 0, 0},
};

static const struct samsung_pll_clock mif_pll_clks[] __initconst = {
	PLL(pll_2555x, CLK_MEM0_PLL, "mem0_pll", "fin_pll", MEM0_PLL_LOCK, MEM0_PLL_CON0, table_mem0),
	PLL(pll_2555x, 0, "media_pll", "fin_pll", MEDIA_PLL_LOCK, MEDIA_PLL_CON0, table_media),
	PLL(pll_2551x, CLK_BUS_PLL, "bus_pll", "fin_pll", BUS_PLL_LOCK, BUS_PLL_CON0, table_bus),
};

static const struct samsung_fixed_factor_clock mif_fixed_factor_clks[] __initconst = {
	FFACTOR(0, "ffac_mif_mem0_pll_div2", "mem0_pll", 1, 2, 0),
	FFACTOR(0, "ffac_mif_media_pll_div2", "media_pll", 1, 2, 0),
};

static const struct samsung_cmu_info mif_cmu_info __initconst = {
	.pll_clks		= mif_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(mif_pll_clks),
	.mux_clks		= mif_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(mif_mux_clks),
	.gate_clks		= mif_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(mif_gate_clks),
	.div_clks		= mif_div_clks,
	.nr_div_clks		= ARRAY_SIZE(mif_div_clks),
	.fixed_factor_clks	= mif_fixed_factor_clks,
	.nr_fixed_factor_clks	= ARRAY_SIZE(mif_fixed_factor_clks),
	.nr_clk_ids		= MIF_NR_CLK,
	.clk_regs		= mif_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(mif_clk_regs),
};

static void __init exynos7580_cmu_mif_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &mif_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_mif, "samsung,exynos7580-cmu-mif",
		exynos7580_cmu_mif_init);

/* Register Offset definitions for CMU_TOP (0x10010000) */
#define AUD_PLL_LOCK			0x0000
#define AUD_PLL_CON0			0x0100
#define AUD_PLL_CON1			0x0104
#define AUD_PLL_CON2			0x0108
#define AUD_PLL_FREQ_DET		0x010C
#define MUX_SEL_TOP0			0x0200
#define MUX_SEL_TOP1			0x0204
#define MUX_SEL_TOP2			0x0208
#define MUX_SEL_TOP_FSYS0		0x0220
#define MUX_SEL_TOP_FSYS1		0x0224
#define MUX_SEL_TOP_FSYS2		0x0228
#define MUX_SEL_TOP_PERI		0x0230
#define MUX_SEL_TOP_ISP			0x0234
#define MUX_EN_TOP0				0x0300
#define MUX_EN_TOP1				0x0304
#define MUX_EN_TOP2				0x0308
#define MUX_EN_TOP_FSYS0		0x0320
#define MUX_EN_TOP_FSYS1		0x0324
#define MUX_EN_TOP_FSYS2		0x0328
#define MUX_EN_TOP_PERI			0x0330
#define MUX_EN_TOP_ISP			0x0334
#define MUX_STAT_TOP0			0x0400
#define MUX_STAT_TOP1			0x0404
#define MUX_STAT_TOP2			0x0408
#define MUX_STAT_TOP_FSYS0		0x0420
#define MUX_STAT_TOP_FSYS1		0x0424
#define MUX_STAT_TOP_FSYS2		0x0428
#define MUX_STAT_TOP_PERI		0x0430
#define MUX_STAT_TOP_ISP		0x0434
#define DIV_TOP0				0x0600
#define DIV_TOP1				0x0604
#define DIV_TOP_FSYS0			0x0620
#define DIV_TOP_FSYS1			0x0624
#define DIV_TOP_FSYS2			0x0628
#define DIV_TOP_PERI0			0x0630
#define DIV_TOP_PERI1			0x0634
#define DIV_TOP_PERI2			0x0638
#define DIV_TOP_PERI3			0x063C
#define DIV_TOP_ISP0			0x0640
#define DIV_TOP_ISP1			0x0644
#define DIV_STAT_TOP0			0x0700
#define DIV_STAT_TOP1			0x0704
#define DIV_STAT_TOP_FSYS0		0x0720
#define DIV_STAT_TOP_FSYS1		0x0724
#define DIV_STAT_TOP_FSYS2		0x0728
#define DIV_STAT_TOP_PERI0		0x0730
#define DIV_STAT_TOP_PERI1		0x0734
#define DIV_STAT_TOP_PERI2		0x0738
#define DIV_STAT_TOP_PERI3		0x073C
#define DIV_STAT_TOP_ISP0		0x0740
#define DIV_STAT_TOP_ISP1		0x0744
#define EN_ACLK_TOP				0x0800
#define EN_SCLK_TOP_FSYS		0x0A00
#define EN_SCLK_TOP_PERI		0x0A04
#define EN_SCLK_TOP_ISP			0x0A0C
#define CLKOUT_CMU_TOP			0x0C00
#define CLKOUT_CMU_TOP_DIV_STAT	0x0C04
#define EN_PDN_TOP				0x0D00

PNAME(mout_bus_media_pll_top_user_p) = {"mout_bus_pll_top_user",
					"mout_media_pll_top_user"};
PNAME(mout_media_bus_pll_top_user_p) = {"mout_media_pll_top_user",
					"mout_bus_pll_top_user"};
PNAME(mout_bus_pll_top_user_p) = {"fin_pll", "mout_bus_pll_top_user"};
PNAME(mout_sclk_fsys_mmc0_b_p) = {"mout_sclk_fsys_mmc0_a", "mout_media_pll_top_user"};
PNAME(mout_sclk_fsys_mmc1_b_p) = {"mout_sclk_fsys_mmc1_a", "mout_media_pll_top_user"};
PNAME(mout_sclk_fsys_mmc2_b_p) = {"mout_sclk_fsys_mmc2_a", "mout_media_pll_top_user"};

static const unsigned long top_clk_regs[] __initconst = {
	EN_SCLK_TOP_FSYS,
	EN_SCLK_TOP_PERI,
	DIV_TOP_FSYS0,
	DIV_TOP_FSYS1,
	DIV_TOP_FSYS2,
	DIV_TOP_PERI0,
	DIV_TOP_PERI1,
	DIV_TOP_PERI2,
	DIV_TOP_PERI3,
	MUX_SEL_TOP_FSYS0,
	MUX_SEL_TOP_FSYS1,
	MUX_SEL_TOP_FSYS2,
	MUX_SEL_TOP_PERI,
	EN_ACLK_TOP,
	DIV_TOP0,
	MUX_SEL_TOP1,
	MUX_SEL_TOP2,
	AUD_PLL_CON1,
	AUD_PLL_CON0,
	MUX_SEL_TOP0,
};

static const struct samsung_mux_clock top_mux_clks[] __initconst = {
	MUX(0, "mout_aclk_bus1_400", mout_bus_media_pll_top_user_p, MUX_SEL_TOP2, 16, 1),
	MUX(0, "mout_aclk_bus0_400", mout_bus_media_pll_top_user_p, MUX_SEL_TOP2, 12, 1),
	MUX(0, "mout_aclk_bus2_400", mout_media_bus_pll_top_user_p, MUX_SEL_TOP2, 20, 1),
	MUX(0, "mout_aclk_isp_266", mout_bus_media_pll_top_user_p, MUX_SEL_TOP2, 0, 1),
	MUX(0, "mout_aclk_mfcmscl_266", mout_bus_media_pll_top_user_p, MUX_SEL_TOP2, 4, 1),
	MUX(0, "mout_aclk_mfcmscl_400", mout_bus_media_pll_top_user_p, MUX_SEL_TOP2, 8, 1),
	MUX(CLK_MOUT_SCLK_ISP_SPI0_EXT_CLK, "mout_sclk_isp_spi0_ext_clk", mout_bus_pll_top_user_p, MUX_SEL_TOP_ISP, 0, 1),
	MUX(CLK_MOUT_SCLK_ISP_SPI1_EXT_CLK, "mout_sclk_isp_spi1_ext_clk", mout_bus_pll_top_user_p, MUX_SEL_TOP_ISP, 4, 1),
	MUX(CLK_MOUT_SCLK_ISP_UART_EXT_UCLK, "mout_sclk_isp_uart_ext_uclk", mout_bus_pll_top_user_p, MUX_SEL_TOP_ISP, 16, 1),
	MUX(CLK_MOUT_SCLK_ISP_SENSOR0, "mout_sclk_isp_sensor0", mout_bus_pll_top_user_p, MUX_SEL_TOP_ISP, 20, 1),
	MUX(CLK_MOUT_SCLK_ISP_SENSOR1, "mout_sclk_isp_sensor1", mout_bus_pll_top_user_p, MUX_SEL_TOP_ISP, 24, 1),
	MUX(0, "mout_sclk_fsys_mmc0_a", mout_bus_pll_top_user_p, MUX_SEL_TOP_FSYS0, 0, 1),
	MUX(0, "mout_sclk_fsys_mmc0_b", mout_sclk_fsys_mmc0_b_p, MUX_SEL_TOP_FSYS0, 4, 1),
	MUX(0, "mout_sclk_fsys_mmc1_a", mout_bus_pll_top_user_p, MUX_SEL_TOP_FSYS1, 0, 1),
	MUX(0, "mout_sclk_fsys_mmc1_b", mout_sclk_fsys_mmc1_b_p, MUX_SEL_TOP_FSYS1, 4, 1),
	MUX(0, "mout_sclk_fsys_mmc2_a", mout_bus_pll_top_user_p, MUX_SEL_TOP_FSYS2, 0, 1),
	MUX(0, "mout_sclk_fsys_mmc2_b", mout_sclk_fsys_mmc2_b_p, MUX_SEL_TOP_FSYS2, 4, 1),
	MUX(0, "mout_sclk_peri_spi0_ext_clk", mout_bus_pll_top_user_p, MUX_SEL_TOP_PERI, 0, 1),
	MUX(0, "mout_sclk_peri_spi1_ext_clk", mout_bus_pll_top_user_p, MUX_SEL_TOP_PERI, 4, 1),
	MUX(0, "mout_sclk_peri_spi2_ext_clk", mout_bus_pll_top_user_p, MUX_SEL_TOP_PERI, 8, 1),
	MUX(0, "mout_sclk_peri_uart0_ext_uclk", mout_bus_pll_top_user_p, MUX_SEL_TOP_PERI, 20, 1),
	MUX(0, "mout_sclk_peri_uart1_ext_uclk", mout_bus_pll_top_user_p, MUX_SEL_TOP_PERI, 24, 1),
	MUX(0, "mout_sclk_peri_uart2_ext_uclk", mout_bus_pll_top_user_p, MUX_SEL_TOP_PERI, 28, 1),
};

static const struct samsung_div_clock top_div_clks[] __initconst = {
	DIV(0, "dout_aclk_fsys_200", "mout_bus_pll_top_user", DIV_TOP0, 0, 4),
	DIV(0, "dout_aclk_imem_266", "mout_bus_pll_top_user", DIV_TOP0, 4, 4),
	DIV(0, "dout_aclk_imem_200", "mout_bus_pll_top_user", DIV_TOP0, 8, 4),
	DIV(CLK_DOUT_ACLK_BUS1_400, "dout_aclk_bus1_400", "mout_aclk_bus1_400", DIV_TOP0, 16, 4),
	DIV(CLK_DOUT_ACLK_BUS0_400, "dout_aclk_bus0_400", "mout_aclk_bus0_400", DIV_TOP0, 12, 4),
	DIV(CLK_DOUT_ACLK_BUS2_400, "dout_aclk_bus2_400", "mout_aclk_bus2_400", DIV_TOP0, 24, 4),
	DIV(CLK_DOUT_ACLK_PERI_66, "dout_aclk_peri_66", "mout_bus_pll_top_user", DIV_TOP0, 28, 4),
	DIV(0, "dout_aclk_g3d_400", "mout_bus_pll_top_user", DIV_TOP1, 28, 4),
	DIV(0, "dout_aclk_isp_400", "mout_bus_pll_top_user", DIV_TOP1, 8, 4),
	DIV(0, "dout_aclk_isp_333", "mout_media_pll_top_user", DIV_TOP1, 12, 4),
	DIV(0, "dout_aclk_isp_266_top", "mout_aclk_isp_266", DIV_TOP1, 16, 4),
	DIV(CLK_DOUT_ACLK_MFCMSCL_266, "dout_aclk_mfcmscl_266", "mout_aclk_mfcmscl_266", DIV_TOP1, 20, 4),
	DIV(0, "dout_aclk_mfcmscl_400", "mout_aclk_mfcmscl_400", DIV_TOP1, 24, 4),
	DIV(CLK_DOUT_SCLK_ISP_SPI0_EXT_CLK_A, "dout_sclk_isp_spi0_ext_clk_a", "mout_sclk_isp_spi0_ext_clk", DIV_TOP_ISP0, 0, 4),
	DIV(CLK_DOUT_SCLK_ISP_SPI0_EXT_CLK_B, "dout_sclk_isp_spi0_ext_clk_b", "dout_sclk_isp_spi0_ext_clk_a", DIV_TOP_ISP0, 4, 8),
	DIV(CLK_DOUT_SCLK_ISP_SPI1_EXT_CLK_A, "dout_sclk_isp_spi1_ext_clk_a", "mout_sclk_isp_spi1_ext_clk", DIV_TOP_ISP0, 12, 4),
	DIV(CLK_DOUT_SCLK_ISP_SPI1_EXT_CLK_B, "dout_sclk_isp_spi1_ext_clk_b", "dout_sclk_isp_spi1_ext_clk_a", DIV_TOP_ISP0, 16, 8),
	DIV(CLK_DOUT_SCLK_ISP_UART_EXT_UCLK, "dout_sclk_isp_uart_ext_uclk", "mout_sclk_isp_uart_ext_uclk", DIV_TOP_ISP0, 24, 4),
	DIV(CLK_DOUT_SCLK_ISP_SENSOR0_A, "dout_sclk_isp_sensor0_a", "mout_sclk_isp_sensor0", DIV_TOP_ISP1, 0, 4),
	DIV(CLK_DOUT_SCLK_ISP_SENSOR0_B, "dout_sclk_isp_sensor0_b", "dout_sclk_isp_sensor0_a", DIV_TOP_ISP1, 4, 4),
	DIV(CLK_DOUT_SCLK_ISP_SENSOR1_A, "dout_sclk_isp_sensor1_a", "mout_sclk_isp_sensor1", DIV_TOP_ISP1, 8, 4),
	DIV(CLK_DOUT_SCLK_ISP_SENSOR1_B, "dout_sclk_isp_sensor1_b", "dout_sclk_isp_sensor1_a", DIV_TOP_ISP1, 12, 4),
	DIV(CLK_DOUT_MMC0_A, "dout_sclk_fsys_mmc0_a", "mout_sclk_fsys_mmc0_b", DIV_TOP_FSYS0, 0, 4),
	DIV(CLK_DOUT_MMC0_B, "dout_sclk_fsys_mmc0_b", "dout_sclk_fsys_mmc0_a", DIV_TOP_FSYS0, 4, 8),
	DIV(CLK_DOUT_MMC1_A, "dout_sclk_fsys_mmc1_a", "mout_sclk_fsys_mmc1_b", DIV_TOP_FSYS1, 0, 4),
	DIV(CLK_DOUT_MMC1_B, "dout_sclk_fsys_mmc1_b", "dout_sclk_fsys_mmc1_a", DIV_TOP_FSYS1, 4, 8),
	DIV(CLK_DOUT_MMC2_A, "dout_sclk_fsys_mmc2_a", "mout_sclk_fsys_mmc2_b", DIV_TOP_FSYS2, 0, 4),
	DIV(CLK_DOUT_MMC2_B, "dout_sclk_fsys_mmc2_b", "dout_sclk_fsys_mmc2_a", DIV_TOP_FSYS2, 4, 8),
	DIV(0, "dout_sclk_peri_aud", "mout_aud_pll_top_user", DIV_TOP_PERI0, 0, 4),
	DIV(0, "dout_sclk_peri_i2s", "dout_sclk_peri_aud", DIV_TOP_PERI0, 4, 6),
	DIV(0, "dout_sclk_peri_spi0_a", "mout_sclk_peri_spi0_ext_clk", DIV_TOP_PERI1, 0, 4),
	DIV(0, "dout_sclk_peri_spi1_a", "mout_sclk_peri_spi1_ext_clk", DIV_TOP_PERI1, 16, 4),
	DIV(0, "dout_sclk_peri_spi0_b", "dout_sclk_peri_spi0_a", DIV_TOP_PERI1, 4, 8),
	DIV(0, "dout_sclk_peri_spi1_b", "dout_sclk_peri_spi1_a", DIV_TOP_PERI1, 20, 8),
	DIV(0, "dout_sclk_peri_spi2_a", "mout_sclk_peri_spi2_ext_clk", DIV_TOP_PERI2, 0, 4),
	DIV(0, "dout_sclk_peri_spi2_b", "dout_sclk_peri_spi2_a", DIV_TOP_PERI2, 4, 8),
	DIV(CLK_BAUD0, "dout_sclk_peri_uart0", "mout_sclk_peri_uart0_ext_uclk", DIV_TOP_PERI3, 16, 4),
	DIV(CLK_BAUD1, "dout_sclk_peri_uart1", "mout_sclk_peri_uart1_ext_uclk", DIV_TOP_PERI3, 20, 4),
	DIV(CLK_BAUD2, "dout_sclk_peri_uart2", "mout_sclk_peri_uart2_ext_uclk", DIV_TOP_PERI3, 24, 4),
};

static const struct samsung_gate_clock top_gate_clks[] __initconst = {
	GATE(CLK_ACLK_FSYS_200, "aclk_fsys_200", "dout_aclk_fsys_200", EN_ACLK_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_ACLK_IMEM_266, "aclk_imem_266", "dout_aclk_imem_266", EN_ACLK_TOP, 1, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_ACLK_IMEM_200, "aclk_imem_200", "dout_aclk_imem_200", EN_ACLK_TOP, 2, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_ACLK_G3D_400, "aclk_g3d_400", "dout_aclk_g3d_400", EN_ACLK_TOP, 14, 0, 0),
	GATE(CLK_ACLK_ISP_400, "aclk_isp_400", "dout_aclk_isp_400", EN_ACLK_TOP, 9, 0, 0),
	GATE(CLK_ACLK_ISP_333, "aclk_isp_333", "dout_aclk_isp_333", EN_ACLK_TOP, 10, 0, 0),
	GATE(CLK_ACLK_ISP_266, "aclk_isp_266", "dout_aclk_isp_266_top", EN_ACLK_TOP, 11, 0, 0),
	GATE(CLK_ACLK_MFCMSCL_266, "aclk_mfcmscl_266", "dout_aclk_mfcmscl_266", EN_ACLK_TOP, 12, 0, 0),
	GATE(CLK_ACLK_MFCMSCL_400, "aclk_mfcmscl_400", "dout_aclk_mfcmscl_400", EN_ACLK_TOP, 13, 0, 0),
	GATE(CLK_SCLK_FSYS_MMC0, "sclk_fsys_mmc0_sdclkin", "dout_sclk_fsys_mmc0_b", EN_SCLK_TOP_FSYS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_FSYS_MMC1, "sclk_fsys_mmc1_sdclkin", "dout_sclk_fsys_mmc1_b", EN_SCLK_TOP_FSYS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_FSYS_MMC2, "sclk_fsys_mmc2_sdclkin", "dout_sclk_fsys_mmc2_b", EN_SCLK_TOP_FSYS, 2, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_ISP_SPI0_EXT_CLK, "sclk_isp_spi0_ext_clk", "dout_sclk_isp_spi0_ext_clk_b", EN_SCLK_TOP_ISP, 0, 0, 0),
	GATE(CLK_SCLK_ISP_SPI1_EXT_CLK, "sclk_isp_spi1_ext_clk", "dout_sclk_isp_spi1_ext_clk_b", EN_SCLK_TOP_ISP, 1, 0, 0),
	GATE(CLK_SCLK_ISP_UART_EXT_CLK, "sclk_isp_uart_ext_clk", "dout_sclk_isp_uart_ext_uclk", EN_SCLK_TOP_ISP, 2, 0, 0),
	GATE(CLK_SCLK_ISP_SENSOR0, "sclk_isp_sensor0", "dout_sclk_isp_sensor0_b", EN_SCLK_TOP_ISP, 3, 0, 0),
	GATE(CLK_SCLK_ISP_SENSOR1, "sclk_isp_sensor1", "dout_sclk_isp_sensor1_b", EN_SCLK_TOP_ISP, 4, 0, 0),
	GATE(CLK_SCLK_PERI_I2S_I2SCODCLKI, "sclk_peri_i2s_i2scodclki", "dout_sclk_peri_i2s", EN_SCLK_TOP_PERI, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_SPI0, "sclk_peri_spi0_ext_clk", "dout_sclk_peri_spi0_b", EN_SCLK_TOP_PERI, 2, CLK_SET_RATE_PARENT, 0),
	GATE(CLK_SCLK_SPI1, "sclk_peri_spi1_ext_clk", "dout_sclk_peri_spi1_b", EN_SCLK_TOP_PERI, 3, CLK_SET_RATE_PARENT, 0),
	GATE(CLK_SCLK_SPI2, "sclk_peri_spi2_ext_clk", "dout_sclk_peri_spi2_b", EN_SCLK_TOP_PERI, 4, CLK_SET_RATE_PARENT, 0),
	GATE(CLK_SCLK_PERI_UART0_EXT_UCLK, "sclk_peri_uart0_ext_uclk", "dout_sclk_peri_uart0", EN_SCLK_TOP_PERI, 7, 0, 0),
	GATE(CLK_SCLK_PERI_UART1_EXT_UCLK, "sclk_peri_uart1_ext_uclk", "dout_sclk_peri_uart1", EN_SCLK_TOP_PERI, 8, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SCLK_PERI_UART2_EXT_UCLK, "sclk_peri_uart2_ext_uclk", "dout_sclk_peri_uart2", EN_SCLK_TOP_PERI, 9, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(0, "mout_media_pll_top_user", "sclk_media_pll_top", MUX_SEL_TOP1, 8, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(CLK_MUX_BUS_PLL_TOP_USER, "mout_bus_pll_top_user", "bus_pll", MUX_SEL_TOP1, 4, CLK_IGNORE_UNUSED, 0),
	MUX_GATE(0, "mout_aud_pll_top_user", "aud_pll", MUX_SEL_TOP1, 0, CLK_IGNORE_UNUSED, 0),
};

struct samsung_pll_rate_table table_aud[] = {
	/* rate		p,  m,  s,  k */
	{393216000U,	3, 181, 2, 31740},
	{294912000U,	9, 408, 2, 22262},
	{196608000U,	5, 302, 3, 31054},
	{147456000U,	9, 408, 3, 22262},
	{98304000U,	5, 302, 4, 31054},
	{73728000U,	9, 408, 4, 22262},
	{67737600U,	3, 250, 5, 7082},
	{65536000U,	5, 403, 5, 19560},
	{49152000U,	5, 302, 5, 31054},
	{45158400U,	3, 167, 5, -17124},
	{0,		0, 0, 0, 0},
};

static const struct samsung_pll_clock top_pll_clks[] __initconst = {
	PLL(pll_2650x, CLK_AUD_PLL, "aud_pll", "fin_pll", AUD_PLL_LOCK, AUD_PLL_CON0, table_aud),
};

static const struct samsung_cmu_info top_cmu_info __initconst = {
	.pll_clks		= top_pll_clks,
	.nr_pll_clks		= ARRAY_SIZE(top_pll_clks),
	.mux_clks		= top_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(top_mux_clks),
	.gate_clks		= top_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(top_gate_clks),
	.div_clks		= top_div_clks,
	.nr_div_clks		= ARRAY_SIZE(top_div_clks),
	.nr_clk_ids		= TOP_NR_CLK,
	.clk_regs		= top_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(top_clk_regs),
};

static void __init exynos7580_cmu_top_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &top_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_top, "samsung,exynos7580-cmu-top",
		exynos7580_cmu_top_init);

/* Register Offset definitions for CMU_PERI (0x139F0000) */
#define MUX_SEL_PERI			0x0200
#define MUX_EN_PERI				0x0300
#define EN_ACLK_PERI			0x0800
#define EN_PCLK_PERIS			0x0900
#define EN_PCLK_PERIC			0x0904
#define EN_PCLK_PERIS_SEC_TZPC		0x0910
#define EN_PCLK_PERIS_SEC_SECKEY	0x0914
#define EN_PCLK_PERIS_SEC_CHIPID	0x0918
#define EN_PCLK_PERIS_SEC_RTC_TOP	0x091C
#define EN_PCLK_PERIS_SEC_ANTIRBK	0x0928
#define EN_PCLK_PERIS_SEC_MONOTONIC	0x092C
#define EN_PCLK_PERIS_SEC_RTC_APBIF	0x0930
#define EN_SCLK_PERIS			0x0A00
#define EN_SCLK_PERIC			0x0A04
#define EN_SCLK_PERIS_SEC_SECKEY	0x0A10
#define EN_SCLK_PERIS_SEC_CHIPID	0x0A14
#define EN_SCLK_PERIS_SEC_ANTIRBK	0x0A20

static const struct samsung_fixed_rate_clock peri_fixed_rate_clks[] __initconst = {
	FRATE(0, "ioclk_peri_i2sbclki", NULL, 0, 12288000),
	FRATE(0, "ioclk_peri_spi", NULL, 0, 50000000),
};

PNAME(mout_sclk_i2s_i2scodclki_p) = {"ioclk_audiocdclk1", "fin_pll",
				     "sclk_peri_i2s_i2scodclki"};

static const unsigned long peri_clk_regs[] __initconst = {
	EN_SCLK_PERIC,
	EN_SCLK_PERIS,
	EN_PCLK_PERIS,
	EN_PCLK_PERIS_SEC_TZPC,
	EN_PCLK_PERIS_SEC_RTC_TOP,
	EN_PCLK_PERIS_SEC_CHIPID,
	EN_PCLK_PERIS_SEC_SECKEY,
	EN_PCLK_PERIS_SEC_ANTIRBK,
	EN_PCLK_PERIS_SEC_MONOTONIC,
	EN_PCLK_PERIS_SEC_RTC_APBIF,
	EN_PCLK_PERIC,
	EN_ACLK_PERI,
};

static const struct samsung_mux_clock peri_mux_clks[] __initconst = {
	MUX(0, "mout_sclk_i2s_i2scodclki", mout_sclk_i2s_i2scodclki_p, MUX_SEL_PERI, 0, 2),
};

static const struct samsung_gate_clock peri_gate_clks[] __initconst = {
	GATE(0, "aclk_ahb2apb_peris0", "dout_aclk_peri_66", EN_ACLK_PERI, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_ahb2apb_peris1", "dout_aclk_peri_66", EN_ACLK_PERI, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aclk_ahb2apb_peric0", "dout_aclk_peri_66", EN_ACLK_PERI, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc0", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc1", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc2", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc3", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc4", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc5", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc6", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc7", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc8", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc9", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tzpc10", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_TZPC, 10, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_MCT, "pclk_mct", "dout_aclk_peri_66", EN_PCLK_PERIS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_PCLK_WDT, "pclk_wdt_cpu", "dout_aclk_peri_66", EN_PCLK_PERIS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_cmu_top_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_abb_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_efuse_writer_sc_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_custom_efuse_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tmu0_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_tmu1_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_abb", "fin_pll", EN_PCLK_PERIS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_tmu0", "fin_pll", EN_PCLK_PERIS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_tmu1", "fin_pll", EN_PCLK_PERIS, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_rtc_top", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_RTC_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_chipid_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_CHIPID, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_chipid", "fin_pll", EN_PCLK_PERIS_SEC_CHIPID, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_seckey", "fin_pll", EN_PCLK_PERIS_SEC_SECKEY, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_seckey_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_SECKEY, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_antirbk_cnt_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_ANTIRBK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_antirbk_cnt", "fin_pll", EN_PCLK_PERIS_SEC_ANTIRBK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_monotonic_cnt_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_MONOTONIC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_GATE_RTC, "pclk_rtc_apbif", "dout_aclk_peri_66", EN_PCLK_PERIS_SEC_RTC_APBIF, 0, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_I2C1, "pclk_i2c1", "dout_aclk_peri_66", EN_PCLK_PERIC, 8, 0, 0),
	GATE(CLK_I2C2, "pclk_i2c2", "dout_aclk_peri_66", EN_PCLK_PERIC, 9, 0, 0),
	GATE(CLK_I2C3, "pclk_i2c3", "dout_aclk_peri_66", EN_PCLK_PERIC, 10, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_PUART0, "pclk_uart0", "dout_aclk_peri_66", EN_PCLK_PERIC, 15, 0, 0),
	GATE(CLK_PUART1, "pclk_uart1", "dout_aclk_peri_66", EN_PCLK_PERIC, 16, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_PUART2, "pclk_uart2", "dout_aclk_peri_66", EN_PCLK_PERIC, 17, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_PCLK_ADCIF, "pclk_adcif", "sclk_adcif_i_osc_sys", EN_PCLK_PERIC, 18, 0, 0),
	GATE(CLK_PCLK_SPI0, "pclk_spi0", "dout_aclk_peri_66", EN_PCLK_PERIC, 19, 0, 0),
	GATE(CLK_PCLK_SPI1, "pclk_spi1", "dout_aclk_peri_66", EN_PCLK_PERIC, 20, 0, 0),
	GATE(CLK_PCLK_SPI2, "pclk_spi2", "dout_aclk_peri_66", EN_PCLK_PERIC, 21, 0, 0),
	GATE(CLK_PCLK_PWM, "pwm-clock", "dout_aclk_peri_66", EN_PCLK_PERIC, 26, CLK_IGNORE_UNUSED | CLK_SET_RATE_PARENT, 0),
	GATE(0, "pclk_gpio_peri", "dout_aclk_peri_66", EN_PCLK_PERIC, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_gpio_nfc", "dout_aclk_peri_66", EN_PCLK_PERIC, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_gpio_touch", "dout_aclk_peri_66", EN_PCLK_PERIC, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "pclk_gpio_alive", "dout_aclk_peri_66", EN_PCLK_PERIC, 6, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_HSI2C0, "pclk_hsi2c0", "dout_aclk_peri_66", EN_PCLK_PERIC, 11, 0, 0),
	GATE(CLK_HSI2C1, "pclk_hsi2c1", "dout_aclk_peri_66", EN_PCLK_PERIC, 12, 0, 0),
	GATE(CLK_HSI2C2, "pclk_hsi2c2", "dout_aclk_peri_66", EN_PCLK_PERIC, 13, 0, 0),
	GATE(CLK_I2C0, "pclk_i2c0", "dout_aclk_peri_66", EN_PCLK_PERIC, 7, 0, 0),
	GATE(CLK_SUART0, "sclk_uart0", "sclk_peri_uart0_ext_uclk", EN_SCLK_PERIC, 0, 0, 0),
	GATE(CLK_SUART1, "sclk_uart1", "sclk_peri_uart1_ext_uclk", EN_SCLK_PERIC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(CLK_SUART2, "sclk_uart2", "sclk_peri_uart2_ext_uclk", EN_SCLK_PERIC, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ioclk_spi0", "ioclk_peri_spi", EN_SCLK_PERIC, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ioclk_spi1", "ioclk_peri_spi", EN_SCLK_PERIC, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ioclk_spi2", "ioclk_peri_spi", EN_SCLK_PERIC, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_adcif_i_osc_sys", "dout_aclk_peri_66", EN_SCLK_PERIC, 17, 0, 0),
	GATE(0, "sclk_pwm_tclk0", "fin_pll", EN_SCLK_PERIC, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_efuse_writer", "fin_pll", EN_SCLK_PERIS, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "sclk_custom_efuse", "fin_pll", EN_SCLK_PERIS, 4, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info peri_cmu_info __initconst = {
	.fixed_clks		= peri_fixed_rate_clks,
	.nr_fixed_clks		= ARRAY_SIZE(peri_fixed_rate_clks),
	.mux_clks		= peri_mux_clks,
	.nr_mux_clks		= ARRAY_SIZE(peri_mux_clks),
	.gate_clks		= peri_gate_clks,
	.nr_gate_clks		= ARRAY_SIZE(peri_gate_clks),
	.nr_clk_ids		= PERI_NR_CLK,
	.clk_regs		= peri_clk_regs,
	.nr_clk_regs		= ARRAY_SIZE(peri_clk_regs),
};

static void __init exynos7580_cmu_peri_init(struct device_node *np)
{
	samsung_cmu_register_one(np, &peri_cmu_info);
}
CLK_OF_DECLARE(exynos7580_cmu_peri, "samsung,exynos7580-cmu-peri",
		exynos7580_cmu_peri_init);