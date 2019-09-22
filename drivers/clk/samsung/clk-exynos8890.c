#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>

#include <dt-bindings/clock/exynos8890.h>

#include "clk.h"

/* Register Offset definitions for CMU_PERIS (0x10040000) */
#define CLK_CON_MUX_ACLK_PERIS_66_USER 0x0200
#define CLK_STAT_MUX_ACLK_PERIS_66_USER 0x0600
#define CG_CTRL_VAL_ACLK_PERIS 0x0800
#define CG_CTRL_VAL_ACLK_PERIS_HPM_APBIF_PERIS 0x0804
#define CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC 0x0808
#define CG_CTRL_VAL_ACLK_PERIS_SECURE_RTC 0x080C
#define CG_CTRL_VAL_ACLK_PERIS_SECURE_OTP 0x0810
#define CG_CTRL_VAL_ACLK_PERIS_SECURE_CHIPID 0x0814
#define CG_CTRL_VAL_SCLK_PERIS_SECURE_OTP 0x0844
#define CG_CTRL_VAL_SCLK_PERIS_SECURE_CHIPID 0x0848
#define CG_CTRL_VAL_SCLK_PERIS 0x084C
#define CG_CTRL_VAL_SCLK_PERIS_PROMISE 0x0850
#define CLKOUT_CMU_PERIS 0x0C00
#define CLKOUT_CMU_PERIS_DIV_STAT 0x0C04
#define PERIS_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_PERIS_SPARE0 0x0D08
#define CMU_PERIS_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_PERIS 0x1800
#define CG_CTRL_MAN_ACLK_PERIS_HPM_APBIF_PERIS 0x1804
#define CG_CTRL_MAN_ACLK_PERIS_SECURE_TZPC 0x1808
#define CG_CTRL_MAN_ACLK_PERIS_SECURE_RTC 0x180C
#define CG_CTRL_MAN_ACLK_PERIS_SECURE_OTP 0x1810
#define CG_CTRL_MAN_ACLK_PERIS_SECURE_CHIPID 0x1814
#define CG_CTRL_MAN_SCLK_PERIS_SECURE_OTP 0x1844
#define CG_CTRL_MAN_SCLK_PERIS_SECURE_CHIPID 0x1848
#define CG_CTRL_MAN_SCLK_PERIS 0x184C
#define CG_CTRL_MAN_SCLK_PERIS_PROMISE 0x1850
#define CG_CTRL_STAT_ACLK_PERIS_0 0x1C00
#define CG_CTRL_STAT_ACLK_PERIS_1 0x1C04
#define CG_CTRL_STAT_ACLK_PERIS_HPM_APBIF_PERIS 0x1C08
#define CG_CTRL_STAT_ACLK_PERIS_SECURE_TZPC_0 0x1C0C
#define CG_CTRL_STAT_ACLK_PERIS_SECURE_TZPC_1 0x1C10
#define CG_CTRL_STAT_ACLK_PERIS_SECURE_RTC 0x1C14
#define CG_CTRL_STAT_ACLK_PERIS_SECURE_OTP 0x1C18
#define CG_CTRL_STAT_ACLK_PERIS_SECURE_CHIPID 0x1C1C
#define CG_CTRL_STAT_SCLK_PERIS_SECURE_OTP 0x1C44
#define CG_CTRL_STAT_SCLK_PERIS_SECURE_CHIPID 0x1C48
#define CG_CTRL_STAT_SCLK_PERIS 0x1C4C
#define CG_CTRL_STAT_SCLK_PERIS_PROMISE 0x1C50
#define QCH_CTRL_AXILHASYNCM_PERIS 0x2000
#define QCH_CTRL_CMU_PERIS 0x2004
#define QCH_CTRL_PMU_PERIS 0x2008
#define QCH_CTRL_SYSREG_PERIS 0x200C
#define QCH_CTRL_MONOCNT_APBIF 0x2010
#define QSTATE_CTRL_MCT 0x240C
#define QSTATE_CTRL_WDT_MNGS 0x2410
#define QSTATE_CTRL_WDT_APOLLO 0x2414
#define QSTATE_CTRL_RTC_APBIF 0x2418
#define QSTATE_CTRL_SFR_APBIF_TMU 0x241C
#define QSTATE_CTRL_SFR_APBIF_HDMI_CEC 0x2420
#define QSTATE_CTRL_HPM_APBIF_PERIS 0x2424
#define QSTATE_CTRL_TZPC_0 0x2428
#define QSTATE_CTRL_TZPC_1 0x242C
#define QSTATE_CTRL_TZPC_2 0x2430
#define QSTATE_CTRL_TZPC_3 0x2434
#define QSTATE_CTRL_TZPC_4 0x2438
#define QSTATE_CTRL_TZPC_5 0x243C
#define QSTATE_CTRL_TZPC_6 0x2440
#define QSTATE_CTRL_TZPC_7 0x2444
#define QSTATE_CTRL_TZPC_8 0x2448
#define QSTATE_CTRL_TZPC_9 0x244C
#define QSTATE_CTRL_TZPC_10 0x2450
#define QSTATE_CTRL_TZPC_11 0x2454
#define QSTATE_CTRL_TZPC_12 0x2458
#define QSTATE_CTRL_TZPC_13 0x245C
#define QSTATE_CTRL_TZPC_14 0x2460
#define QSTATE_CTRL_TZPC_15 0x2464
#define QSTATE_CTRL_TOP_RTC 0x2468
#define QSTATE_CTRL_OTP_CON_TOP 0x246C
#define QSTATE_CTRL_SFR_APBIF_CHIPID 0x2470
#define QSTATE_CTRL_TMU 0x2474
#define QSTATE_CTRL_CHIPID 0x2484
#define QSTATE_CTRL_PROMISE_PERIS 0x2488

PNAME(peris_mux_aclk_peris_66_user_p) = { "oscclk", "top_gate_aclk_peris_66" };

static const struct samsung_mux_clock peris_mux_clks[] __initconst = {
	MUX(0, "peris_mux_aclk_peris_66_user", peris_mux_aclk_peris_66_user_p,
	    CLK_CON_MUX_ACLK_PERIS_66_USER, 12, 1),
};

static const struct samsung_gate_clock peris_gate_clks[] __initconst = {
	GATE(0, "peris_gate_pclk_sfr_apbif_hdmi_cec",
	     "peris_mux_aclk_peris_66_user", CG_CTRL_VAL_ACLK_PERIS, 13,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_sfr_apbif_tmu", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_rtc_apbif", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_monocnt_apbif", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_wdt_apollo", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_wdt_mngs", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_mct", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_sysreg_peris", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_pmu_peris", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_cmu_peris", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_aclk_axi2apb_peris1",
	     "peris_mux_aclk_peris_66_user", CG_CTRL_VAL_ACLK_PERIS, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_aclk_axi2apb_peris0",
	     "peris_mux_aclk_peris_66_user", CG_CTRL_VAL_ACLK_PERIS, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_aclk_xiu_peris", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_aclk_axi_lh_async", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_hpm_apbif_peris",
	     "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_HPM_APBIF_PERIS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_15", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_14", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_13", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_12", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_11", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_10", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_9", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_8", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_7", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_6", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_5", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_4", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_3", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_2", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_1", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_tzpc_0", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_TZPC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_top_rtc", "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_RTC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_pclk_sfr_apbif_chipid",
	     "peris_mux_aclk_peris_66_user",
	     CG_CTRL_VAL_ACLK_PERIS_SECURE_CHIPID, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_sclk_otp_con_top", "oscclk",
	     CG_CTRL_VAL_SCLK_PERIS_SECURE_OTP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_sclk_chipid", "oscclk",
	     CG_CTRL_VAL_SCLK_PERIS_SECURE_CHIPID, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_sclk_tmu", "oscclk", CG_CTRL_VAL_SCLK_PERIS, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_gate_sclk_promise_peris", "top_gate_sclk_promise_int",
	     CG_CTRL_VAL_SCLK_PERIS_PROMISE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_mct", "peris_mux_aclk_peris_66_user", QSTATE_CTRL_MCT, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_wdt_mngs", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_WDT_MNGS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_wdt_apollo", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_WDT_APOLLO, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_rtc_apbif", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_RTC_APBIF, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_sfr_apbif_tmu", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_SFR_APBIF_TMU, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_sfr_apbif_hdmi_cec", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_SFR_APBIF_HDMI_CEC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_hpm_apbif_peris", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_HPM_APBIF_PERIS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_0", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_1", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_2", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_3", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_3, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_4", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_4, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_5", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_5, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_6", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_6, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_7", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_7, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_8", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_8, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_9", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_9, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_10", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_10, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_11", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_11, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_12", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_12, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_13", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_13, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_14", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_14, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tzpc_15", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TZPC_15, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_top_rtc", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_TOP_RTC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_otp_con_top", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_OTP_CON_TOP, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_sfr_apbif_chipid", "peris_mux_aclk_peris_66_user",
	     QSTATE_CTRL_SFR_APBIF_CHIPID, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_tmu", "oscclk", QSTATE_CTRL_TMU, 1, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "peris_chipid", "oscclk", QSTATE_CTRL_CHIPID, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peris_promise_peris", "top_gate_sclk_promise_int",
	     QSTATE_CTRL_PROMISE_PERIS, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info peris_cmu_info __initconst = {
	.mux_clks = peris_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(peris_mux_clks),
	.gate_clks = peris_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(peris_gate_clks),
	.nr_clk_ids = PERIS_NR_CLK,
};

/* Register Offset definitions for CMU_TOP (0x10570000) */
#define BUS0_PLL_LOCK 0x0000
#define BUS1_PLL_LOCK 0x0020
#define BUS2_PLL_LOCK 0x0040
#define BUS3_PLL_LOCK 0x0060
#define MFC_PLL_LOCK 0x0080
#define ISP_PLL_LOCK 0x00A0
#define AUD_PLL_LOCK 0x00C0
#define G3D_PLL_LOCK 0x00E0
#define BUS0_PLL_CON0 0x0100
#define BUS0_PLL_CON1 0x0104
#define BUS0_PLL_FREQ_DET 0x010C
#define BUS1_PLL_CON0 0x0120
#define BUS1_PLL_CON1 0x0124
#define BUS1_PLL_FREQ_DET 0x012C
#define BUS2_PLL_CON0 0x0140
#define BUS2_PLL_CON1 0x0144
#define BUS2_PLL_FREQ_DET 0x014C
#define BUS3_PLL_CON0 0x0160
#define BUS3_PLL_CON1 0x0164
#define BUS3_PLL_FREQ_DET 0x016C
#define MFC_PLL_CON0 0x0180
#define MFC_PLL_CON1 0x0184
#define MFC_PLL_FREQ_DET 0x018C
#define ISP_PLL_CON0 0x01A0
#define ISP_PLL_CON1 0x01A4
#define ISP_PLL_FREQ_DET 0x01AC
#define AUD_PLL_CON0 0x01C0
#define AUD_PLL_CON1 0x01C4
#define AUD_PLL_CON2 0x01C8
#define AUD_PLL_FREQ_DET 0x01D0
#define G3D_PLL_CON0 0x01E0
#define G3D_PLL_CON1 0x01E4
#define G3D_PLL_FREQ_DET 0x01EC
#define CLK_CON_MUX_BUS0_PLL 0x0200
#define CLK_CON_MUX_BUS1_PLL 0x0204
#define CLK_CON_MUX_BUS2_PLL 0x0208
#define CLK_CON_MUX_BUS3_PLL 0x020C
#define CLK_CON_MUX_MFC_PLL 0x0210
#define CLK_CON_MUX_ISP_PLL 0x0214
#define CLK_CON_MUX_AUD_PLL 0x0218
#define CLK_CON_MUX_G3D_PLL 0x021C
#define CLK_CON_MUX_SCLK_BUS0_PLL 0x0220
#define CLK_CON_MUX_SCLK_BUS1_PLL 0x0224
#define CLK_CON_MUX_SCLK_BUS2_PLL 0x0228
#define CLK_CON_MUX_SCLK_BUS3_PLL 0x022C
#define CLK_CON_MUX_SCLK_MFC_PLL 0x0230
#define CLK_CON_MUX_SCLK_ISP_PLL 0x0234
#define CLK_CON_MUX_ACLK_CCORE_800 0x0240
#define CLK_CON_MUX_ACLK_CCORE_264 0x0244
#define CLK_CON_MUX_ACLK_CCORE_G3D_800 0x0248
#define CLK_CON_MUX_ACLK_CCORE_528 0x024C
#define CLK_CON_MUX_ACLK_CCORE_132 0x0250
#define CLK_CON_MUX_PCLK_CCORE_66 0x0254
#define CLK_CON_MUX_ACLK_BUS0_528 0x0258
#define CLK_CON_MUX_ACLK_BUS0_200 0x025C
#define CLK_CON_MUX_PCLK_BUS0_132 0x0260
#define CLK_CON_MUX_ACLK_BUS1_528 0x0264
#define CLK_CON_MUX_PCLK_BUS1_132 0x0268
#define CLK_CON_MUX_ACLK_DISP0_0_400 0x026C
#define CLK_CON_MUX_ACLK_DISP0_1_400_TOP 0x0270
#define CLK_CON_MUX_ACLK_DISP1_0_400 0x0274
#define CLK_CON_MUX_ACLK_DISP1_1_400_TOP 0x0278
#define CLK_CON_MUX_ACLK_MFC_600 0x027C
#define CLK_CON_MUX_ACLK_MSCL0_528 0x0280
#define CLK_CON_MUX_ACLK_MSCL1_528_TOP 0x0284
#define CLK_CON_MUX_ACLK_IMEM_266 0x0288
#define CLK_CON_MUX_ACLK_IMEM_200 0x028C
#define CLK_CON_MUX_ACLK_IMEM_100 0x0290
#define CLK_CON_MUX_ACLK_FSYS0_200 0x0294
#define CLK_CON_MUX_ACLK_FSYS1_200 0x0298
#define CLK_CON_MUX_ACLK_PERIS_66 0x029C
#define CLK_CON_MUX_ACLK_PERIC0_66 0x02A0
#define CLK_CON_MUX_ACLK_PERIC1_66 0x02A4
#define CLK_CON_MUX_ACLK_ISP0_ISP0_528 0x02A8
#define CLK_CON_MUX_ACLK_ISP0_TPU_400 0x02AC
#define CLK_CON_MUX_ACLK_ISP0_TREX_528 0x02B0
#define CLK_CON_MUX_ACLK_ISP1_ISP1_468 0x02B4
#define CLK_CON_MUX_ACLK_CAM0_CSIS0_414 0x02B8
#define CLK_CON_MUX_ACLK_CAM0_CSIS1_168 0x02BC
#define CLK_CON_MUX_ACLK_CAM0_CSIS2_234 0x02C0
#define CLK_CON_MUX_ACLK_CAM0_3AA0_414 0x02C4
#define CLK_CON_MUX_ACLK_CAM0_3AA1_414 0x02C8
#define CLK_CON_MUX_ACLK_CAM0_CSIS3_132 0x02CC
#define CLK_CON_MUX_ACLK_CAM0_TREX_528 0x02D0
#define CLK_CON_MUX_ACLK_CAM1_ARM_672 0x02D4
#define CLK_CON_MUX_ACLK_CAM1_TREX_VRA_528 0x02D8
#define CLK_CON_MUX_ACLK_CAM1_TREX_B_528 0x02DC
#define CLK_CON_MUX_ACLK_CAM1_BUS_264 0x02E0
#define CLK_CON_MUX_ACLK_CAM1_PERI_84 0x02E4
#define CLK_CON_MUX_ACLK_CAM1_CSIS2_414 0x02E8
#define CLK_CON_MUX_ACLK_CAM1_CSIS3_132 0x02EC
#define CLK_CON_MUX_ACLK_CAM1_SCL_566 0x02F0
#define CLK_CON_MUX_SCLK_DISP0_DECON0_ECLK0_TOP 0x02F4
#define CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK0_TOP 0x02F8
#define CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK1_TOP 0x02FC
#define CLK_CON_MUX_SCLK_DISP0_HDMI_AUDIO_TOP 0x0300
#define CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK0_TOP 0x0304
#define CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK1_TOP 0x0308
#define CLK_CON_MUX_SCLK_FSYS0_USBDRD30 0x030C
#define CLK_CON_MUX_SCLK_FSYS0_MMC0 0x0310
#define CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO20 0x0314
#define CLK_CON_MUX_SCLK_FSYS0_PHY_24M 0x0318
#define CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO_CFG 0x031C
#define CLK_CON_MUX_SCLK_FSYS1_MMC2 0x0320
#define CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO20 0x0324
#define CLK_CON_MUX_SCLK_FSYS1_PCIE_PHY 0x0328
#define CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO_CFG 0x032C
#define CLK_CON_MUX_SCLK_PERIC0_UART0 0x0330
#define CLK_CON_MUX_SCLK_PERIC1_SPI0 0x0334
#define CLK_CON_MUX_SCLK_PERIC1_SPI1 0x0338
#define CLK_CON_MUX_SCLK_PERIC1_SPI2 0x033C
#define CLK_CON_MUX_SCLK_PERIC1_SPI3 0x0340
#define CLK_CON_MUX_SCLK_PERIC1_SPI4 0x0344
#define CLK_CON_MUX_SCLK_PERIC1_SPI5 0x0348
#define CLK_CON_MUX_SCLK_PERIC1_SPI6 0x034C
#define CLK_CON_MUX_SCLK_PERIC1_SPI7 0x0350
#define CLK_CON_MUX_SCLK_PERIC1_UART1 0x0354
#define CLK_CON_MUX_SCLK_PERIC1_UART2 0x0358
#define CLK_CON_MUX_SCLK_PERIC1_UART3 0x035C
#define CLK_CON_MUX_SCLK_PERIC1_UART4 0x0360
#define CLK_CON_MUX_SCLK_PERIC1_UART5 0x0364
#define CLK_CON_MUX_SCLK_CAM1_ISP_SPI0 0x0368
#define CLK_CON_MUX_SCLK_CAM1_ISP_SPI1 0x036C
#define CLK_CON_MUX_SCLK_CAM1_ISP_UART 0x0370
#define CLK_CON_MUX_SCLK_AP2CP_MIF_PLL_OUT 0x0374
#define CLK_CON_MUX_ACLK_PSCDC_400 0x0378
#define CLK_CON_MUX_SCLK_BUS_PLL_MNGS 0x0380
#define CLK_CON_MUX_SCLK_BUS_PLL_APOLLO 0x0384
#define CLK_CON_MUX_SCLK_BUS_PLL_MIF 0x0388
#define CLK_CON_MUX_SCLK_BUS_PLL_G3D 0x038C
#define CLK_CON_MUX_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP 0x0390
#define CLK_CON_DIV_ACLK_CCORE_800 0x03A0
#define CLK_CON_DIV_ACLK_CCORE_264 0x03A4
#define CLK_CON_DIV_ACLK_CCORE_G3D_800 0x03A8
#define CLK_CON_DIV_ACLK_CCORE_528 0x03AC
#define CLK_CON_DIV_ACLK_CCORE_132 0x03B0
#define CLK_CON_DIV_PCLK_CCORE_66 0x03B4
#define CLK_CON_DIV_ACLK_BUS0_528 0x03B8
#define CLK_CON_DIV_ACLK_BUS0_200 0x03BC
#define CLK_CON_DIV_PCLK_BUS0_132 0x03C0
#define CLK_CON_DIV_ACLK_BUS1_528 0x03C4
#define CLK_CON_DIV_PCLK_BUS1_132 0x03C8
#define CLK_CON_DIV_ACLK_DISP0_0_400 0x03CC
#define CLK_CON_DIV_ACLK_DISP0_1_400 0x03D0
#define CLK_CON_DIV_ACLK_DISP1_0_400 0x03D4
#define CLK_CON_DIV_ACLK_DISP1_1_400 0x03D8
#define CLK_CON_DIV_ACLK_MFC_600 0x03DC
#define CLK_CON_DIV_ACLK_MSCL0_528 0x03E0
#define CLK_CON_DIV_ACLK_MSCL1_528 0x03E4
#define CLK_CON_DIV_ACLK_IMEM_266 0x03E8
#define CLK_CON_DIV_ACLK_IMEM_200 0x03EC
#define CLK_CON_DIV_ACLK_IMEM_100 0x03F0
#define CLK_CON_DIV_ACLK_FSYS0_200 0x03F4
#define CLK_CON_DIV_ACLK_FSYS1_200 0x03F8
#define CLK_CON_DIV_ACLK_PERIS_66 0x03FC
#define CLK_CON_DIV_ACLK_PERIC0_66 0x0400
#define CLK_CON_DIV_ACLK_PERIC1_66 0x0404
#define CLK_CON_DIV_ACLK_ISP0_ISP0_528 0x0408
#define CLK_CON_DIV_ACLK_ISP0_TPU_400 0x040C
#define CLK_CON_DIV_ACLK_ISP0_TREX_528 0x0410
#define CLK_CON_DIV_ACLK_ISP1_ISP1_468 0x0414
#define CLK_CON_DIV_ACLK_CAM0_CSIS0_414 0x0418
#define CLK_CON_DIV_ACLK_CAM0_CSIS1_168 0x041C
#define CLK_CON_DIV_ACLK_CAM0_CSIS2_234 0x0420
#define CLK_CON_DIV_ACLK_CAM0_3AA0_414 0x0424
#define CLK_CON_DIV_ACLK_CAM0_3AA1_414 0x0428
#define CLK_CON_DIV_ACLK_CAM0_CSIS3_132 0x042C
#define CLK_CON_DIV_ACLK_CAM0_TREX_528 0x0430
#define CLK_CON_DIV_ACLK_CAM1_ARM_672 0x0434
#define CLK_CON_DIV_ACLK_CAM1_TREX_VRA_528 0x0438
#define CLK_CON_DIV_ACLK_CAM1_TREX_B_528 0x043C
#define CLK_CON_DIV_ACLK_CAM1_BUS_264 0x0440
#define CLK_CON_DIV_ACLK_CAM1_PERI_84 0x0444
#define CLK_CON_DIV_ACLK_CAM1_CSIS2_414 0x0448
#define CLK_CON_DIV_ACLK_CAM1_CSIS3_132 0x044C
#define CLK_CON_DIV_ACLK_CAM1_SCL_566 0x0450
#define CLK_CON_DIV_SCLK_DISP0_DECON0_ECLK0 0x0454
#define CLK_CON_DIV_SCLK_DISP0_DECON0_VCLK0 0x0458
#define CLK_CON_DIV_SCLK_DISP0_DECON0_VCLK1 0x045C
#define CLK_CON_DIV_SCLK_DISP0_HDMI_AUDIO 0x0460
#define CLK_CON_DIV_SCLK_DISP1_DECON1_ECLK0 0x0464
#define CLK_CON_DIV_SCLK_DISP1_DECON1_ECLK1 0x0468
#define CLK_CON_DIV_SCLK_FSYS0_USBDRD30 0x046C
#define CLK_CON_DIV_SCLK_FSYS0_MMC0 0x0470
#define CLK_CON_DIV_SCLK_FSYS0_UFSUNIPRO20 0x0474
#define CLK_CON_DIV_SCLK_FSYS0_PHY_24M 0x0478
#define CLK_CON_DIV_SCLK_FSYS0_UFSUNIPRO_CFG 0x047C
#define CLK_CON_DIV_SCLK_FSYS1_MMC2 0x0480
#define CLK_CON_DIV_SCLK_FSYS1_UFSUNIPRO20 0x0484
#define CLK_CON_DIV_SCLK_FSYS1_PCIE_PHY 0x0488
#define CLK_CON_DIV_SCLK_FSYS1_UFSUNIPRO_CFG 0x048C
#define CLK_CON_DIV_SCLK_PERIC0_UART0 0x0490
#define CLK_CON_DIV_SCLK_PERIC1_SPI0 0x0494
#define CLK_CON_DIV_SCLK_PERIC1_SPI1 0x0498
#define CLK_CON_DIV_SCLK_PERIC1_SPI2 0x049C
#define CLK_CON_DIV_SCLK_PERIC1_SPI3 0x04A0
#define CLK_CON_DIV_SCLK_PERIC1_SPI4 0x04A4
#define CLK_CON_DIV_SCLK_PERIC1_SPI5 0x04A8
#define CLK_CON_DIV_SCLK_PERIC1_SPI6 0x04AC
#define CLK_CON_DIV_SCLK_PERIC1_SPI7 0x04B0
#define CLK_CON_DIV_SCLK_PERIC1_UART1 0x04B4
#define CLK_CON_DIV_SCLK_PERIC1_UART2 0x04B8
#define CLK_CON_DIV_SCLK_PERIC1_UART3 0x04BC
#define CLK_CON_DIV_SCLK_PERIC1_UART4 0x04C0
#define CLK_CON_DIV_SCLK_PERIC1_UART5 0x04C4
#define CLK_CON_DIV_SCLK_CAM1_ISP_SPI0 0x04C8
#define CLK_CON_DIV_SCLK_CAM1_ISP_SPI1 0x04CC
#define CLK_CON_DIV_SCLK_CAM1_ISP_UART 0x04D0
#define CLK_CON_DIV_SCLK_AP2CP_MIF_PLL_OUT 0x04D4
#define CLK_CON_DIV_ACLK_PSCDC_400 0x04D8
#define CLK_CON_DIV_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP 0x04DC
#define CLK_CON_DIV_SCLK_BUS_PLL_MNGS 0x04F0
#define CLK_CON_DIV_SCLK_BUS_PLL_APOLLO 0x04F4
#define CLK_CON_DIV_SCLK_BUS_PLL_MIF 0x04F8
#define CLK_CON_DIV_SCLK_BUS_PLL_G3D 0x04FC
#define CLK_STAT_MUX_BUS0_PLL 0x0500
#define CLK_STAT_MUX_BUS1_PLL 0x0504
#define CLK_STAT_MUX_BUS2_PLL 0x0508
#define CLK_STAT_MUX_BUS3_PLL 0x050C
#define CLK_STAT_MUX_MFC_PLL 0x0510
#define CLK_STAT_MUX_ISP_PLL 0x0514
#define CLK_STAT_MUX_AUD_PLL 0x0518
#define CLK_STAT_MUX_G3D_PLL 0x051C
#define CLK_STAT_MUX_SCLK_BUS0_PLL 0x0520
#define CLK_STAT_MUX_SCLK_BUS1_PLL 0x0524
#define CLK_STAT_MUX_SCLK_BUS2_PLL 0x0528
#define CLK_STAT_MUX_SCLK_BUS3_PLL 0x052C
#define CLK_STAT_MUX_SCLK_MFC_PLL 0x0530
#define CLK_STAT_MUX_SCLK_ISP_PLL 0x0534
#define CLK_STAT_MUX_ACLK_CCORE_800 0x0540
#define CLK_STAT_MUX_ACLK_CCORE_264 0x0544
#define CLK_STAT_MUX_ACLK_CCORE_G3D_800 0x0548
#define CLK_STAT_MUX_ACLK_CCORE_528 0x054C
#define CLK_STAT_MUX_ACLK_CCORE_132 0x0550
#define CLK_STAT_MUX_PCLK_CCORE_66 0x0554
#define CLK_STAT_MUX_ACLK_BUS0_528 0x0558
#define CLK_STAT_MUX_ACLK_BUS0_200 0x055C
#define CLK_STAT_MUX_PCLK_BUS0_132 0x0560
#define CLK_STAT_MUX_ACLK_BUS1_528 0x0564
#define CLK_STAT_MUX_PCLK_BUS1_132 0x0568
#define CLK_STAT_MUX_ACLK_DISP0_0_400 0x056C
#define CLK_STAT_MUX_ACLK_DISP0_1_400_TOP 0x0570
#define CLK_STAT_MUX_ACLK_DISP1_0_400 0x0574
#define CLK_STAT_MUX_ACLK_DISP1_1_400_TOP 0x0578
#define CLK_STAT_MUX_ACLK_MFC_600 0x057C
#define CLK_STAT_MUX_ACLK_MSCL0_528 0x0580
#define CLK_STAT_MUX_ACLK_MSCL1_528_TOP 0x0584
#define CLK_STAT_MUX_ACLK_IMEM_266 0x0588
#define CLK_STAT_MUX_ACLK_IMEM_200 0x058C
#define CLK_STAT_MUX_ACLK_IMEM_100 0x0590
#define CLK_STAT_MUX_ACLK_FSYS0_200 0x0594
#define CLK_STAT_MUX_ACLK_FSYS1_200 0x0598
#define CLK_STAT_MUX_ACLK_PERIS_66 0x059C
#define CLK_STAT_MUX_ACLK_PERIC0_66 0x05A0
#define CLK_STAT_MUX_ACLK_PERIC1_66 0x05A4
#define CLK_STAT_MUX_ACLK_ISP0_ISP0_528 0x05A8
#define CLK_STAT_MUX_ACLK_ISP0_TPU_400 0x05AC
#define CLK_STAT_MUX_ACLK_ISP0_TREX_528 0x05B0
#define CLK_STAT_MUX_ACLK_ISP1_ISP1_468 0x05B4
#define CLK_STAT_MUX_ACLK_CAM0_CSIS0_414 0x05B8
#define CLK_STAT_MUX_ACLK_CAM0_CSIS1_168 0x05BC
#define CLK_STAT_MUX_ACLK_CAM0_CSIS2_234 0x05C0
#define CLK_STAT_MUX_ACLK_CAM0_3AA0_414 0x05C4
#define CLK_STAT_MUX_ACLK_CAM0_3AA1_414 0x05C8
#define CLK_STAT_MUX_ACLK_CAM0_CSIS3_132 0x05CC
#define CLK_STAT_MUX_ACLK_CAM0_TREX_528 0x05D0
#define CLK_STAT_MUX_ACLK_CAM1_ARM_672 0x05D4
#define CLK_STAT_MUX_ACLK_CAM1_TREX_VRA_528 0x05D8
#define CLK_STAT_MUX_ACLK_CAM1_TREX_B_528 0x05DC
#define CLK_STAT_MUX_ACLK_CAM1_BUS_264 0x05E0
#define CLK_STAT_MUX_ACLK_CAM1_PERI_84 0x05E4
#define CLK_STAT_MUX_ACLK_CAM1_CSIS2_414 0x05E8
#define CLK_STAT_MUX_ACLK_CAM1_CSIS3_132 0x05EC
#define CLK_STAT_MUX_ACLK_CAM1_SCL_566 0x05F0
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_ECLK0_TOP 0x05F4
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_VCLK0_TOP 0x05F8
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_VCLK1_TOP 0x05FC
#define CLK_STAT_MUX_SCLK_DISP0_HDMI_AUDIO_TOP 0x0600
#define CLK_STAT_MUX_SCLK_DISP1_DECON1_ECLK0_TOP 0x0604
#define CLK_STAT_MUX_SCLK_DISP1_DECON1_ECLK1_TOP 0x0608
#define CLK_STAT_MUX_SCLK_FSYS0_USBDRD30 0x060C
#define CLK_STAT_MUX_SCLK_FSYS0_MMC0 0x0610
#define CLK_STAT_MUX_SCLK_FSYS0_UFSUNIPRO20 0x0614
#define CLK_STAT_MUX_SCLK_FSYS0_PHY_24M 0x0618
#define CLK_STAT_MUX_SCLK_FSYS0_UFSUNIPRO_CFG 0x061C
#define CLK_STAT_MUX_SCLK_FSYS1_MMC2 0x0620
#define CLK_STAT_MUX_SCLK_FSYS1_UFSUNIPRO20 0x0624
#define CLK_STAT_MUX_SCLK_FSYS1_PCIE_PHY 0x0628
#define CLK_STAT_MUX_SCLK_FSYS1_UFSUNIPRO_CFG 0x062C
#define CLK_STAT_MUX_SCLK_PERIC0_UART0 0x0630
#define CLK_STAT_MUX_SCLK_PERIC1_SPI0 0x0634
#define CLK_STAT_MUX_SCLK_PERIC1_SPI1 0x0638
#define CLK_STAT_MUX_SCLK_PERIC1_SPI2 0x063C
#define CLK_STAT_MUX_SCLK_PERIC1_SPI3 0x0640
#define CLK_STAT_MUX_SCLK_PERIC1_SPI4 0x0644
#define CLK_STAT_MUX_SCLK_PERIC1_SPI5 0x0648
#define CLK_STAT_MUX_SCLK_PERIC1_SPI6 0x064C
#define CLK_STAT_MUX_SCLK_PERIC1_SPI7 0x0650
#define CLK_STAT_MUX_SCLK_PERIC1_UART1 0x0654
#define CLK_STAT_MUX_SCLK_PERIC1_UART2 0x0658
#define CLK_STAT_MUX_SCLK_PERIC1_UART3 0x065C
#define CLK_STAT_MUX_SCLK_PERIC1_UART4 0x0660
#define CLK_STAT_MUX_SCLK_PERIC1_UART5 0x0664
#define CLK_STAT_MUX_SCLK_CAM1_ISP_SPI0 0x0668
#define CLK_STAT_MUX_SCLK_CAM1_ISP_SPI1 0x066C
#define CLK_STAT_MUX_SCLK_CAM1_ISP_UART 0x0670
#define CLK_STAT_MUX_SCLK_AP2CP_MIF_PLL_OUT 0x0674
#define CLK_STAT_MUX_ACLK_PSCDC_400 0x0678
#define CLK_STAT_MUX_SCLK_BUS_PLL_MNGS 0x0680
#define CLK_STAT_MUX_SCLK_BUS_PLL_APOLLO 0x0684
#define CLK_STAT_MUX_SCLK_BUS_PLL_MIF 0x0688
#define CLK_STAT_MUX_SCLK_BUS_PLL_G3D 0x068C
#define CLK_STAT_MUX_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP 0x0690
#define CLK_ENABLE_ACLK_CCORE_800 0x0800
#define CLK_ENABLE_ACLK_CCORE_264 0x0804
#define CLK_ENABLE_ACLK_CCORE_G3D_800 0x0808
#define CLK_ENABLE_ACLK_CCORE_528 0x080C
#define CLK_ENABLE_ACLK_CCORE_132 0x0810
#define CLK_ENABLE_PCLK_CCORE_66 0x0814
#define CLK_ENABLE_ACLK_BUS0_528_TOP 0x0818
#define CLK_ENABLE_ACLK_BUS0_200_TOP 0x081C
#define CLK_ENABLE_PCLK_BUS0_132_TOP 0x0820
#define CLK_ENABLE_ACLK_BUS1_528_TOP 0x0824
#define CLK_ENABLE_PCLK_BUS1_132_TOP 0x0828
#define CLK_ENABLE_ACLK_DISP0_0_400 0x082C
#define CLK_ENABLE_ACLK_DISP0_1_400 0x0830
#define CLK_ENABLE_ACLK_DISP1_0_400 0x0834
#define CLK_ENABLE_ACLK_DISP1_1_400 0x0838
#define CLK_ENABLE_ACLK_MFC_600 0x083C
#define CLK_ENABLE_ACLK_MSCL0_528 0x0840
#define CLK_ENABLE_ACLK_MSCL1_528 0x0844
#define CLK_ENABLE_ACLK_IMEM_266 0x0848
#define CLK_ENABLE_ACLK_IMEM_200 0x084C
#define CLK_ENABLE_ACLK_IMEM_100 0x0850
#define CLK_ENABLE_ACLK_FSYS0_200 0x0854
#define CLK_ENABLE_ACLK_FSYS1_200 0x0858
#define CLK_ENABLE_ACLK_PERIS_66 0x085C
#define CLK_ENABLE_ACLK_PERIC0_66 0x0860
#define CLK_ENABLE_ACLK_PERIC1_66 0x0864
#define CLK_ENABLE_ACLK_ISP0_ISP0_528 0x0868
#define CLK_ENABLE_ACLK_ISP0_TPU_400 0x086C
#define CLK_ENABLE_ACLK_ISP0_TREX_528 0x0870
#define CLK_ENABLE_ACLK_ISP1_ISP1_468 0x0874
#define CLK_ENABLE_ACLK_CAM0_CSIS1_414 0x0878
#define CLK_ENABLE_ACLK_CAM0_CSIS1_168_TOP 0x087C
#define CLK_ENABLE_ACLK_CAM0_CSIS2_234_TOP 0x0880
#define CLK_ENABLE_ACLK_CAM0_3AA0_414_TOP 0x0884
#define CLK_ENABLE_ACLK_CAM0_3AA1_414_TOP 0x0888
#define CLK_ENABLE_ACLK_CAM0_CSIS3_132_TOP 0x088C
#define CLK_ENABLE_ACLK_CAM0_TREX_528_TOP 0x0890
#define CLK_ENABLE_ACLK_CAM1_ARM_672_TOP 0x0894
#define CLK_ENABLE_ACLK_CAM1_TREX_VRA_528_TOP 0x0898
#define CLK_ENABLE_ACLK_CAM1_TREX_B_528_TOP 0x089C
#define CLK_ENABLE_ACLK_CAM1_BUS_264_TOP 0x08A0
#define CLK_ENABLE_ACLK_CAM1_PERI_84 0x08A4
#define CLK_ENABLE_ACLK_CAM1_CSIS2_414_TOP 0x08A8
#define CLK_ENABLE_ACLK_CAM1_CSIS3_132_TOP 0x08AC
#define CLK_ENABLE_ACLK_CAM1_SCL_566_TOP 0x08B0
#define CLK_ENABLE_SCLK_DISP0_DECON0_ECLK0 0x0900
#define CLK_ENABLE_SCLK_DISP0_DECON0_VCLK0 0x0904
#define CLK_ENABLE_SCLK_DISP0_DECON0_VCLK1 0x0908
#define CLK_ENABLE_SCLK_DISP0_HDMI_ADUIO 0x090C
#define CLK_ENABLE_SCLK_DISP1_DECON1_ECLK0 0x0910
#define CLK_ENABLE_SCLK_DISP1_DECON1_ECLK1 0x0914
#define CLK_ENABLE_SCLK_FSYS0_USBDRD30 0x0918
#define CLK_ENABLE_SCLK_FSYS0_MMC0 0x091C
#define CLK_ENABLE_SCLK_FSYS0_UFSUNIPRO20 0x0920
#define CLK_ENABLE_SCLK_FSYS0_PHY_24M 0x0924
#define CLK_ENABLE_SCLK_FSYS0_UFSUNIPRO_CFG 0x0928
#define CLK_ENABLE_SCLK_FSYS1_MMC2 0x092C
#define CLK_ENABLE_SCLK_FSYS1_UFSUNIPRO20 0x0930
#define CLK_ENABLE_SCLK_FSYS1_PCIE_PHY 0x0934
#define CLK_ENABLE_SCLK_FSYS1_UFSUNIPRO_CFG 0x0938
#define CLK_ENABLE_SCLK_PERIC0_UART0 0x093C
#define CLK_ENABLE_SCLK_PERIC1_SPI0 0x0940
#define CLK_ENABLE_SCLK_PERIC1_SPI1 0x0944
#define CLK_ENABLE_SCLK_PERIC1_SPI2 0x0948
#define CLK_ENABLE_SCLK_PERIC1_SPI3 0x094C
#define CLK_ENABLE_SCLK_PERIC1_SPI4 0x0950
#define CLK_ENABLE_SCLK_PERIC1_SPI5 0x0954
#define CLK_ENABLE_SCLK_PERIC1_SPI6 0x0958
#define CLK_ENABLE_SCLK_PERIC1_SPI7 0x095C
#define CLK_ENABLE_SCLK_PERIC1_UART1 0x0960
#define CLK_ENABLE_SCLK_PERIC1_UART2 0x0964
#define CLK_ENABLE_SCLK_PERIC1_UART3 0x0968
#define CLK_ENABLE_SCLK_PERIC1_UART4 0x096C
#define CLK_ENABLE_SCLK_PERIC1_UART5 0x0970
#define CLK_ENABLE_SCLK_CAM1_ISP_SPI0_TOP 0x0974
#define CLK_ENABLE_SCLK_CAM1_ISP_SPI1_TOP 0x0978
#define CLK_ENABLE_SCLK_CAM1_ISP_UART_TOP 0x097C
#define CLK_ENABLE_SCLK_AP2CP_MIF_PLL_OUT 0x0980
#define CLK_ENABLE_ACLK_PSCDC_400 0x0984
#define CLK_ENABLE_SCLK_BUS_PLL_MNGS 0x0990
#define CLK_ENABLE_SCLK_BUS_PLL_APOLLO 0x0994
#define CLK_ENABLE_SCLK_BUS_PLL_MIF 0x0998
#define CLK_ENABLE_SCLK_BUS_PLL_G3D 0x099C
#define CLK_ENABLE_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP 0x09A0
#define CLK_CON_MUX_SCLK_ISP_SENSOR0 0x0B00
#define CLK_CON_DIV_SCLK_ISP_SENSOR0 0x0B04
#define CLK_STAT_MUX_SCLK_ISP_SENSOR0 0x0B08
#define CLK_ENABLE_SCLK_ISP_SENSOR0 0x0B0C
#define CLK_CON_MUX_SCLK_ISP_SENSOR1 0x0B10
#define CLK_CON_DIV_SCLK_ISP_SENSOR1 0x0B14
#define CLK_STAT_MUX_SCLK_ISP_SENSOR1 0x0B18
#define CLK_ENABLE_SCLK_ISP_SENSOR1 0x0B1C
#define CLK_CON_MUX_SCLK_ISP_SENSOR2 0x0B20
#define CLK_CON_DIV_SCLK_ISP_SENSOR2 0x0B24
#define CLK_STAT_MUX_SCLK_ISP_SENSOR2 0x0B28
#define CLK_ENABLE_SCLK_ISP_SENSOR2 0x0B2C
#define CLK_CON_MUX_SCLK_ISP_SENSOR3 0x0B30
#define CLK_CON_DIV_SCLK_ISP_SENSOR3 0x0B34
#define CLK_STAT_MUX_SCLK_ISP_SENSOR3 0x0B38
#define CLK_ENABLE_SCLK_ISP_SENSOR3 0x0B3C
#define CLK_CON_MUX_SCLK_PROMISE_INT 0x0B40
#define CLK_CON_DIV_SCLK_PROMISE_INT 0x0B44
#define CLK_STAT_MUX_SCLK_PROMISE_INT 0x0B48
#define CLK_ENABLE_SCLK_PROMISE_INT 0x0B4C
#define CLK_CON_MUX_SCLK_PROMISE_DISP 0x0B50
#define CLK_CON_DIV_SCLK_PROMISE_DISP 0x0B54
#define CLK_STAT_MUX_SCLK_PROMISE_DISP 0x0B58
#define CLK_ENABLE_SCLK_PROMISE_DISP 0x0B5C
#define CLKOUT_CMU_TOP0 0x0C00
#define CLKOUT_CMU_TOP0_DIV_STAT 0x0C04
#define CLKOUT_CMU_TOP1 0x0C10
#define CLKOUT_CMU_TOP1_DIV_STAT 0x0C14
#define CLKOUT_CMU_TOP2 0x0C20
#define CLKOUT_CMU_TOP2_DIV_STAT 0x0C24
#define CMU_TOP__CLKOUT0 0x0C30
#define CMU_TOP__CLKOUT1 0x0C34
#define CMU_TOP__CLKOUT2 0x0C38
#define CMU_TOP__CLKOUT3 0x0C3C
#define CLK_CON_MUX_CP2AP_MIF_CLK_USER 0x0D00
#define CLK_STAT_MUX_CP2AP_MIF_CLK_USER 0x0D0C
#define AP2CP_CLK_CTRL 0x0D10
#define CLK_ENABLE_PDN_TOP 0x0E00
#define TOP_ROOTCLKEN 0x0F04
#define TOP0_ROOTCLKEN_ON_GATE 0x0F10
#define TOP1_ROOTCLKEN_ON_GATE 0x0F14
#define TOP2_ROOTCLKEN_ON_GATE 0x0F18
#define TOP3_ROOTCLKEN_ON_GATE 0x0F1C
#define TOP0_ROOTCLKEN_ON_MUX 0x0F20
#define TOP1_ROOTCLKEN_ON_MUX 0x0F24
#define TOP2_ROOTCLKEN_ON_MUX 0x0F28
#define TOP3_ROOTCLKEN_ON_MUX 0x0F2C
#define TOP_ROOTCLKEN_AFTER_PLL_MUX 0x0F30
#define TOP0_ROOTCLKEN_ON_GATE_STATUS 0x0F40
#define TOP1_ROOTCLKEN_ON_GATE_STATUS 0x0F44
#define TOP2_ROOTCLKEN_ON_GATE_STATUS 0x0F48
#define TOP3_ROOTCLKEN_ON_GATE_STATUS 0x0F4C
#define TOP0_ROOTCLKEN_ON_MUX_STATUS 0x0F50
#define TOP1_ROOTCLKEN_ON_MUX_STATUS 0x0F54
#define TOP2_ROOTCLKEN_ON_MUX_STATUS 0x0F58
#define TOP3_ROOTCLKEN_ON_MUX_STATUS 0x0F5C
#define TOP_ROOTCLKEN_AFTER_PLL_MUX_STATUS 0x0F60
#define TOP_SFR_IGNORE_REQ_SYSCLK 0x0F80
#define PSCDC_CTRL0 0x1000
#define PSCDC_CTRL1 0x1004
#define PSCDC_CTRL2 0x1008
#define PSCDC_CTRL3 0x100C
#define PSCDC_SCI_FIFO_CLK_CON0 0x1010
#define PSCDC_SCI_FIFO_CLK_CON1 0x1014
#define PSCDC_SCI_FIFO_CLK_CON2 0x1018
#define PSCDC_SCI_FIFO_CLK_CON3 0x101C
#define PSCDC_SMC_FIFO_CLK_CON0 0x1020
#define PSCDC_SMC_FIFO_CLK_CON1 0x1024
#define PSCDC_SMC_FIFO_CLK_CON2 0x1028
#define PSCDC_SMC_FIFO_CLK_CON3 0x102C
#define PSCDC_SMC_FIFO_CLK_CON4 0x1030
#define PSCDC_SMC_FIFO_CLK_CON5 0x1034
#define CCORE_CLK_CTRL0 0x1060
#define MIF_CLK_CTRL0 0x1080
#define MIF_CLK_CTRL1 0x1084
#define MIF_CLK_CTRL2 0x1088
#define MIF_CLK_CTRL3 0x108C
#define MIF_CLK_CTRL4 0x1090
#define ACD_PSCDC_CTRL_0 0x1094
#define ACD_PSCDC_CTRL_1 0x1098
#define ACD_PSCDC_STAT 0x109C
#define CMU_TOP__SPARE0 0x1100
#define CMU_TOP__SPARE1 0x1104
#define CMU_TOP__SPARE2 0x1108
#define CMU_TOP__SPARE3 0x110C

PNAME(top_mux_bus0_pll_p) = { "oscclk", "bus0_pll" };
PNAME(top_mux_bus1_pll_p) = { "oscclk", "bus1_pll" };
PNAME(top_mux_bus2_pll_p) = { "oscclk", "bus2_pll" };
PNAME(top_mux_bus3_pll_p) = { "oscclk", "bus3_pll" };
PNAME(top_mux_mfc_pll_p) = { "oscclk", "mfc_pll" };
PNAME(top_mux_isp_pll_p) = { "oscclk", "isp_pll" };
PNAME(top_mux_aud_pll_p) = { "oscclk", "aud_pll" };
PNAME(top_mux_g3d_pll_p) = { "oscclk", "g3d_pll" };
PNAME(top_mux_sclk_bus0_pll_p) = { "top_mux_bus0_pll", "top_ff_bus0_pll_div2" };
PNAME(top_mux_sclk_bus1_pll_p) = { "top_mux_bus1_pll", "top_ff_bus1_pll_div2" };
PNAME(top_mux_sclk_bus2_pll_p) = { "top_mux_bus2_pll", "top_ff_bus2_pll_div2" };
PNAME(top_mux_sclk_bus3_pll_p) = { "top_mux_bus3_pll", "top_ff_bus3_pll_div2",
				   "top_ff_bus3_pll_div4" };
PNAME(top_mux_sclk_mfc_pll_p) = { "top_mux_mfc_pll", "top_ff_mfc_pll_div2" };
PNAME(top_mux_sclk_isp_pll_p) = { "top_mux_isp_pll", "top_ff_isp_pll_div2" };
PNAME(top_mux_aclk_ccore_800_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_bus1_pll",
	"top_mux_bus2_pll",	 "top_mux_sclk_bus3_pll",
	"top_ff_bus3_pll_div2",	 "top_mux_sclk_mfc_pll",
	"top_mux_isp_pll",	 "top_mux_cp2ap_mif_clk_user"
};
PNAME(top_mux_aclk_ccore_264_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll", "oscclk",
				    "top_mux_cp2ap_mif_clk_user" };
PNAME(top_mux_aclk_ccore_g3d_800_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_bus1_pll",
	"top_mux_bus2_pll",	 "top_mux_sclk_bus3_pll",
	"top_ff_bus3_pll_div2",	 "top_mux_sclk_mfc_pll",
	"top_mux_isp_pll",	 "top_mux_cp2ap_mif_clk_user"
};
PNAME(top_mux_aclk_ccore_528_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll",
				    "top_mux_sclk_bus2_pll",
				    "top_mux_sclk_bus3_pll",
				    "top_mux_cp2ap_mif_clk_user" };
PNAME(top_mux_aclk_ccore_132_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll", "oscclk",
				    "top_mux_cp2ap_mif_clk_user" };
PNAME(top_mux_pclk_ccore_66_p) = { "top_mux_sclk_bus0_pll",
				   "top_mux_sclk_bus1_pll", "oscclk",
				   "top_mux_cp2ap_mif_clk_user" };
PNAME(top_mux_aclk_bus0_528_p) = { "top_mux_sclk_bus0_pll",
				   "top_mux_sclk_bus1_pll",
				   "top_mux_sclk_bus2_pll",
				   "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_bus0_200_p) = { "top_mux_sclk_bus0_pll",
				   "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_pclk_bus0_132_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_aclk_bus1_528_p) = { "top_mux_sclk_bus0_pll",
				   "top_mux_sclk_bus1_pll",
				   "top_mux_sclk_bus2_pll",
				   "top_mux_sclk_bus3_pll" };
PNAME(top_mux_pclk_bus1_132_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_aclk_disp0_0_400_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll",
				      "top_mux_sclk_bus2_pll",
				      "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_disp0_1_400_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll",
				      "top_mux_sclk_bus2_pll",
				      "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_disp1_0_400_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll",
				      "top_mux_sclk_bus2_pll",
				      "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_disp1_1_400_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll",
				      "top_mux_sclk_bus2_pll",
				      "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_mfc_600_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll", "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_mscl0_528_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll",
				    "top_mux_sclk_bus2_pll",
				    "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_mscl1_528_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll",
				    "top_mux_sclk_bus2_pll",
				    "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_imem_266_p) = { "top_mux_sclk_bus0_pll",
				   "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_aclk_imem_200_p) = { "top_mux_sclk_bus0_pll",
				   "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_aclk_imem_100_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_aclk_fsys0_200_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_aclk_fsys1_200_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_aclk_peris_66_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_aclk_peric0_66_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_aclk_peric1_66_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_aclk_isp0_isp0_528_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_isp0_tpu_400_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_isp0_trex_528_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_isp0_pxl_asbs_is_c_from_is_d_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_isp1_isp1_468_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam0_csis0_414_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam0_csis1_168_p) = { "top_mux_sclk_bus0_pll",
					 "top_mux_sclk_bus2_pll" };
PNAME(top_mux_aclk_cam0_csis2_234_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam0_3aa0_414_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam0_3aa1_414_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam0_csis3_132_p) = { "top_mux_sclk_bus0_pll",
					 "top_mux_sclk_bus1_pll" };
PNAME(top_mux_aclk_cam0_trex_528_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam1_arm_672_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll",
	"top_mux_bus2_pll"
};
PNAME(top_mux_aclk_cam1_trex_vra_528_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam1_trex_b_528_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam1_bus_264_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam1_peri_84_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus2_pll" };
PNAME(top_mux_aclk_cam1_csis2_414_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_aclk_cam1_csis3_132_p) = { "top_mux_sclk_bus0_pll",
					 "top_mux_sclk_bus1_pll" };
PNAME(top_mux_aclk_cam1_scl_566_p) = {
	"top_mux_sclk_bus0_pll", "top_mux_sclk_bus1_pll",
	"top_mux_sclk_bus2_pll", "top_mux_sclk_bus3_pll",
	"top_mux_sclk_isp_pll",	 "top_mux_sclk_mfc_pll"
};
PNAME(top_mux_sclk_disp0_decon0_eclk0_p) = { "top_mux_sclk_bus0_pll", "oscclk",
					     "top_mux_bus2_pll" };
PNAME(top_mux_sclk_disp0_decon0_vclk0_p) = {
	"top_mux_sclk_bus0_pll",
	"oscclk",
	"top_mux_bus2_pll",
};
PNAME(top_mux_sclk_disp0_decon0_vclk1_p) = { "top_mux_sclk_bus0_pll", "oscclk",
					     "top_mux_bus2_pll" };
PNAME(top_mux_sclk_disp0_hdmi_audio_p) = { "top_mux_sclk_bus0_pll",
					   "top_mux_aud_pll" };
PNAME(top_mux_sclk_disp1_decon1_eclk0_p) = { "top_mux_sclk_bus0_pll",
					     "top_mux_sclk_bus1_pll",
					     "top_mux_bus2_pll" };
PNAME(top_mux_sclk_disp1_decon1_eclk1_p) = { "top_mux_sclk_bus0_pll",
					     "top_mux_sclk_bus1_pll",
					     "top_mux_bus2_pll" };
PNAME(top_mux_sclk_fsys0_usbdrd30_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_sclk_fsys0_mmc0_p) = { "top_mux_sclk_bus0_pll",
				     "top_mux_bus1_pll",
				     "top_mux_sclk_bus2_pll",
				     "top_ff_bus3_pll_div2" };
PNAME(top_mux_sclk_fsys0_ufsunipro20_p) = { "top_mux_sclk_bus0_pll",
					    "top_mux_sclk_bus1_pll",
					    "top_mux_sclk_bus2_pll" };
PNAME(top_mux_sclk_fsys0_phy_24m_p) = { "top_mux_sclk_bus0_pll", "oscclk" };
PNAME(top_mux_sclk_fsys0_ufsunipro_cfg_p) = { "top_mux_sclk_bus0_pll", "oscclk",
					      "top_mux_sclk_bus2_pll" };
PNAME(top_mux_sclk_fsys1_mmc2_p) = { "top_mux_sclk_bus0_pll",
				     "top_mux_bus1_pll",
				     "top_mux_sclk_bus2_pll",
				     "top_ff_bus3_pll_div2" };
PNAME(top_mux_sclk_fsys1_ufsunipro20_p) = { "top_mux_sclk_bus0_pll",
					    "top_mux_sclk_bus1_pll",
					    "top_mux_sclk_bus2_pll" };
PNAME(top_mux_sclk_fsys1_pcie_phy_p) = { "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_fsys1_ufsunipro_cfg_p) = { "top_mux_sclk_bus0_pll", "oscclk",
					      "top_mux_sclk_bus2_pll" };
PNAME(top_mux_sclk_peric0_uart0_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi0_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi1_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi2_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi3_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi4_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi5_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi6_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_spi7_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_uart1_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_uart2_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_uart3_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_uart4_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_peric1_uart5_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_cam1_isp_spi0_p) = { "top_mux_sclk_bus0_pll",
					"top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_cam1_isp_spi1_p) = { "top_mux_sclk_bus0_pll",
					"top_mux_sclk_bus1_pll", "oscclk" };
PNAME(top_mux_sclk_cam1_isp_uart_p) = { "top_mux_sclk_bus0_pll",
					"top_mux_sclk_bus1_pll" };
PNAME(top_mux_sclk_ap2cp_mif_pll_out_p) = { "top_mux_sclk_bus0_pll",
					    "top_mux_sclk_bus1_pll",
					    "top_mux_sclk_bus2_pll",
					    "top_mux_sclk_bus3_pll" };
PNAME(top_mux_aclk_pscdc_400_p) = { "top_mux_sclk_bus0_pll",
				    "top_mux_sclk_bus1_pll",
				    "top_mux_sclk_bus3_pll",
				    "top_mux_cp2ap_mif_clk_user" };
PNAME(top_mux_sclk_bus_pll_mngs_p) = { "top_mux_bus0_pll", "top_mux_bus1_pll",
				       "top_mux_bus2_pll",
				       "top_ff_bus3_pll_div2" };
PNAME(top_mux_sclk_bus_pll_apollo_p) = { "top_mux_bus0_pll", "top_mux_bus1_pll",
					 "top_mux_bus2_pll",
					 "top_ff_bus3_pll_div2" };
PNAME(top_mux_sclk_bus_pll_mif_p) = {
	"top_mux_bus0_pll", "top_mux_bus1_pll",	    "top_mux_bus2_pll",
	"top_mux_bus3_pll", "top_ff_bus3_pll_div2", "top_mux_cp2ap_mif_clk_user"
};
PNAME(top_mux_sclk_bus_pll_g3d_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_bus1_pll", "top_mux_bus2_pll",
				      "top_ff_bus3_pll_div2",
				      "top_mux_g3d_pll" };
PNAME(top_mux_sclk_isp_sensor0_p) = { "oscclk" };
PNAME(top_mux_sclk_isp_sensor1_p) = { "oscclk" };
PNAME(top_mux_sclk_isp_sensor2_p) = { "oscclk" };
PNAME(top_mux_sclk_isp_sensor3_p) = { "oscclk" };
PNAME(top_mux_sclk_promise_int_p) = { "top_mux_sclk_bus0_pll",
				      "top_mux_sclk_bus1_pll",
				      "top_mux_sclk_bus2_pll" };
PNAME(top_mux_sclk_promise_disp_p) = { "top_mux_sclk_bus0_pll",
				       "top_mux_sclk_bus1_pll",
				       "top_mux_sclk_bus2_pll" };
PNAME(top_mux_cp2ap_mif_clk_user_p) = {
	"oscclk",
	"i_cp2ap_mif_clk",
};
PNAME(top_mux_mif_pll_p) = { "oscclk", "mif0_pll", "mif1_pll", "mif2_pll", "mif3_pll" };
PNAME(top_mux_bus_pll_mif_p) = { "oscclk", "top_mux_sclk_bus1_pll" };
PNAME(top_mux_aclk_mif_pll_p) = { "top_mux_mif_pll", "top_mux_bus_pll_mif" };

/* G3D_PLL */
static const struct samsung_pll_rate_table
	exynos8890_g3d_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 806000000U, 124, 4, 0),
		PLL_35XX_RATE(26 * MHZ, 764400000U, 147, 5, 0),
		PLL_35XX_RATE(26 * MHZ, 754000000U, 116, 4, 0),
		PLL_35XX_RATE(26 * MHZ, 728000000U, 112, 4, 0),
		PLL_35XX_RATE(26 * MHZ, 702000000U, 108, 4, 0),
		PLL_35XX_RATE(26 * MHZ, 682500000U, 105, 4, 0),
		PLL_35XX_RATE(26 * MHZ, 650000000U, 100, 4, 0),
		PLL_35XX_RATE(26 * MHZ, 600166666U, 277, 6, 1),
		PLL_35XX_RATE(26 * MHZ, 572000000U, 176, 4, 1),
		PLL_35XX_RATE(26 * MHZ, 546000000U, 168, 4, 1),
		PLL_35XX_RATE(26 * MHZ, 455000000U, 140, 4, 1),
		PLL_35XX_RATE(26 * MHZ, 419250000U, 129, 4, 1),
		PLL_35XX_RATE(26 * MHZ, 338000000U, 104, 4, 1),
		PLL_35XX_RATE(26 * MHZ, 260000000U, 160, 4, 2),
		PLL_35XX_RATE(26 * MHZ, 169000000U, 104, 4, 2),
		PLL_35XX_RATE(26 * MHZ, 112125000U, 138, 4, 3),
		{ /* sentinel */ }
	};

/* BUS0_PLL */
static const struct samsung_pll_rate_table
	exynos8890_bus0_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 1056000000U, 528, 13, 0),
		{ /* sentinel */ }
	};

/* BUS1_PLL */
static const struct samsung_pll_rate_table
	exynos8890_bus1_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 800000000U, 400, 13, 0),
		{ /* sentinel */ }
	};

/* BUS2_PLL */
static const struct samsung_pll_rate_table
	exynos8890_bus2_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 672000000U, 336, 13, 0),
		{ /* sentinel */ }
	};

/* BUS3_PLL */
static const struct samsung_pll_rate_table
	exynos8890_bus3_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 1872000000U, 288, 4, 0),
		PLL_35XX_RATE(26 * MHZ, 1352000000U, 156, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1092000000U, 126, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 841750000U, 259, 4, 1),
		PLL_35XX_RATE(26 * MHZ, 572000000U, 132, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 416000000U, 192, 3, 2),
		{ /* sentinel */ }
	};

/* MFC_PLL */
static const struct samsung_pll_rate_table
	exynos8890_mfc_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 598000000U, 184, 4, 1),
		{ /* sentinel */ }
	};

/* ISP_PLL */
static const struct samsung_pll_rate_table
	exynos8890_isp_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 425750000U, 131, 4, 1),
		PLL_35XX_RATE(26 * MHZ, 409500000U, 126, 4, 1),
		{ /* sentinel */ }
	};

/* AUD_PLL */
static const struct samsung_pll_rate_table
	exynos8890_aud_pll_rates[] __initconst = {
		PLL_36XX_RATE(26 * MHZ, 592000076U, 46, 1, 1, -30247),
		PLL_36XX_RATE(26 * MHZ, 589824020U, 45, 1, 1, 24319),
		PLL_36XX_RATE(26 * MHZ, 492000091U, 38, 1, 1, -10082),
		PLL_36XX_RATE(26 * MHZ, 491520050U, 38, 1, 1, -12502),
		PLL_36XX_RATE(26 * MHZ, 410000473U, 32, 1, 1, -30245),
		PLL_36XX_RATE(26 * MHZ, 294912010U, 45, 1, 2, 24319),
		PLL_36XX_RATE(26 * MHZ, 196608039U, 30, 1, 2, 16213),
		PLL_36XX_RATE(26 * MHZ, 135475200U, 42, 1, 3, -20665),
		{ /* sentinel */ }
	};

static const struct samsung_pll_clock top_pll_clks[] __initconst = {
	PLL(pll_141xx, 0, "g3d_pll", "oscclk", G3D_PLL_LOCK, G3D_PLL_CON0,
	    exynos8890_g3d_pll_rates),
	PLL(pll_141xx, 0, "bus0_pll", "oscclk", BUS0_PLL_LOCK, BUS0_PLL_CON0,
	    exynos8890_bus0_pll_rates),
	PLL(pll_141xx, 0, "bus1_pll", "oscclk", BUS1_PLL_LOCK, BUS1_PLL_CON0,
	    exynos8890_bus1_pll_rates),
	PLL(pll_141xx, 0, "bus2_pll", "oscclk", BUS2_PLL_LOCK, BUS2_PLL_CON0,
	    exynos8890_bus2_pll_rates),
	PLL(pll_141xx, 0, "bus3_pll", "oscclk", BUS3_PLL_LOCK, BUS3_PLL_CON0,
	    exynos8890_bus3_pll_rates),
	PLL(pll_141xx, 0, "mfc_pll", "oscclk", MFC_PLL_LOCK, MFC_PLL_CON0,
	    exynos8890_mfc_pll_rates),
	PLL(pll_141xx, 0, "isp_pll", "oscclk", ISP_PLL_LOCK, ISP_PLL_CON0,
	    exynos8890_isp_pll_rates),
	PLL(pll_1431x, 0, "aud_pll", "oscclk", AUD_PLL_LOCK, AUD_PLL_CON0,
	    exynos8890_aud_pll_rates),
};

static const struct samsung_fixed_factor_clock
	top_fixed_factor_clks[] __initconst = {
		FFACTOR(0, "top_ff_bus0_pll_div2", "top_mux_bus0_pll", 1, 2, 0),
		FFACTOR(0, "top_ff_bus1_pll_div2", "top_mux_bus1_pll", 1, 2, 0),
		FFACTOR(0, "top_ff_bus2_pll_div2", "top_mux_bus2_pll", 1, 2, 0),
		FFACTOR(0, "top_ff_bus3_pll_div2", "top_mux_bus3_pll", 1, 2, 0),
		FFACTOR(0, "top_ff_bus3_pll_div4", "top_mux_bus3_pll", 1, 4, 0),
		FFACTOR(0, "top_ff_mfc_pll_div2", "top_mux_mfc_pll", 1, 2, 0),
		FFACTOR(0, "top_ff_isp_pll_div2", "top_mux_isp_pll", 1, 2, 0),
	};

static const struct samsung_fixed_rate_clock top_fixed_clks[] __initconst = {
	FRATE(0, "i_cp2ap_mif_clk", NULL, 0, 400000000),
};

static const struct samsung_mux_clock top_mux_clks[] __initconst = {
	MUX(0, "top_mux_bus0_pll", top_mux_bus0_pll_p, CLK_CON_MUX_BUS0_PLL, 12,
	    1),
	MUX(0, "top_mux_bus1_pll", top_mux_bus1_pll_p, CLK_CON_MUX_BUS1_PLL, 12,
	    1),
	MUX(0, "top_mux_bus2_pll", top_mux_bus2_pll_p, CLK_CON_MUX_BUS2_PLL, 12,
	    1),
	MUX(0, "top_mux_bus3_pll", top_mux_bus3_pll_p, CLK_CON_MUX_BUS3_PLL, 12,
	    1),
	MUX(0, "top_mux_mfc_pll", top_mux_mfc_pll_p, CLK_CON_MUX_MFC_PLL, 12,
	    1),
	MUX(0, "top_mux_isp_pll", top_mux_isp_pll_p, CLK_CON_MUX_ISP_PLL, 12,
	    1),
	MUX(0, "top_mux_aud_pll", top_mux_aud_pll_p, CLK_CON_MUX_AUD_PLL, 12,
	    1),
	MUX(0, "top_mux_g3d_pll", top_mux_g3d_pll_p, CLK_CON_MUX_G3D_PLL, 12,
	    1),
	MUX(0, "top_mux_sclk_bus0_pll", top_mux_sclk_bus0_pll_p,
	    CLK_CON_MUX_SCLK_BUS0_PLL, 12, 1),
	MUX(0, "top_mux_sclk_bus1_pll", top_mux_sclk_bus1_pll_p,
	    CLK_CON_MUX_SCLK_BUS1_PLL, 12, 1),
	MUX(0, "top_mux_sclk_bus2_pll", top_mux_sclk_bus2_pll_p,
	    CLK_CON_MUX_SCLK_BUS2_PLL, 12, 1),
	MUX(0, "top_mux_sclk_bus3_pll", top_mux_sclk_bus3_pll_p,
	    CLK_CON_MUX_SCLK_BUS3_PLL, 12, 2),
	MUX(0, "top_mux_sclk_mfc_pll", top_mux_sclk_mfc_pll_p,
	    CLK_CON_MUX_SCLK_MFC_PLL, 12, 1),
	MUX(0, "top_mux_sclk_isp_pll", top_mux_sclk_isp_pll_p,
	    CLK_CON_MUX_SCLK_ISP_PLL, 12, 1),
	MUX(0, "top_mux_aclk_ccore_800", top_mux_aclk_ccore_800_p,
	    CLK_CON_MUX_ACLK_CCORE_800, 12, 3),
	MUX(0, "top_mux_aclk_ccore_264", top_mux_aclk_ccore_264_p,
	    CLK_CON_MUX_ACLK_CCORE_264, 12, 2),
	MUX(0, "top_mux_aclk_ccore_g3d_800", top_mux_aclk_ccore_g3d_800_p,
	    CLK_CON_MUX_ACLK_CCORE_G3D_800, 12, 3),
	MUX(0, "top_mux_aclk_ccore_528", top_mux_aclk_ccore_528_p,
	    CLK_CON_MUX_ACLK_CCORE_528, 12, 3),
	MUX(0, "top_mux_aclk_ccore_132", top_mux_aclk_ccore_132_p,
	    CLK_CON_MUX_ACLK_CCORE_132, 12, 2),
	MUX(0, "top_mux_pclk_ccore_66", top_mux_pclk_ccore_66_p,
	    CLK_CON_MUX_PCLK_CCORE_66, 12, 2),
	MUX(0, "top_mux_aclk_bus0_528", top_mux_aclk_bus0_528_p,
	    CLK_CON_MUX_ACLK_BUS0_528, 12, 2),
	MUX(0, "top_mux_aclk_bus0_200", top_mux_aclk_bus0_200_p,
	    CLK_CON_MUX_ACLK_BUS0_200, 12, 2),
	MUX(0, "top_mux_pclk_bus0_132", top_mux_pclk_bus0_132_p,
	    CLK_CON_MUX_PCLK_BUS0_132, 12, 2),
	MUX(0, "top_mux_aclk_bus1_528", top_mux_aclk_bus1_528_p,
	    CLK_CON_MUX_ACLK_BUS1_528, 12, 2),
	MUX(0, "top_mux_pclk_bus1_132", top_mux_pclk_bus1_132_p,
	    CLK_CON_MUX_PCLK_BUS1_132, 12, 2),
	MUX(0, "top_mux_aclk_disp0_0_400", top_mux_aclk_disp0_0_400_p,
	    CLK_CON_MUX_ACLK_DISP0_0_400, 12, 2),
	MUX(0, "top_mux_aclk_disp0_1_400", top_mux_aclk_disp0_1_400_p,
	    CLK_CON_MUX_ACLK_DISP0_1_400_TOP, 12, 2),
	MUX(0, "top_mux_aclk_disp1_0_400", top_mux_aclk_disp1_0_400_p,
	    CLK_CON_MUX_ACLK_DISP1_0_400, 12, 2),
	MUX(0, "top_mux_aclk_disp1_1_400", top_mux_aclk_disp1_1_400_p,
	    CLK_CON_MUX_ACLK_DISP1_1_400_TOP, 12, 2),
	MUX(0, "top_mux_aclk_mfc_600", top_mux_aclk_mfc_600_p,
	    CLK_CON_MUX_ACLK_MFC_600, 12, 3),
	MUX(0, "top_mux_aclk_mscl0_528", top_mux_aclk_mscl0_528_p,
	    CLK_CON_MUX_ACLK_MSCL0_528, 12, 2),
	MUX(0, "top_mux_aclk_mscl1_528", top_mux_aclk_mscl1_528_p,
	    CLK_CON_MUX_ACLK_MSCL1_528_TOP, 12, 2),
	MUX(0, "top_mux_aclk_imem_266", top_mux_aclk_imem_266_p,
	    CLK_CON_MUX_ACLK_IMEM_266, 12, 2),
	MUX(0, "top_mux_aclk_imem_200", top_mux_aclk_imem_200_p,
	    CLK_CON_MUX_ACLK_IMEM_200, 12, 2),
	MUX(0, "top_mux_aclk_imem_100", top_mux_aclk_imem_100_p,
	    CLK_CON_MUX_ACLK_IMEM_100, 12, 2),
	MUX(0, "top_mux_aclk_fsys0_200", top_mux_aclk_fsys0_200_p,
	    CLK_CON_MUX_ACLK_FSYS0_200, 12, 2),
	MUX(0, "top_mux_aclk_fsys1_200", top_mux_aclk_fsys1_200_p,
	    CLK_CON_MUX_ACLK_FSYS1_200, 12, 2),
	MUX(0, "top_mux_aclk_peris_66", top_mux_aclk_peris_66_p,
	    CLK_CON_MUX_ACLK_PERIS_66, 12, 2),
	MUX(0, "top_mux_aclk_peric0_66", top_mux_aclk_peric0_66_p,
	    CLK_CON_MUX_ACLK_PERIC0_66, 12, 2),
	MUX(0, "top_mux_aclk_peric1_66", top_mux_aclk_peric1_66_p,
	    CLK_CON_MUX_ACLK_PERIC1_66, 12, 2),
	MUX(0, "top_mux_aclk_isp0_isp0_528", top_mux_aclk_isp0_isp0_528_p,
	    CLK_CON_MUX_ACLK_ISP0_ISP0_528, 12, 3),
	MUX(0, "top_mux_aclk_isp0_tpu_400", top_mux_aclk_isp0_tpu_400_p,
	    CLK_CON_MUX_ACLK_ISP0_TPU_400, 12, 3),
	MUX(0, "top_mux_aclk_isp0_trex_528", top_mux_aclk_isp0_trex_528_p,
	    CLK_CON_MUX_ACLK_ISP0_TREX_528, 12, 3),
	MUX(0, "top_mux_aclk_isp0_pxl_asbs_is_c_from_is_d",
	    top_mux_aclk_isp0_pxl_asbs_is_c_from_is_d_p,
	    CLK_CON_MUX_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP, 12, 3),
	MUX(0, "top_mux_aclk_isp1_isp1_468", top_mux_aclk_isp1_isp1_468_p,
	    CLK_CON_MUX_ACLK_ISP1_ISP1_468, 12, 3),
	MUX(0, "top_mux_aclk_cam0_csis0_414", top_mux_aclk_cam0_csis0_414_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS0_414, 12, 3),
	MUX(0, "top_mux_aclk_cam0_csis1_168", top_mux_aclk_cam0_csis1_168_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS1_168, 12, 1),
	MUX(0, "top_mux_aclk_cam0_csis2_234", top_mux_aclk_cam0_csis2_234_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS2_234, 12, 3),
	MUX(0, "top_mux_aclk_cam0_3aa0_414", top_mux_aclk_cam0_3aa0_414_p,
	    CLK_CON_MUX_ACLK_CAM0_3AA0_414, 12, 3),
	MUX(0, "top_mux_aclk_cam0_3aa1_414", top_mux_aclk_cam0_3aa1_414_p,
	    CLK_CON_MUX_ACLK_CAM0_3AA1_414, 12, 3),
	MUX(0, "top_mux_aclk_cam0_csis3_132", top_mux_aclk_cam0_csis3_132_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS3_132, 12, 1),
	MUX(0, "top_mux_aclk_cam0_trex_528", top_mux_aclk_cam0_trex_528_p,
	    CLK_CON_MUX_ACLK_CAM0_TREX_528, 12, 3),
	MUX(0, "top_mux_aclk_cam1_arm_672", top_mux_aclk_cam1_arm_672_p,
	    CLK_CON_MUX_ACLK_CAM1_ARM_672, 12, 3),
	MUX(0, "top_mux_aclk_cam1_trex_vra_528",
	    top_mux_aclk_cam1_trex_vra_528_p,
	    CLK_CON_MUX_ACLK_CAM1_TREX_VRA_528, 12, 3),
	MUX(0, "top_mux_aclk_cam1_trex_b_528", top_mux_aclk_cam1_trex_b_528_p,
	    CLK_CON_MUX_ACLK_CAM1_TREX_B_528, 12, 3),
	MUX(0, "top_mux_aclk_cam1_bus_264", top_mux_aclk_cam1_bus_264_p,
	    CLK_CON_MUX_ACLK_CAM1_BUS_264, 12, 3),
	MUX(0, "top_mux_aclk_cam1_peri_84", top_mux_aclk_cam1_peri_84_p,
	    CLK_CON_MUX_ACLK_CAM1_PERI_84, 12, 1),
	MUX(0, "top_mux_aclk_cam1_csis2_414", top_mux_aclk_cam1_csis2_414_p,
	    CLK_CON_MUX_ACLK_CAM1_CSIS2_414, 12, 3),
	MUX(0, "top_mux_aclk_cam1_csis3_132", top_mux_aclk_cam1_csis3_132_p,
	    CLK_CON_MUX_ACLK_CAM1_CSIS3_132, 12, 1),
	MUX(0, "top_mux_aclk_cam1_scl_566", top_mux_aclk_cam1_scl_566_p,
	    CLK_CON_MUX_ACLK_CAM1_SCL_566, 12, 3),
	MUX(0, "top_mux_sclk_disp0_decon0_eclk0",
	    top_mux_sclk_disp0_decon0_eclk0_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_ECLK0_TOP, 12, 2),
	MUX(0, "top_mux_sclk_disp0_decon0_vclk0",
	    top_mux_sclk_disp0_decon0_vclk0_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK0_TOP, 12, 2),
	MUX(0, "top_mux_sclk_disp0_decon0_vclk1",
	    top_mux_sclk_disp0_decon0_vclk1_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK1_TOP, 12, 2),
	MUX(0, "top_mux_sclk_disp0_hdmi_audio", top_mux_sclk_disp0_hdmi_audio_p,
	    CLK_CON_MUX_SCLK_DISP0_HDMI_AUDIO_TOP, 12, 1),
	MUX(0, "top_mux_sclk_disp1_decon1_eclk0",
	    top_mux_sclk_disp1_decon1_eclk0_p,
	    CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK0_TOP, 12, 2),
	MUX(0, "top_mux_sclk_disp1_decon1_eclk1",
	    top_mux_sclk_disp1_decon1_eclk1_p,
	    CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK1_TOP, 12, 2),
	MUX(0, "top_mux_sclk_fsys0_usbdrd30", top_mux_sclk_fsys0_usbdrd30_p,
	    CLK_CON_MUX_SCLK_FSYS0_USBDRD30, 12, 2),
	MUX(0, "top_mux_sclk_fsys0_mmc0", top_mux_sclk_fsys0_mmc0_p,
	    CLK_CON_MUX_SCLK_FSYS0_MMC0, 12, 2),
	MUX(0, "top_mux_sclk_fsys0_ufsunipro20",
	    top_mux_sclk_fsys0_ufsunipro20_p,
	    CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO20, 12, 2),
	MUX(0, "top_mux_sclk_fsys0_phy_24m", top_mux_sclk_fsys0_phy_24m_p,
	    CLK_CON_MUX_SCLK_FSYS0_PHY_24M, 12, 1),
	MUX(0, "top_mux_sclk_fsys0_ufsunipro_cfg",
	    top_mux_sclk_fsys0_ufsunipro_cfg_p,
	    CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO_CFG, 12, 2),
	MUX(0, "top_mux_sclk_fsys1_mmc2", top_mux_sclk_fsys1_mmc2_p,
	    CLK_CON_MUX_SCLK_FSYS1_MMC2, 12, 2),
	MUX(0, "top_mux_sclk_fsys1_ufsunipro20",
	    top_mux_sclk_fsys1_ufsunipro20_p,
	    CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO20, 12, 2),
	MUX(0, "top_mux_sclk_fsys1_pcie_phy", top_mux_sclk_fsys1_pcie_phy_p,
	    CLK_CON_MUX_SCLK_FSYS1_PCIE_PHY, 12, 1),
	MUX(0, "top_mux_sclk_fsys1_ufsunipro_cfg",
	    top_mux_sclk_fsys1_ufsunipro_cfg_p,
	    CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO_CFG, 12, 2),
	MUX(0, "top_mux_sclk_peric0_uart0", top_mux_sclk_peric0_uart0_p,
	    CLK_CON_MUX_SCLK_PERIC0_UART0, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi0", top_mux_sclk_peric1_spi0_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI0, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi1", top_mux_sclk_peric1_spi1_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI1, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi2", top_mux_sclk_peric1_spi2_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI2, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi3", top_mux_sclk_peric1_spi3_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI3, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi4", top_mux_sclk_peric1_spi4_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI4, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi5", top_mux_sclk_peric1_spi5_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI5, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi6", top_mux_sclk_peric1_spi6_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI6, 12, 2),
	MUX(0, "top_mux_sclk_peric1_spi7", top_mux_sclk_peric1_spi7_p,
	    CLK_CON_MUX_SCLK_PERIC1_SPI7, 12, 2),
	MUX(0, "top_mux_sclk_peric1_uart1", top_mux_sclk_peric1_uart1_p,
	    CLK_CON_MUX_SCLK_PERIC1_UART1, 12, 2),
	MUX(0, "top_mux_sclk_peric1_uart2", top_mux_sclk_peric1_uart2_p,
	    CLK_CON_MUX_SCLK_PERIC1_UART2, 12, 2),
	MUX(0, "top_mux_sclk_peric1_uart3", top_mux_sclk_peric1_uart3_p,
	    CLK_CON_MUX_SCLK_PERIC1_UART3, 12, 2),
	MUX(0, "top_mux_sclk_peric1_uart4", top_mux_sclk_peric1_uart4_p,
	    CLK_CON_MUX_SCLK_PERIC1_UART4, 12, 2),
	MUX(0, "top_mux_sclk_peric1_uart5", top_mux_sclk_peric1_uart5_p,
	    CLK_CON_MUX_SCLK_PERIC1_UART5, 12, 2),
	MUX(0, "top_mux_sclk_cam1_isp_spi0", top_mux_sclk_cam1_isp_spi0_p,
	    CLK_CON_MUX_SCLK_CAM1_ISP_SPI0, 12, 2),
	MUX(0, "top_mux_sclk_cam1_isp_spi1", top_mux_sclk_cam1_isp_spi1_p,
	    CLK_CON_MUX_SCLK_CAM1_ISP_SPI1, 12, 2),
	MUX(0, "top_mux_sclk_cam1_isp_uart", top_mux_sclk_cam1_isp_uart_p,
	    CLK_CON_MUX_SCLK_CAM1_ISP_UART, 12, 1),
	MUX(0, "top_mux_sclk_ap2cp_mif_pll_out",
	    top_mux_sclk_ap2cp_mif_pll_out_p,
	    CLK_CON_MUX_SCLK_AP2CP_MIF_PLL_OUT, 12, 2),
	MUX(0, "top_mux_aclk_pscdc_400", top_mux_aclk_pscdc_400_p,
	    CLK_CON_MUX_ACLK_PSCDC_400, 12, 2),
	MUX(0, "top_mux_sclk_bus_pll_mngs", top_mux_sclk_bus_pll_mngs_p,
	    CLK_CON_MUX_SCLK_BUS_PLL_MNGS, 12, 2),
	MUX(0, "top_mux_sclk_bus_pll_apollo", top_mux_sclk_bus_pll_apollo_p,
	    CLK_CON_MUX_SCLK_BUS_PLL_APOLLO, 12, 2),
	MUX(0, "top_mux_sclk_bus_pll_mif", top_mux_sclk_bus_pll_mif_p,
	    CLK_CON_MUX_SCLK_BUS_PLL_MIF, 12, 3),
	MUX(0, "top_mux_sclk_bus_pll_g3d", top_mux_sclk_bus_pll_g3d_p,
	    CLK_CON_MUX_SCLK_BUS_PLL_G3D, 12, 3),
	MUX(0, "top_mux_sclk_isp_sensor0", top_mux_sclk_isp_sensor0_p,
	    CLK_CON_MUX_SCLK_ISP_SENSOR0, 12, 3),
	MUX(0, "top_mux_sclk_isp_sensor1", top_mux_sclk_isp_sensor1_p,
	    CLK_CON_MUX_SCLK_ISP_SENSOR1, 12, 3),
	MUX(0, "top_mux_sclk_isp_sensor2", top_mux_sclk_isp_sensor2_p,
	    CLK_CON_MUX_SCLK_ISP_SENSOR2, 12, 3),
	MUX(0, "top_mux_sclk_isp_sensor3", top_mux_sclk_isp_sensor3_p,
	    CLK_CON_MUX_SCLK_ISP_SENSOR3, 12, 3),
	MUX(0, "top_mux_sclk_promise_int", top_mux_sclk_promise_int_p,
	    CLK_CON_MUX_SCLK_PROMISE_INT, 12, 2),
	MUX(0, "top_mux_sclk_promise_disp", top_mux_sclk_promise_disp_p,
	    CLK_CON_MUX_SCLK_PROMISE_DISP, 12, 2),
	MUX(0, "top_mux_cp2ap_mif_clk_user", top_mux_cp2ap_mif_clk_user_p,
	    CLK_CON_MUX_CP2AP_MIF_CLK_USER, 12, 1),
	MUX(0, "top_mux_mif_pll", top_mux_mif_pll_p, MIF_CLK_CTRL2, 12, 1),
	MUX(0, "top_mux_bus_pll_mif", top_mux_bus_pll_mif_p, MIF_CLK_CTRL3, 12,
	    1),
	MUX(0, "top_mux_aclk_mif_pll", top_mux_aclk_mif_pll_p, MIF_CLK_CTRL4,
	    12, 1),
};

static const struct samsung_gate_clock top_gate_clks[] __initconst = {
	GATE(0, "top_gate_aclk_ccore_800", "top_div_aclk_ccore_800",
	     CLK_ENABLE_ACLK_CCORE_800, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_ccore_264", "top_div_aclk_ccore_264",
	     CLK_ENABLE_ACLK_CCORE_264, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_ccore_g3d_800", "top_div_aclk_ccore_g3d_800",
	     CLK_ENABLE_ACLK_CCORE_G3D_800, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_ccore_528", "top_div_aclk_ccore_528",
	     CLK_ENABLE_ACLK_CCORE_528, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_ccore_132", "top_div_aclk_ccore_132",
	     CLK_ENABLE_ACLK_CCORE_132, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_pclk_ccore_66", "top_div_pclk_ccore_66",
	     CLK_ENABLE_PCLK_CCORE_66, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_bus0_528", "top_div_aclk_bus0_528",
	     CLK_ENABLE_ACLK_BUS0_528_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_bus0_200", "top_div_aclk_bus0_200",
	     CLK_ENABLE_ACLK_BUS0_200_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_pclk_bus0_132", "top_div_pclk_bus0_132",
	     CLK_ENABLE_PCLK_BUS0_132_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_bus1_528", "top_div_aclk_bus1_528",
	     CLK_ENABLE_ACLK_BUS1_528_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_pclk_bus1_132", "top_div_pclk_bus1_132",
	     CLK_ENABLE_PCLK_BUS1_132_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_disp0_0_400", "top_div_aclk_disp0_0_400",
	     CLK_ENABLE_ACLK_DISP0_0_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_disp0_1_400", "top_div_aclk_disp0_1_400",
	     CLK_ENABLE_ACLK_DISP0_1_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_disp1_0_400", "top_div_aclk_disp1_0_400",
	     CLK_ENABLE_ACLK_DISP1_0_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_disp1_1_400", "top_div_aclk_disp1_1_400",
	     CLK_ENABLE_ACLK_DISP1_1_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_mfc_600", "top_div_aclk_mfc_600",
	     CLK_ENABLE_ACLK_MFC_600, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_mscl0_528", "top_div_aclk_mscl0_528",
	     CLK_ENABLE_ACLK_MSCL0_528, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_mscl1_528", "top_div_aclk_mscl1_528",
	     CLK_ENABLE_ACLK_MSCL1_528, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_imem_266", "top_div_aclk_imem_266",
	     CLK_ENABLE_ACLK_IMEM_266, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_imem_200", "top_div_aclk_imem_200",
	     CLK_ENABLE_ACLK_IMEM_200, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_imem_100", "top_div_aclk_imem_100",
	     CLK_ENABLE_ACLK_IMEM_100, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_fsys0_200", "top_div_aclk_fsys0_200",
	     CLK_ENABLE_ACLK_FSYS0_200, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_fsys1_200", "top_div_aclk_fsys1_200",
	     CLK_ENABLE_ACLK_FSYS1_200, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_peris_66", "top_div_aclk_peris_66",
	     CLK_ENABLE_ACLK_PERIS_66, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_peric0_66", "top_div_aclk_peric0_66",
	     CLK_ENABLE_ACLK_PERIC0_66, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_peric1_66", "top_div_aclk_peric1_66",
	     CLK_ENABLE_ACLK_PERIC1_66, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_isp0_isp0_528", "top_div_aclk_isp0_isp0_528",
	     CLK_ENABLE_ACLK_ISP0_ISP0_528, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_isp0_tpu_400", "top_div_aclk_isp0_tpu_400",
	     CLK_ENABLE_ACLK_ISP0_TPU_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_isp0_trex_528", "top_div_aclk_isp0_trex_528",
	     CLK_ENABLE_ACLK_ISP0_TREX_528, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_isp0_pxl_asbs_is_c_from_is_d",
	     "top_div_aclk_isp0_pxl_asbs_is_c_from_is_d",
	     CLK_ENABLE_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_isp1_isp1_468", "top_div_aclk_isp1_isp1_468",
	     CLK_ENABLE_ACLK_ISP1_ISP1_468, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam0_csis0_414", "top_div_aclk_cam0_csis0_414",
	     CLK_ENABLE_ACLK_CAM0_CSIS1_414, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam0_csis1_168", "top_div_aclk_cam0_csis1_168",
	     CLK_ENABLE_ACLK_CAM0_CSIS1_168_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam0_csis2_234", "top_div_aclk_cam0_csis2_234",
	     CLK_ENABLE_ACLK_CAM0_CSIS2_234_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam0_3aa0_414", "top_div_aclk_cam0_3aa0_414",
	     CLK_ENABLE_ACLK_CAM0_3AA0_414_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam0_3aa1_414", "top_div_aclk_cam0_3aa1_414",
	     CLK_ENABLE_ACLK_CAM0_3AA1_414_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam0_csis3_132", "top_div_aclk_cam0_csis3_132",
	     CLK_ENABLE_ACLK_CAM0_CSIS3_132_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam0_trex_528", "top_div_aclk_cam0_trex_528",
	     CLK_ENABLE_ACLK_CAM0_TREX_528_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_arm_672", "top_div_aclk_cam1_arm_672",
	     CLK_ENABLE_ACLK_CAM1_ARM_672_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_trex_vra_528",
	     "top_div_aclk_cam1_trex_vra_528",
	     CLK_ENABLE_ACLK_CAM1_TREX_VRA_528_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_trex_b_528", "top_div_aclk_cam1_trex_b_528",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_bus_264", "top_div_aclk_cam1_bus_264",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_peri_84", "top_div_aclk_cam1_peri_84",
	     CLK_ENABLE_ACLK_CAM1_PERI_84, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_csis2_414", "top_div_aclk_cam1_csis2_414",
	     CLK_ENABLE_ACLK_CAM1_CSIS2_414_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_csis3_132", "top_div_aclk_cam1_csis3_132",
	     CLK_ENABLE_ACLK_CAM1_CSIS3_132_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_cam1_scl_566", "top_div_aclk_cam1_scl_566",
	     CLK_ENABLE_ACLK_CAM1_SCL_566_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_disp0_decon0_eclk0",
	     "top_div_sclk_disp0_decon0_eclk0",
	     CLK_ENABLE_SCLK_DISP0_DECON0_ECLK0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_disp0_decon0_vclk0",
	     "top_div_sclk_disp0_decon0_vclk0",
	     CLK_ENABLE_SCLK_DISP0_DECON0_VCLK0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_disp0_decon0_vclk1",
	     "top_div_sclk_disp0_decon0_vclk1",
	     CLK_ENABLE_SCLK_DISP0_DECON0_VCLK1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_disp0_hdmi_audio",
	     "top_div_sclk_disp0_hdmi_audio", CLK_ENABLE_SCLK_DISP0_HDMI_ADUIO,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_disp1_decon1_eclk0",
	     "top_div_sclk_disp1_decon1_eclk0",
	     CLK_ENABLE_SCLK_DISP1_DECON1_ECLK0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_disp1_decon1_eclk1",
	     "top_div_sclk_disp1_decon1_eclk1",
	     CLK_ENABLE_SCLK_DISP1_DECON1_ECLK1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys0_usbdrd30", "top_div_sclk_fsys0_usbdrd30",
	     CLK_ENABLE_SCLK_FSYS0_USBDRD30, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys0_mmc0", "top_div_sclk_fsys0_mmc0",
	     CLK_ENABLE_SCLK_FSYS0_MMC0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys0_ufsunipro20",
	     "top_div_sclk_fsys0_ufsunipro20",
	     CLK_ENABLE_SCLK_FSYS0_UFSUNIPRO20, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys0_phy_24m", "top_div_sclk_fsys0_phy_24m",
	     CLK_ENABLE_SCLK_FSYS0_PHY_24M, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys0_ufsunipro_cfg",
	     "top_div_sclk_fsys0_ufsunipro_cfg",
	     CLK_ENABLE_SCLK_FSYS0_UFSUNIPRO_CFG, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys1_mmc2", "top_div_sclk_fsys1_mmc2",
	     CLK_ENABLE_SCLK_FSYS1_MMC2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys1_ufsunipro20",
	     "top_div_sclk_fsys1_ufsunipro20",
	     CLK_ENABLE_SCLK_FSYS1_UFSUNIPRO20, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys1_pcie_phy", "top_div_sclk_fsys1_pcie_phy",
	     CLK_ENABLE_SCLK_FSYS1_PCIE_PHY, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_fsys1_ufsunipro_cfg",
	     "top_div_sclk_fsys1_ufsunipro_cfg",
	     CLK_ENABLE_SCLK_FSYS1_UFSUNIPRO_CFG, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric0_uart0", "top_div_sclk_peric0_uart0",
	     CLK_ENABLE_SCLK_PERIC0_UART0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi0", "top_div_sclk_peric1_spi0",
	     CLK_ENABLE_SCLK_PERIC1_SPI0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi1", "top_div_sclk_peric1_spi1",
	     CLK_ENABLE_SCLK_PERIC1_SPI1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi2", "top_div_sclk_peric1_spi2",
	     CLK_ENABLE_SCLK_PERIC1_SPI2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi3", "top_div_sclk_peric1_spi3",
	     CLK_ENABLE_SCLK_PERIC1_SPI3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi4", "top_div_sclk_peric1_spi4",
	     CLK_ENABLE_SCLK_PERIC1_SPI4, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi5", "top_div_sclk_peric1_spi5",
	     CLK_ENABLE_SCLK_PERIC1_SPI5, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi6", "top_div_sclk_peric1_spi6",
	     CLK_ENABLE_SCLK_PERIC1_SPI6, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_spi7", "top_div_sclk_peric1_spi7",
	     CLK_ENABLE_SCLK_PERIC1_SPI7, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_uart1", "top_div_sclk_peric1_uart1",
	     CLK_ENABLE_SCLK_PERIC1_UART1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_uart2", "top_div_sclk_peric1_uart2",
	     CLK_ENABLE_SCLK_PERIC1_UART2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_uart3", "top_div_sclk_peric1_uart3",
	     CLK_ENABLE_SCLK_PERIC1_UART3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_uart4", "top_div_sclk_peric1_uart4",
	     CLK_ENABLE_SCLK_PERIC1_UART4, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_peric1_uart5", "top_div_sclk_peric1_uart5",
	     CLK_ENABLE_SCLK_PERIC1_UART5, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_cam1_isp_spi0", "top_div_sclk_cam1_isp_spi0",
	     CLK_ENABLE_SCLK_CAM1_ISP_SPI0_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_cam1_isp_spi1", "top_div_sclk_cam1_isp_spi1",
	     CLK_ENABLE_SCLK_CAM1_ISP_SPI1_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_cam1_isp_uart", "top_div_sclk_cam1_isp_uart",
	     CLK_ENABLE_SCLK_CAM1_ISP_UART_TOP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_ap2cp_mif_pll_out",
	     "top_div_sclk_ap2cp_mif_pll_out",
	     CLK_ENABLE_SCLK_AP2CP_MIF_PLL_OUT, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_aclk_pscdc_400", "top_div_aclk_pscdc_400",
	     CLK_ENABLE_ACLK_PSCDC_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_bus_pll_mngs", "top_div_sclk_bus_pll_mngs",
	     CLK_ENABLE_SCLK_BUS_PLL_MNGS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_bus_pll_apollo", "top_div_sclk_bus_pll_apollo",
	     CLK_ENABLE_SCLK_BUS_PLL_APOLLO, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_bus_pll_mif", "top_mux_sclk_bus_pll_mif",
	     CLK_ENABLE_SCLK_BUS_PLL_MIF, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_bus_pll_g3d", "top_div_sclk_bus_pll_g3d",
	     CLK_ENABLE_SCLK_BUS_PLL_G3D, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_isp_sensor0", "top_div_sclk_isp_sensor0",
	     CLK_ENABLE_SCLK_ISP_SENSOR0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_isp_sensor1", "top_div_sclk_isp_sensor1",
	     CLK_ENABLE_SCLK_ISP_SENSOR1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_isp_sensor2", "top_div_sclk_isp_sensor2",
	     CLK_ENABLE_SCLK_ISP_SENSOR2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_isp_sensor3", "top_div_sclk_isp_sensor3",
	     CLK_ENABLE_SCLK_ISP_SENSOR3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_promise_int", "top_div_sclk_promise_int",
	     CLK_ENABLE_SCLK_PROMISE_INT, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_gate_sclk_promise_disp", "top_div_sclk_promise_disp",
	     CLK_ENABLE_SCLK_PROMISE_DISP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_bus0_pll", "top_mux_bus0_pll",
	     CLK_CON_MUX_BUS0_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_bus1_pll", "top_mux_bus1_pll",
	     CLK_CON_MUX_BUS1_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_bus2_pll", "top_mux_bus2_pll",
	     CLK_CON_MUX_BUS2_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_bus3_pll", "top_mux_bus3_pll",
	     CLK_CON_MUX_BUS3_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_mfc_pll", "top_mux_mfc_pll", CLK_CON_MUX_MFC_PLL,
	     21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_isp_pll", "top_mux_isp_pll", CLK_CON_MUX_ISP_PLL,
	     21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aud_pll", "top_mux_aud_pll", CLK_CON_MUX_AUD_PLL,
	     21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_g3d_pll", "top_mux_g3d_pll", CLK_CON_MUX_G3D_PLL,
	     21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus0_pll", "top_mux_sclk_bus0_pll",
	     CLK_CON_MUX_SCLK_BUS0_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus1_pll", "top_mux_sclk_bus1_pll",
	     CLK_CON_MUX_SCLK_BUS1_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus2_pll", "top_mux_sclk_bus2_pll",
	     CLK_CON_MUX_SCLK_BUS2_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus3_pll", "top_mux_sclk_bus3_pll",
	     CLK_CON_MUX_SCLK_BUS3_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_mfc_pll", "top_mux_sclk_mfc_pll",
	     CLK_CON_MUX_SCLK_MFC_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_isp_pll", "top_mux_sclk_isp_pll",
	     CLK_CON_MUX_SCLK_ISP_PLL, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_ccore_800", "top_mux_aclk_ccore_800",
	     CLK_CON_MUX_ACLK_CCORE_800, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_ccore_264", "top_mux_aclk_ccore_264",
	     CLK_CON_MUX_ACLK_CCORE_264, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_ccore_g3d_800", "top_mux_aclk_ccore_g3d_800",
	     CLK_CON_MUX_ACLK_CCORE_G3D_800, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_ccore_528", "top_mux_aclk_ccore_528",
	     CLK_CON_MUX_ACLK_CCORE_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_ccore_132", "top_mux_aclk_ccore_132",
	     CLK_CON_MUX_ACLK_CCORE_132, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_pclk_ccore_66", "top_mux_pclk_ccore_66",
	     CLK_CON_MUX_PCLK_CCORE_66, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_bus0_528", "top_mux_aclk_bus0_528",
	     CLK_CON_MUX_ACLK_BUS0_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_bus0_200", "top_mux_aclk_bus0_200",
	     CLK_CON_MUX_ACLK_BUS0_200, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_pclk_bus0_132", "top_mux_pclk_bus0_132",
	     CLK_CON_MUX_PCLK_BUS0_132, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_bus1_528", "top_mux_aclk_bus1_528",
	     CLK_CON_MUX_ACLK_BUS1_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_pclk_bus1_132", "top_mux_pclk_bus1_132",
	     CLK_CON_MUX_PCLK_BUS1_132, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_disp0_0_400", "top_mux_aclk_disp0_0_400",
	     CLK_CON_MUX_ACLK_DISP0_0_400, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_disp0_1_400", "top_mux_aclk_disp0_1_400",
	     CLK_CON_MUX_ACLK_DISP0_1_400_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_disp1_0_400", "top_mux_aclk_disp1_0_400",
	     CLK_CON_MUX_ACLK_DISP1_0_400, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_disp1_1_400", "top_mux_aclk_disp1_1_400",
	     CLK_CON_MUX_ACLK_DISP1_1_400_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_mfc_600", "top_mux_aclk_mfc_600",
	     CLK_CON_MUX_ACLK_MFC_600, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_mscl0_528", "top_mux_aclk_mscl0_528",
	     CLK_CON_MUX_ACLK_MSCL0_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_mscl1_528", "top_mux_aclk_mscl1_528",
	     CLK_CON_MUX_ACLK_MSCL1_528_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_imem_266", "top_mux_aclk_imem_266",
	     CLK_CON_MUX_ACLK_IMEM_266, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_imem_200", "top_mux_aclk_imem_200",
	     CLK_CON_MUX_ACLK_IMEM_200, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_imem_100", "top_mux_aclk_imem_100",
	     CLK_CON_MUX_ACLK_IMEM_100, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_fsys0_200", "top_mux_aclk_fsys0_200",
	     CLK_CON_MUX_ACLK_FSYS0_200, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_fsys1_200", "top_mux_aclk_fsys1_200",
	     CLK_CON_MUX_ACLK_FSYS1_200, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_peris_66", "top_mux_aclk_peris_66",
	     CLK_CON_MUX_ACLK_PERIS_66, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_peric0_66", "top_mux_aclk_peric0_66",
	     CLK_CON_MUX_ACLK_PERIC0_66, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_peric1_66", "top_mux_aclk_peric1_66",
	     CLK_CON_MUX_ACLK_PERIC1_66, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_isp0_isp0_528", "top_mux_aclk_isp0_isp0_528",
	     CLK_CON_MUX_ACLK_ISP0_ISP0_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_isp0_tpu_400", "top_mux_aclk_isp0_tpu_400",
	     CLK_CON_MUX_ACLK_ISP0_TPU_400, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_isp0_trex_528", "top_mux_aclk_isp0_trex_528",
	     CLK_CON_MUX_ACLK_ISP0_TREX_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_isp0_pxl_asbs_is_c_from_is_d",
	     "top_mux_aclk_isp0_pxl_asbs_is_c_from_is_d",
	     CLK_CON_MUX_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_isp1_isp1_468", "top_mux_aclk_isp1_isp1_468",
	     CLK_CON_MUX_ACLK_ISP1_ISP1_468, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam0_csis0_414",
	     "top_mux_aclk_cam0_csis0_414", CLK_CON_MUX_ACLK_CAM0_CSIS0_414, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam0_csis1_168",
	     "top_mux_aclk_cam0_csis1_168", CLK_CON_MUX_ACLK_CAM0_CSIS1_168, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam0_csis2_234",
	     "top_mux_aclk_cam0_csis2_234", CLK_CON_MUX_ACLK_CAM0_CSIS2_234, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam0_3aa0_414", "top_mux_aclk_cam0_3aa0_414",
	     CLK_CON_MUX_ACLK_CAM0_3AA0_414, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam0_3aa1_414", "top_mux_aclk_cam0_3aa1_414",
	     CLK_CON_MUX_ACLK_CAM0_3AA1_414, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam0_csis3_132",
	     "top_mux_aclk_cam0_csis3_132", CLK_CON_MUX_ACLK_CAM0_CSIS3_132, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam0_trex_528", "top_mux_aclk_cam0_trex_528",
	     CLK_CON_MUX_ACLK_CAM0_TREX_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_arm_672", "top_mux_aclk_cam1_arm_672",
	     CLK_CON_MUX_ACLK_CAM1_ARM_672, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_trex_vra_528",
	     "top_mux_aclk_cam1_trex_vra_528",
	     CLK_CON_MUX_ACLK_CAM1_TREX_VRA_528, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_trex_b_528",
	     "top_mux_aclk_cam1_trex_b_528", CLK_CON_MUX_ACLK_CAM1_TREX_B_528,
	     21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_bus_264", "top_mux_aclk_cam1_bus_264",
	     CLK_CON_MUX_ACLK_CAM1_BUS_264, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_peri_84", "top_mux_aclk_cam1_peri_84",
	     CLK_CON_MUX_ACLK_CAM1_PERI_84, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_csis2_414",
	     "top_mux_aclk_cam1_csis2_414", CLK_CON_MUX_ACLK_CAM1_CSIS2_414, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_csis3_132",
	     "top_mux_aclk_cam1_csis3_132", CLK_CON_MUX_ACLK_CAM1_CSIS3_132, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_cam1_scl_566", "top_mux_aclk_cam1_scl_566",
	     CLK_CON_MUX_ACLK_CAM1_SCL_566, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_disp0_decon0_eclk0",
	     "top_mux_sclk_disp0_decon0_eclk0",
	     CLK_CON_MUX_SCLK_DISP0_DECON0_ECLK0_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_disp0_decon0_vclk0",
	     "top_mux_sclk_disp0_decon0_vclk0",
	     CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK0_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_disp0_decon0_vclk1",
	     "top_mux_sclk_disp0_decon0_vclk1",
	     CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK1_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_disp0_hdmi_audio",
	     "top_mux_sclk_disp0_hdmi_audio",
	     CLK_CON_MUX_SCLK_DISP0_HDMI_AUDIO_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_disp1_decon1_eclk0",
	     "top_mux_sclk_disp1_decon1_eclk0",
	     CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK0_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_disp1_decon1_eclk1",
	     "top_mux_sclk_disp1_decon1_eclk1",
	     CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK1_TOP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys0_usbdrd30",
	     "top_mux_sclk_fsys0_usbdrd30", CLK_CON_MUX_SCLK_FSYS0_USBDRD30, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys0_mmc0", "top_mux_sclk_fsys0_mmc0",
	     CLK_CON_MUX_SCLK_FSYS0_MMC0, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys0_ufsunipro20",
	     "top_mux_sclk_fsys0_ufsunipro20",
	     CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO20, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys0_phy_24m", "top_mux_sclk_fsys0_phy_24m",
	     CLK_CON_MUX_SCLK_FSYS0_PHY_24M, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys0_ufsunipro_cfg",
	     "top_mux_sclk_fsys0_ufsunipro_cfg",
	     CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO_CFG, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys1_mmc2", "top_mux_sclk_fsys1_mmc2",
	     CLK_CON_MUX_SCLK_FSYS1_MMC2, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys1_ufsunipro20",
	     "top_mux_sclk_fsys1_ufsunipro20",
	     CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO20, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys1_pcie_phy",
	     "top_mux_sclk_fsys1_pcie_phy", CLK_CON_MUX_SCLK_FSYS1_PCIE_PHY, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_fsys1_ufsunipro_cfg",
	     "top_mux_sclk_fsys1_ufsunipro_cfg",
	     CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO_CFG, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric0_uart0", "top_mux_sclk_peric0_uart0",
	     CLK_CON_MUX_SCLK_PERIC0_UART0, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi0", "top_mux_sclk_peric1_spi0",
	     CLK_CON_MUX_SCLK_PERIC1_SPI0, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi1", "top_mux_sclk_peric1_spi1",
	     CLK_CON_MUX_SCLK_PERIC1_SPI1, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi2", "top_mux_sclk_peric1_spi2",
	     CLK_CON_MUX_SCLK_PERIC1_SPI2, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi3", "top_mux_sclk_peric1_spi3",
	     CLK_CON_MUX_SCLK_PERIC1_SPI3, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi4", "top_mux_sclk_peric1_spi4",
	     CLK_CON_MUX_SCLK_PERIC1_SPI4, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi5", "top_mux_sclk_peric1_spi5",
	     CLK_CON_MUX_SCLK_PERIC1_SPI5, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi6", "top_mux_sclk_peric1_spi6",
	     CLK_CON_MUX_SCLK_PERIC1_SPI6, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_spi7", "top_mux_sclk_peric1_spi7",
	     CLK_CON_MUX_SCLK_PERIC1_SPI7, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_uart1", "top_mux_sclk_peric1_uart1",
	     CLK_CON_MUX_SCLK_PERIC1_UART1, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_uart2", "top_mux_sclk_peric1_uart2",
	     CLK_CON_MUX_SCLK_PERIC1_UART2, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_uart3", "top_mux_sclk_peric1_uart3",
	     CLK_CON_MUX_SCLK_PERIC1_UART3, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_uart4", "top_mux_sclk_peric1_uart4",
	     CLK_CON_MUX_SCLK_PERIC1_UART4, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_peric1_uart5", "top_mux_sclk_peric1_uart5",
	     CLK_CON_MUX_SCLK_PERIC1_UART5, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_cam1_isp_spi0", "top_mux_sclk_cam1_isp_spi0",
	     CLK_CON_MUX_SCLK_CAM1_ISP_SPI0, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_cam1_isp_spi1", "top_mux_sclk_cam1_isp_spi1",
	     CLK_CON_MUX_SCLK_CAM1_ISP_SPI1, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_cam1_isp_uart", "top_mux_sclk_cam1_isp_uart",
	     CLK_CON_MUX_SCLK_CAM1_ISP_UART, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_ap2cp_mif_pll_out",
	     "top_mux_sclk_ap2cp_mif_pll_out",
	     CLK_CON_MUX_SCLK_AP2CP_MIF_PLL_OUT, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_aclk_pscdc_400", "top_mux_aclk_pscdc_400",
	     CLK_CON_MUX_ACLK_PSCDC_400, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus_pll_mngs", "top_mux_sclk_bus_pll_mngs",
	     CLK_CON_MUX_SCLK_BUS_PLL_MNGS, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus_pll_apollo",
	     "top_mux_sclk_bus_pll_apollo", CLK_CON_MUX_SCLK_BUS_PLL_APOLLO, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus_pll_mif", "top_mux_sclk_bus_pll_mif",
	     CLK_CON_MUX_SCLK_BUS_PLL_MIF, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_bus_pll_g3d", "top_mux_sclk_bus_pll_g3d",
	     CLK_CON_MUX_SCLK_BUS_PLL_G3D, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_isp_sensor0", "top_mux_sclk_isp_sensor0",
	     CLK_CON_MUX_SCLK_ISP_SENSOR0, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_isp_sensor1", "top_mux_sclk_isp_sensor1",
	     CLK_CON_MUX_SCLK_ISP_SENSOR1, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_isp_sensor2", "top_mux_sclk_isp_sensor2",
	     CLK_CON_MUX_SCLK_ISP_SENSOR2, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_isp_sensor3", "top_mux_sclk_isp_sensor3",
	     CLK_CON_MUX_SCLK_ISP_SENSOR3, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_promise_int", "top_mux_sclk_promise_int",
	     CLK_CON_MUX_SCLK_PROMISE_INT, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_sclk_promise_disp", "top_mux_sclk_promise_disp",
	     CLK_CON_MUX_SCLK_PROMISE_DISP, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "top_muxgate_cp2ap_mif_clk_user", "top_mux_cp2ap_mif_clk_user",
	     CLK_CON_MUX_CP2AP_MIF_CLK_USER, 21, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock top_div_clks[] __initconst = {
	DIV(0, "top_div_aclk_ccore_800", "top_mux_aclk_ccore_800",
	    CLK_CON_DIV_ACLK_CCORE_800, 0, 4),
	DIV(0, "top_div_aclk_ccore_264", "top_mux_aclk_ccore_264",
	    CLK_CON_DIV_ACLK_CCORE_264, 0, 4),
	DIV(0, "top_div_aclk_ccore_g3d_800", "top_mux_aclk_ccore_g3d_800",
	    CLK_CON_DIV_ACLK_CCORE_G3D_800, 0, 4),
	DIV(0, "top_div_aclk_ccore_528", "top_mux_aclk_ccore_528",
	    CLK_CON_DIV_ACLK_CCORE_528, 0, 4),
	DIV(0, "top_div_aclk_ccore_132", "top_mux_aclk_ccore_132",
	    CLK_CON_DIV_ACLK_CCORE_132, 0, 4),
	DIV(0, "top_div_pclk_ccore_66", "top_mux_pclk_ccore_66",
	    CLK_CON_DIV_PCLK_CCORE_66, 0, 4),
	DIV(0, "top_div_aclk_bus0_528", "top_mux_aclk_bus0_528",
	    CLK_CON_DIV_ACLK_BUS0_528, 0, 4),
	DIV(0, "top_div_aclk_bus0_200", "top_mux_aclk_bus0_200",
	    CLK_CON_DIV_ACLK_BUS0_200, 0, 4),
	DIV(0, "top_div_pclk_bus0_132", "top_mux_pclk_bus0_132",
	    CLK_CON_DIV_PCLK_BUS0_132, 0, 4),
	DIV(0, "top_div_aclk_bus1_528", "top_mux_aclk_bus1_528",
	    CLK_CON_DIV_ACLK_BUS1_528, 0, 4),
	DIV(0, "top_div_pclk_bus1_132", "top_mux_pclk_bus1_132",
	    CLK_CON_DIV_PCLK_BUS1_132, 0, 4),
	DIV(0, "top_div_aclk_disp0_0_400", "top_mux_aclk_disp0_0_400",
	    CLK_CON_DIV_ACLK_DISP0_0_400, 0, 4),
	DIV(0, "top_div_aclk_disp0_1_400", "top_mux_aclk_disp0_1_400",
	    CLK_CON_DIV_ACLK_DISP0_1_400, 0, 4),
	DIV(0, "top_div_aclk_disp1_0_400", "top_mux_aclk_disp1_0_400",
	    CLK_CON_DIV_ACLK_DISP1_0_400, 0, 4),
	DIV(0, "top_div_aclk_disp1_1_400", "top_mux_aclk_disp1_1_400",
	    CLK_CON_DIV_ACLK_DISP1_1_400, 0, 4),
	DIV(0, "top_div_aclk_mfc_600", "top_mux_aclk_mfc_600",
	    CLK_CON_DIV_ACLK_MFC_600, 0, 4),
	DIV(0, "top_div_aclk_mscl0_528", "top_mux_aclk_mscl0_528",
	    CLK_CON_DIV_ACLK_MSCL0_528, 0, 4),
	DIV(0, "top_div_aclk_mscl1_528", "top_mux_aclk_mscl1_528",
	    CLK_CON_DIV_ACLK_MSCL1_528, 0, 4),
	DIV(0, "top_div_aclk_imem_266", "top_mux_aclk_imem_266",
	    CLK_CON_DIV_ACLK_IMEM_266, 0, 4),
	DIV(0, "top_div_aclk_imem_200", "top_mux_aclk_imem_200",
	    CLK_CON_DIV_ACLK_IMEM_200, 0, 4),
	DIV(0, "top_div_aclk_imem_100", "top_mux_aclk_imem_100",
	    CLK_CON_DIV_ACLK_IMEM_100, 0, 4),
	DIV(0, "top_div_aclk_fsys0_200", "top_mux_aclk_fsys0_200",
	    CLK_CON_DIV_ACLK_FSYS0_200, 0, 4),
	DIV(0, "top_div_aclk_fsys1_200", "top_mux_aclk_fsys1_200",
	    CLK_CON_DIV_ACLK_FSYS1_200, 0, 4),
	DIV(0, "top_div_aclk_peris_66", "top_mux_aclk_peris_66",
	    CLK_CON_DIV_ACLK_PERIS_66, 0, 4),
	DIV(0, "top_div_aclk_peric0_66", "top_mux_aclk_peric0_66",
	    CLK_CON_DIV_ACLK_PERIC0_66, 0, 4),
	DIV(0, "top_div_aclk_peric1_66", "top_mux_aclk_peric1_66",
	    CLK_CON_DIV_ACLK_PERIC1_66, 0, 4),
	DIV(0, "top_div_aclk_isp0_isp0_528", "top_mux_aclk_isp0_isp0_528",
	    CLK_CON_DIV_ACLK_ISP0_ISP0_528, 0, 4),
	DIV(0, "top_div_aclk_isp0_tpu_400", "top_mux_aclk_isp0_tpu_400",
	    CLK_CON_DIV_ACLK_ISP0_TPU_400, 0, 4),
	DIV(0, "top_div_aclk_isp0_trex_528", "top_mux_aclk_isp0_trex_528",
	    CLK_CON_DIV_ACLK_ISP0_TREX_528, 0, 4),
	DIV(0, "top_div_aclk_isp0_pxl_asbs_is_c_from_is_d",
	    "top_mux_aclk_isp0_pxl_asbs_is_c_from_is_d",
	    CLK_CON_DIV_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_TOP, 0, 4),
	DIV(0, "top_div_aclk_isp1_isp1_468", "top_mux_aclk_isp1_isp1_468",
	    CLK_CON_DIV_ACLK_ISP1_ISP1_468, 0, 4),
	DIV(0, "top_div_aclk_cam0_csis0_414", "top_mux_aclk_cam0_csis0_414",
	    CLK_CON_DIV_ACLK_CAM0_CSIS0_414, 0, 4),
	DIV(0, "top_div_aclk_cam0_csis1_168", "top_mux_aclk_cam0_csis1_168",
	    CLK_CON_DIV_ACLK_CAM0_CSIS1_168, 0, 4),
	DIV(0, "top_div_aclk_cam0_csis2_234", "top_mux_aclk_cam0_csis2_234",
	    CLK_CON_DIV_ACLK_CAM0_CSIS2_234, 0, 4),
	DIV(0, "top_div_aclk_cam0_3aa0_414", "top_mux_aclk_cam0_3aa0_414",
	    CLK_CON_DIV_ACLK_CAM0_3AA0_414, 0, 4),
	DIV(0, "top_div_aclk_cam0_3aa1_414", "top_mux_aclk_cam0_3aa1_414",
	    CLK_CON_DIV_ACLK_CAM0_3AA1_414, 0, 4),
	DIV(0, "top_div_aclk_cam0_csis3_132", "top_mux_aclk_cam0_csis3_132",
	    CLK_CON_DIV_ACLK_CAM0_CSIS3_132, 0, 4),
	DIV(0, "top_div_aclk_cam0_trex_528", "top_mux_aclk_cam0_trex_528",
	    CLK_CON_DIV_ACLK_CAM0_TREX_528, 0, 4),
	DIV(0, "top_div_aclk_cam1_arm_672", "top_mux_aclk_cam1_arm_672",
	    CLK_CON_DIV_ACLK_CAM1_ARM_672, 0, 4),
	DIV(0, "top_div_aclk_cam1_trex_vra_528",
	    "top_mux_aclk_cam1_trex_vra_528",
	    CLK_CON_DIV_ACLK_CAM1_TREX_VRA_528, 0, 4),
	DIV(0, "top_div_aclk_cam1_trex_b_528", "top_mux_aclk_cam1_trex_b_528",
	    CLK_CON_DIV_ACLK_CAM1_TREX_B_528, 0, 4),
	DIV(0, "top_div_aclk_cam1_bus_264", "top_mux_aclk_cam1_bus_264",
	    CLK_CON_DIV_ACLK_CAM1_BUS_264, 0, 4),
	DIV(0, "top_div_aclk_cam1_peri_84", "top_mux_aclk_cam1_peri_84",
	    CLK_CON_DIV_ACLK_CAM1_PERI_84, 0, 4),
	DIV(0, "top_div_aclk_cam1_csis2_414", "top_mux_aclk_cam1_csis2_414",
	    CLK_CON_DIV_ACLK_CAM1_CSIS2_414, 0, 8),
	DIV(0, "top_div_aclk_cam1_csis3_132", "top_mux_aclk_cam1_csis3_132",
	    CLK_CON_DIV_ACLK_CAM1_CSIS3_132, 0, 4),
	DIV(0, "top_div_aclk_cam1_scl_566", "top_mux_aclk_cam1_scl_566",
	    CLK_CON_DIV_ACLK_CAM1_SCL_566, 0, 4),
	DIV(0, "top_div_sclk_disp0_decon0_eclk0",
	    "top_mux_sclk_disp0_decon0_eclk0",
	    CLK_CON_DIV_SCLK_DISP0_DECON0_ECLK0, 0, 4),
	DIV(0, "top_div_sclk_disp0_decon0_vclk0",
	    "top_mux_sclk_disp0_decon0_vclk0",
	    CLK_CON_DIV_SCLK_DISP0_DECON0_VCLK0, 0, 4),
	DIV(0, "top_div_sclk_disp0_decon0_vclk1",
	    "top_mux_sclk_disp0_decon0_vclk1",
	    CLK_CON_DIV_SCLK_DISP0_DECON0_VCLK1, 0, 4),
	DIV(0, "top_div_sclk_disp0_hdmi_audio", "top_mux_sclk_disp0_hdmi_audio",
	    CLK_CON_DIV_SCLK_DISP0_HDMI_AUDIO, 0, 4),
	DIV(0, "top_div_sclk_disp1_decon1_eclk0",
	    "top_mux_sclk_disp1_decon1_eclk0",
	    CLK_CON_DIV_SCLK_DISP1_DECON1_ECLK0, 0, 4),
	DIV(0, "top_div_sclk_disp1_decon1_eclk1",
	    "top_mux_sclk_disp1_decon1_eclk1",
	    CLK_CON_DIV_SCLK_DISP1_DECON1_ECLK1, 0, 4),
	DIV(0, "top_div_sclk_fsys0_usbdrd30", "top_mux_sclk_fsys0_usbdrd30",
	    CLK_CON_DIV_SCLK_FSYS0_USBDRD30, 0, 4),
	DIV(0, "top_div_sclk_fsys0_mmc0", "top_mux_sclk_fsys0_mmc0",
	    CLK_CON_DIV_SCLK_FSYS0_MMC0, 0, 10),
	DIV(0, "top_div_sclk_fsys0_ufsunipro20",
	    "top_mux_sclk_fsys0_ufsunipro20",
	    CLK_CON_DIV_SCLK_FSYS0_UFSUNIPRO20, 0, 6),
	DIV(0, "top_div_sclk_fsys0_phy_24m", "top_mux_sclk_fsys0_phy_24m",
	    CLK_CON_DIV_SCLK_FSYS0_PHY_24M, 0, 6),
	DIV(0, "top_div_sclk_fsys0_ufsunipro_cfg",
	    "top_mux_sclk_fsys0_ufsunipro_cfg",
	    CLK_CON_DIV_SCLK_FSYS0_UFSUNIPRO_CFG, 0, 4),
	DIV(0, "top_div_sclk_fsys1_mmc2", "top_mux_sclk_fsys1_mmc2",
	    CLK_CON_DIV_SCLK_FSYS1_MMC2, 0, 10),
	DIV(0, "top_div_sclk_fsys1_ufsunipro20",
	    "top_mux_sclk_fsys1_ufsunipro20",
	    CLK_CON_DIV_SCLK_FSYS1_UFSUNIPRO20, 0, 6),
	DIV(0, "top_div_sclk_fsys1_pcie_phy", "top_mux_sclk_fsys1_pcie_phy",
	    CLK_CON_DIV_SCLK_FSYS1_PCIE_PHY, 0, 6),
	DIV(0, "top_div_sclk_fsys1_ufsunipro_cfg",
	    "top_mux_sclk_fsys1_ufsunipro_cfg",
	    CLK_CON_DIV_SCLK_FSYS1_UFSUNIPRO_CFG, 0, 4),
	DIV(0, "top_div_sclk_peric0_uart0", "top_mux_sclk_peric0_uart0",
	    CLK_CON_DIV_SCLK_PERIC0_UART0, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi0", "top_mux_sclk_peric1_spi0",
	    CLK_CON_DIV_SCLK_PERIC1_SPI0, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi1", "top_mux_sclk_peric1_spi1",
	    CLK_CON_DIV_SCLK_PERIC1_SPI1, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi2", "top_mux_sclk_peric1_spi2",
	    CLK_CON_DIV_SCLK_PERIC1_SPI2, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi3", "top_mux_sclk_peric1_spi3",
	    CLK_CON_DIV_SCLK_PERIC1_SPI3, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi4", "top_mux_sclk_peric1_spi4",
	    CLK_CON_DIV_SCLK_PERIC1_SPI4, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi5", "top_mux_sclk_peric1_spi5",
	    CLK_CON_DIV_SCLK_PERIC1_SPI5, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi6", "top_mux_sclk_peric1_spi6",
	    CLK_CON_DIV_SCLK_PERIC1_SPI6, 0, 4),
	DIV(0, "top_div_sclk_peric1_spi7", "top_mux_sclk_peric1_spi7",
	    CLK_CON_DIV_SCLK_PERIC1_SPI7, 0, 4),
	DIV(0, "top_div_sclk_peric1_uart1", "top_mux_sclk_peric1_uart1",
	    CLK_CON_DIV_SCLK_PERIC1_UART1, 0, 4),
	DIV(0, "top_div_sclk_peric1_uart2", "top_mux_sclk_peric1_uart2",
	    CLK_CON_DIV_SCLK_PERIC1_UART2, 0, 4),
	DIV(0, "top_div_sclk_peric1_uart3", "top_mux_sclk_peric1_uart3",
	    CLK_CON_DIV_SCLK_PERIC1_UART3, 0, 4),
	DIV(0, "top_div_sclk_peric1_uart4", "top_mux_sclk_peric1_uart4",
	    CLK_CON_DIV_SCLK_PERIC1_UART4, 0, 4),
	DIV(0, "top_div_sclk_peric1_uart5", "top_mux_sclk_peric1_uart5",
	    CLK_CON_DIV_SCLK_PERIC1_UART5, 0, 4),
	DIV(0, "top_div_sclk_cam1_isp_spi0", "top_mux_sclk_cam1_isp_spi0",
	    CLK_CON_DIV_SCLK_CAM1_ISP_SPI0, 0, 4),
	DIV(0, "top_div_sclk_cam1_isp_spi1", "top_mux_sclk_cam1_isp_spi1",
	    CLK_CON_DIV_SCLK_CAM1_ISP_SPI1, 0, 4),
	DIV(0, "top_div_sclk_cam1_isp_uart", "top_mux_sclk_cam1_isp_uart",
	    CLK_CON_DIV_SCLK_CAM1_ISP_UART, 0, 4),
	DIV(0, "top_div_sclk_ap2cp_mif_pll_out",
	    "top_mux_sclk_ap2cp_mif_pll_out",
	    CLK_CON_DIV_SCLK_AP2CP_MIF_PLL_OUT, 0, 4),
	DIV(0, "top_div_aclk_pscdc_400", "top_mux_aclk_pscdc_400",
	    CLK_CON_DIV_ACLK_PSCDC_400, 0, 4),
	DIV(0, "top_div_sclk_bus_pll_mngs", "top_mux_sclk_bus_pll_mngs",
	    CLK_CON_DIV_SCLK_BUS_PLL_MNGS, 0, 4),
	DIV(0, "top_div_sclk_bus_pll_apollo", "top_mux_sclk_bus_pll_apollo",
	    CLK_CON_DIV_SCLK_BUS_PLL_APOLLO, 0, 4),
	DIV(0, "top_div_sclk_bus_pll_mif", "top_mux_sclk_bus_pll_mif",
	    CLK_CON_DIV_SCLK_BUS_PLL_MIF, 0, 4),
	DIV(0, "top_div_sclk_bus_pll_g3d", "top_mux_sclk_bus_pll_g3d",
	    CLK_CON_DIV_SCLK_BUS_PLL_G3D, 0, 4),
	DIV(0, "top_div_sclk_isp_sensor0", "top_mux_sclk_isp_sensor0",
	    CLK_CON_DIV_SCLK_ISP_SENSOR0, 0, 8),
	DIV(0, "top_div_sclk_isp_sensor1", "top_mux_sclk_isp_sensor1",
	    CLK_CON_DIV_SCLK_ISP_SENSOR1, 0, 8),
	DIV(0, "top_div_sclk_isp_sensor2", "top_mux_sclk_isp_sensor2",
	    CLK_CON_DIV_SCLK_ISP_SENSOR2, 0, 8),
	DIV(0, "top_div_sclk_isp_sensor3", "top_mux_sclk_isp_sensor3",
	    CLK_CON_DIV_SCLK_ISP_SENSOR3, 0, 8),
	DIV(0, "top_div_sclk_promise_int", "top_mux_sclk_promise_int",
	    CLK_CON_DIV_SCLK_PROMISE_INT, 0, 4),
	DIV(0, "top_div_sclk_promise_disp", "top_mux_sclk_promise_disp",
	    CLK_CON_DIV_SCLK_PROMISE_DISP, 0, 4),
};

static const struct samsung_cmu_info top_cmu_info __initconst = {
	.pll_clks = top_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(top_pll_clks),
	.mux_clks = top_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(top_mux_clks),
	.div_clks = top_div_clks,
	.nr_div_clks = ARRAY_SIZE(top_div_clks),
	.gate_clks = top_gate_clks,
	.fixed_clks = top_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(top_fixed_clks),
	.fixed_factor_clks = top_fixed_factor_clks,
	.nr_fixed_factor_clks = ARRAY_SIZE(top_fixed_factor_clks),
	.nr_gate_clks = ARRAY_SIZE(top_gate_clks),
	.nr_clk_ids = TOP_NR_CLK,
};

/* Register Offset definitions for CMU_CCORE (0x105B0000) */
#define CLK_CON_MUX_ACLK_CCORE_800_USER 0x0200
#define CLK_CON_MUX_ACLK_CCORE_264_USER 0x0204
#define CLK_CON_MUX_ACLK_CCORE_G3D_800_USER 0x0208
#define CLK_CON_MUX_ACLK_CCORE_528_USER 0x020C
#define CLK_CON_MUX_ACLK_CCORE_132_USER 0x0210
#define CLK_CON_MUX_PCLK_CCORE_66_USER 0x0214
#define CLK_CON_DIV_SCLK_HPM_CCORE 0x0400
#define CLK_STAT_MUX_ACLK_CCORE_800_USER 0x0500
#define CLK_STAT_MUX_ACLK_CCORE_264_USER 0x0504
#define CLK_STAT_MUX_ACLK_CCORE_G3D_800_USER 0x0508
#define CLK_STAT_MUX_ACLK_CCORE_528_USER 0x050C
#define CLK_STAT_MUX_ACLK_CCORE_132_USER 0x0510
#define CLK_STAT_MUX_PCLK_CCORE_66_USER 0x0514
#define CLK_ENABLE_ACLK_CCORE0 0x0800
#define CLK_ENABLE_ACLK_CCORE1 0x0804
#define CLK_ENABLE_ACLK_CCORE2 0x0808
#define CLK_ENABLE_ACLK_CCORE3 0x080C
#define CLK_ENABLE_ACLK_CCORE4 0x0810
#define CLK_ENABLE_ACLK_CCORE_AP 0x0814
#define CLK_ENABLE_ACLK_CCORE_CP 0x0818
#define CLK_ENABLE_PCLK_CCORE_AP 0x0900
#define CLK_ENABLE_PCLK_CCORE_CP 0x0904
#define CLK_ENABLE_SCLK_HPM_CCORE 0x0A00
#define CG_CTRL_VAL_ACLK_CCORE0 0x0800
#define CG_CTRL_VAL_ACLK_CCORE1 0x0804
#define CG_CTRL_VAL_ACLK_CCORE2 0x0808
#define CG_CTRL_VAL_ACLK_CCORE3 0x080C
#define CG_CTRL_VAL_ACLK_CCORE4 0x0810
#define CG_CTRL_VAL_ACLK_CCORE_AP 0x0814
#define CG_CTRL_VAL_ACLK_CCORE_CP 0x0818
#define CG_CTRL_VAL_PCLK_CCORE_AP 0x0900
#define CG_CTRL_VAL_PCLK_CCORE_CP 0x0904
#define CG_CTRL_VAL_SCLK_HPM_CCORE 0x0A00
#define CLKOUT_CMU_CCORE 0x0C00
#define CLKOUT_CMU_CCORE_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_CCORE 0x0E00
#define CCORE_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define PSCDC_CTRL_CCORE 0x1000
#define CLK_STOPCTRL_CCORE 0x1004
#define CG_CTRL_MAN_ACLK_CCORE0 0x1800
#define CG_CTRL_MAN_ACLK_CCORE1 0x1804
#define CG_CTRL_MAN_ACLK_CCORE2 0x1808
#define CG_CTRL_MAN_ACLK_CCORE3 0x180C
#define CG_CTRL_MAN_ACLK_CCORE4 0x1810
#define CG_CTRL_MAN_ACLK_CCORE_AP 0x1814
#define CG_CTRL_MAN_ACLK_CCORE_CP 0x1818
#define CG_CTRL_MAN_PCLK_CCORE_AP 0x1900
#define CG_CTRL_MAN_PCLK_CCORE_CP 0x1904
#define CG_CTRL_MAN_SCLK_HPM_CCORE 0x1A00
#define CG_CTRL_STAT_ACLK_CCORE0_0 0x1C00
#define CG_CTRL_STAT_ACLK_CCORE0_1 0x1C04
#define CG_CTRL_STAT_ACLK_CCORE1_0 0x1C08
#define CG_CTRL_STAT_ACLK_CCORE1_1 0x1C0C
#define CG_CTRL_STAT_ACLK_CCORE2 0x1C10
#define CG_CTRL_STAT_ACLK_CCORE3 0x1C14
#define CG_CTRL_STAT_ACLK_CCORE4_0 0x1C18
#define CG_CTRL_STAT_ACLK_CCORE4_1 0x1C1C
#define CG_CTRL_STAT_ACLK_CCORE4_2 0x1C20
#define CG_CTRL_STAT_ACLK_CCORE4_3 0x1C24
#define CG_CTRL_STAT_ACLK_CCORE_AP 0x1C28
#define CG_CTRL_STAT_ACLK_CCORE_CP 0x1C2C
#define CG_CTRL_STAT_PCLK_CCORE_AP 0x1D00
#define CG_CTRL_STAT_PCLK_CCORE_CP 0x1D04
#define CG_CTRL_STAT_SCLK_HPM_CCORE 0x1E00
#define CMU_CCORE_SPARE0 0x1100
#define CMU_CCORE_SPARE1 0x1104
#define QCH_CTRL_TREX_CCORE 0x2000
#define QCH_CTRL_TREX_P_CCORE 0x2004
#define QCH_CTRL_LH_G3DIRAM 0x2008
#define QCH_CTRL_LH_CCORESFRX 0x200C
#define QCH_CTRL_LH_CPPERI 0x2010
#define QCH_CTRL_LH_G3D0 0x2014
#define QCH_CTRL_LH_G3D1 0x2018
#define QCH_CTRL_LH_AUD 0x201C
#define QCH_CTRL_LH_IMEM 0x2020
#define QCH_CTRL_LH_CPDATA 0x2024
#define QCH_CTRL_LH_AUDP 0x2028
#define QCH_CTRL_LH_G3DP 0x202C
#define QCH_CTRL_LH_MIF0P 0x2030
#define QCH_CTRL_LH_MIF1P 0x2034
#define QCH_CTRL_LH_MIF2P 0x2038
#define QCH_CTRL_LH_MIF3P 0x203C
#define QCH_CTRL_PMU_CCORE 0x2040
#define QCH_CTRL_SYSREG_CCORE 0x2044
#define QCH_CTRL_CMU_CCORE 0x2048
#define QSTATE_CTRL_AXI_AS_MI_MNGSCS_CCORETD 0x2400
#define QSTATE_CTRL_ATB_APL_MNGS 0x2404
#define QSTATE_CTRL_APB_AS_MI_MNGSCS_CCOREBDU 0x2408
#define QSTATE_CTRL_APB_PDU 0x240C
#define QSTATE_CTRL_HSI2C 0x2410
#define QSTATE_CTRL_HSI2C_BAT_AP 0x2414
#define QSTATE_CTRL_HSI2C_BAT_CP 0x2418
#define QSTATE_CTRL_PROMISE 0x241C
#define QSTATE_CTRL_AXI_AS_SI_CCORETP_MNGS 0x2420
#define QSTATE_CTRL_AXI_AS_SI_CCORETP_APL 0x24240

PNAME(ccore_mux_aclk_ccore_800_user_p) = { "oscclk"
					   "top_gate_aclk_ccore_800" };
PNAME(ccore_mux_aclk_ccore_264_user_p) = { "oscclk"
					   "top_gate_aclk_ccore_264" };
PNAME(ccore_mux_aclk_ccore_g3d_800_user_p) = { "oscclk"
					       "top_gate_aclk_ccore_g3d_800" };
PNAME(ccore_mux_aclk_ccore_528_user_p) = { "oscclk"
					   "top_gate_aclk_ccore_528" };
PNAME(ccore_mux_aclk_ccore_132_user_p) = { "oscclk"
					   "top_gate_aclk_ccore_132" };
PNAME(ccore_mux_pclk_ccore_66_user_p) = { "oscclk"
					  "top_gate_pclk_ccore_66" };

static const struct samsung_mux_clock ccore_mux_clks[] __initconst = {
	MUX(0, "ccore_mux_aclk_ccore_800_user", ccore_mux_aclk_ccore_800_user_p,
	    CLK_CON_MUX_ACLK_CCORE_800_USER, 12, 1),
	MUX(0, "ccore_mux_aclk_ccore_264_user", ccore_mux_aclk_ccore_264_user_p,
	    CLK_CON_MUX_ACLK_CCORE_264_USER, 12, 1),
	MUX(0, "ccore_mux_aclk_ccore_g3d_800_user",
	    ccore_mux_aclk_ccore_g3d_800_user_p,
	    CLK_CON_MUX_ACLK_CCORE_G3D_800_USER, 12, 1),
	MUX(0, "ccore_mux_aclk_ccore_528_user", ccore_mux_aclk_ccore_528_user_p,
	    CLK_CON_MUX_ACLK_CCORE_528_USER, 12, 1),
	MUX(0, "ccore_mux_aclk_ccore_132_user", ccore_mux_aclk_ccore_132_user_p,
	    CLK_CON_MUX_ACLK_CCORE_132_USER, 12, 1),
	MUX(0, "ccore_mux_pclk_ccore_66_user", ccore_mux_pclk_ccore_66_user_p,
	    CLK_CON_MUX_PCLK_CCORE_66_USER, 12, 1),
};

static const struct samsung_gate_clock ccore_gate_clks[] __initconst = {
	GATE(0, "ccore_gate_aclk_axi_as_si_irpm",
	     "ccore_mux_aclk_ccore_800_user", CG_CTRL_VAL_ACLK_CCORE0, 11,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_mpacebridge", "ccore_mux_aclk_ccore_800_user",
	     CG_CTRL_VAL_ACLK_CCORE0, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_pulse2hs", "ccore_mux_aclk_ccore_800_user",
	     CG_CTRL_VAL_ACLK_CCORE0, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_dbg_lh_mi_mif_ccore",
	     "ccore_mux_aclk_ccore_800_user", CG_CTRL_VAL_ACLK_CCORE0, 8,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_sci_ppc_wrapper",
	     "ccore_mux_aclk_ccore_800_user", CG_CTRL_VAL_ACLK_CCORE0, 7,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_ace_as_mi_apl_ccore",
	     "ccore_mux_aclk_ccore_800_user", CG_CTRL_VAL_ACLK_CCORE0, 6,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_mpace_si", "ccore_mux_aclk_ccore_800_user",
	     CG_CTRL_VAL_ACLK_CCORE0, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_cpace_mi", "ccore_mux_aclk_ccore_800_user",
	     CG_CTRL_VAL_ACLK_CCORE0, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_atb_si_ccorebdu_mngscs",
	     "ccore_mux_aclk_ccore_800_user", CG_CTRL_VAL_ACLK_CCORE0, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_bdu", "ccore_mux_aclk_ccore_800_user",
	     CG_CTRL_VAL_ACLK_CCORE0, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_trex_ccore_sci",
	     "ccore_mux_aclk_ccore_800_user", CG_CTRL_VAL_ACLK_CCORE0, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_sci", "ccore_mux_aclk_ccore_800_user",
	     CG_CTRL_VAL_ACLK_CCORE0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_cleany_cpperi",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 9,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_us_cpperi",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 8,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_mi_cpperi_ccore",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 7,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_si_ccoresfrx_imemx",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 6,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_mi_g3dxiram_ccoresfr",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_ds_irpm", "ccore_mux_aclk_ccore_264_user",
	     CG_CTRL_VAL_ACLK_CCORE1, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_xiu_ccoresfrx",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_trex_p_ccore_bus",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_trex_ccore_peri",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_as_mi_irpm",
	     "ccore_mux_aclk_ccore_264_user", CG_CTRL_VAL_ACLK_CCORE1, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_trex_ccore_g3d",
	     "ccore_mux_aclk_ccore_g3d_800_user", CG_CTRL_VAL_ACLK_CCORE2, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_acel_lh_mi_g3dx1_ccoretd",
	     "ccore_mux_aclk_ccore_g3d_800_user", CG_CTRL_VAL_ACLK_CCORE2, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_acel_lh_mi_g3dx0_ccoretd",
	     "ccore_mux_aclk_ccore_g3d_800_user", CG_CTRL_VAL_ACLK_CCORE2, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_atb_apl_mngs", "ccore_mux_aclk_ccore_528_user",
	     CG_CTRL_VAL_ACLK_CCORE3, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_xiu_cpx", "ccore_mux_aclk_ccore_528_user",
	     CG_CTRL_VAL_ACLK_CCORE3, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_cleany_cpdata",
	     "ccore_mux_aclk_ccore_528_user", CG_CTRL_VAL_ACLK_CCORE3, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_mi_cpdata_ccore",
	     "ccore_mux_aclk_ccore_528_user", CG_CTRL_VAL_ACLK_CCORE3, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_mi_imemx_ccoretd",
	     "ccore_mux_aclk_ccore_528_user", CG_CTRL_VAL_ACLK_CCORE3, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_mi_audx_ccoretd",
	     "ccore_mux_aclk_ccore_528_user", CG_CTRL_VAL_ACLK_CCORE3, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_as_mi_mngscs_ccoretd",
	     "ccore_mux_aclk_ccore_528_user", CG_CTRL_VAL_ACLK_CCORE3, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_trex_ccore", "ccore_mux_aclk_ccore_528_user",
	     CG_CTRL_VAL_ACLK_CCORE3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_cmu", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 30, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_hpm_apbif", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 29, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_sci", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 28, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_gpio_ccore", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 27, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_s_mailbox", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 26, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_mailbox", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 25, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_sysreg_ccore", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 24, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_gpio_apbif_alive",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 23,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_sci_ppc_wrapper",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 22,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_vt_mon_apb", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_pmu_ccore", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 20, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_pmu_apbif", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 19, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_cmu_topc_apbif",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 18,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_axi2apb_coresight",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 17,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_axi2apb_trex_p_ccore",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 16,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_axi2apb_trex_ccore",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 15,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_axi2apb_ccore",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 14,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_trex_p_ccore", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_trex_ccore", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_bdu", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_si_ccoretp_mif3p",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 10,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_si_ccoretp_mif2p",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 9,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_si_ccoretp_mif1p",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 8,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_si_ccoretp_mif0p",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 7,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_si_ccoretp_g3dp",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 6,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_lh_si_ccoretp_audx",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_as_si_ccoretp_apl",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_axi_as_si_ccoretp_mngs",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_apb_as_mi_ccoretp_mngscs",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_apb_as_mi_mngscs_ccorebdu",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE4, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_aclk_trex_p_ccore", "ccore_mux_aclk_ccore_132_user",
	     CG_CTRL_VAL_ACLK_CCORE4, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_apbasync_bat_ap",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE_AP, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_apbasync_bat_cp",
	     "ccore_mux_aclk_ccore_132_user", CG_CTRL_VAL_ACLK_CCORE_CP, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_hsi2c_bat_ap", "ccore_mux_pclk_ccore_66_user",
	     CG_CTRL_VAL_PCLK_CCORE_AP, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_hsi2c", "ccore_mux_pclk_ccore_66_user",
	     CG_CTRL_VAL_PCLK_CCORE_AP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_hsi2c_bat_cp", "ccore_mux_pclk_ccore_66_user",
	     CG_CTRL_VAL_PCLK_CCORE_CP, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_pclk_hsi2c_cp", "ccore_mux_pclk_ccore_66_user",
	     CG_CTRL_VAL_PCLK_CCORE_CP, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "ccore_gate_sclk_promise", "ccore_div_sclk_hpm_ccore",
	     CG_CTRL_VAL_SCLK_HPM_CCORE, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock ccore_div_clks[] __initconst = {
	DIV(0, "ccore_div_sclk_hpm_ccore", "ccore_mux_aclk_ccore_800_user",
	    CLK_CON_DIV_SCLK_HPM_CCORE, 0, 4),
};

static const struct samsung_cmu_info ccore_cmu_info __initconst = {
	.mux_clks = ccore_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(ccore_mux_clks),
	.div_clks = ccore_div_clks,
	.nr_div_clks = ARRAY_SIZE(ccore_div_clks),
	.gate_clks = ccore_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(ccore_gate_clks),
	.nr_clk_ids = CCORE_NR_CLK,
};

/* Register Offset definitions for CMU_MIF0 (0x10850000) */
#define MIF0_PLL_LOCK 0x0000
#define MIF0_PLL_CON0 0x0100
#define MIF0_PLL_CON1 0x0104
#define MIF0_PLL_FREQ_DET 0x010C
#define CLK_CON_MUX_MIF0_PLL 0x0200
#define CLK_CON_MUX_BUS_PLL_USER_MIF0 0x0204
#define CLK_CON_MUX_ACLK_MIF0_PLL 0x0208
#define CLK_CON_MUX_PCLK_MIF0 0x0210
#define CLK_CON_MUX_SCLK_HPM_MIF0 0x0214
#define CLK_CON_MUX_PCLK_SMC_MIF0 0x0218
#define CLK_CON_DIV_PCLK_MIF0 0x0400
#define CLK_CON_DIV_PCLK_SMC_MIF0 0x0404
#define CLK_CON_DIV_SCLK_HPM_MIF0 0x0408
#define CLK_STAT_MUX_MIF0_PLL 0x0600
#define CLK_STAT_MUX_BUS_PLL_USER_MIF0 0x0604
#define CLK_STAT_MUX_ACLK_MIF0_PLL 0x0608
#define CLK_STAT_MUX_PCLK_MIF0 0x0610
#define CLK_STAT_MUX_SCLK_HPM_MIF0 0x0614
#define CLK_STAT_MUX_PCLK_SMC_MIF0 0x0618
#define CLK_ENABLE_ACLK_MIF0 0x0800
#define CLK_ENABLE_PCLK_MIF0 0x0900
#define CLK_ENABLE_ACLK_MIF0_SECURE_DREX_TZ 0x0904
#define CLK_ENABLE_SCLK_HPM_MIF0 0x0A00
#define CLK_ENABLE_SCLK_RCLK_DREX_MIF0 0x0A04
#define CG_CTRL_VAL_PCLK_MIF0 0x0900
#define CG_CTRL_VAL_SCLK_HPM_MIF0 0x0A00
#define CG_CTRL_VAL_SCLK_RCLK_DREX0 0x0A04
#define CG_CTRL_VAL_DDRPHY0 0x0A08
#define CLKOUT_CMU_MIF0 0x0C00
#define CLKOUT_CMU_MIF0_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_MIF0 0x0E00
#define MIF0_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define PSCDC_CTRL_MIF0 0x1000
#define CLK_STOPCTRL_MIF0 0x1004
#define CMU_MIF0_SPARE0 0x1100
#define CMU_MIF0_SPARE1 0x1104
#define CG_CTRL_MAN_PCLK_MIF0 0x1900
#define CG_CTRL_MAN_SCLK_HPM_MIF0 0x1A00
#define CG_CTRL_MAN_SCLK_RCLK_DREX0 0x1A04
#define CG_CTRL_MAN_DDRPHY0 0x1A08
#define CG_CTRL_STAT_PCLK_MIF0_0 0x1D00
#define CG_CTRL_STAT_PCLK_MIF0_1 0x1D04
#define CG_CTRL_STAT_SCLK_HPM_MIF0 0x1E00
#define CG_CTRL_STAT_SCLK_RCLK_DREX0 0x1E04
#define CG_CTRL_STAT_DDRPHY0 0x1E08
#define QCH_CTRL_LH_AXI_P_MIF0 0x2000
#define QCH_CTRL_PMU_MIF0 0x2004
#define QCH_CTRL_SYSREG_MIF0 0x2008
#define QCH_CTRL_CMU_MIF0 0x200C
#define QCH_CTRL_SMC_MIF0 0x2010
#define QSTATE_CTRL_PROMISE_MIF0 0x2400
#define QSTATE_CTRL_RCLK_DREX0 0x2404

/* MIF_PLL */
static const struct samsung_pll_rate_table
	exynos8890_mif_pll_rates[] __initconst = {
		PLL_1419X_RATE(26 * MHZ, 3588000000U, 207, 3, 0),
		PLL_1419X_RATE(26 * MHZ, 3432000000U, 198, 3, 0),
		PLL_1419X_RATE(26 * MHZ, 3078400000U, 296, 5, 0),
		PLL_1419X_RATE(26 * MHZ, 2704000000U, 156, 3, 0),
		PLL_1419X_RATE(26 * MHZ, 2288000000U, 132, 3, 0),
		PLL_1419X_RATE(26 * MHZ, 2028000000U, 117, 3, 0),
		PLL_1419X_RATE(26 * MHZ, 1690000000U, 195, 3, 1),
		PLL_1419X_RATE(26 * MHZ, 1352000000U, 156, 3, 1),
		PLL_1419X_RATE(26 * MHZ, 1092000000U, 126, 3, 1),
		PLL_1419X_RATE(26 * MHZ, 841750000U, 259, 4, 2),
		PLL_1419X_RATE(26 * MHZ, 572000000U, 132, 3, 2),
		PLL_1419X_RATE(26 * MHZ, 416000000U, 192, 3, 3),
		{ /* sentinel */ }
	};

static const struct samsung_fixed_factor_clock
	mif0_fixed_factor_clks[] __initconst = {
		FFACTOR(0, "mif0_ff_aclk_mif_pll_div2", "mif0_mux_aclk_mif_pll",
			1, 2, 0),
		FFACTOR(0, "mif0_ff_aclk_mif_pll_div4", "mif0_mux_aclk_mif_pll",
			1, 4, 0),
		FFACTOR(0, "u_dfi_clk_gen_mif0", "mif0_mux_aclk_mif_pll", 1, 4,
			0),
	};

static const struct samsung_pll_clock mif0_pll_clks[] __initconst = {
	PLL(pll_1419x, 0, "mif0_pll", "oscclk", MIF0_PLL_LOCK, MIF0_PLL_CON0,
	    exynos8890_mif_pll_rates),
};

PNAME(mif0_mux_mif_pll_p) = { "oscclk", "mif0_pll" };
PNAME(mif0_mux_bus_pll_user_p) = { "oscclk", "top_gate_sclk_bus_pll_mif" };
PNAME(mif0_mux_aclk_mif_pll_p) = { "mif0_mux_mif_pll",
				   "mif0_mux_bus_pll_user" };
PNAME(mif0_mux_pclk_mif_p) = { "mif0_mux_aclk_mif_pll",
			       "mif0_ff_aclk_mif_pll_div2",
			       "mif0_ff_aclk_mif_pll_div4" };
PNAME(mif0_mux_sclk_hpm_mif_p) = { "mif0_mux_aclk_mif_pll",
				   "mif0_ff_aclk_mif_pll_div2",
				   "mif0_ff_aclk_mif_pll_div4" };
PNAME(mif0_mux_pclk_smc_p) = { "mif0_mux_aclk_mif_pll",
			       "mif0_ff_aclk_mif_pll_div2",
			       "mif0_ff_aclk_mif_pll_div4",
			       "u_dfi_clk_gen_mif0" };

static const struct samsung_mux_clock mif0_mux_clks[] __initconst = {
	MUX(0, "mif0_mux_mif_pll", mif0_mux_mif_pll_p, CLK_CON_MUX_MIF0_PLL, 12,
	    1),
	MUX(0, "mif0_mux_bus_pll_user", mif0_mux_bus_pll_user_p,
	    CLK_CON_MUX_BUS_PLL_USER_MIF0, 12, 1),
	MUX(0, "mif0_mux_aclk_mif_pll", mif0_mux_aclk_mif_pll_p,
	    CLK_CON_MUX_ACLK_MIF0_PLL, 12, 1),
	MUX(0, "mif0_mux_pclk_mif", mif0_mux_pclk_mif_p, CLK_CON_MUX_PCLK_MIF0,
	    12, 2),
	MUX(0, "mif0_mux_sclk_hpm_mif", mif0_mux_sclk_hpm_mif_p,
	    CLK_CON_MUX_SCLK_HPM_MIF0, 12, 2),
	MUX(0, "mif0_mux_pclk_smc", mif0_mux_pclk_smc_p,
	    CLK_CON_MUX_PCLK_SMC_MIF0, 12, 2),
};

static const struct samsung_gate_clock mif0_gate_clks[] __initconst = {
	GATE(0, "mif0_gate_aclk_apscdc", "u_dfi_clk_gen_mif0",
	     CLK_ENABLE_ACLK_MIF0, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_aclk_ppc_debug", "u_dfi_clk_gen_mif0",
	     CLK_ENABLE_ACLK_MIF0, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_aclk_ppc_dvfs", "u_dfi_clk_gen_mif0",
	     CLK_ENABLE_ACLK_MIF0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_aclk_smc", "u_dfi_clk_gen_mif0",
	     CLK_ENABLE_ACLK_MIF0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_smc1", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_dmc_misc", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_ppc_debug", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_ppc_dvfs", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_sysreg_mif", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_hpm", "mif0_div_pclk_mif", CLK_ENABLE_PCLK_MIF0,
	     4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_aclk_axi_async", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_mifp", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_pmu_mif", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_lpddr4phy", "mif0_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_pclk_smc2", "mif0_div_pclk_mif",
	     CLK_ENABLE_ACLK_MIF0_SECURE_DREX_TZ, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_sclk_promise", "mif0_div_sclk_hpm_mif",
	     CLK_ENABLE_SCLK_HPM_MIF0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif0_gate_rclk_drex", "oscclk", CLK_ENABLE_SCLK_RCLK_DREX_MIF0,
	     0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock mif0_div_clks[] __initconst = {
	DIV(0, "mif0_div_pclk_mif", "mif0_mux_pclk_mif", CLK_CON_DIV_PCLK_MIF0,
	    0, 3),
	DIV(0, "mif0_div_sclk_hpm_mif", "mif0_mux_sclk_hpm_mif",
	    CLK_CON_DIV_SCLK_HPM_MIF0, 0, 2),
	DIV(0, "mif0_div_pclk_smc", "mif0_mux_pclk_smc",
	    CLK_CON_DIV_PCLK_SMC_MIF0, 0, 3),
};

static const struct samsung_cmu_info mif0_cmu_info __initconst = {
	.pll_clks = mif0_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(mif0_pll_clks),
	.mux_clks = mif0_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(mif0_mux_clks),
	.div_clks = mif0_div_clks,
	.nr_div_clks = ARRAY_SIZE(mif0_div_clks),
	.gate_clks = mif0_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(mif0_gate_clks),
	.fixed_factor_clks = mif0_fixed_factor_clks,
	.nr_fixed_factor_clks = ARRAY_SIZE(mif0_fixed_factor_clks),
	.nr_clk_ids = MIF0_NR_CLK,
};

/* Register Offset definitions for CMU_MIF1 (0x10950000) */
#define MIF1_PLL_LOCK 0x0000
#define MIF1_PLL_CON0 0x0100
#define MIF1_PLL_CON1 0x0104
#define MIF1_PLL_FREQ_DET 0x010C
#define CLK_CON_MUX_MIF1_PLL 0x0200
#define CLK_CON_MUX_BUS_PLL_USER_MIF1 0x0204
#define CLK_CON_MUX_ACLK_MIF1_PLL 0x0208
#define CLK_CON_MUX_PCLK_MIF1 0x0210
#define CLK_CON_MUX_SCLK_HPM_MIF1 0x0214
#define CLK_CON_MUX_PCLK_SMC_MIF1 0x0218
#define CLK_CON_DIV_PCLK_MIF1 0x0400
#define CLK_CON_DIV_PCLK_SMC_MIF1 0x0404
#define CLK_CON_DIV_SCLK_HPM_MIF1 0x0408
#define CLK_STAT_MUX_MIF1_PLL 0x0600
#define CLK_STAT_MUX_BUS_PLL_USER_MIF1 0x0604
#define CLK_STAT_MUX_ACLK_MIF1_PLL 0x0608
#define CLK_STAT_MUX_PCLK_MIF1 0x0610
#define CLK_STAT_MUX_SCLK_HPM_MIF1 0x0614
#define CLK_STAT_MUX_PCLK_SMC_MIF1 0x0618
#define CLK_ENABLE_ACLK_MIF1 0x0800
#define CLK_ENABLE_PCLK_MIF1 0x0900
#define CLK_ENABLE_ACLK_MIF1_SECURE_DREX_TZ 0x0904
#define CLK_ENABLE_SCLK_HPM_MIF1 0x0A00
#define CLK_ENABLE_SCLK_RCLK_DREX_MIF1 0x0A04
#define CG_CTRL_VAL_PCLK_MIF1 0x0900
#define CG_CTRL_VAL_SCLK_HPM_MIF1 0x0A00
#define CG_CTRL_VAL_SCLK_RCLK_DREX1 0x0A04
#define CG_CTRL_VAL_DDRPHY1 0x0A08
#define CLKOUT_CMU_MIF1 0x0C00
#define CLKOUT_CMU_MIF1_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_MIF1 0x0E00
#define MIF1_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define PSCDC_CTRL_MIF1 0x1000
#define CLK_STOPCTRL_MIF1 0x1004
#define CMU_MIF1_SPARE0 0x1100
#define CMU_MIF1_SPARE1 0x1104
#define CG_CTRL_MAN_PCLK_MIF1 0x1900
#define CG_CTRL_MAN_SCLK_HPM_MIF1 0x1A00
#define CG_CTRL_MAN_SCLK_RCLK_DREX1 0x1A04
#define CG_CTRL_MAN_DDRPHY1 0x1A08
#define CG_CTRL_STAT_PCLK_MIF1_0 0x1D00
#define CG_CTRL_STAT_PCLK_MIF1_1 0x1D04
#define CG_CTRL_STAT_SCLK_HPM_MIF1 0x1E00
#define CG_CTRL_STAT_SCLK_RCLK_DREX1 0x1E04
#define CG_CTRL_STAT_DDRPHY1 0x1E08
#define QCH_CTRL_LH_AXI_P_MIF1 0x2000
#define QCH_CTRL_PMU_MIF1 0x2004
#define QCH_CTRL_SYSREG_MIF1 0x2008
#define QCH_CTRL_CMU_MIF1 0x200C
#define QCH_CTRL_SMC_MIF1 0x2010
#define QSTATE_CTRL_PROMISE_MIF1 0x2400
#define QSTATE_CTRL_RCLK_DREX1 0x2404

static const struct samsung_pll_clock mif1_pll_clks[] __initconst = {
	PLL(pll_1419x, 0, "mif1_pll", "oscclk", MIF1_PLL_LOCK, MIF1_PLL_CON0,
	    exynos8890_mif_pll_rates),
};

PNAME(mif1_mux_mif_pll_p) = { "oscclk", "mif1_pll" };
PNAME(mif1_mux_bus_pll_user_p) = { "oscclk", "top_gate_sclk_bus_pll_mif" };
PNAME(mif1_mux_aclk_mif_pll_p) = { "mif1_mux_mif_pll",
				   "mif1_mux_bus_pll_user" };
PNAME(mif1_mux_pclk_mif_p) = { "mif1_mux_aclk_mif_pll",
			       "mif1_ff_aclk_mif_pll_div2",
			       "mif1_ff_aclk_mif_pll_div4" };
PNAME(mif1_mux_sclk_hpm_mif_p) = { "mif1_mux_aclk_mif_pll",
				   "mif1_ff_aclk_mif_pll_div2",
				   "mif1_ff_aclk_mif_pll_div4" };
PNAME(mif1_mux_pclk_smc_p) = { "mif1_mux_aclk_mif_pll",
			       "mif1_ff_aclk_mif_pll_div2",
			       "mif1_ff_aclk_mif_pll_div4",
			       "u_dfi_clk_gen_mif1" };

static const struct samsung_fixed_factor_clock
	mif1_fixed_factor_clks[] __initconst = {
		FFACTOR(0, "mif1_ff_aclk_mif_pll_div2", "mif1_mux_aclk_mif_pll",
			1, 2, 0),
		FFACTOR(0, "mif1_ff_aclk_mif_pll_div4", "mif1_mux_aclk_mif_pll",
			1, 4, 0),
		FFACTOR(0, "u_dfi_clk_gen_mif1", "mif1_mux_aclk_mif_pll", 1, 4,
			0),
	};

static const struct samsung_mux_clock mif1_mux_clks[] __initconst = {
	MUX(0, "mif1_mux_mif_pll", mif1_mux_mif_pll_p, CLK_CON_MUX_MIF1_PLL, 12,
	    1),
	MUX(0, "mif1_mux_bus_pll_user", mif1_mux_bus_pll_user_p,
	    CLK_CON_MUX_BUS_PLL_USER_MIF1, 12, 1),
	MUX(0, "mif1_mux_aclk_mif_pll", mif1_mux_aclk_mif_pll_p,
	    CLK_CON_MUX_ACLK_MIF1_PLL, 12, 1),
	MUX(0, "mif1_mux_pclk_mif", mif1_mux_pclk_mif_p, CLK_CON_MUX_PCLK_MIF1,
	    12, 2),
	MUX(0, "mif1_mux_sclk_hpm_mif", mif1_mux_sclk_hpm_mif_p,
	    CLK_CON_MUX_SCLK_HPM_MIF1, 12, 2),
	MUX(0, "mif1_mux_pclk_smc", mif1_mux_pclk_smc_p,
	    CLK_CON_MUX_PCLK_SMC_MIF1, 12, 2),
};

static const struct samsung_gate_clock mif1_gate_clks[] __initconst = {
	GATE(0, "mif1_gate_aclk_apscdc", "u_dfi_clk_gen_mif1",
	     CLK_ENABLE_ACLK_MIF1, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_aclk_ppc_debug", "u_dfi_clk_gen_mif1",
	     CLK_ENABLE_ACLK_MIF1, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_aclk_ppc_dvfs", "u_dfi_clk_gen_mif1",
	     CLK_ENABLE_ACLK_MIF1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_aclk_smc", "u_dfi_clk_gen_mif1",
	     CLK_ENABLE_ACLK_MIF1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_smc1", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_dmc_misc", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_ppc_debug", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_ppc_dvfs", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_sysreg_mif", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_hpm", "mif1_div_pclk_mif", CLK_ENABLE_PCLK_MIF1,
	     4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_aclk_axi_async", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_mifp", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_pmu_mif", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_lpddr4phy", "mif1_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_pclk_smc2", "mif1_div_pclk_mif",
	     CLK_ENABLE_ACLK_MIF1_SECURE_DREX_TZ, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_sclk_promise", "mif1_div_sclk_hpm_mif",
	     CLK_ENABLE_SCLK_HPM_MIF1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif1_gate_rclk_drex", "oscclk", CLK_ENABLE_SCLK_RCLK_DREX_MIF1,
	     0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock mif1_div_clks[] __initconst = {
	DIV(0, "mif1_div_pclk_mif", "mif1_mux_pclk_mif", CLK_CON_DIV_PCLK_MIF1,
	    0, 3),
	DIV(0, "mif1_div_sclk_hpm_mif", "mif1_mux_sclk_hpm_mif",
	    CLK_CON_DIV_SCLK_HPM_MIF1, 0, 2),
	DIV(0, "mif1_div_pclk_smc", "mif1_mux_pclk_smc",
	    CLK_CON_DIV_PCLK_SMC_MIF1, 0, 3),
};

static const struct samsung_cmu_info mif1_cmu_info __initconst = {
	.pll_clks = mif1_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(mif1_pll_clks),
	.mux_clks = mif1_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(mif1_mux_clks),
	.div_clks = mif1_div_clks,
	.nr_div_clks = ARRAY_SIZE(mif1_div_clks),
	.gate_clks = mif1_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(mif1_gate_clks),
	.fixed_factor_clks = mif1_fixed_factor_clks,
	.nr_fixed_factor_clks = ARRAY_SIZE(mif1_fixed_factor_clks),
	.nr_clk_ids = MIF1_NR_CLK,
};

/* Register Offset definitions for CMU_MIF2 (0x10A50000) */
#define MIF2_PLL_LOCK 0x0000
#define MIF2_PLL_CON0 0x0100
#define MIF2_PLL_CON1 0x0104
#define MIF2_PLL_FREQ_DET 0x010C
#define CLK_CON_MUX_MIF2_PLL 0x0200
#define CLK_CON_MUX_BUS_PLL_USER_MIF2 0x0204
#define CLK_CON_MUX_ACLK_MIF2_PLL 0x0208
#define CLK_CON_MUX_PCLK_MIF2 0x0210
#define CLK_CON_MUX_SCLK_HPM_MIF2 0x0214
#define CLK_CON_MUX_PCLK_SMC_MIF2 0x0218
#define CLK_CON_DIV_PCLK_MIF2 0x0400
#define CLK_CON_DIV_PCLK_SMC_MIF2 0x0404
#define CLK_CON_DIV_SCLK_HPM_MIF2 0x0408
#define CLK_STAT_MUX_MIF2_PLL 0x0600
#define CLK_STAT_MUX_BUS_PLL_USER_MIF2 0x0604
#define CLK_STAT_MUX_ACLK_MIF2_PLL 0x0608
#define CLK_STAT_MUX_PCLK_MIF2 0x0610
#define CLK_STAT_MUX_SCLK_HPM_MIF2 0x0614
#define CLK_STAT_MUX_PCLK_SMC_MIF2 0x0618
#define CLK_ENABLE_ACLK_MIF2 0x0800
#define CLK_ENABLE_PCLK_MIF2 0x0900
#define CLK_ENABLE_ACLK_MIF2_SECURE_DREX_TZ 0x0904
#define CLK_ENABLE_SCLK_HPM_MIF2 0x0A00
#define CLK_ENABLE_SCLK_RCLK_DREX_MIF2 0x0A04
#define CG_CTRL_VAL_PCLK_MIF2 0x0900
#define CG_CTRL_VAL_SCLK_HPM_MIF2 0x0A00
#define CG_CTRL_VAL_SCLK_RCLK_DREX2 0x0A04
#define CG_CTRL_VAL_DDRPHY2 0x0A08
#define CLKOUT_CMU_MIF2 0x0C00
#define CLKOUT_CMU_MIF2_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_MIF2 0x0E00
#define MIF2_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define PSCDC_CTRL_MIF2 0x1000
#define CLK_STOPCTRL_MIF2 0x1004
#define CMU_MIF2_SPARE0 0x1100
#define CMU_MIF2_SPARE1 0x1104
#define CG_CTRL_MAN_PCLK_MIF2 0x1900
#define CG_CTRL_MAN_SCLK_HPM_MIF2 0x1A00
#define CG_CTRL_MAN_SCLK_RCLK_DREX2 0x1A04
#define CG_CTRL_MAN_DDRPHY2 0x1A08
#define CG_CTRL_STAT_PCLK_MIF2_0 0x1D00
#define CG_CTRL_STAT_PCLK_MIF2_1 0x1D04
#define CG_CTRL_STAT_SCLK_HPM_MIF2 0x1E00
#define CG_CTRL_STAT_SCLK_RCLK_DREX2 0x1E04
#define CG_CTRL_STAT_DDRPHY2 0x1E08
#define QCH_CTRL_LH_AXI_P_MIF2 0x2000
#define QCH_CTRL_PMU_MIF2 0x2004
#define QCH_CTRL_SYSREG_MIF2 0x2008
#define QCH_CTRL_CMU_MIF2 0x200C
#define QCH_CTRL_SMC_MIF2 0x2010
#define QSTATE_CTRL_PROMISE_MIF2 0x2400
#define QSTATE_CTRL_RCLK_DREX2 0x2404

static const struct samsung_pll_clock mif2_pll_clks[] __initconst = {
	PLL(pll_1419x, 0, "mif2_pll", "oscclk", MIF2_PLL_LOCK, MIF2_PLL_CON0,
	    exynos8890_mif_pll_rates),
};

PNAME(mif2_mux_mif_pll_p) = { "oscclk", "mif2_pll" };
PNAME(mif2_mux_bus_pll_user_p) = { "oscclk", "top_gate_sclk_bus_pll_mif" };
PNAME(mif2_mux_aclk_mif_pll_p) = { "mif2_mux_mif_pll",
				   "mif2_mux_bus_pll_user" };
PNAME(mif2_mux_pclk_mif_p) = { "mif2_mux_aclk_mif_pll",
			       "mif2_ff_aclk_mif_pll_div2",
			       "mif2_ff_aclk_mif_pll_div4" };
PNAME(mif2_mux_sclk_hpm_mif_p) = { "mif2_mux_aclk_mif_pll",
				   "mif2_ff_aclk_mif_pll_div2",
				   "mif2_ff_aclk_mif_pll_div4" };
PNAME(mif2_mux_pclk_smc_p) = { "mif2_mux_aclk_mif_pll",
			       "mif2_ff_aclk_mif_pll_div2",
			       "mif2_ff_aclk_mif_pll_div4",
			       "u_dfi_clk_gen_mif2" };

static const struct samsung_fixed_factor_clock
	mif2_fixed_factor_clks[] __initconst = {
		FFACTOR(0, "mif2_ff_aclk_mif_pll_div2", "mif2_mux_aclk_mif_pll",
			1, 2, 0),
		FFACTOR(0, "mif2_ff_aclk_mif_pll_div4", "mif2_mux_aclk_mif_pll",
			1, 4, 0),
		FFACTOR(0, "u_dfi_clk_gen_mif2", "mif2_mux_aclk_mif_pll", 1, 4,
			0),
	};

static const struct samsung_mux_clock mif2_mux_clks[] __initconst = {
	MUX(0, "mif2_mux_mif_pll", mif2_mux_mif_pll_p, CLK_CON_MUX_MIF2_PLL, 12,
	    1),
	MUX(0, "mif2_mux_bus_pll_user", mif2_mux_bus_pll_user_p,
	    CLK_CON_MUX_BUS_PLL_USER_MIF2, 12, 1),
	MUX(0, "mif2_mux_aclk_mif_pll", mif2_mux_aclk_mif_pll_p,
	    CLK_CON_MUX_ACLK_MIF2_PLL, 12, 1),
	MUX(0, "mif2_mux_pclk_mif", mif2_mux_pclk_mif_p, CLK_CON_MUX_PCLK_MIF2,
	    12, 2),
	MUX(0, "mif2_mux_sclk_hpm_mif", mif2_mux_sclk_hpm_mif_p,
	    CLK_CON_MUX_SCLK_HPM_MIF2, 12, 2),
	MUX(0, "mif2_mux_pclk_smc", mif2_mux_pclk_smc_p,
	    CLK_CON_MUX_PCLK_SMC_MIF2, 12, 2),
};

static const struct samsung_gate_clock mif2_gate_clks[] __initconst = {
	GATE(0, "mif2_gate_aclk_apscdc", "u_dfi_clk_gen_mif2",
	     CLK_ENABLE_ACLK_MIF2, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_aclk_ppc_debug", "u_dfi_clk_gen_mif2",
	     CLK_ENABLE_ACLK_MIF2, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_aclk_ppc_dvfs", "u_dfi_clk_gen_mif2",
	     CLK_ENABLE_ACLK_MIF2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_aclk_smc", "u_dfi_clk_gen_mif2",
	     CLK_ENABLE_ACLK_MIF2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_smc1", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_dmc_misc", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_ppc_debug", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_ppc_dvfs", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_sysreg_mif", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_hpm", "mif2_div_pclk_mif", CLK_ENABLE_PCLK_MIF2,
	     4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_aclk_axi_async", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_mifp", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_pmu_mif", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_lpddr4phy", "mif2_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_pclk_smc2", "mif2_div_pclk_mif",
	     CLK_ENABLE_ACLK_MIF2_SECURE_DREX_TZ, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_sclk_promise", "mif2_div_sclk_hpm_mif",
	     CLK_ENABLE_SCLK_HPM_MIF2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif2_gate_rclk_drex", "oscclk", CLK_ENABLE_SCLK_RCLK_DREX_MIF2,
	     0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock mif2_div_clks[] __initconst = {
	DIV(0, "mif2_div_pclk_mif", "mif2_mux_pclk_mif", CLK_CON_DIV_PCLK_MIF2,
	    0, 3),
	DIV(0, "mif2_div_sclk_hpm_mif", "mif2_mux_sclk_hpm_mif",
	    CLK_CON_DIV_SCLK_HPM_MIF2, 0, 2),
	DIV(0, "mif2_div_pclk_smc", "mif2_mux_pclk_smc",
	    CLK_CON_DIV_PCLK_SMC_MIF2, 0, 3),
};

static const struct samsung_cmu_info mif2_cmu_info __initconst = {
	.pll_clks = mif2_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(mif2_pll_clks),
	.mux_clks = mif2_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(mif2_mux_clks),
	.div_clks = mif2_div_clks,
	.nr_div_clks = ARRAY_SIZE(mif2_div_clks),
	.gate_clks = mif2_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(mif2_gate_clks),
	.fixed_factor_clks = mif2_fixed_factor_clks,
	.nr_fixed_factor_clks = ARRAY_SIZE(mif2_fixed_factor_clks),
	.nr_clk_ids = MIF2_NR_CLK,
};

/* Register Offset definitions for CMU_MIF3 (0x10B50000) */
#define MIF3_PLL_LOCK 0x0000
#define MIF3_PLL_CON0 0x0100
#define MIF3_PLL_CON1 0x0104
#define MIF3_PLL_FREQ_DET 0x010C
#define CLK_CON_MUX_MIF3_PLL 0x0200
#define CLK_CON_MUX_BUS_PLL_USER_MIF3 0x0204
#define CLK_CON_MUX_ACLK_MIF3_PLL 0x0208
#define CLK_CON_MUX_PCLK_MIF3 0x0210
#define CLK_CON_MUX_SCLK_HPM_MIF3 0x0214
#define CLK_CON_MUX_PCLK_SMC_MIF3 0x0218
#define CLK_CON_DIV_PCLK_MIF3 0x0400
#define CLK_CON_DIV_PCLK_SMC_MIF3 0x0404
#define CLK_CON_DIV_SCLK_HPM_MIF3 0x0408
#define CLK_STAT_MUX_MIF3_PLL 0x0600
#define CLK_STAT_MUX_BUS_PLL_USER_MIF3 0x0604
#define CLK_STAT_MUX_ACLK_MIF3_PLL 0x0608
#define CLK_STAT_MUX_PCLK_MIF3 0x0610
#define CLK_STAT_MUX_SCLK_HPM_MIF3 0x0614
#define CLK_STAT_MUX_PCLK_SMC_MIF3 0x0618
#define CLK_ENABLE_ACLK_MIF3 0x0800
#define CLK_ENABLE_PCLK_MIF3 0x0900
#define CLK_ENABLE_ACLK_MIF3_SECURE_DREX_TZ 0x0904
#define CLK_ENABLE_SCLK_HPM_MIF3 0x0A00
#define CLK_ENABLE_SCLK_RCLK_DREX_MIF3 0x0A04
#define CG_CTRL_VAL_PCLK_MIF3 0x0900
#define CG_CTRL_VAL_SCLK_HPM_MIF3 0x0A00
#define CG_CTRL_VAL_SCLK_RCLK_DREX3 0x0A04
#define CG_CTRL_VAL_DDRPHY3 0x0A08
#define CLKOUT_CMU_MIF3 0x0C00
#define CLKOUT_CMU_MIF3_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_MIF3 0x0E00
#define MIF3_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define PSCDC_CTRL_MIF3 0x1000
#define CLK_STOPCTRL_MIF3 0x1004
#define CMU_MIF3_SPARE0 0x1100
#define CMU_MIF3_SPARE1 0x1104
#define CG_CTRL_MAN_PCLK_MIF3 0x1900
#define CG_CTRL_MAN_SCLK_HPM_MIF3 0x1A00
#define CG_CTRL_MAN_SCLK_RCLK_DREX3 0x1A04
#define CG_CTRL_MAN_DDRPHY3 0x1A08
#define CG_CTRL_STAT_PCLK_MIF3_0 0x1D00
#define CG_CTRL_STAT_PCLK_MIF3_1 0x1D04
#define CG_CTRL_STAT_SCLK_HPM_MIF3 0x1E00
#define CG_CTRL_STAT_SCLK_RCLK_DREX3 0x1E04
#define CG_CTRL_STAT_DDRPHY3 0x1E08
#define QCH_CTRL_LH_AXI_P_MIF3 0x2000
#define QCH_CTRL_PMU_MIF3 0x2004
#define QCH_CTRL_SYSREG_MIF3 0x2008
#define QCH_CTRL_CMU_MIF3 0x200C
#define QCH_CTRL_SMC_MIF3 0x2010
#define QSTATE_CTRL_PROMISE_MIF3 0x2400
#define QSTATE_CTRL_RCLK_DREX3 0x2404

static const struct samsung_pll_clock mif3_pll_clks[] __initconst = {
	PLL(pll_1419x, 0, "mif3_pll", "oscclk", MIF3_PLL_LOCK, MIF3_PLL_CON0,
	    exynos8890_mif_pll_rates),
};

PNAME(mif3_mux_mif_pll_p) = { "oscclk", "mif3_pll" };
PNAME(mif3_mux_bus_pll_user_p) = { "oscclk", "top_gate_sclk_bus_pll_mif" };
PNAME(mif3_mux_aclk_mif_pll_p) = { "mif3_mux_mif_pll",
				   "mif3_mux_bus_pll_user" };
PNAME(mif3_mux_pclk_mif_p) = { "mif3_mux_aclk_mif_pll",
			       "mif3_ff_aclk_mif_pll_div2",
			       "mif3_ff_aclk_mif_pll_div4" };
PNAME(mif3_mux_sclk_hpm_mif_p) = { "mif3_mux_aclk_mif_pll",
				   "mif3_ff_aclk_mif_pll_div2",
				   "mif3_ff_aclk_mif_pll_div4" };
PNAME(mif3_mux_pclk_smc_p) = { "mif3_mux_aclk_mif_pll",
			       "mif3_ff_aclk_mif_pll_div2",
			       "mif3_ff_aclk_mif_pll_div4",
			       "u_dfi_clk_gen_mif3" };

static const struct samsung_fixed_factor_clock
	mif3_fixed_factor_clks[] __initconst = {
		FFACTOR(0, "mif3_ff_aclk_mif_pll_div2", "mif3_mux_aclk_mif_pll",
			1, 2, 0),
		FFACTOR(0, "mif3_ff_aclk_mif_pll_div4", "mif3_mux_aclk_mif_pll",
			1, 4, 0),
		FFACTOR(0, "u_dfi_clk_gen_mif3", "mif3_mux_aclk_mif_pll", 1, 4,
			0),
	};

static const struct samsung_mux_clock mif3_mux_clks[] __initconst = {
	MUX(0, "mif3_mux_mif_pll", mif3_mux_mif_pll_p, CLK_CON_MUX_MIF3_PLL, 12,
	    1),
	MUX(0, "mif3_mux_bus_pll_user", mif3_mux_bus_pll_user_p,
	    CLK_CON_MUX_BUS_PLL_USER_MIF3, 12, 1),
	MUX(0, "mif3_mux_aclk_mif_pll", mif3_mux_aclk_mif_pll_p,
	    CLK_CON_MUX_ACLK_MIF3_PLL, 12, 1),
	MUX(0, "mif3_mux_pclk_mif", mif3_mux_pclk_mif_p, CLK_CON_MUX_PCLK_MIF3,
	    12, 2),
	MUX(0, "mif3_mux_sclk_hpm_mif", mif3_mux_sclk_hpm_mif_p,
	    CLK_CON_MUX_SCLK_HPM_MIF3, 12, 2),
	MUX(0, "mif3_mux_pclk_smc", mif3_mux_pclk_smc_p,
	    CLK_CON_MUX_PCLK_SMC_MIF3, 12, 2),
};

static const struct samsung_gate_clock mif3_gate_clks[] __initconst = {
	GATE(0, "mif3_gate_aclk_apscdc", "u_dfi_clk_gen_mif3",
	     CLK_ENABLE_ACLK_MIF3, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_aclk_ppc_debug", "u_dfi_clk_gen_mif3",
	     CLK_ENABLE_ACLK_MIF3, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_aclk_ppc_dvfs", "u_dfi_clk_gen_mif3",
	     CLK_ENABLE_ACLK_MIF3, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_aclk_smc", "u_dfi_clk_gen_mif3",
	     CLK_ENABLE_ACLK_MIF3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_smc1", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_dmc_misc", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_ppc_debug", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_ppc_dvfs", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_sysreg_mif", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_hpm", "mif3_div_pclk_mif", CLK_ENABLE_PCLK_MIF3,
	     4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_aclk_axi_async", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_mifp", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_pmu_mif", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_lpddr4phy", "mif3_div_pclk_mif",
	     CLK_ENABLE_PCLK_MIF3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_pclk_smc2", "mif3_div_pclk_mif",
	     CLK_ENABLE_ACLK_MIF3_SECURE_DREX_TZ, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_sclk_promise", "mif3_div_sclk_hpm_mif",
	     CLK_ENABLE_SCLK_HPM_MIF3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mif3_gate_rclk_drex", "oscclk", CLK_ENABLE_SCLK_RCLK_DREX_MIF3,
	     0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock mif3_div_clks[] __initconst = {
	DIV(0, "mif3_div_pclk_mif", "mif3_mux_pclk_mif", CLK_CON_DIV_PCLK_MIF3,
	    0, 3),
	DIV(0, "mif3_div_sclk_hpm_mif", "mif3_mux_sclk_hpm_mif",
	    CLK_CON_DIV_SCLK_HPM_MIF3, 0, 2),
	DIV(0, "mif3_div_pclk_smc", "mif3_mux_pclk_smc",
	    CLK_CON_DIV_PCLK_SMC_MIF3, 0, 3),
};

static const struct samsung_cmu_info mif3_cmu_info __initconst = {
	.pll_clks = mif3_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(mif3_pll_clks),
	.mux_clks = mif3_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(mif3_mux_clks),
	.div_clks = mif3_div_clks,
	.nr_div_clks = ARRAY_SIZE(mif3_div_clks),
	.gate_clks = mif3_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(mif3_gate_clks),
	.fixed_factor_clks = mif3_fixed_factor_clks,
	.nr_fixed_factor_clks = ARRAY_SIZE(mif3_fixed_factor_clks),
	.nr_clk_ids = MIF3_NR_CLK,
};

/* Register Offset definitions for CMU_FSYS0 (0x10E90000) */
#define CLK_CON_MUX_ACLK_FSYS0_200_USER 0x0200
#define CLK_CON_MUX_SCLK_FSYS0_USBDRD30_USER 0x0204
#define CLK_CON_MUX_SCLK_FSYS0_MMC0_USER 0x0208
#define CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO_EMBEDDED_USER 0x020C
#define CLK_CON_MUX_SCLK_FSYS0_24M_USER 0x0210
#define CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO_EMBEDDED_CFG_USER 0x0214
#define CLK_CON_MUX_PHYCLK_USBDRD30_UDRD30_PHYCLOCK_USER 0x0218
#define CLK_CON_MUX_PHYCLK_USBDRD30_UDRD30_PIPE_PCLK_USER 0x021C
#define CLK_CON_MUX_PHYCLK_UFS_TX0_SYMBOL_USER 0x0220
#define CLK_CON_MUX_PHYCLK_UFS_RX0_SYMBOL_USER 0x0224
#define CLK_CON_MUX_PHYCLK_USBHOST20_PHYCLOCK_USER 0x0228
#define CLK_CON_MUX_PHYCLK_USBHOST20_FREECLK_USER 0x022C
#define CLK_CON_MUX_PHYCLK_USBHOST20_CLK48MOHCI_USER 0x0230
#define CLK_CON_MUX_PHYCLK_USBHOST20PHY_REF_CLK 0x0234
#define CLK_CON_MUX_PHYCLK_UFS_RX_PWM_CLK_USER 0x0238
#define CLK_CON_MUX_PHYCLK_UFS_TX_PWM_CLK_USER 0x023C
#define CLK_CON_MUX_PHYCLK_UFS_REFCLK_OUT_SOC_USER 0x0240
#define CLK_STAT_MUX_ACLK_FSYS0_200_USER 0x0600
#define CLK_STAT_MUX_SCLK_FSYS0_USBDRD30_USER 0x0604
#define CLK_STAT_MUX_SCLK_FSYS0_MMC0_USER 0x0608
#define CLK_STAT_MUX_SCLK_FSYS0_UFSUNIPRO_EMBEDDED_USER 0x060C
#define CLK_STAT_MUX_SCLK_FSYS0_24M_USER 0x0610
#define CLK_STAT_MUX_SCLK_FSYS0_UFSUNIPRO_EMBEDDED_CFG_USER 0x0614
#define CLK_STAT_MUX_PHYCLK_USBDRD30_UDRD30_PHYCLOCK_USER 0x0618
#define CLK_STAT_MUX_PHYCLK_USBDRD30_UDRD30_PIPE_PCLK_USER 0x061C
#define CLK_STAT_MUX_PHYCLK_UFS_TX0_SYMBOL_USER 0x0620
#define CLK_STAT_MUX_PHYCLK_UFS_RX0_SYMBOL_USER 0x0624
#define CLK_STAT_MUX_PHYCLK_USBHOST20_PHYCLOCK_USER 0x0628
#define CLK_STAT_MUX_PHYCLK_USBHOST20_FREECLK_USER 0x062C
#define CLK_STAT_MUX_PHYCLK_USBHOST20_CLK48MOHCI_USER 0x0630
#define CLK_STAT_MUX_PHYCLK_USBHOST20PHY_REF_CLK 0x0634
#define CLK_STAT_MUX_PHYCLK_UFS_RX_PWM_CLK_USER 0x0638
#define CLK_STAT_MUX_PHYCLK_UFS_TX_PWM_CLK_USER 0x063C
#define CLK_STAT_MUX_PHYCLK_UFS_REFCLK_OUT_SOC_USER 0x0640
#define CG_CTRL_VAL_ACLK_FSYS0_200 0x0800
#define CG_CTRL_VAL_PCLK_HPM_APBIF_FSYS0 0x0820
#define CG_CTRL_VAL_SCLK_USBDRD30_SUSPEND_CLK 0x0840
#define CG_CTRL_VAL_SCLK_MMC0 0x0844
#define CG_CTRL_VAL_SCLK_UFSUNIPRO_EMBEDDED 0x0848
#define CG_CTRL_VAL_SCLK_USBDRD30_REF_CLK 0x084C
#define CG_CTRL_VAL_PHYCLK_USBDRD30_UDRD30_PHYCLOCK 0x0850
#define CG_CTRL_VAL_PHYCLK_USBDRD30_UDRD30_PIPE_PCLK 0x0854
#define CG_CTRL_VAL_PHYCLK_UFS_TX0_SYMBOL 0x0858
#define CG_CTRL_VAL_PHYCLK_UFS_RX0_SYMBOL 0x085C
#define CG_CTRL_VAL_PHYCLK_USBHOST20_PHYCLOCK 0x0860
#define CG_CTRL_VAL_SCLK_USBHOST20_REF_CLK 0x0864
#define CG_CTRL_VAL_PHYCLK_USBHOST20_FREECLK 0x0864
#define CG_CTRL_VAL_PHYCLK_USBHOST20_CLK48MOHCI 0x0868
#define CG_CTRL_VAL_PHYCLK_UFS_RX_PWM_CLK 0x086C
#define CG_CTRL_VAL_PHYCLK_UFS_TX_PWM_CLK 0x0870
#define CG_CTRL_VAL_PHYCLK_UFS_REFCLK_OUT_SOC 0x0874
#define CG_CTRL_VAL_SCLK_PROMISE_FSYS0 0x0878
#define CG_CTRL_VAL_SCLK_USBHOST20PHY_REF_CLK 0x087C
#define CG_CTRL_VAL_SCLK_UFSUNIPRO_EMBEDDED_CFG 0x0880
#define CLKOUT_CMU_FSYS0 0x0C00
#define CLKOUT_CMU_FSYS0_DIV_STAT 0x0C04
#define FSYS0_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_FSYS0_SPARE0 0x0D08
#define CMU_FSYS0_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_FSYS0_200 0x1800
#define CG_CTRL_MAN_PCLK_HPM_APBIF_FSYS0 0x1820
#define CG_CTRL_MAN_SCLK_USBDRD30_SUSPEND_CLK 0x1840
#define CG_CTRL_MAN_SCLK_MMC0 0x1844
#define CG_CTRL_MAN_SCLK_UFSUNIPRO_EMBEDDED 0x1848
#define CG_CTRL_MAN_SCLK_USBDRD30_REF_CLK 0x184C
#define CG_CTRL_MAN_PHYCLK_USBDRD30_UDRD30_PHYCLOCK 0x1850
#define CG_CTRL_MAN_PHYCLK_USBDRD30_UDRD30_PIPE_PCLK 0x1854
#define CG_CTRL_MAN_PHYCLK_UFS_TX0_SYMBOL 0x1858
#define CG_CTRL_MAN_PHYCLK_UFS_RX0_SYMBOL 0x185C
#define CG_CTRL_MAN_PHYCLK_USBHOST20_PHYCLOCK 0x1860
#define CG_CTRL_MAN_PHYCLK_USBHOST20_FREECLK 0x1864
#define CG_CTRL_MAN_PHYCLK_USBHOST20_CLK48MOHCI 0x1868
#define CG_CTRL_MAN_PHYCLK_UFS_RX_PWM_CLK 0x186C
#define CG_CTRL_MAN_PHYCLK_UFS_TX_PWM_CLK 0x1870
#define CG_CTRL_MAN_PHYCLK_UFS_REFCLK_OUT_SOC 0x1874
#define CG_CTRL_MAN_SCLK_USBHOST20_REF_CLK 0x1864
#define CG_CTRL_MAN_SCLK_PROMISE_FSYS0 0x1878
#define CG_CTRL_MAN_SCLK_USBHOST20PHY_REF_CLK 0x187C
#define CG_CTRL_MAN_SCLK_UFSUNIPRO_EMBEDDED_CFG 0x1880
#define CG_CTRL_STAT_ACLK_FSYS0_200_0 0x1C00
#define CG_CTRL_STAT_ACLK_FSYS0_200_1 0x1C04
#define CG_CTRL_STAT_ACLK_FSYS0_200_2 0x1C08
#define CG_CTRL_STAT_ACLK_FSYS0_200_3 0x1C0C
#define CG_CTRL_STAT_ACLK_FSYS0_200_4 0x1C10
#define CG_CTRL_STAT_PCLK_HPM_APBIF_FSYS0 0x1C20
#define CG_CTRL_STAT_SCLK_USBDRD30_SUSPEND_CLK 0x1C40
#define CG_CTRL_STAT_SCLK_MMC0 0x1C44
#define CG_CTRL_STAT_SCLK_UFSUNIPRO_EMBEDDED 0x1C48
#define CG_CTRL_STAT_SCLK_USBDRD30_REF_CLK 0x1C4C
#define CG_CTRL_STAT_PHYCLK_USBDRD30_UDRD30_PHYCLOCK 0x1C50
#define CG_CTRL_STAT_PHYCLK_USBDRD30_UDRD30_PIPE_PCLK 0x1C54
#define CG_CTRL_STAT_PHYCLK_UFS_TX0_SYMBOL 0x1C58
#define CG_CTRL_STAT_PHYCLK_UFS_RX0_SYMBOL 0x1C5C
#define CG_CTRL_STAT_PHYCLK_USBHOST20_PHYCLOCK 0x1C60
#define CG_CTRL_STAT_PHYCLK_USBHOST20_FREECLK 0x1C64
#define CG_CTRL_STAT_PHYCLK_USBHOST20_CLK48MOHCI 0x1C68
#define CG_CTRL_STAT_PHYCLK_UFS_RX_PWM_CLK 0x1C6C
#define CG_CTRL_STAT_PHYCLK_UFS_TX_PWM_CLK 0x1C70
#define CG_CTRL_STAT_PHYCLK_UFS_REFCLK_OUT_SOC 0x1C74
#define CG_CTRL_STAT_SCLK_PROMISE_FSYS0 0x1C78
#define CG_CTRL_STAT_SCLK_USBHOST20PHY_REF_CLK 0x1C7C
#define CG_CTRL_STAT_SCLK_UFSUNIPRO_EMBEDDED_CFG 0x1C80
#define QCH_CTRL_AXI_LH_ASYNC_MI_TOP_FSYS0 0x2000
#define QCH_CTRL_AXI_LH_ASYNC_MI_ETR_USB_FSYS0 0x2004
#define QCH_CTRL_ETR_USB_FSYS0_ACLK 0x2008
#define QCH_CTRL_ETR_USB_FSYS0_PCLK 0x200C
#define QCH_CTRL_CMU_FSYS0 0x2010
#define QCH_CTRL_PMU_FSYS0 0x2014
#define QCH_CTRL_SYSREG_FSYS0 0x2018
#define QCH_CTRL_USBDRD30 0x201C
#define QCH_CTRL_MMC0 0x2020
#define QCH_CTRL_UFS_LINK_EMBEDDED 0x2024
#define QCH_CTRL_USBHOST20 0x2028
#define QCH_CTRL_PDMA0 0x202C
#define QCH_CTRL_PDMAS 0x2034
#define QCH_CTRL_PPMU_FSYS0 0x2038
#define QCH_CTRL_ACEL_LH_ASYNC_SI_TOP_FSYS0 0x203C
#define QCH_CTRL_USBDRD30_PHYCTRL 0x2040
#define QCH_CTRL_USBHOST20_PHYCTRL 0x2044
#define QSTATE_CTRL_USBDRD30 0x2400
#define QSTATE_CTRL_UFS_LINK_EMBEDDED 0x2404
#define QSTATE_CTRL_USBHOST20 0x2408
#define QSTATE_CTRL_USBHOST20_PHY 0x240C
#define QSTATE_CTRL_GPIO_FSYS0 0x2410
#define QSTATE_CTRL_HPM_APBIF_FSYS0 0x2414
#define QSTATE_CTRL_PROMISE_FSYS0 0x2418

PNAME(fsys0_mux_aclk_fsys0_200_user_p) = { "oscclk",
					   "top_gate_aclk_fsys0_200" };
PNAME(fsys0_mux_sclk_fsys0_usbdrd30_user_p) = {
	"oscclk", "top_gate_sclk_fsys0_usbdrd30"
};
PNAME(fsys0_mux_sclk_fsys0_mmc0_user_p) = { "oscclk",
					    "top_gate_sclk_fsys0_mmc0" };
PNAME(fsys0_mux_sclk_fsys0_ufsunipro_embedded_user_p) = {
	"oscclk", "top_gate_sclk_fsys0_ufsunipro20"
};
PNAME(fsys0_mux_sclk_fsys0_24m_user_p) = { "oscclk",
					   "top_gate_sclk_fsys0_phy_24m" };
PNAME(fsys0_mux_sclk_fsys0_ufsunipro_embedded_cfg_user_p) = {
	"oscclk", "top_gate_sclk_fsys0_ufsunipro_cfg"
};
PNAME(fsys0_mux_phyclk_usbdrd30_udrd30_phyclock_user_p) = {
	"oscclk", "phyclk_usbdrd30_udrd30_phyclock_phy"
};
PNAME(fsys0_mux_phyclk_usbdrd30_udrd30_pipe_pclk_user_p) = {
	"oscclk", "phyclk_usbdrd30_udrd30_pipe_pclk_phy"
};
PNAME(fsys0_mux_phyclk_ufs_tx0_symbol_user_p) = { "oscclk",
						  "phyclk_ufs_tx0_symbol_phy" };
PNAME(fsys0_mux_phyclk_ufs_rx0_symbol_user_p) = { "oscclk",
						  "phyclk_ufs_rx0_symbol_phy" };
PNAME(fsys0_mux_phyclk_usbhost20_phyclock_user_p) = {
	"oscclk", "phyclk_usbhost20_phyclock_phy"
};
PNAME(fsys0_mux_phyclk_usbhost20phy_ref_clk_p) = {
	"fsys0_mux_sclk_fsys0_24m_user", "phyclk_usb30_12mohci"
};

static const struct samsung_fixed_rate_clock fsys0_fixed_clks[] __initconst = {
	FRATE(0, "phyclk_usb30_12mohci", NULL, 0, 30000000),
	FRATE(0, "phyclk_usbdrd30_udrd30_phyclock_phy", NULL, 0, 60000000),
	FRATE(0, "phyclk_usbdrd30_udrd30_pipe_pclk_phy", NULL, 0, 125000000),
	FRATE(0, "phyclk_ufs_tx0_symbol_phy", NULL, 0, 300000000),
	FRATE(0, "phyclk_ufs_rx0_symbol_phy", NULL, 0, 300000000),
	FRATE(0, "phyclk_usbhost20_phyclock_phy", NULL, 0, 30000000),
};

static const struct samsung_mux_clock fsys0_mux_clks[] __initconst = {
	MUX(0, "fsys0_mux_aclk_fsys0_200_user", fsys0_mux_aclk_fsys0_200_user_p,
	    CLK_CON_MUX_ACLK_FSYS0_200_USER, 12, 1),
	MUX(0, "fsys0_mux_sclk_fsys0_usbdrd30_user",
	    fsys0_mux_sclk_fsys0_usbdrd30_user_p,
	    CLK_CON_MUX_SCLK_FSYS0_USBDRD30_USER, 12, 1),
	MUX(0, "fsys0_mux_sclk_fsys0_mmc0_user",
	    fsys0_mux_sclk_fsys0_mmc0_user_p, CLK_CON_MUX_SCLK_FSYS0_MMC0_USER,
	    12, 1),
	MUX(0, "fsys0_mux_sclk_fsys0_ufsunipro_embedded_user",
	    fsys0_mux_sclk_fsys0_ufsunipro_embedded_user_p,
	    CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO_EMBEDDED_USER, 12, 1),
	MUX(0, "fsys0_mux_sclk_fsys0_24m_user", fsys0_mux_sclk_fsys0_24m_user_p,
	    CLK_CON_MUX_SCLK_FSYS0_24M_USER, 12, 1),
	MUX(0, "fsys0_mux_sclk_fsys0_ufsunipro_embedded_cfg_user",
	    fsys0_mux_sclk_fsys0_ufsunipro_embedded_cfg_user_p,
	    CLK_CON_MUX_SCLK_FSYS0_UFSUNIPRO_EMBEDDED_CFG_USER, 12, 1),
	MUX(0, "fsys0_mux_phyclk_usbdrd30_udrd30_phyclock_user",
	    fsys0_mux_phyclk_usbdrd30_udrd30_phyclock_user_p,
	    CLK_CON_MUX_PHYCLK_USBDRD30_UDRD30_PHYCLOCK_USER, 12, 1),
	MUX(0, "fsys0_mux_phyclk_usbdrd30_udrd30_pipe_pclk_user",
	    fsys0_mux_phyclk_usbdrd30_udrd30_pipe_pclk_user_p,
	    CLK_CON_MUX_PHYCLK_USBDRD30_UDRD30_PIPE_PCLK_USER, 12, 1),
	MUX(0, "fsys0_mux_phyclk_ufs_tx0_symbol_user",
	    fsys0_mux_phyclk_ufs_tx0_symbol_user_p,
	    CLK_CON_MUX_PHYCLK_UFS_TX0_SYMBOL_USER, 12, 1),
	MUX(0, "fsys0_mux_phyclk_ufs_rx0_symbol_user",
	    fsys0_mux_phyclk_ufs_rx0_symbol_user_p,
	    CLK_CON_MUX_PHYCLK_UFS_RX0_SYMBOL_USER, 12, 1),
	MUX(0, "fsys0_mux_phyclk_usbhost20_phyclock_user",
	    fsys0_mux_phyclk_usbhost20_phyclock_user_p,
	    CLK_CON_MUX_PHYCLK_USBHOST20_PHYCLOCK_USER, 12, 1),
	MUX(0, "fsys0_mux_phyclk_usbhost20phy_ref_clk",
	    fsys0_mux_phyclk_usbhost20phy_ref_clk_p,
	    CLK_CON_MUX_PHYCLK_USBHOST20PHY_REF_CLK, 12, 1),
};

static const struct samsung_gate_clock fsys0_gate_clks[] __initconst = {
	GATE(0, "fsys0_gate_aclk_axi2acel_fsys0x",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 31,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_pclk_cmu_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 30, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_pclk_gpio_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 29, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_pclk_sysreg_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 28, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_pclk_ppmu_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 27, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_pclk_pmu_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 26, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_pclk_etr_usb_fsys0",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 25,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_hclk_usbhost20", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 24, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi_us_usbhs_fsys0x",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 21,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_etr_usb_fsys0",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 20,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_ufs_link_embedded",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 19,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_usbdrd30", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 18, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_mmc0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 17, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_pdmas", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_pdma0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_ppmu_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_xiu_fsys0sfrx",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 13,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi_us_usbdrd30x_fsys0x",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 12,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi_us_pdmax_fsys0x",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 11,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi2ahb_fsys0h",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 10,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi2ahb_usbdrd30h",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 9,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi_lh_async_mi_etr_usb_fsys0",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 8,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_xiu_pdmax", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_xiu_usbx", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_xiu_embeddedx",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_xiu_fsys0x", "fsys0_mux_aclk_fsys0_200_user",
	     CG_CTRL_VAL_ACLK_FSYS0_200, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi2apb_fsys0p",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_ahb_bridge_fsys0h",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_axi_lh_async_mi_top_fsys0",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_aclk_acel_lh_async_si_top_fsys0",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_ACLK_FSYS0_200, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_pclk_hpm_apbif_fsys0",
	     "fsys0_mux_aclk_fsys0_200_user", CG_CTRL_VAL_PCLK_HPM_APBIF_FSYS0,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_usbdrd30_suspend_clk",
	     "fsys0_mux_sclk_fsys0_usbdrd30_user",
	     CG_CTRL_VAL_SCLK_USBDRD30_SUSPEND_CLK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_mmc0", "fsys0_mux_sclk_fsys0_mmc0_user",
	     CG_CTRL_VAL_SCLK_MMC0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_ufsunipro_embedded",
	     "fsys0_mux_sclk_fsys0_ufsunipro_embedded_user",
	     CG_CTRL_VAL_SCLK_UFSUNIPRO_EMBEDDED, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_usbdrd30_ref_clk",
	     "fsys0_mux_sclk_fsys0_usbdrd30_user",
	     CG_CTRL_VAL_SCLK_USBDRD30_REF_CLK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_phyclk_usbdrd30_udrd30_phyclock",
	     "fsys0_mux_phyclk_usbdrd30_udrd30_phyclock_user",
	     CG_CTRL_VAL_PHYCLK_USBDRD30_UDRD30_PHYCLOCK, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "fsys0_gate_phyclk_usbdrd30_udrd30_pipe_pclk",
	     "fsys0_mux_phyclk_usbdrd30_udrd30_pipe_pclk_user",
	     CG_CTRL_VAL_PHYCLK_USBDRD30_UDRD30_PIPE_PCLK, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "fsys0_gate_phyclk_ufs_tx0_symbol",
	     "fsys0_mux_phyclk_ufs_tx0_symbol_user",
	     CG_CTRL_VAL_PHYCLK_UFS_TX0_SYMBOL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_phyclk_ufs_rx0_symbol",
	     "fsys0_mux_phyclk_ufs_rx0_symbol_user",
	     CG_CTRL_VAL_PHYCLK_UFS_RX0_SYMBOL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_phyclk_usbhost20_phyclock",
	     "fsys0_mux_phyclk_usbhost20_phyclock_user",
	     CG_CTRL_VAL_PHYCLK_USBHOST20_PHYCLOCK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_promise_fsys0", "top_gate_sclk_promise_int",
	     CG_CTRL_VAL_SCLK_PROMISE_FSYS0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_usbhost20phy_ref_clk",
	     "fsys0_mux_phyclk_usbhost20phy_ref_clk",
	     CG_CTRL_VAL_SCLK_USBHOST20PHY_REF_CLK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_ufsunipro_embedded_cfg",
	     "fsys0_mux_sclk_fsys0_ufsunipro_embedded_cfg_user",
	     CG_CTRL_VAL_SCLK_UFSUNIPRO_EMBEDDED_CFG, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_gate_sclk_usbhost20_ref_clk",
	     "fsys0_mux_sclk_fsys0_usbdrd30_user",
	     CG_CTRL_VAL_SCLK_USBHOST20_REF_CLK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_usbdrd30", "fsys0_mux_aclk_fsys0_200_user",
	     QSTATE_CTRL_USBDRD30, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_qch_usbdrd30",
	     "phyclk_usbdrd30_udrd30_phyclock_phy", QCH_CTRL_USBDRD30, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_ufs_link_embedded",
	     "fsys0_mux_aclk_fsys0_200_user", QSTATE_CTRL_UFS_LINK_EMBEDDED, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_usbhost20", "fsys0_mux_aclk_fsys0_200_user",
	     QSTATE_CTRL_USBHOST20, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_usbhost20_phy", "fsys0_mux_aclk_fsys0_200_user",
	     QSTATE_CTRL_USBHOST20_PHY, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_gpio_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     QSTATE_CTRL_GPIO_FSYS0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_hpm_apbif_fsys0", "fsys0_mux_aclk_fsys0_200_user",
	     QSTATE_CTRL_HPM_APBIF_FSYS0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys0_hwacg_promise_fsys0", "top_gate_sclk_promise_int",
	     QSTATE_CTRL_PROMISE_FSYS0, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info fsys0_cmu_info __initconst = {
	.mux_clks = fsys0_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(fsys0_mux_clks),
	.gate_clks = fsys0_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(fsys0_gate_clks),
	.fixed_clks = fsys0_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(fsys0_fixed_clks),
	.nr_clk_ids = FSYS0_NR_CLK,
};

/* Register Offset definitions for CMU_IMEM (0x11060000) */
#define CLK_CON_MUX_ACLK_IMEM_266_USER 0x0200
#define CLK_CON_MUX_ACLK_IMEM_200_USER 0x0204
#define CLK_CON_MUX_ACLK_IMEM_100_USER 0x0208
#define CLK_STAT_MUX_ACLK_IMEM_266_USER 0x0600
#define CLK_STAT_MUX_ACLK_IMEM_200_USER 0x0604
#define CLK_STAT_MUX_ACLK_IMEM_100_USER 0x0608
#define CG_CTRL_VAL_ACLK_IMEM_266 0x0800
#define CG_CTRL_VAL_ACLK_IMEM_266_SECURE_SSS 0x0804
#define CG_CTRL_VAL_ACLK_IMEM_266_SECURE_RTIC 0x0808
#define CG_CTRL_VAL_ACLK_IMEM_200 0x080C
#define CG_CTRL_VAL_PCLK_IMEM_200_SECURE_SSS 0x0810
#define CG_CTRL_VAL_PCLK_IMEM_200_SECURE_RTIC 0x0814
#define CG_CTRL_VAL_ACLK_IMEM_100 0x0818
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_CM3_APM 0x081C
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_APM 0x0820
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_AHB_BUSMATRIX_APM 0x0820
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_ISRAMC_APM 0x0824
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_AHB2APB_BRIDGE_APM 0x0828
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_MAILBOX_APM 0x082C
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_TIMER_APM 0x0830
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_WDT_APM 0x0834
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_ISRAMC_SFR_APM 0x0838
#define CG_CTRL_VAL_ACLK_IMEM_100_SECURE_SFR_APM 0x083C
#define CLKOUT_CMU_IMEM 0x0C00
#define CLKOUT_CMU_IMEM_DIV_STAT 0x0C04
#define CMU_IMEM_SPARE0 0x0C08
#define CMU_IMEM_SPARE1 0x0C0C
#define IMEM_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CG_CTRL_MAN_ACLK_IMEM_266 0x1800
#define CG_CTRL_MAN_ACLK_IMEM_266_SECURE_SSS 0x1804
#define CG_CTRL_MAN_ACLK_IMEM_266_SECURE_RTIC 0x1808
#define CG_CTRL_MAN_ACLK_IMEM_200 0x180C
#define CG_CTRL_MAN_PCLK_IMEM_200_SECURE_SSS 0x1810
#define CG_CTRL_MAN_PCLK_IMEM_200_SECURE_RTIC 0x1814
#define CG_CTRL_MAN_ACLK_IMEM_100 0x1818
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_CM3_APM 0x181C
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_AHB_BUSMATRIX_APM 0x1820
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_ISRAMC_APM 0x1824
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_AHB2APB_BRIDGE_APM 0x1828
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_MAILBOX_APM 0x182C
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_TIMER_APM 0x1830
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_WDT_APM 0x1834
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_ISRAMC_SFR_APM 0x1838
#define CG_CTRL_MAN_ACLK_IMEM_100_SECURE_SFR_APM 0x183C
#define CG_CTRL_STAT_ACLK_IMEM_266_0 0x1C00
#define CG_CTRL_STAT_ACLK_IMEM_266_1 0x1C04
#define CG_CTRL_STAT_ACLK_IMEM_266_SECURE_SSS 0x1C08
#define CG_CTRL_STAT_ACLK_IMEM_266_SECURE_RTIC 0x1C0C
#define CG_CTRL_STAT_ACLK_IMEM_200_0 0x1C10
#define CG_CTRL_STAT_ACLK_IMEM_200_1 0x1C14
#define CG_CTRL_STAT_ACLK_IMEM_200_2 0x1C18
#define CG_CTRL_STAT_PCLK_IMEM_200_SECURE_SSS 0x1C1C
#define CG_CTRL_STAT_PCLK_IMEM_200_SECURE_RTIC 0x1C20
#define CG_CTRL_STAT_ACLK_IMEM_100 0x1C24
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_CM3_APM 0x1C28
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_AHB_BUSMATRIX_APM 0x1C2C
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_ISRAMC_APM 0x1C30
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_AHB2APB_BRIDGE_APM 0x1C34
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_MAILBOX_APM 0x1C38
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_TIMER_APM 0x1C3C
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_WDT_APM 0x1C40
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_ISRAMC_SFR_APM 0x1C44
#define CG_CTRL_STAT_ACLK_IMEM_100_SECURE_SFR_APM 0x1C48
#define QCH_CTRL_AXI_LH_ASYNC_MI_IMEM 0x2000
#define QCH_CTRL_SSS 0x2004
#define QCH_CTRL_RTIC 0x2008
#define QCH_CTRL_INT_MEM 0x200C
#define QCH_CTRL_INT_MEM_ALV 0x2010
#define QCH_CTRL_MCOMP 0x2014
#define QCH_CTRL_CMU_IMEM 0x2018
#define QCH_CTRL_PMU_IMEM 0x201C
#define QCH_CTRL_SYSREG_IMEM 0x2020
#define QCH_CTRL_PPMU_SSSX 0x2024
#define QCH_CTRL_LH_ASYNC_SI_IMEM 0x2028
#define QCH_CTRL_APM 0x202C
#define QCH_CTRL_CM3_APM 0x2030
#define QSTATE_CTRL_GIC 0x2400
#define QSTATE_CTRL_APM 0x2404
#define QSTATE_CTRL_ASYNCAHBM_SSS_ATLAS 0x2408

PNAME(imem_mux_aclk_imem_266_user_p) = { "oscclk", "top_gate_aclk_imem_266" };
PNAME(imem_mux_aclk_imem_200_user_p) = { "oscclk", "top_gate_aclk_imem_200" };
PNAME(imem_mux_aclk_imem_100_user_p) = { "oscclk", "top_gate_aclk_imem_100" };

static const struct samsung_mux_clock imem_mux_clks[] __initconst = {
	MUX(0, "imem_mux_aclk_imem_266_user", imem_mux_aclk_imem_266_user_p,
	    CLK_CON_MUX_ACLK_IMEM_266_USER, 12, 1),
	MUX(0, "imem_mux_aclk_imem_200_user", imem_mux_aclk_imem_200_user_p,
	    CLK_CON_MUX_ACLK_IMEM_200_USER, 12, 1),
	MUX(0, "imem_mux_aclk_imem_100_user", imem_mux_aclk_imem_100_user_p,
	    CLK_CON_MUX_ACLK_IMEM_100_USER, 12, 1),
};

static const struct samsung_gate_clock imem_gate_clks[] __initconst = {
	GATE(0, "imem_gate_aclk_mc", "imem_mux_aclk_imem_266_user",
	     CG_CTRL_VAL_ACLK_IMEM_266, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_xiu_3x1_sss", "imem_mux_aclk_imem_266_user",
	     CG_CTRL_VAL_ACLK_IMEM_266, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_axi_us_apm", "imem_mux_aclk_imem_266_user",
	     CG_CTRL_VAL_ACLK_IMEM_266, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_asyncahbmstm_apm",
	     "imem_mux_aclk_imem_266_user", CG_CTRL_VAL_ACLK_IMEM_266, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_asyncahbm_sss_atlas",
	     "imem_mux_aclk_imem_266_user", CG_CTRL_VAL_ACLK_IMEM_266, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_lh_async_si_imem",
	     "imem_mux_aclk_imem_266_user", CG_CTRL_VAL_ACLK_IMEM_266, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_ppmu_sssx", "imem_mux_aclk_imem_266_user",
	     CG_CTRL_VAL_ACLK_IMEM_266, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_xiu_imemx", "imem_mux_aclk_imem_266_user",
	     CG_CTRL_VAL_ACLK_IMEM_266, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_sss", "imem_mux_aclk_imem_266_user",
	     CG_CTRL_VAL_ACLK_IMEM_266_SECURE_SSS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_rtic", "imem_mux_aclk_imem_266_user",
	     CG_CTRL_VAL_ACLK_IMEM_266_SECURE_RTIC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_pclk_cmu_imem", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_pclk_sysreg_imem", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_pclk_mc", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_pclk_ppmu_sssx", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_pclk_pmu_imem", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_asyncahbss_apm", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_axi2ahb_apm", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_int_mem_alv", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_int_mem", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_axids_pimemx_imem",
	     "imem_mux_aclk_imem_200_user", CG_CTRL_VAL_ACLK_IMEM_200, 6,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_axilhasyncm_pimemx",
	     "imem_mux_aclk_imem_200_user", CG_CTRL_VAL_ACLK_IMEM_200, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_axi2apb_imem_1", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_axi2apb_imem_0", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_xiu_pimemx1", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_xiu_pimemx0", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_gic", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_ACLK_IMEM_200, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_pclk_sss", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_PCLK_IMEM_200_SECURE_SSS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_pclk_rtic", "imem_mux_aclk_imem_200_user",
	     CG_CTRL_VAL_PCLK_IMEM_200_SECURE_RTIC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_asyncahbsm_apm", "imem_mux_aclk_imem_100_user",
	     CG_CTRL_VAL_ACLK_IMEM_100, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_ahb2axi_apm", "imem_mux_aclk_imem_100_user",
	     CG_CTRL_VAL_ACLK_IMEM_266, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_asyncahbmsts_apm",
	     "imem_mux_aclk_imem_100_user", CG_CTRL_VAL_ACLK_IMEM_100, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_cm3_apm", "imem_mux_aclk_imem_100_user",
	     CG_CTRL_VAL_ACLK_IMEM_100_SECURE_CM3_APM, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_ahb_busmatrix_apm",
	     "imem_mux_aclk_imem_100_user",
	     CG_CTRL_VAL_ACLK_IMEM_100_SECURE_AHB_BUSMATRIX_APM, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_sclk_cm3_apm", "imem_mux_aclk_imem_100_user",
	     CG_CTRL_VAL_ACLK_IMEM_100_SECURE_CM3_APM, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_gate_aclk_apm", "imem_mux_aclk_imem_100_user",
	     CG_CTRL_VAL_ACLK_IMEM_100_SECURE_APM, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_hwacg_gic", "imem_mux_aclk_imem_200_user",
	     QSTATE_CTRL_GIC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "imem_hwacg_asyncahbm_sss_atlas", "imem_mux_aclk_imem_266_user",
	     QSTATE_CTRL_ASYNCAHBM_SSS_ATLAS, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info imem_cmu_info __initconst = {
	.mux_clks = imem_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(imem_mux_clks),
	.gate_clks = imem_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(imem_gate_clks),
	.nr_clk_ids = IMEM_NR_CLK,
};

/* Register Offset definitions for CMU_AUD (0x114C0000) */
#define CLK_CON_MUX_AUD_PLL_USER 0x0200
#define CLK_CON_MUX_SCLK_I2S 0x0204
#define CLK_CON_MUX_SCLK_PCM 0x0208
#define CLK_CON_MUX_CP2AP_AUD_CLK_USER 0x020C
#define CLK_CON_MUX_ACLK_CA5 0x0210
#define CLK_CON_MUX_CDCLK_AUD 0x0214
#define CLK_CON_DIV_AUD_CA5 0x0400
#define CLK_CON_DIV_ACLK_AUD 0x0404
#define CLK_CON_DIV_PCLK_DBG 0x0408
#define CLK_CON_DIV_ATCLK_AUD 0x040C
#define CLK_CON_DIV_AUD_CDCLK 0x0410
#define CLK_CON_DIV_SCLK_I2S 0x0414
#define CLK_CON_DIV_SCLK_PCM 0x0418
#define CLK_CON_DIV_SCLK_SLIMBUS 0x041C
#define CLK_CON_DIV_SCLK_CP_I2S 0x0424
#define CLK_CON_DIV_SCLK_ASRC 0x0428
#define CLK_CON_DIV_CP_CA5 0x042C
#define CLK_CON_DIV_CP_CDCLK 0x0430
#define CLK_STAT_MUX_AUD_PLL_USER 0x0600
#define CLK_STAT_MUX_CP2AP_AUD_CLK_USER 0x060C
#define CLK_STAT_MUX_ACLK_CA5 0x0610
#define CLK_STAT_MUX_CDCLK_AUD 0x0614
#define CLK_ENABLE_SCLK_CA5 0x0800
#define CLK_ENABLE_ACLK_AUD 0x0804
#define CLK_ENABLE_PCLK_AUD 0x0808
#define CLK_ENABLE_ACLK_ATCLK_AUD 0x080C
#define CLK_ENABLE_SCLK_I2S 0x0810
#define CLK_ENABLE_SCLK_PCM 0x0814
#define CLK_ENABLE_SCLK_SLIMBUS 0x0818
#define CLK_ENABLE_SCLK_CP_I2S 0x081C
#define CLK_ENABLE_SCLK_ASRC 0x0820
#define CLK_ENABLE_SCLK_SLIMBUS_CLKIN 0x0824
#define CLK_ENABLE_SCLK_I2S_BCLK 0x0828
#define CLKOUT_CMU_AUD 0x0D00
#define CLKOUT_CMU_AUD_DIV_STAT 0x0D04
#define CMU_AUD_SPARE0 0x0D08
#define CMU_AUD_SPARE1 0x0D0C
#define CLK_ENABLE_PDN_AUD 0x0E00
#define AUD_SFR_IGNORE_REQ_SYSCLK 0x0F28

PNAME(aud_mux_aud_pll_user_p) = { "oscclk", "aud_pll" };
PNAME(aud_mux_sclk_i2s_p) = { "aud_mux_cdclk_aud", "ioclk_audiocdclk0" };
PNAME(aud_mux_sclk_pcm_p) = { "aud_mux_cdclk_aud", "ioclk_audiocdclk0" };
PNAME(aud_mux_cp2ap_aud_clk_user_p) = { "oscclk", "sclk_cp2ap_aud_clk" };
PNAME(aud_mux_aclk_ca5_p) = { "aud_div_aud_ca5", "aud_div_cp_ca5" };
PNAME(aud_mux_cdclk_aud_p) = { "aud_div_aud_cdclk", "aud_div_cp_cdclk" };

static const struct samsung_fixed_rate_clock aud_fixed_clks[] __initconst = {
	FRATE(0, "sclk_cp2ap_aud_clk", NULL, 0, 30000000),
	FRATE(0, "ioclk_audiocdclk0", NULL, 0, 30000000),
	FRATE(0, "ioclk_slimbus_clk", NULL, 0, 30000000),
	FRATE(0, "ioclk_i2s_bclk", NULL, 0, 30000000),
};

static const struct samsung_mux_clock aud_mux_clks[] __initconst = {
	MUX(0, "aud_mux_aud_pll_user", aud_mux_aud_pll_user_p,
	    CLK_CON_MUX_AUD_PLL_USER, 12, 1),
	MUX(0, "aud_mux_sclk_i2s", aud_mux_sclk_i2s_p, CLK_CON_MUX_SCLK_I2S, 12,
	    1),
	MUX(0, "aud_mux_sclk_pcm", aud_mux_sclk_pcm_p, CLK_CON_MUX_SCLK_PCM, 12,
	    1),
	MUX(0, "aud_mux_cp2ap_aud_clk_user", aud_mux_cp2ap_aud_clk_user_p,
	    CLK_CON_MUX_CP2AP_AUD_CLK_USER, 12, 1),
	MUX(0, "aud_mux_aclk_ca5", aud_mux_aclk_ca5_p, CLK_CON_MUX_ACLK_CA5, 12,
	    1),
	MUX(0, "aud_mux_cdclk_aud", aud_mux_cdclk_aud_p, CLK_CON_MUX_CDCLK_AUD,
	    12, 1),
};

static const struct samsung_gate_clock aud_gate_clks[] __initconst = {
	GATE(0, "aud_gate_sclk_ca5", "aud_mux_aclk_ca5", CLK_ENABLE_SCLK_CA5, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_ppmu_aud", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 20, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_cp_i2s", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     19, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_sysreg_aud", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 18, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_gpio_aud", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 17, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_pmu_aud", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_slimbus", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_pcm", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_i2s", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_timer", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_sfr1", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_sfr0", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_smmu", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_ppmu_aud", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_intr", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_xiu_lpassx", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_smmu", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_axi_lh_async_si_top", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_axi_lh_async_mi_top", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_axi_us_32to64", "aud_div_aclk_aud",
	     CLK_ENABLE_ACLK_AUD, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_sramc", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_dmac", "aud_div_aclk_aud", CLK_ENABLE_ACLK_AUD,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_pclk_dbg", "aud_div_pclk_dbg", CLK_ENABLE_PCLK_AUD, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_aclk_atclk_aud", "aud_div_atclk_aud",
	     CLK_ENABLE_ACLK_ATCLK_AUD, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_sclk_i2s", "aud_div_sclk_i2s", CLK_ENABLE_SCLK_I2S, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_sclk_pcm", "aud_div_sclk_pcm", CLK_ENABLE_SCLK_PCM, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_sclk_slimbus", "aud_div_sclk_slimbus",
	     CLK_ENABLE_SCLK_SLIMBUS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_sclk_cp_i2s", "aud_div_sclk_cp_i2s",
	     CLK_ENABLE_SCLK_CP_I2S, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_sclk_asrc", "aud_div_sclk_asrc", CLK_ENABLE_SCLK_ASRC,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_sclk_slimbus_clkin", "ioclk_slimbus_clk",
	     CLK_ENABLE_SCLK_SLIMBUS_CLKIN, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "aud_gate_sclk_i2s_bclk", "ioclk_i2s_bclk",
	     CLK_ENABLE_SCLK_I2S_BCLK, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock aud_div_clks[] __initconst = {
	DIV(0, "aud_div_aud_ca5", "aud_mux_aud_pll_user", CLK_CON_DIV_AUD_CA5,
	    0, 4),
	DIV(0, "aud_div_aclk_aud", "aud_mux_aclk_ca5", CLK_CON_DIV_ACLK_AUD, 0,
	    4),
	DIV(0, "aud_div_pclk_dbg", "aud_mux_aclk_ca5", CLK_CON_DIV_PCLK_DBG, 0,
	    4),
	DIV(0, "aud_div_atclk_aud", "aud_mux_aclk_ca5", CLK_CON_DIV_ATCLK_AUD,
	    0, 4),
	DIV(0, "aud_div_aud_cdclk", "aud_mux_aud_pll_user",
	    CLK_CON_DIV_AUD_CDCLK, 0, 4),
	DIV(0, "aud_div_sclk_i2s", "aud_mux_sclk_i2s", CLK_CON_DIV_SCLK_I2S, 0,
	    4),
	DIV(0, "aud_div_sclk_pcm", "aud_mux_sclk_pcm", CLK_CON_DIV_SCLK_PCM, 0,
	    8),
	DIV(0, "aud_div_sclk_slimbus", "aud_mux_cdclk_aud",
	    CLK_CON_DIV_SCLK_SLIMBUS, 0, 5),
	DIV(0, "aud_div_sclk_cp_i2s", "aud_mux_cdclk_aud",
	    CLK_CON_DIV_SCLK_CP_I2S, 0, 6),
	DIV(0, "aud_div_sclk_asrc", "aud_mux_cdclk_aud", CLK_CON_DIV_SCLK_ASRC,
	    0, 5),
	DIV(0, "aud_div_cp_ca5", "aud_mux_cp2ap_aud_clk_user",
	    CLK_CON_DIV_CP_CA5, 0, 5),
	DIV(0, "aud_div_cp_cdclk", "aud_mux_cp2ap_aud_clk_user",
	    CLK_CON_DIV_CP_CDCLK, 0, 5),
};

static const struct samsung_cmu_info aud_cmu_info __initconst = {
	.mux_clks = aud_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(aud_mux_clks),
	.div_clks = aud_div_clks,
	.nr_div_clks = ARRAY_SIZE(aud_div_clks),
	.gate_clks = aud_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(aud_gate_clks),
	.fixed_clks = aud_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(aud_fixed_clks),
	.nr_clk_ids = AUD_NR_CLK,
};

/* Register Offset definitions for CMU_MNGS (0x11800000) */
#define MNGS_PLL_LOCK 0x0000
#define MNGS_PLL_CON0 0x0100
#define MNGS_PLL_CON1 0x0104
#define MNGS_PLL_FREQ_DET 0x010C
#define CLK_CON_MUX_MNGS_PLL 0x0200
#define CLK_CON_MUX_BUS_PLL_MNGS_USER 0x0204
#define CLK_CON_MUX_MNGS 0x0208
#define CLK_CON_DIV_MNGS 0x0400
#define CLK_CON_DIV_ACLK_MNGS 0x0404
#define CLK_CON_DIV_ATCLK_MNGS_CORE 0x0408
#define CLK_CON_DIV_ATCLK_MNGS_SOC 0x040C
#define CLK_CON_DIV_ATCLK_MNGS_CSSYS_TRACECLK 0x0410
#define CLK_CON_DIV_ATCLK_MNGS_ASYNCATB_CAM1 0x0414
#define CLK_CON_DIV_ATCLK_MNGS_ASYNCATB_AUD 0x0418
#define CLK_CON_DIV_PCLK_DBG_MNGS 0x041C
#define CLK_CON_DIV_PCLK_RUN_MONITOR 0x0420
#define CLK_CON_DIV_PCLK_MNGS 0x0424
#define CLK_CON_DIV_CNTCLK_MNGS 0x0428
#define CLK_CON_DIV_MNGS_RUN_MONITOR 0x042C
#define CLK_CON_DIV_SCLK_PROMISE_MNGS 0x0430
#define CLK_CON_DIV_MNGS_PLL 0x0434
#define CLK_STAT_MUX_MNGS_PLL 0x0600
#define CLK_STAT_MUX_BUS_PLL_MNGS_USER 0x0604
#define CLK_STAT_MUX_MNGS 0x0608
#define CLK_ENABLE_ACLK_MNGS 0x0800
#define CLK_ENABLE_ATCLK_MNGS_CORE 0x0804
#define CLK_ENABLE_ATCLK_MNGS_SOC 0x0808
#define CLK_ENABLE_ATCLK_MNGS_CSSYS_TRACECLK 0x080C
#define CLK_ENABLE_ATCLK_MNGS_ASYNCATB_CAM1 0x0810
#define CLK_ENABLE_ATCLK_MNGS_ASYNCATB_AUD 0x0814
#define CLK_ENABLE_PCLK_DBG_MNGS 0x0900
#define CLK_ENABLE_PCLK_MNGS 0x0904
#define CLK_ENABLE_PCLK_HPM_MNGS 0x0908
#define CLK_ENABLE_SCLK_MNGS 0x0A00
#define CLK_ENABLE_SCLK_PROMISE_MNGS 0x0A04
#define CLKOUT_CMU_MNGS 0x0C00
#define CLKOUT_CMU_MNGS_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_MNGS 0x0D00
#define MNGS_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_MNGS_SPARE0 0x0D08
#define CMU_MNGS_SPARE1 0x0D0C
#define ARMCLK_STOPCTRL_MNGS 0x1000
#define PWR_CTRL_MNGS 0x1020
#define PWR_CTRL2_MNGS 0x1024
#define PWR_CTRL3_MNGS 0x1028
#define PWR_CTRL4_MNGS 0x102C
#define INTR_SPREAD_ENABLE_MNGS 0x1080
#define INTR_SPREAD_USE_STANDBYWFI_MNGS 0x1084
#define INTR_SPREAD_BLOCKING_DURATION_MNGS 0x1088

/* MNGS_PLL */
static const struct samsung_pll_rate_table
	exynos8890_mngs_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 3016000000U, 348, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2912000000U, 336, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2808000000U, 324, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2704000000U, 312, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2600000000U, 300, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2496000000U, 288, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2392000000U, 276, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2288000000U, 264, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2184000000U, 252, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 2080000000U, 240, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1976000000U, 228, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1872000000U, 216, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1768000000U, 204, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1664000000U, 192, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1560000000U, 360, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 1456000000U, 336, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 1352000000U, 312, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 1248000000U, 288, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 1144000000U, 264, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 1040000000U, 240, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 936000000U, 216, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 832000000U, 192, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 728000000U, 336, 3, 2),
		PLL_35XX_RATE(26 * MHZ, 624000000U, 288, 3, 2),
		PLL_35XX_RATE(26 * MHZ, 520000000U, 240, 3, 2),
		PLL_35XX_RATE(26 * MHZ, 416000000U, 192, 3, 2),
		PLL_35XX_RATE(26 * MHZ, 312000000U, 288, 3, 3),
		PLL_35XX_RATE(26 * MHZ, 208000000U, 192, 3, 3),
		{ /* sentinel */ }
	};

static const struct samsung_pll_clock mngs_pll_clks[] __initconst = {
	PLL(pll_141xx, 0, "mngs_pll", "oscclk", MNGS_PLL_LOCK, MNGS_PLL_CON0,
	    exynos8890_mngs_pll_rates),
};

PNAME(mngs_mux_mngs_pll_p) = { "oscclk", "mngs_pll" };
PNAME(mngs_mux_bus_pll_mngs_user_p) = { "oscclk",
					"top_gate_sclk_bus_pll_mngs" };
PNAME(mngs_mux_mngs_p) = { "mngs_mux_mngs_pll", "mngs_mux_bus_pll_mngs_user" };

static const struct samsung_mux_clock mngs_mux_clks[] __initconst = {
	MUX(0, "mngs_mux_mngs_pll", mngs_mux_mngs_pll_p, CLK_CON_MUX_MNGS_PLL,
	    12, 1),
	MUX(0, "mngs_mux_bus_pll_mngs_user", mngs_mux_bus_pll_mngs_user_p,
	    CLK_CON_MUX_BUS_PLL_MNGS_USER, 12, 1),
	MUX(0, "mngs_mux_mngs", mngs_mux_mngs_p, CLK_CON_MUX_MNGS, 12, 1),
};

static const struct samsung_gate_clock mngs_gate_clks[] __initconst = {
	GATE(0, "mngs_gate_aclk_asyncpaces_mngs_sci", "mngs_div_aclk_mngs",
	     CLK_ENABLE_ACLK_MNGS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclks_atb_mngs3_cssys", "mngs_div_atclk_mngs_core",
	     CLK_ENABLE_ATCLK_MNGS_CORE, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclks_atb_mngs2_cssys", "mngs_div_atclk_mngs_core",
	     CLK_ENABLE_ATCLK_MNGS_CORE, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclks_atb_mngs1_cssys", "mngs_div_atclk_mngs_core",
	     CLK_ENABLE_ATCLK_MNGS_CORE, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclks_atb_mngs0_cssys", "mngs_div_atclk_mngs_core",
	     CLK_ENABLE_ATCLK_MNGS_CORE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_xiu_mngsx_2x1", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_stm_txactor", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_atb_bdu_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_atb_aud_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_atb_cam1_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_atb_apollo3_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_atb_apollo2_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_atb_apollo1_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_atb_apollo0_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclkm_atb_mngs3_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclkm_atb_mngs2_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclkm_atb_mngs1_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclkm_atb_mngs0_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_asyncahb_cssys_sss_aclk",
	     "mngs_div_atclk_mngs_soc", CLK_ENABLE_ATCLK_MNGS_SOC, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_asynclhaxi_cssys_etr_aclk",
	     "mngs_div_atclk_mngs_soc", CLK_ENABLE_ATCLK_MNGS_SOC, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_cssys_hclk", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_cssys", "mngs_div_atclk_mngs_soc",
	     CLK_ENABLE_ATCLK_MNGS_SOC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_cssys_traceclk",
	     "mngs_div_atclk_mngs_cssys_traceclk",
	     CLK_ENABLE_ATCLK_MNGS_CSSYS_TRACECLK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_asyncatb_cam1",
	     "mngs_div_atclk_mngs_asyncatb_cam1",
	     CLK_ENABLE_ATCLK_MNGS_ASYNCATB_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_atclk_asyncatb_aud",
	     "mngs_div_atclk_mngs_asyncatb_aud",
	     CLK_ENABLE_ATCLK_MNGS_ASYNCATB_AUD, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_asapbmst_ccore_cssys",
	     "mngs_div_pclk_dbg_mngs", CLK_ENABLE_PCLK_DBG_MNGS, 11,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_asapbslv_cssys_bdu",
	     "mngs_div_pclk_dbg_mngs", CLK_ENABLE_PCLK_DBG_MNGS, 10,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_asapbslv_cssys_cam1",
	     "mngs_div_pclk_dbg_mngs", CLK_ENABLE_PCLK_DBG_MNGS, 9,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_asapbslv_cssys_aud",
	     "mngs_div_pclk_dbg_mngs", CLK_ENABLE_PCLK_DBG_MNGS, 8,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_asapbslv_cssys_apollo",
	     "mngs_div_pclk_dbg_mngs", CLK_ENABLE_PCLK_DBG_MNGS, 7,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_dump_pc_mngs", "mngs_div_pclk_dbg_mngs",
	     CLK_ENABLE_PCLK_DBG_MNGS, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_secjtag", "mngs_div_pclk_dbg_mngs",
	     CLK_ENABLE_PCLK_DBG_MNGS, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_axiap", "mngs_div_pclk_dbg_mngs",
	     CLK_ENABLE_PCLK_DBG_MNGS, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_cssys_ctmclk", "mngs_div_pclk_dbg_mngs",
	     CLK_ENABLE_PCLK_DBG_MNGS, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_cssys", "mngs_div_pclk_dbg_mngs",
	     CLK_ENABLE_PCLK_DBG_MNGS, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_mngs", "mngs_div_pclk_dbg_mngs",
	     CLK_ENABLE_PCLK_DBG_MNGS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclkdbg_asyncdapslv", "mngs_div_pclk_dbg_mngs",
	     CLK_ENABLE_PCLK_DBG_MNGS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclk_sysreg_mngs", "mngs_div_pclk_mngs",
	     CLK_ENABLE_PCLK_MNGS, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclk_stm_txactor", "mngs_div_pclk_mngs",
	     CLK_ENABLE_PCLK_MNGS, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclk_xiu_peri_mngs_aclk", "mngs_div_pclk_mngs",
	     CLK_ENABLE_PCLK_MNGS, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclk_pmu_mngs", "mngs_div_pclk_mngs",
	     CLK_ENABLE_PCLK_MNGS, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclk_xiu_mngssfrx_1x2", "mngs_div_pclk_mngs",
	     CLK_ENABLE_PCLK_MNGS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclk_axi2apb_mngs_aclk", "mngs_div_pclk_mngs",
	     CLK_ENABLE_PCLK_MNGS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_pclk_hpm_mngs", "mngs_div_pclk_mngs",
	     CLK_ENABLE_PCLK_HPM_MNGS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_sclk_mngs", "mngs_div_mngs", CLK_ENABLE_SCLK_MNGS, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_sclk_promise2_mngs", "mngs_div_sclk_promise_mngs",
	     CLK_ENABLE_SCLK_PROMISE_MNGS, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_sclk_promise1_mngs", "mngs_div_sclk_promise_mngs",
	     CLK_ENABLE_SCLK_PROMISE_MNGS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mngs_gate_sclk_promise0_mngs", "mngs_div_sclk_promise_mngs",
	     CLK_ENABLE_SCLK_PROMISE_MNGS, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock mngs_div_clks[] __initconst = {
	DIV(0, "mngs_div_mngs", "mngs_mux_mngs", CLK_CON_DIV_MNGS, 0, 6),
	DIV(0, "mngs_div_aclk_mngs", "mngs_div_mngs", CLK_CON_DIV_ACLK_MNGS, 0,
	    4),
	DIV(0, "mngs_div_atclk_mngs_core", "mngs_div_mngs",
	    CLK_CON_DIV_ATCLK_MNGS_CORE, 0, 4),
	DIV(0, "mngs_div_atclk_mngs_soc", "mngs_div_mngs",
	    CLK_CON_DIV_ATCLK_MNGS_SOC, 0, 6),
	DIV(0, "mngs_div_atclk_mngs_cssys_traceclk", "mngs_div_atclk_mngs_soc",
	    CLK_CON_DIV_ATCLK_MNGS_CSSYS_TRACECLK, 0, 4),
	DIV(0, "mngs_div_atclk_mngs_asyncatb_cam1", "mngs_div_atclk_mngs_soc",
	    CLK_CON_DIV_ATCLK_MNGS_ASYNCATB_CAM1, 0, 4),
	DIV(0, "mngs_div_atclk_mngs_asyncatb_aud", "mngs_div_atclk_mngs_soc",
	    CLK_CON_DIV_ATCLK_MNGS_ASYNCATB_AUD, 0, 4),
	DIV(0, "mngs_div_pclk_dbg_mngs", "mngs_div_mngs",
	    CLK_CON_DIV_PCLK_DBG_MNGS, 0, 6),
	DIV(0, "mngs_div_pclk_run_monitor", "mngs_div_pclk_dbg_mngs",
	    CLK_CON_DIV_PCLK_RUN_MONITOR, 0, 3),
	DIV(0, "mngs_div_pclk_mngs", "mngs_div_mngs", CLK_CON_DIV_PCLK_MNGS, 0,
	    6),
	DIV(0, "mngs_div_cntclk_mngs", "mngs_div_mngs", CLK_CON_DIV_CNTCLK_MNGS,
	    0, 4),
	DIV(0, "mngs_div_mngs_run_monitor", "mngs_div_mngs",
	    CLK_CON_DIV_MNGS_RUN_MONITOR, 0, 3),
	DIV(0, "mngs_div_sclk_promise_mngs", "mngs_mux_mngs",
	    CLK_CON_DIV_SCLK_PROMISE_MNGS, 0, 3),
	DIV(0, "mngs_div_mngs_pll", "mngs_mux_mngs", CLK_CON_DIV_MNGS_PLL, 0,
	    3),
};

static const struct samsung_cmu_info mngs_cmu_info __initconst = {
	.pll_clks = mngs_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(mngs_pll_clks),
	.mux_clks = mngs_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(mngs_mux_clks),
	.div_clks = mngs_div_clks,
	.nr_div_clks = ARRAY_SIZE(mngs_div_clks),
	.gate_clks = mngs_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(mngs_gate_clks),
	.nr_clk_ids = MNGS_NR_CLK,
};

/* Register Offset definitions for CMU_APOLLO (0x11900000) */
#define APOLLO_PLL_LOCK 0x0000
#define APOLLO_PLL_CON0 0x0100
#define APOLLO_PLL_CON1 0x0104
#define APOLLO_PLL_FREQ_DET 0x010C
#define CLK_CON_MUX_APOLLO_PLL 0x0200
#define CLK_CON_MUX_BUS_PLL_APOLLO_USER 0x0204
#define CLK_CON_MUX_APOLLO 0x0208
#define CLK_CON_DIV_APOLLO 0x0400
#define CLK_CON_DIV_ACLK_APOLLO 0x0404
#define CLK_CON_DIV_ATCLK_APOLLO 0x0408
#define CLK_CON_DIV_PCLK_DBG_APOLLO 0x040C
#define CLK_CON_DIV_PCLK_APOLLO 0x0410
#define CLK_CON_DIV_CNTCLK_APOLLO 0x0414
#define CLK_CON_DIV_APOLLO_RUN_MONITOR 0x0418
#define CLK_CON_DIV_SCLK_PROMISE_APOLLO 0x041C
#define CLK_CON_DIV_APOLLO_PLL 0x0420
#define CLK_STAT_MUX_APOLLO_PLL 0x0600
#define CLK_STAT_MUX_BUS_PLL_APOLLO_USER 0x0604
#define CLK_STAT_MUX_APOLLO 0x0608
#define CLK_ENABLE_ACLK_APOLLO 0x0800
#define CLK_ENABLE_ATCLK_APOLLO 0x0804
#define CLK_ENABLE_PCLK_DBG_APOLLO 0x0900
#define CLK_ENABLE_PCLK_APOLLO 0x0904
#define CLK_ENABLE_PCLK_HPM_APOLLO 0x0908
#define CLK_ENABLE_SCLK_APOLLO 0x0A00
#define CLK_ENABLE_SCLK_PROMISE_APOLLO 0x0A04
#define CLKOUT_CMU_APOLLO 0x0C00
#define CLKOUT_CMU_APOLLO_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_APOLLO 0x0D00
#define APOLLO_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_APOLLO_SPARE0 0x0D08
#define CMU_APOLLO_SPARE1 0x0D0C
#define ARMCLK_STOPCTRL_APOLLO 0x1000
#define PWR_CTRL_APOLLO 0x1020
#define PWR_CTRL2_APOLLO 0x1024
#define PWR_CTRL3_APOLLO 0x1028
#define PWR_CTRL4_APOLLO 0x102C
#define INTR_SPREAD_ENABLE_APOLLO 0x1080
#define INTR_SPREAD_USE_STANDBYWFI_APOLLO 0x1084
#define INTR_SPREAD_BLOCKING_DURATION_APOLLO 0x1088

/* APOLLO_PLL */
static const struct samsung_pll_rate_table
	exynos8890_apollo_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 1976000000U, 228, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1898000000U, 219, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1794000000U, 207, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1690000000U, 195, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1586000000U, 183, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1482000000U, 171, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1378000000U, 159, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1274000000U, 147, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1170000000U, 135, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 1066000000U, 123, 3, 0),
		PLL_35XX_RATE(26 * MHZ, 962000000U, 222, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 858000000U, 198, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 754000000U, 174, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 650000000U, 150, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 546000000U, 126, 3, 1),
		PLL_35XX_RATE(26 * MHZ, 442000000U, 204, 3, 2),
		PLL_35XX_RATE(26 * MHZ, 338000000U, 156, 3, 2),
		PLL_35XX_RATE(26 * MHZ, 234000000U, 216, 3, 3),
		PLL_35XX_RATE(26 * MHZ, 130000000U, 120, 3, 3),
		{ /* sentinel */ }
	};

static const struct samsung_pll_clock apollo_pll_clks[] __initconst = {
	PLL(pll_141xx, 0, "apollo_pll", "oscclk", APOLLO_PLL_LOCK,
	    APOLLO_PLL_CON0, exynos8890_apollo_pll_rates),
};

PNAME(apollo_mux_apollo_pll_p) = { "oscclk", "apollo_pll" };
PNAME(apollo_mux_bus_pll_apollo_user_p) = { "oscclk",
					    "top_gate_sclk_bus_pll_apollo" };
PNAME(apollo_mux_apollo_p) = { "apollo_mux_apollo_pll",
			       "apollo_mux_bus_pll_apollo_user" };

static const struct samsung_mux_clock apollo_mux_clks[] __initconst = {
	MUX(0, "apollo_mux_apollo_pll", apollo_mux_apollo_pll_p,
	    CLK_CON_MUX_APOLLO_PLL, 12, 1),
	MUX(0, "apollo_mux_bus_pll_apollo_user",
	    apollo_mux_bus_pll_apollo_user_p, CLK_CON_MUX_BUS_PLL_APOLLO_USER,
	    12, 1),
	MUX(0, "apollo_mux_apollo", apollo_mux_apollo_p, CLK_CON_MUX_APOLLO, 12,
	    1),
};

static const struct samsung_gate_clock apollo_gate_clks[] __initconst = {
	GATE(0, "apollo_gate_aclk_asyncaces_apollo_cci",
	     "apollo_div_aclk_apollo", CLK_ENABLE_ACLK_APOLLO, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_aclk_asatbslv_apollo3_cssys",
	     "apollo_div_atclk_apollo", CLK_ENABLE_ATCLK_APOLLO, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_aclk_asatbslv_apollo2_cssys",
	     "apollo_div_atclk_apollo", CLK_ENABLE_ATCLK_APOLLO, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_aclk_asatbslv_apollo1_cssys",
	     "apollo_div_atclk_apollo", CLK_ENABLE_ATCLK_APOLLO, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_aclk_asatbslv_apollo0_cssys",
	     "apollo_div_atclk_apollo", CLK_ENABLE_ATCLK_APOLLO, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_pclkdbg_dump_pc_apollo",
	     "apollo_div_pclk_dbg_apollo", CLK_ENABLE_PCLK_DBG_APOLLO, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_pclkdbg_asapbmst_cssys_apollo",
	     "apollo_div_pclk_dbg_apollo", CLK_ENABLE_PCLK_DBG_APOLLO, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_pclk_sysreg_apollo", "apollo_div_pclk_apollo",
	     CLK_ENABLE_PCLK_APOLLO, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_pclk_pmu_apollo", "apollo_div_pclk_apollo",
	     CLK_ENABLE_PCLK_APOLLO, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_pclk_axi2apb_apollo_aclk",
	     "apollo_div_pclk_apollo", CLK_ENABLE_PCLK_APOLLO, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_pclk_xiu_peri_apollo_aclk",
	     "apollo_div_pclk_apollo", CLK_ENABLE_PCLK_APOLLO, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_pclk_hpm_apollo", "apollo_div_pclk_apollo",
	     CLK_ENABLE_PCLK_HPM_APOLLO, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_sclk_apollo", "apollo_div_apollo",
	     CLK_ENABLE_SCLK_APOLLO, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "apollo_gate_sclk_promise_apollo",
	     "apollo_div_sclk_promise_apollo", CLK_ENABLE_SCLK_PROMISE_APOLLO,
	     0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock apollo_div_clks[] __initconst = {
	DIV(0, "apollo_div_apollo", "apollo_mux_apollo", CLK_CON_DIV_APOLLO, 0,
	    6),
	DIV(0, "apollo_div_aclk_apollo", "apollo_div_apollo",
	    CLK_CON_DIV_ACLK_APOLLO, 0, 3),
	DIV(0, "apollo_div_atclk_apollo", "apollo_div_apollo",
	    CLK_CON_DIV_ATCLK_APOLLO, 0, 4),
	DIV(0, "apollo_div_pclk_dbg_apollo", "apollo_div_apollo",
	    CLK_CON_DIV_PCLK_DBG_APOLLO, 0, 4),
	DIV(0, "apollo_div_pclk_apollo", "apollo_div_apollo",
	    CLK_CON_DIV_PCLK_APOLLO, 0, 4),
	DIV(0, "apollo_div_cntclk_apollo", "apollo_div_apollo",
	    CLK_CON_DIV_CNTCLK_APOLLO, 0, 4),
	DIV(0, "apollo_div_apollo_run_monitor", "apollo_div_apollo",
	    CLK_CON_DIV_APOLLO_RUN_MONITOR, 0, 3),
	DIV(0, "apollo_div_sclk_promise_apollo", "apollo_mux_apollo",
	    CLK_CON_DIV_SCLK_PROMISE_APOLLO, 0, 3),
	DIV(0, "apollo_div_apollo_pll", "apollo_mux_apollo",
	    CLK_CON_DIV_APOLLO_PLL, 0, 3),
};

static const struct samsung_cmu_info apollo_cmu_info __initconst = {
	.pll_clks = apollo_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(apollo_pll_clks),
	.mux_clks = apollo_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(apollo_mux_clks),
	.div_clks = apollo_div_clks,
	.nr_div_clks = ARRAY_SIZE(apollo_div_clks),
	.gate_clks = apollo_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(apollo_gate_clks),
	.nr_clk_ids = APOLLO_NR_CLK,
};

/* Register Offset definitions for CMU_BUS0 (0x13400000) */
#define CLK_CON_MUX_ACLK_BUS0_528_USER 0x0200
#define CLK_CON_MUX_ACLK_BUS0_200_USER 0x0204
#define CLK_CON_MUX_PCLK_BUS0_132_USER 0x0208
#define CLK_STAT_MUX_ACLK_BUS0_528_USER 0x0600
#define CLK_STAT_MUX_ACLK_BUS0_200_USER 0x0604
#define CLK_STAT_MUX_PCLK_BUS0_132_USER 0x0608
#define CLK_ENABLE_ACLK_BUS0_528_BUS0 0x0800
#define CLK_ENABLE_ACLK_BUS0_200_BUS0 0x0804
#define CLK_ENABLE_PCLK_BUS0_132_BUS0 0x0900
#define CG_CTRL_VAL_ACLK_BUS0_528_BUS0 0x0800
#define CG_CTRL_VAL_ACLK_BUS0_200_BUS0 0x0804
#define CG_CTRL_VAL_PCLK_BUS0_132_BUS0 0x0900
#define CLKOUT_CMU_BUS0 0x0D00
#define CLKOUT_CMU_BUS0_DIV_STAT 0x0D04
#define CMU_BUS0_SPARE0 0x0D08
#define CMU_BUS0_SPARE1 0x0D0C
#define CLK_ENABLE_PDN_BUS0 0x0E00
#define BUS0_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define CG_CTRL_MAN_ACLK_BUS0_528 0x1800
#define CG_CTRL_MAN_ACLK_BUS0_200 0x1804
#define CG_CTRL_MAN_PCLK_BUS0_132 0x1900
#define CG_CTRL_STAT_ACLK_BUS0_528_0 0x1C00
#define CG_CTRL_STAT_ACLK_BUS0_528_1 0x1C04
#define CG_CTRL_STAT_ACLK_BUS0_200 0x1C08
#define CG_CTRL_STAT_PCLK_BUS0_132_0 0x1D00
#define CG_CTRL_STAT_PCLK_BUS0_132_1 0x1D04
#define CG_CTRL_STAT_PCLK_BUS0_132_2 0x1D08
#define QCH_CTRL_TREX_D_BUS0 0x2000
#define QCH_CTRL_CAM0_D 0x2004
#define QCH_CTRL_CAM1_D 0x2008
#define QCH_CTRL_DISP00_D 0x200C
#define QCH_CTRL_DISP01_D 0x2010
#define QCH_CTRL_DISP10_D 0x2014
#define QCH_CTRL_DISP11_D 0x201C
#define QCH_CTRL_ISP0_D 0x2020
#define QCH_CTRL_FSYS1_D 0x2024
#define QCH_CTRL_TREX_P_BUS0 0x2100
#define QCH_CTRL_ISP0_SFR 0x2104
#define QCH_CTRL_ISP1_SFR 0x2108
#define QCH_CTRL_DISP0_SFR 0x210C
#define QCH_CTRL_DISP1_SFR 0x2110
#define QCH_CTRL_PERIS_SFR 0x2114
#define QCH_CTRL_PERIC0_SFR 0x2118
#define QCH_CTRL_PERIC1_SFR 0x211C
#define QCH_CTRL_FSYS1_SFR 0x2120
#define QCH_CTRL_SYSREG_BUS0 0x2124
#define QCH_CTRL_PMU_BUS0 0x2128
#define QCH_CTRL_CMU_BUS0 0x212C

PNAME(bus0_mux_aclk_bus0_528_user_p) = { "oscclk", "top_gate_aclk_bus0_528" };
PNAME(bus0_mux_aclk_bus0_200_user_p) = { "oscclk", "top_gate_aclk_bus0_200" };
PNAME(bus0_mux_pclk_bus0_132_user_p) = { "oscclk", "top_gate_pclk_bus0_132" };

static const struct samsung_mux_clock bus0_mux_clks[] __initconst = {
	MUX(0, "bus0_mux_aclk_bus0_528_user", bus0_mux_aclk_bus0_528_user_p,
	    CLK_CON_MUX_ACLK_BUS0_528_USER, 12, 1),
	MUX(0, "bus0_mux_aclk_bus0_200_user", bus0_mux_aclk_bus0_200_user_p,
	    CLK_CON_MUX_ACLK_BUS0_200_USER, 12, 1),
	MUX(0, "bus0_mux_pclk_bus0_132_user", bus0_mux_pclk_bus0_132_user_p,
	    CLK_CON_MUX_PCLK_BUS0_132_USER, 12, 1),
};

static const struct samsung_gate_clock bus0_gate_clks[] __initconst = {
	GATE(0, "bus0_gate_aclk_ace_fsys1", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_isp0", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_disp11", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_disp10", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_disp01", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_disp00", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_cam1", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_cam0", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_trex_bus0", "bus0_mux_aclk_bus0_528_user",
	     CG_CTRL_VAL_ACLK_BUS0_528_BUS0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_lh_fsys1", "bus0_mux_aclk_bus0_200_user",
	     CG_CTRL_VAL_ACLK_BUS0_200_BUS0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_cmu_bus0", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_trex_bus0", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_pmu_bus0", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_sysreg_bus0", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_fsys1sfrx", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_peric1p", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_peric0p", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_perisfrx", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_disp1sfrx", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_disp0sfrx", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_isphx", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_lh_is0x", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_axi2apb_2mb_bus0_tp",
	     "bus0_mux_pclk_bus0_132_user", CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_ahb2apb_bus0p", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_axi2apb_2mb_bus0_td",
	     "bus0_mux_pclk_bus0_132_user", CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_pclk_trex_p_bus0", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus0_gate_aclk_trex_p_bus0", "bus0_mux_pclk_bus0_132_user",
	     CG_CTRL_VAL_PCLK_BUS0_132_BUS0, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info bus0_cmu_info __initconst = {
	.mux_clks = bus0_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(bus0_mux_clks),
	.gate_clks = bus0_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(bus0_gate_clks),
	.nr_clk_ids = BUS0_NR_CLK,
};

/* Register Offset definitions for CMU_BUS1 (0x14800000) */
#define CLK_CON_MUX_ACLK_BUS1_528_USER 0x0200
#define CLK_CON_MUX_PCLK_BUS1_132_USER 0x0204
#define CLK_STAT_MUX_ACLK_BUS1_528_USER 0x0600
#define CLK_STAT_MUX_PCLK_BUS1_132_USER 0x0604
#define CLK_ENABLE_ACLK_BUS1_528_BUS1 0x0800
#define CLK_ENABLE_PCLK_BUS1_132_BUS1 0x0900
#define CG_CTRL_VAL_ACLK_BUS1_528_BUS1 0x0800
#define CG_CTRL_VAL_PCLK_BUS1_132_BUS1 0x0900
#define CLKOUT_CMU_BUS1 0x0D00
#define CLKOUT_CMU_BUS1_DIV_STAT 0x0D04
#define CMU_BUS1_SPARE0 0x0D08
#define CMU_BUS1_SPARE1 0x0D0C
#define CLK_ENABLE_PDN_BUS1 0x0E00
#define BUS1_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define CG_CTRL_MAN_ACLK_BUS1_528 0x1800
#define CG_CTRL_MAN_PCLK_BUS1_132 0x1900
#define CG_CTRL_STAT_ACLK_BUS1_528 0x1C00
#define CG_CTRL_STAT_PCLK_BUS1_132_0 0x1D00
#define CG_CTRL_STAT_PCLK_BUS1_132_1 0x1D04
#define QCH_CTRL_TREX_D_BUS1 0x2000
#define QCH_CTRL_FSYS0_D 0x2004
#define QCH_CTRL_MFC0_D 0x2008
#define QCH_CTRL_MFC1_D 0x200C
#define QCH_CTRL_MSCL0_D 0x2010
#define QCH_CTRL_MSCL1_D 0x2014
#define QCH_CTRL_TREX_P_BUS1 0x2018
#define QCH_CTRL_FSYS0_SFR 0x201C
#define QCH_CTRL_MFC_SFR 0x2020
#define QCH_CTRL_MSCL_SFR 0x2024
#define QCH_CTRL_PMU_BUS1 0x2028
#define QCH_CTRL_SYSREG_BUS1 0x202C
#define QCH_CTRL_CMU_BUS1 0x2030

PNAME(bus1_mux_aclk_bus1_528_user_p) = { "oscclk", "top_gate_aclk_bus1_528" };
PNAME(bus1_mux_pclk_bus1_132_user_p) = { "oscclk", "top_gate_pclk_bus1_132" };

static const struct samsung_mux_clock bus1_mux_clks[] __initconst = {
	MUX(0, "bus1_mux_aclk_bus1_528_user", bus1_mux_aclk_bus1_528_user_p,
	    CLK_CON_MUX_ACLK_BUS1_528_USER, 12, 1),
	MUX(0, "bus1_mux_pclk_bus1_132_user", bus1_mux_pclk_bus1_132_user_p,
	    CLK_CON_MUX_PCLK_BUS1_132_USER, 12, 1),
};

static const struct samsung_gate_clock bus1_gate_clks[] __initconst = {
	GATE(0, "bus1_gate_aclk_lh_mscl1", "bus1_mux_aclk_bus1_528_user",
	     CG_CTRL_VAL_ACLK_BUS1_528_BUS1, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_aclk_lh_mscl0", "bus1_mux_aclk_bus1_528_user",
	     CG_CTRL_VAL_ACLK_BUS1_528_BUS1, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_aclk_lh_mfc1", "bus1_mux_aclk_bus1_528_user",
	     CG_CTRL_VAL_ACLK_BUS1_528_BUS1, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_aclk_lh_mfc0", "bus1_mux_aclk_bus1_528_user",
	     CG_CTRL_VAL_ACLK_BUS1_528_BUS1, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_aclk_lh_fsys0", "bus1_mux_aclk_bus1_528_user",
	     CG_CTRL_VAL_ACLK_BUS1_528_BUS1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_aclk_trex_bus1", "bus1_mux_aclk_bus1_528_user",
	     CG_CTRL_VAL_ACLK_BUS1_528_BUS1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_cmu_bus1", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_trex_bus1", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_sysreg_bus1", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_pmu_bus1", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_lh_msclsfrx", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_lh_mfcp", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_lh_fsys0sfrx", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_ahb2apb_bus1p", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_axi2apb_2mb_bus1_tp",
	     "bus1_mux_pclk_bus1_132_user", CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_axi2apb_2mb_bus1_td",
	     "bus1_mux_pclk_bus1_132_user", CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_pclk_trex_p_bus1", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "bus1_gate_aclk_trex_p_bus1", "bus1_mux_pclk_bus1_132_user",
	     CG_CTRL_VAL_PCLK_BUS1_132_BUS1, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info bus1_cmu_info __initconst = {
	.mux_clks = bus1_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(bus1_mux_clks),
	.gate_clks = bus1_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(bus1_gate_clks),
	.nr_clk_ids = BUS1_NR_CLK,
};

/* Register Offset definitions for CMU_PERIC0 (0x13610000) */
#define CLK_CON_MUX_ACLK_PERIC0_66_USER 0x0200
#define CLK_CON_MUX_SCLK_UART0_USER 0x0204
#define CLK_STAT_MUX_ACLK_PERIC0_66_USER 0x0600
#define CLK_STAT_MUX_SCLK_UART0_USER 0x0604
#define CG_CTRL_VAL_ACLK_PERIC0_66 0x0800
#define CG_CTRL_VAL_SCLK_UART0 0x0840
#define CG_CTRL_VAL_SCLK_PWM 0x0844
#define CLKOUT_CMU_PERIC0 0x0C00
#define CLKOUT_CMU_PERIC0_DIV_STAT 0x0C04
#define PERIC0_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_PERIC0_SPARE0 0x0D08
#define CMU_PERIC0_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_PERIC0_66 0x1800
#define CG_CTRL_MAN_SCLK_UART0 0x1840
#define CG_CTRL_MAN_SCLK_PWM 0x1844
#define CG_CTRL_STAT_ACLK_PERIC0_66_0 0x1C00
#define CG_CTRL_STAT_ACLK_PERIC0_66_1 0x1C04
#define CG_CTRL_STAT_SCLK_UART0 0x1C40
#define CG_CTRL_STAT_SCLK_PWM 0x1C44
#define QCH_CTRL_AXILHASYNCM_PERIC0 0x2000
#define QCH_CTRL_CMU_PERIC0 0x2004
#define QCH_CTRL_PMU_PERIC0 0x2008
#define QCH_CTRL_SYSREG_PERIC0 0x200C
#define QSTATE_CTRL_GPIO_BUS0 0x2404
#define QSTATE_CTRL_UART0 0x2408
#define QSTATE_CTRL_ADCIF 0x240C
#define QSTATE_CTRL_PWM 0x2410
#define QSTATE_CTRL_HSI2C0 0x2414
#define QSTATE_CTRL_HSI2C1 0x2418
#define QSTATE_CTRL_HSI2C4 0x241C
#define QSTATE_CTRL_HSI2C5 0x2420
#define QSTATE_CTRL_HSI2C9 0x2424
#define QSTATE_CTRL_HSI2C10 0x2428
#define QSTATE_CTRL_HSI2C11 0x242C

PNAME(peric0_mux_aclk_peric0_66_user_p) = { "oscclk",
					    "top_gate_aclk_peric0_66" };
PNAME(peric0_mux_sclk_uart0_user_p) = { "oscclk",
					"top_gate_sclk_peric0_uart0" };

static const struct samsung_mux_clock peric0_mux_clks[] __initconst = {
	MUX(0, "peric0_mux_aclk_peric0_66_user",
	    peric0_mux_aclk_peric0_66_user_p, CLK_CON_MUX_ACLK_PERIC0_66_USER,
	    12, 1),
	MUX(0, "peric0_mux_sclk_uart0_user", peric0_mux_sclk_uart0_user_p,
	    CLK_CON_MUX_SCLK_UART0_USER, 12, 1),
};

static const struct samsung_gate_clock peric0_gate_clks[] __initconst = {
	GATE(0, "peric0_gate_pclk_hsi2c11", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_hsi2c10", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_hsi2c9", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_hsi2c5", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_hsi2c4", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_hsi2c1", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_hsi2c0", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_pwm", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_adcif", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_uart0", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_gpio_bus0", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_sysreg_peric0",
	     "peric0_mux_aclk_peric0_66_user", CG_CTRL_VAL_ACLK_PERIC0_66, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_pmu_peric0", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_pclk_cmu_peric0", "peric0_mux_aclk_peric0_66_user",
	     CG_CTRL_VAL_ACLK_PERIC0_66, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_aclk_axi2apb_peric0p",
	     "peric0_mux_aclk_peric0_66_user", CG_CTRL_VAL_ACLK_PERIC0_66, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_aclk_axilhasyncm_peric0",
	     "peric0_mux_aclk_peric0_66_user", CG_CTRL_VAL_ACLK_PERIC0_66, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_sclk_uart0", "peric0_mux_sclk_uart0_user",
	     CG_CTRL_VAL_SCLK_UART0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_gate_sclk_pwm", "oscclk", CG_CTRL_VAL_SCLK_PWM, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_gpio_bus0", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_GPIO_BUS0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_uart0", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_UART0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_adcif", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_ADCIF, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_pwm", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_PWM, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_hsi2c0", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_HSI2C0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_hsi2c1", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_HSI2C1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_hsi2c4", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_HSI2C4, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_hsi2c5", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_HSI2C5, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_hsi2c9", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_HSI2C9, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_hsi2c10", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_HSI2C10, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric0_hwacg_hsi2c11", "peric0_mux_aclk_peric0_66_user",
	     QSTATE_CTRL_HSI2C11, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info peric0_cmu_info __initconst = {
	.mux_clks = peric0_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(peric0_mux_clks),
	.gate_clks = peric0_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(peric0_gate_clks),
	.nr_clk_ids = PERIC0_NR_CLK,
};

/* Register Offset definitions for CMU_DISP0 (0x13AD0000) */
#define DISP_PLL_LOCK 0x0000
#define DISP_PLL_CON0 0x0100
#define DISP_PLL_CON1 0x0104
#define DISP_PLL_FREQ_DET 0x010C
#define CLK_CON_MUX_DISP_PLL 0x0200
#define CLK_CON_MUX_ACLK_DISP0_0_400_USER 0x0204
#define CLK_CON_MUX_ACLK_DISP0_1_400_USER 0x0208
#define CLK_CON_MUX_SCLK_DISP0_DECON0_ECLK0_USER 0x020C
#define CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK0_USER 0x0210
#define CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK1_USER 0x0214
#define CLK_CON_MUX_SCLK_DISP0_HDMI_AUDIO_USER 0x0218
#define CLK_CON_MUX_PHYCLK_HDMIPHY_PIXEL_CLKO_USER 0x021C
#define CLK_CON_MUX_PHYCLK_HDMIPHY_TMDS_CLKO_USER 0x0220
#define CLK_CON_MUX_PHYCLK_MIPIDPHY0_RXCLKESC0_USER 0x0224
#define CLK_CON_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV2_USER_DISP0 0x0228
#define CLK_CON_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV8_USER 0x022C
#define CLK_CON_MUX_PHYCLK_MIPIDPHY1_RXCLKESC0_USER 0x0230
#define CLK_CON_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV2_USER_DISP0 0x0234
#define CLK_CON_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV8_USER 0x0238
#define CLK_CON_MUX_PHYCLK_MIPIDPHY2_RXCLKESC0_USER 0x023C
#define CLK_CON_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV2_USER_DISP0 0x0240
#define CLK_CON_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV8_USER 0x0244
#define CLK_CON_MUX_PHYCLK_DPPHY_CH0_TXD_CLK_USER 0x0248
#define CLK_CON_MUX_PHYCLK_DPPHY_CH1_TXD_CLK_USER 0x024C
#define CLK_CON_MUX_PHYCLK_DPPHY_CH2_TXD_CLK_USER 0x0250
#define CLK_CON_MUX_PHYCLK_DPPHY_CH3_TXD_CLK_USER 0x0254
#define CLK_CON_MUX_ACLK_DISP0_1_400_DISP0 0x0258
#define CLK_CON_MUX_SCLK_DISP0_DECON0_ECLK0_DISP0 0x025C
#define CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK0_DISP0 0x0260
#define CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK1_DISP0 0x0264
#define CLK_CON_MUX_SCLK_DISP0_HDMI_AUDIO_DISP0 0x0268
#define CLK_CON_DIV_PCLK_DISP0_0_133 0x0400
#define CLK_CON_DIV_SCLK_DECON0_ECLK0 0x0404
#define CLK_CON_DIV_SCLK_DECON0_VCLK0 0x0408
#define CLK_CON_DIV_SCLK_DECON0_VCLK1 0x040C
#define CLK_CON_DIV_PHYCLK_HDMIPHY_PIXEL_CLKO 0x0410
#define CLK_CON_DIV_PHYCLK_HDMIPHY_TMDS_20B_CLKO 0x0414
#define CLK_CON_DSM_DIV_M_SCLK_HDMI_AUDIO 0x0500
#define CLK_CON_DSM_DIV_N_SCLK_HDMI_AUDIO 0x0504
#define CLK_STAT_MUX_DISP_PLL 0x0600
#define CLK_STAT_MUX_ACLK_DISP0_0_400_USER 0x0604
#define CLK_STAT_MUX_ACLK_DISP0_1_400_USER 0x0608
#define CLK_STAT_MUX_SCLK_DISP0_HDMI_AUDIO_USER 0x060C
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_ECLK0_USER 0x0610
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_VCLK0_USER 0x0614
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_VCLK1_USER 0x0618
#define CLK_STAT_MUX_PHYCLK_HDMIPHY_PIXEL_CLKO_USER 0x061C
#define CLK_STAT_MUX_PHYCLK_HDMIPHY_TMDS_CLKO_USER 0x0620
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY0_RXCLKESC0_USER 0x0624
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV2_USER_DISP0 0x0628
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV8_USER 0x062C
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY1_RXCLKESC0_USER 0x0630
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV2_USER_DISP0 0x0634
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV8_USER 0x0638
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY2_RXCLKESC0_USER 0x063C
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV2_USER_DISP0 0x0640
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV8_USER 0x0644
#define CLK_STAT_MUX_PHYCLK_DPPHY_CH0_TXD_CLK_USER 0x0648
#define CLK_STAT_MUX_PHYCLK_DPPHY_CH1_TXD_CLK_USER 0x064C
#define CLK_STAT_MUX_PHYCLK_DPPHY_CH2_TXD_CLK_USER 0x0650
#define CLK_STAT_MUX_PHYCLK_DPPHY_CH3_TXD_CLK_USER 0x0654
#define CLK_STAT_MUX_ACLK_DISP0_1_400_DISP0 0x0658
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_ECLK0_DISP0 0x065C
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_VCLK0_DISP0 0x0660
#define CLK_STAT_MUX_SCLK_DISP0_DECON0_VCLK1_DISP0 0x0664
#define CLK_STAT_MUX_SCLK_DISP0_HDMI_AUDIO_DISP0 0x0668
#define CG_CTRL_VAL_ACLK_DISP0_0_400 0x0800
#define CG_CTRL_VAL_ACLK_DISP0_1_400 0x0804
#define CG_CTRL_VAL_ACLK_DISP0_0_400_SECURE_SFW_DISP0_0 0x0808
#define CG_CTRL_VAL_ACLK_DISP0_1_400_SECURE_SFW_DISP0_1 0x080C
#define CG_CTRL_VAL_PCLK_DISP0_0_133 0x0820
#define CG_CTRL_VAL_PCLK_DISP0_0_133_HPM_APBIF_DISP0 0x0824
#define CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_DECON0 0x0828
#define CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_VPP0 0x082C
#define CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_SFW_DISP0_0 0x0830
#define CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_SFW_DISP0_1 0x0834
#define CG_CTRL_VAL_SCLK_DISP1_400 0x0840
#define CG_CTRL_VAL_SCLK_DECON0_ECLK0 0x0844
#define CG_CTRL_VAL_SCLK_DECON0_VCLK0 0x0848
#define CG_CTRL_VAL_SCLK_DECON0_VCLK1 0x084C
#define CG_CTRL_VAL_SCLK_HDMI_AUDIO 0x0850
#define CG_CTRL_VAL_SCLK_DISP0_PROMISE 0x0854
#define CG_CTRL_VAL_PHYCLK_HDMIPHY 0x0858
#define CG_CTRL_VAL_PHYCLK_MIPIDPHY0 0x085C
#define CG_CTRL_VAL_PHYCLK_MIPIDPHY1 0x0860
#define CG_CTRL_VAL_PHYCLK_MIPIDPHY2 0x0864
#define CG_CTRL_VAL_PHYCLK_DPPHY 0x0868
#define CG_CTRL_VAL_OSCCLK 0x086C
#define CLKOUT_CMU_DISP0 0x0C00
#define CLKOUT_CMU_DISP0_DIV_STAT 0x0C04
#define DISP0_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_DISP0_SPARE0 0x0D08
#define CMU_DISP0_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_DISP0_0_400 0x1800
#define CG_CTRL_MAN_ACLK_DISP0_1_400 0x1804
#define CG_CTRL_MAN_ACLK_DISP0_0_400_SECURE_SFW_DISP0_0 0x1808
#define CG_CTRL_MAN_ACLK_DISP0_1_400_SECURE_SFW_DISP0_1 0x180C
#define CG_CTRL_MAN_PCLK_DISP0_0_133 0x1820
#define CG_CTRL_MAN_PCLK_DISP0_0_133_HPM_APBIF_DISP0 0x1824
#define CG_CTRL_MAN_PCLK_DISP0_0_133_SECURE_DECON0 0x1828
#define CG_CTRL_MAN_PCLK_DISP0_0_133_SECURE_VPP0 0x182C
#define CG_CTRL_MAN_PCLK_DISP0_0_133_SECURE_SFW_DISP0_0 0x1830
#define CG_CTRL_MAN_PCLK_DISP0_0_133_SECURE_SFW_DISP0_1 0x1834
#define CG_CTRL_MAN_SCLK_DISP1_400 0x1840
#define CG_CTRL_MAN_SCLK_DECON0_ECLK0 0x1844
#define CG_CTRL_MAN_SCLK_DECON0_VCLK0 0x1848
#define CG_CTRL_MAN_SCLK_DECON0_VCLK1 0x184C
#define CG_CTRL_MAN_SCLK_HDMI_AUDIO 0x1850
#define CG_CTRL_MAN_SCLK_DISP0_PROMISE 0x1854
#define CG_CTRL_MAN_PHYCLK_HDMIPHY 0x1858
#define CG_CTRL_MAN_PHYCLK_MIPIDPHY0 0x185C
#define CG_CTRL_MAN_PHYCLK_MIPIDPHY1 0x1860
#define CG_CTRL_MAN_PHYCLK_MIPIDPHY2 0x1864
#define CG_CTRL_MAN_PHYCLK_DPPHY 0x1868
#define CG_CTRL_MAN_OSCCLK 0x186C
#define CG_CTRL_STAT_ACLK_DISP0_0_400 0x1C00
#define CG_CTRL_STAT_ACLK_DISP0_1_400 0x1C04
#define CG_CTRL_STAT_ACLK_DISP0_0_400_SECURE_SFW_DISP0_0 0x1C08
#define CG_CTRL_STAT_ACLK_DISP0_1_400_SECURE_SFW_DISP0_1 0x1C0C
#define CG_CTRL_STAT_PCLK_DISP0_0_133_0 0x1C20
#define CG_CTRL_STAT_PCLK_DISP0_0_133_1 0x1C24
#define CG_CTRL_STAT_PCLK_DISP0_0_133_2 0x1C28
#define CG_CTRL_STAT_PCLK_DISP0_0_133_HPM_APBIF_DISP0 0x1C2C
#define CG_CTRL_STAT_PCLK_DISP0_0_133_SECURE_DECON0 0x1C30
#define CG_CTRL_STAT_PCLK_DISP0_0_133_SECURE_VPP0 0x1C34
#define CG_CTRL_STAT_PCLK_DISP0_0_133_SECURE_SFW_DISP0_0 0x1C38
#define CG_CTRL_STAT_PCLK_DISP0_0_133_SECURE_SFW_DISP0_1 0x1C3C
#define CG_CTRL_STAT_SCLK_DISP1_400 0x1C40
#define CG_CTRL_STAT_SCLK_DECON0_ECLK0 0x1C44
#define CG_CTRL_STAT_SCLK_DECON0_VCLK0 0x1C48
#define CG_CTRL_STAT_SCLK_DECON0_VCLK1 0x1C4C
#define CG_CTRL_STAT_SCLK_HDMI_AUDIO 0x1C50
#define CG_CTRL_STAT_SCLK_DISP0_PROMISE 0x1C54
#define CG_CTRL_STAT_PHYCLK_HDMIPHY 0x1C58
#define CG_CTRL_STAT_PHYCLK_MIPIDPHY0 0x1C5C
#define CG_CTRL_STAT_PHYCLK_MIPIDPHY1 0x1C60
#define CG_CTRL_STAT_PHYCLK_MIPIDPHY2 0x1C64
#define CG_CTRL_STAT_PHYCLK_DPPHY 0x1C68
#define CG_CTRL_STAT_OSCCLK 0x1C6C
#define QCH_CTRL_AXI_LH_ASYNC_MI_DISP0SFR 0x2000
#define QCH_CTRL_CMU_DISP0 0x2004
#define QCH_CTRL_PMU_DISP0 0x2008
#define QCH_CTRL_SYSREG_DISP0 0x200C
#define QCH_CTRL_DECON0 0x2010
#define QCH_CTRL_VPP0 0x2014
#define QCH_CTRL_VPP0_G0 0x2014
#define QCH_CTRL_VPP0_G1 0x2018
#define QCH_CTRL_DSIM0 0x2020
#define QCH_CTRL_DSIM1 0x2024
#define QCH_CTRL_DSIM2 0x2028
#define QCH_CTRL_HDMI 0x202C
#define QCH_CTRL_DP 0x2030
#define QCH_CTRL_PPMU_DISP0_0 0x2034
#define QCH_CTRL_PPMU_DISP0_1 0x203C
#define QCH_CTRL_SMMU_DISP0_0 0x2040
#define QCH_CTRL_SMMU_DISP0_1 0x2044
#define QCH_CTRL_SFW_DISP0_0 0x2048
#define QCH_CTRL_SFW_DISP0_1 0x204C
#define QCH_CTRL_LH_ASYNC_SI_R_TOP_DISP 0x2050
#define QCH_CTRL_LH_ASYNC_SI_TOP_DISP 0x2054
#define QSTATE_CTRL_DSIM0 0x240C
#define QSTATE_CTRL_DSIM1 0x2410
#define QSTATE_CTRL_DSIM2 0x2414
#define QSTATE_CTRL_HDMI 0x2418
#define QSTATE_CTRL_HDMI_AUDIO 0x241C
#define QSTATE_CTRL_DP 0x2420
#define QSTATE_CTRL_DISP0_MUX 0x2424
#define QSTATE_CTRL_HDMI_PHY 0x2428
#define QSTATE_CTRL_DISP1_400 0x2434
#define QSTATE_CTRL_DECON0 0x2438
#define QSTATE_CTRL_HPM_APBIF_DISP0 0x2444
#define QSTATE_CTRL_PROMISE_DISP0 0x2448
#define QSTATE_CTRL_DPTX_PHY 0x2484
#define QSTATE_CTRL_MIPI_DPHY_M1S0 0x2488
#define QSTATE_CTRL_MIPI_DPHY_M4S0 0x248C
#define QSTATE_CTRL_MIPI_DPHY_M4S4 0x2490

/* DISP0_PLL */
static const struct samsung_pll_rate_table
	exynos8890_disp0_pll_rates[] __initconst = {
		PLL_35XX_RATE(26 * MHZ, 134333333U, 248, 6, 3),
		PLL_35XX_RATE(26 * MHZ, 126000000U, 504, 13, 3),
		PLL_35XX_RATE(26 * MHZ, 71000000U, 568, 13, 4),
		PLL_35XX_RATE(26 * MHZ, 63000000U, 504, 13, 4),
		PLL_35XX_RATE(26 * MHZ, 62500000U, 500, 13, 4),
		PLL_35XX_RATE(26 * MHZ, 50000000U, 400, 13, 4),
		PLL_35XX_RATE(26 * MHZ, 42000000U, 336, 13, 4),
		{ /* sentinel */ }
	};

static const struct samsung_pll_clock disp0_pll_clks[] __initconst = {
	PLL(pll_141xx, 0, "disp_pll", "oscclk", DISP_PLL_LOCK, DISP_PLL_CON0,
	    exynos8890_disp0_pll_rates),
};

PNAME(disp0_mux_disp_pll_p) = { "oscclk", "disp_pll" };
PNAME(disp0_mux_aclk_disp0_0_400_user_p) = { "oscclk",
					     "top_gate_aclk_disp0_0_400" };
PNAME(disp0_mux_aclk_disp0_1_400_user_p) = { "oscclk",
					     "top_gate_aclk_disp0_1_400" };
PNAME(disp0_mux_sclk_disp0_decon0_eclk0_user_p) = {
	"oscclk", "top_gate_sclk_disp0_decon0_eclk0"
};
PNAME(disp0_mux_sclk_disp0_decon0_vclk0_user_p) = {
	"oscclk", "top_gate_sclk_disp0_decon0_vclk0"
};
PNAME(disp0_mux_sclk_disp0_decon0_vclk1_user_p) = {
	"oscclk", "top_gate_sclk_disp0_decon0_vclk1"
};
PNAME(disp0_mux_sclk_disp0_hdmi_audio_user_p) = {
	"oscclk", "top_gate_sclk_disp0_hdmi_audio"
};
PNAME(disp0_mux_phyclk_hdmiphy_pixel_clko_user_p) = {
	"oscclk", "phyclk_hdmiphy_pixel_clko_phy"
};
PNAME(disp0_mux_phyclk_hdmiphy_tmds_clko_user_p) = {
	"oscclk", "phyclk_hdmiphy_tmds_clko_phy"
};
PNAME(disp0_mux_phyclk_mipidphy0_rxclkesc0_user_p) = {
	"oscclk", "phyclk_mipidphy0_rxclkesc0_phy"
};
PNAME(disp0_mux_phyclk_mipidphy0_bitclkdiv2_user_p) = {
	"oscclk", "phyclk_mipidphy0_bitclkdiv2_phy"
};
PNAME(disp0_mux_phyclk_mipidphy0_bitclkdiv8_user_p) = {
	"oscclk", "phyclk_mipidphy0_bitclkdiv8_phy"
};
PNAME(disp0_mux_phyclk_mipidphy1_rxclkesc0_user_p) = {
	"oscclk", "phyclk_mipidphy1_rxclkesc0_phy"
};
PNAME(disp0_mux_phyclk_mipidphy1_bitclkdiv2_user_p) = {
	"oscclk", "phyclk_mipidphy1_bitclkdiv2_phy"
};
PNAME(disp0_mux_phyclk_mipidphy1_bitclkdiv8_user_p) = {
	"oscclk", "phyclk_mipidphy1_bitclkdiv8_phy"
};
PNAME(disp0_mux_phyclk_mipidphy2_rxclkesc0_user_p) = {
	"oscclk", "phyclk_mipidphy2_rxclkesc0_phy"
};
PNAME(disp0_mux_phyclk_mipidphy2_bitclkdiv2_user_p) = {
	"oscclk", "phyclk_mipidphy2_bitclkdiv2_phy"
};
PNAME(disp0_mux_phyclk_mipidphy2_bitclkdiv8_user_p) = {
	"oscclk", "phyclk_mipidphy2_bitclkdiv8_phy"
};
PNAME(disp0_mux_phyclk_dpphy_ch0_txd_clk_user_p) = {
	"oscclk", "phyclk_dpphy_ch0_txd_clk_phy"
};
PNAME(disp0_mux_phyclk_dpphy_ch1_txd_clk_user_p) = {
	"oscclk", "phyclk_dpphy_ch1_txd_clk_phy"
};
PNAME(disp0_mux_phyclk_dpphy_ch2_txd_clk_user_p) = {
	"oscclk", "phyclk_dpphy_ch2_txd_clk_phy"
};
PNAME(disp0_mux_phyclk_dpphy_ch3_txd_clk_user_p) = {
	"oscclk", "phyclk_dpphy_ch3_txd_clk_phy"
};
PNAME(disp0_mux_aclk_disp0_1_400_p) = { "disp0_mux_aclk_disp0_0_400_user",
					"disp0_mux_aclk_disp0_1_400_user" };
PNAME(disp0_mux_sclk_disp0_decon0_eclk0_p) = {
	"disp0_mux_sclk_disp0_decon0_eclk0_user", "disp0_mux_disp_pll",
	"disp0_mux_phyclk_mipidphy0_bitclkdiv2_user",
	"disp0_mux_phyclk_mipidphy1_bitclkdiv2_user",
	"disp0_mux_phyclk_mipidphy2_bitclkdiv2_user"
};
PNAME(disp0_mux_sclk_disp0_decon0_vclk0_p) = {
	"disp0_mux_sclk_disp0_decon0_vclk0_user", "disp0_mux_disp_pll",
	"disp0_mux_phyclk_mipidphy0_bitclkdiv2_user",
	"disp0_mux_phyclk_mipidphy1_bitclkdiv2_user",
	"disp0_mux_phyclk_mipidphy2_bitclkdiv2_user"
};
PNAME(disp0_mux_sclk_disp0_decon0_vclk1_p) = {
	"disp0_mux_sclk_disp0_decon0_vclk1_user", "disp0_mux_disp_pll",
	"disp0_mux_phyclk_mipidphy0_bitclkdiv2_user",
	"disp0_mux_phyclk_mipidphy1_bitclkdiv2_user",
	"disp0_mux_phyclk_mipidphy2_bitclkdiv2_user"
};
PNAME(disp0_mux_sclk_disp0_hdmi_audio_p) = {
	"disp0_mux_phyclk_hdmiphy_tmds_clko_user",
	"disp0_mux_sclk_disp0_hdmi_audio_user"
};

static const struct samsung_fixed_rate_clock disp0_fixed_clks[] __initconst = {
	FRATE(0, "phyclk_mipidphy0_bitclkdiv2_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_mipidphy1_bitclkdiv2_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_mipidphy2_bitclkdiv2_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_hdmiphy_pixel_clko_phy", NULL, 0, 300000000),
	FRATE(0, "phyclk_hdmiphy_tmds_clko_phy", NULL, 0, 300000000),
	FRATE(0, "phyclk_mipidphy0_rxclkesc0_phy", NULL, 0, 20000000),
	FRATE(0, "phyclk_mipidphy0_bitclkdiv8_phy", NULL, 0, 187500000),
	FRATE(0, "phyclk_mipidphy1_rxclkesc0_phy", NULL, 0, 20000000),
	FRATE(0, "phyclk_mipidphy1_bitclkdiv8_phy", NULL, 0, 187500000),
	FRATE(0, "phyclk_mipidphy2_rxclkesc0_phy", NULL, 0, 20000000),
	FRATE(0, "phyclk_mipidphy2_bitclkdiv8_phy", NULL, 0, 187500000),
	FRATE(0, "phyclk_dpphy_ch0_txd_clk_phy", NULL, 0, 270000000),
	FRATE(0, "phyclk_dpphy_ch1_txd_clk_phy", NULL, 0, 270000000),
	FRATE(0, "phyclk_dpphy_ch2_txd_clk_phy", NULL, 0, 270000000),
	FRATE(0, "phyclk_dpphy_ch3_txd_clk_phy", NULL, 0, 270000000),
};

static const struct samsung_mux_clock disp0_mux_clks[] __initconst = {
	MUX(0, "disp0_mux_disp_pll", disp0_mux_disp_pll_p, CLK_CON_MUX_DISP_PLL,
	    12, 1),
	MUX(0, "disp0_mux_aclk_disp0_0_400_user",
	    disp0_mux_aclk_disp0_0_400_user_p,
	    CLK_CON_MUX_ACLK_DISP0_0_400_USER, 12, 1),
	MUX(0, "disp0_mux_aclk_disp0_1_400_user",
	    disp0_mux_aclk_disp0_1_400_user_p,
	    CLK_CON_MUX_ACLK_DISP0_1_400_USER, 12, 1),
	MUX(0, "disp0_mux_sclk_disp0_decon0_eclk0_user",
	    disp0_mux_sclk_disp0_decon0_eclk0_user_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_ECLK0_USER, 12, 1),
	MUX(0, "disp0_mux_sclk_disp0_decon0_vclk0_user",
	    disp0_mux_sclk_disp0_decon0_vclk0_user_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK0_USER, 12, 1),
	MUX(0, "disp0_mux_sclk_disp0_decon0_vclk1_user",
	    disp0_mux_sclk_disp0_decon0_vclk1_user_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK1_USER, 12, 1),
	MUX(0, "disp0_mux_sclk_disp0_hdmi_audio_user",
	    disp0_mux_sclk_disp0_hdmi_audio_user_p,
	    CLK_CON_MUX_SCLK_DISP0_HDMI_AUDIO_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_hdmiphy_pixel_clko_user",
	    disp0_mux_phyclk_hdmiphy_pixel_clko_user_p,
	    CLK_CON_MUX_PHYCLK_HDMIPHY_PIXEL_CLKO_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_hdmiphy_tmds_clko_user",
	    disp0_mux_phyclk_hdmiphy_tmds_clko_user_p,
	    CLK_CON_MUX_PHYCLK_HDMIPHY_TMDS_CLKO_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy0_rxclkesc0_user",
	    disp0_mux_phyclk_mipidphy0_rxclkesc0_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY0_RXCLKESC0_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy0_bitclkdiv2_user",
	    disp0_mux_phyclk_mipidphy0_bitclkdiv2_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV2_USER_DISP0, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy0_bitclkdiv8_user",
	    disp0_mux_phyclk_mipidphy0_bitclkdiv8_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV8_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy1_rxclkesc0_user",
	    disp0_mux_phyclk_mipidphy1_rxclkesc0_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY1_RXCLKESC0_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy1_bitclkdiv2_user",
	    disp0_mux_phyclk_mipidphy1_bitclkdiv2_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV2_USER_DISP0, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy1_bitclkdiv8_user",
	    disp0_mux_phyclk_mipidphy1_bitclkdiv8_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV8_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy2_rxclkesc0_user",
	    disp0_mux_phyclk_mipidphy2_rxclkesc0_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY2_RXCLKESC0_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy2_bitclkdiv2_user",
	    disp0_mux_phyclk_mipidphy2_bitclkdiv2_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV2_USER_DISP0, 12, 1),
	MUX(0, "disp0_mux_phyclk_mipidphy2_bitclkdiv8_user",
	    disp0_mux_phyclk_mipidphy2_bitclkdiv8_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV8_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_dpphy_ch0_txd_clk_user",
	    disp0_mux_phyclk_dpphy_ch0_txd_clk_user_p,
	    CLK_CON_MUX_PHYCLK_DPPHY_CH0_TXD_CLK_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_dpphy_ch1_txd_clk_user",
	    disp0_mux_phyclk_dpphy_ch1_txd_clk_user_p,
	    CLK_CON_MUX_PHYCLK_DPPHY_CH1_TXD_CLK_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_dpphy_ch2_txd_clk_user",
	    disp0_mux_phyclk_dpphy_ch2_txd_clk_user_p,
	    CLK_CON_MUX_PHYCLK_DPPHY_CH2_TXD_CLK_USER, 12, 1),
	MUX(0, "disp0_mux_phyclk_dpphy_ch3_txd_clk_user",
	    disp0_mux_phyclk_dpphy_ch3_txd_clk_user_p,
	    CLK_CON_MUX_PHYCLK_DPPHY_CH3_TXD_CLK_USER, 12, 1),
	MUX(0, "disp0_mux_aclk_disp0_1_400", disp0_mux_aclk_disp0_1_400_p,
	    CLK_CON_MUX_ACLK_DISP0_1_400_DISP0, 12, 1),
	MUX(0, "disp0_mux_sclk_disp0_decon0_eclk0",
	    disp0_mux_sclk_disp0_decon0_eclk0_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_ECLK0_DISP0, 12, 3),
	MUX(0, "disp0_mux_sclk_disp0_decon0_vclk0",
	    disp0_mux_sclk_disp0_decon0_vclk0_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK0_DISP0, 12, 3),
	MUX(0, "disp0_mux_sclk_disp0_decon0_vclk1",
	    disp0_mux_sclk_disp0_decon0_vclk1_p,
	    CLK_CON_MUX_SCLK_DISP0_DECON0_VCLK1_DISP0, 12, 3),
	MUX(0, "disp0_mux_sclk_disp0_hdmi_audio",
	    disp0_mux_sclk_disp0_hdmi_audio_p,
	    CLK_CON_MUX_SCLK_DISP0_HDMI_AUDIO_DISP0, 12, 1),
};

static const struct samsung_gate_clock disp0_gate_clks[] __initconst = {
	GATE(0, "disp0_gate_aclk_ppmu_disp0_0",
	     "disp0_mux_aclk_disp0_0_400_user", CG_CTRL_VAL_ACLK_DISP0_0_400, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_smmu_disp0_0",
	     "disp0_mux_aclk_disp0_0_400_user", CG_CTRL_VAL_ACLK_DISP0_0_400, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_xiu_disp0_0",
	     "disp0_mux_aclk_disp0_0_400_user", CG_CTRL_VAL_ACLK_DISP0_0_400, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_lh_async_si_r_top_disp",
	     "disp0_mux_aclk_disp0_0_400_user", CG_CTRL_VAL_ACLK_DISP0_0_400, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_vpp0_aclk_0",
	     "disp0_mux_aclk_disp0_0_400_user", CG_CTRL_VAL_ACLK_DISP0_0_400, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_ppmu_disp0_1", "disp0_mux_aclk_disp0_1_400",
	     CG_CTRL_VAL_ACLK_DISP0_1_400, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_smmu_disp0_1", "disp0_mux_aclk_disp0_1_400",
	     CG_CTRL_VAL_ACLK_DISP0_1_400, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_xiu_disp0_1", "disp0_mux_aclk_disp0_1_400",
	     CG_CTRL_VAL_ACLK_DISP0_1_400, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_lh_async_si_top_disp",
	     "disp0_mux_aclk_disp0_1_400", CG_CTRL_VAL_ACLK_DISP0_1_400, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_vpp0_aclk_1", "disp0_mux_aclk_disp0_1_400",
	     CG_CTRL_VAL_ACLK_DISP0_1_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_sfw_disp0_0",
	     "disp0_mux_aclk_disp0_0_400_user",
	     CG_CTRL_VAL_ACLK_DISP0_0_400_SECURE_SFW_DISP0_0, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_sfw_disp0_1", "disp0_mux_aclk_disp0_1_400",
	     CG_CTRL_VAL_ACLK_DISP0_1_400_SECURE_SFW_DISP0_1, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_vpp0_1", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 19, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_smmu_disp0_1", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 18, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_smmu_disp0_0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 17, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_ppmu_disp0_1", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_ppmu_disp0_0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_hdmi_phy", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_disp0_mux", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_dp", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_hdmi_audio", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_hdmi", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_dsim2", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_dsim1", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_dsim0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_sysreg_disp0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_pmu_disp0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_cmu_disp0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_xiu_disp0sfrx", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_axi2apb_disp0_1p",
	     "disp0_div_pclk_disp0_0_133", CG_CTRL_VAL_PCLK_DISP0_0_133, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_axi2apb_disp0_0p",
	     "disp0_div_pclk_disp0_0_133", CG_CTRL_VAL_PCLK_DISP0_0_133, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_aclk_axi_lh_async_mi_disp0sfr",
	     "disp0_div_pclk_disp0_0_133", CG_CTRL_VAL_PCLK_DISP0_0_133, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_hpm_apbif_disp0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133_HPM_APBIF_DISP0, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "disp0_gate_pclk_decon0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_DECON0, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "disp0_gate_pclk_vpp0_0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_VPP0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_sfw_disp0_0", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_SFW_DISP0_0, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_pclk_sfw_disp0_1", "disp0_div_pclk_disp0_0_133",
	     CG_CTRL_VAL_PCLK_DISP0_0_133_SECURE_SFW_DISP0_1, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_sclk_disp1_400", "disp0_mux_disp_pll",
	     CG_CTRL_VAL_SCLK_DISP1_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_sclk_decon0_eclk0", "disp0_div_sclk_decon0_eclk0",
	     CG_CTRL_VAL_SCLK_DECON0_ECLK0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_sclk_decon0_vclk0", "disp0_div_sclk_decon0_vclk0",
	     CG_CTRL_VAL_SCLK_DECON0_VCLK0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_sclk_decon0_vclk1", "disp0_div_sclk_decon0_vclk1",
	     CG_CTRL_VAL_SCLK_DECON0_VCLK1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_sclk_hdmi_audio", "disp0_mux_sclk_disp0_hdmi_audio",
	     CG_CTRL_VAL_SCLK_HDMI_AUDIO, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_sclk_promise_disp0", "top_gate_sclk_promise_disp",
	     CG_CTRL_VAL_SCLK_DISP0_PROMISE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_hdmiphy_tmds_20b_clko",
	     "disp0_div_phyclk_hdmiphy_tmds_20b_clko",
	     CG_CTRL_VAL_PHYCLK_HDMIPHY, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_hdmiphy_tmds_10b_clko",
	     "disp0_mux_phyclk_hdmiphy_tmds_clko_user",
	     CG_CTRL_VAL_PHYCLK_HDMIPHY, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_hdmiphy_pixel_clko",
	     "disp0_div_phyclk_hdmiphy_pixel_clko", CG_CTRL_VAL_PHYCLK_HDMIPHY,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_mipidphy0_bitclkdiv8",
	     "disp0_mux_phyclk_mipidphy0_bitclkdiv8_user",
	     CG_CTRL_VAL_PHYCLK_MIPIDPHY0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_mipidphy0_rxclkesc0",
	     "disp0_mux_phyclk_mipidphy0_rxclkesc0_user",
	     CG_CTRL_VAL_PHYCLK_MIPIDPHY0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_mipidphy1_bitclkdiv8",
	     "disp0_mux_phyclk_mipidphy1_bitclkdiv8_user",
	     CG_CTRL_VAL_PHYCLK_MIPIDPHY1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_mipidphy1_rxclkesc0",
	     "disp0_mux_phyclk_mipidphy1_rxclkesc0_user",
	     CG_CTRL_VAL_PHYCLK_MIPIDPHY1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_mipidphy2_bitclkdiv8",
	     "disp0_mux_phyclk_mipidphy2_bitclkdiv8_user",
	     CG_CTRL_VAL_PHYCLK_MIPIDPHY2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_mipidphy2_rxclkesc0",
	     "disp0_mux_phyclk_mipidphy2_rxclkesc0_user",
	     CG_CTRL_VAL_PHYCLK_MIPIDPHY2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_dpphy_ch3_txd_clk",
	     "disp0_mux_phyclk_dpphy_ch3_txd_clk_user",
	     CG_CTRL_VAL_PHYCLK_DPPHY, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_dpphy_ch2_txd_clk",
	     "disp0_mux_phyclk_dpphy_ch2_txd_clk_user",
	     CG_CTRL_VAL_PHYCLK_DPPHY, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_dpphy_ch1_txd_clk",
	     "disp0_mux_phyclk_dpphy_ch1_txd_clk_user",
	     CG_CTRL_VAL_PHYCLK_DPPHY, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_phyclk_dpphy_ch0_txd_clk",
	     "disp0_mux_phyclk_dpphy_ch0_txd_clk_user",
	     CG_CTRL_VAL_PHYCLK_DPPHY, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_oscclk_i_mipi_dphy_m4s4_m_xi", "oscclk",
	     CG_CTRL_VAL_OSCCLK, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_oscclk_i_mipi_dphy_m4s0_m_xi", "oscclk",
	     CG_CTRL_VAL_OSCCLK, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_oscclk_i_mipi_dphy_m1s0_m_xi", "oscclk",
	     CG_CTRL_VAL_OSCCLK, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_oscclk_i_dptx_phy_i_ref_clk_24m", "oscclk",
	     CG_CTRL_VAL_OSCCLK, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_gate_oscclk_dp_i_clk_24m", "oscclk", CG_CTRL_VAL_OSCCLK,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_dsim0", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_DSIM0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_dsim1", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_DSIM1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_dsim2", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_DSIM2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_hdmi", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_HDMI, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_hdmi_audio", "disp0_mux_sclk_disp0_hdmi_audio",
	     QSTATE_CTRL_HDMI_AUDIO, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_dp", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_DP, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_disp0_mux", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_DISP0_MUX, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_hdmi_phy", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_HDMI_PHY, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_disp1_400", "disp0_mux_aclk_disp0_1_400",
	     QSTATE_CTRL_DISP1_400, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_decon0", "disp0_mux_aclk_disp0_0_400_user",
	     QSTATE_CTRL_DECON0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_hpm_apbif_disp0", "disp0_div_pclk_disp0_0_133",
	     QSTATE_CTRL_HPM_APBIF_DISP0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_promise_disp0", "top_gate_sclk_promise_int",
	     QSTATE_CTRL_PROMISE_DISP0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_dptx_phy", "oscclk", QSTATE_CTRL_DPTX_PHY, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_mipi_dphy_m1s0", "oscclk",
	     QSTATE_CTRL_MIPI_DPHY_M1S0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_mipi_dphy_m4s0", "oscclk",
	     QSTATE_CTRL_MIPI_DPHY_M4S0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp0_hwacg_mipi_dphy_m4s4", "oscclk",
	     QSTATE_CTRL_MIPI_DPHY_M4S4, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock disp0_div_clks[] __initconst = {
	DIV(0, "disp0_div_pclk_disp0_0_133", "disp0_mux_aclk_disp0_0_400_user",
	    CLK_CON_DIV_PCLK_DISP0_0_133, 0, 3),
	DIV(0, "disp0_div_sclk_decon0_eclk0",
	    "disp0_mux_sclk_disp0_decon0_eclk0", CLK_CON_DIV_SCLK_DECON0_ECLK0,
	    0, 3),
	DIV(0, "disp0_div_sclk_decon0_vclk0",
	    "disp0_mux_sclk_disp0_decon0_vclk0", CLK_CON_DIV_SCLK_DECON0_VCLK0,
	    0, 3),
	DIV(0, "disp0_div_sclk_decon0_vclk1",
	    "disp0_mux_sclk_disp0_decon0_vclk1", CLK_CON_DIV_SCLK_DECON0_VCLK1,
	    0, 3),
	DIV(0, "disp0_div_phyclk_hdmiphy_pixel_clko",
	    "disp0_mux_phyclk_hdmiphy_pixel_clko_user",
	    CLK_CON_DIV_PHYCLK_HDMIPHY_PIXEL_CLKO, 0, 1),
	DIV(0, "disp0_div_phyclk_hdmiphy_tmds_20b_clko",
	    "disp0_mux_phyclk_hdmiphy_tmds_clko_user",
	    CLK_CON_DIV_PHYCLK_HDMIPHY_TMDS_20B_CLKO, 0, 1),
};

static const struct samsung_cmu_info disp0_cmu_info __initconst = {
	.pll_clks = disp0_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(disp0_pll_clks),
	.mux_clks = disp0_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(disp0_mux_clks),
	.div_clks = disp0_div_clks,
	.nr_div_clks = ARRAY_SIZE(disp0_div_clks),
	.gate_clks = disp0_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(disp0_gate_clks),
	.fixed_clks = disp0_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(disp0_fixed_clks),
	.nr_clk_ids = DISP0_NR_CLK,
};

/* Register Offset definitions for CMU_DISP1 (0x13F00000) */
#define CLK_CON_MUX_ACLK_DISP1_0_400_USER 0x0200
#define CLK_CON_MUX_ACLK_DISP1_1_400_USER 0x0204
#define CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK0_USER 0x0208
#define CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK1_USER 0x020C
#define CLK_CON_MUX_SCLK_DISP1_600_USER 0x0210
#define CLK_CON_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV2_USER_DISP1 0x0214
#define CLK_CON_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV2_USER_DISP1 0x0218
#define CLK_CON_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV2_USER_DISP1 0x021C
#define CLK_CON_MUX_PHYCLK_DISP1_HDMIPHY_PIXEL_CLKO_USER 0x0220
#define CLK_CON_MUX_ACLK_DISP1_1_400_DISP1 0x0224
#define CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK0_DISP1 0x0228
#define CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK1_DISP1 0x022C
#define CLK_CON_MUX_SCLK_DECON1_ECLK1 0x0230
#define CLK_CON_DIV_PCLK_DISP1_0_133 0x0400
#define CLK_CON_DIV_SCLK_DECON1_ECLK0 0x0404
#define CLK_CON_DIV_SCLK_DECON1_ECLK1 0x0408
#define CLK_STAT_MUX_ACLK_DISP1_0_400_USER 0x0600
#define CLK_STAT_MUX_ACLK_DISP1_1_400_USER 0x0604
#define CLK_STAT_MUX_SCLK_DISP1_DECON1_ECLK0_USER 0x0608
#define CLK_STAT_MUX_SCLK_DISP1_DECON1_ECLK1_USER 0x060C
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV2_USER_DISP1 0x0610
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV2_USER_DISP1 0x0614
#define CLK_STAT_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV2_USER_DISP1 0x0618
#define CLK_STAT_MUX_PHYCLK_DISP1_HDMIPHY_PIXEL_CLKO_USER 0x061C
#define CLK_STAT_MUX_SCLK_DISP1_600_USER 0x0620
#define CLK_STAT_MUX_ACLK_DISP1_1_400_DISP1 0x0624
#define CLK_STAT_MUX_SCLK_DISP1_DECON1_ECLK0_DISP1 0x0628
#define CLK_STAT_MUX_SCLK_DISP1_DECON1_ECLK1_DISP1 0x062C
#define CLK_STAT_MUX_SCLK_DECON1_ECLK1 0x0630
#define CG_CTRL_VAL_ACLK_DISP1_0_400 0x0800
#define CG_CTRL_VAL_ACLK_DISP1_1_400 0x0804
#define CG_CTRL_VAL_ACLK_DISP1_0_400_SECURE_SFW_DISP1_0 0x0808
#define CG_CTRL_VAL_ACLK_DISP1_1_400_SECURE_SFW_DISP1_1 0x080C
#define CG_CTRL_VAL_PCLK_DISP1_0_133 0x0820
#define CG_CTRL_VAL_PCLK_DISP1_0_133_HPM_APBIF_DISP1 0x0824
#define CG_CTRL_VAL_PCLK_DISP1_0_133_SECURE_SFW_DISP1_0 0x0828
#define CG_CTRL_VAL_PCLK_DISP1_0_133_SECURE_SFW_DISP1_1 0x082C
#define CG_CTRL_VAL_SCLK_DECON1_ECLK_0 0x0840
#define CG_CTRL_VAL_SCLK_DECON1_ECLK_1 0x0844
#define CG_CTRL_VAL_SCLK_DISP1_PROMISE 0x0848
#define CLKOUT_CMU_DISP1 0x0C00
#define CLKOUT_CMU_DISP1_DIV_STAT 0x0C04
#define DISP1_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_DISP1_SPARE0 0x0D08
#define CMU_DISP1_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_DISP1_0_400 0x1800
#define CG_CTRL_MAN_ACLK_DISP1_1_400 0x1804
#define CG_CTRL_MAN_ACLK_DISP1_0_400_SECURE_SFW_DISP1_0 0x1808
#define CG_CTRL_MAN_ACLK_DISP1_1_400_SECURE_SFW_DISP1_1 0x180C
#define CG_CTRL_MAN_PCLK_DISP1_0_133 0x1820
#define CG_CTRL_MAN_PCLK_DISP1_0_133_HPM_APBIF_DISP1 0x1824
#define CG_CTRL_MAN_PCLK_DISP1_0_133_SECURE_SFW_DISP1_0 0x1828
#define CG_CTRL_MAN_PCLK_DISP1_0_133_SECURE_SFW_DISP1_1 0x182C
#define CG_CTRL_MAN_SCLK_DECON1_ECLK_0 0x1840
#define CG_CTRL_MAN_SCLK_DECON1_ECLK_1 0x1844
#define CG_CTRL_MAN_SCLK_DISP1_PROMISE 0x1848
#define CG_CTRL_STAT_ACLK_DISP1_0_400 0x1C00
#define CG_CTRL_STAT_ACLK_DISP1_1_400 0x1C04
#define CG_CTRL_STAT_ACLK_DISP1_0_400_SECURE_SFW_DISP1_0 0x1C08
#define CG_CTRL_STAT_ACLK_DISP1_1_400_SECURE_SFW_DISP1_1 0x1C0C
#define CG_CTRL_STAT_PCLK_DISP1_0_133_0 0x1C20
#define CG_CTRL_STAT_PCLK_DISP1_0_133_1 0x1C24
#define CG_CTRL_STAT_PCLK_DISP1_0_133_2 0x1C28
#define CG_CTRL_STAT_PCLK_DISP1_0_133_HPM_APBIF_DISP1 0x1C2C
#define CG_CTRL_STAT_PCLK_DISP1_0_133_SECURE_SFW_DISP1_0 0x1C30
#define CG_CTRL_STAT_PCLK_DISP1_0_133_SECURE_SFW_DISP1_1 0x1C34
#define CG_CTRL_STAT_SCLK_DECON1_ECLK_0 0x1C40
#define CG_CTRL_STAT_SCLK_DECON1_ECLK_1 0x1C44
#define CG_CTRL_STAT_SCLK_DISP1_PROMISE 0x1C48
#define QCH_CTRL_AXI_LH_ASYNC_MI_DISP1SFR 0x2000
#define QCH_CTRL_CMU_DISP1 0x2004
#define QCH_CTRL_PMU_DISP1 0x2008
#define QCH_CTRL_SYSREG_DISP1 0x200C
#define QCH_CTRL_VPP1 0x2010
#define QCH_CTRL_VPP1_G2 0x2010
#define QCH_CTRL_VPP1_G3 0x2014
#define QCH_CTRL_DECON1_PCLK_0 0x201C
#define QCH_CTRL_DECON1_PCLK_1 0x2020
#define QCH_CTRL_PPMU_DISP1_0 0x2028
#define QCH_CTRL_PPMU_DISP1_1 0x202C
#define QCH_CTRL_SMMU_DISP1_0 0x2030
#define QCH_CTRL_SMMU_DISP1_1 0x2034
#define QCH_CTRL_SFW_DISP1_0 0x2038
#define QCH_CTRL_SFW_DISP1_1 0x203C
#define QCH_CTRL_AXI_LH_ASYNC_SI_DISP1_0 0x2040
#define QCH_CTRL_AXI_LH_ASYNC_SI_DISP1_1 0x2044
#define QSTATE_CTRL_DECON1_ECLK_0 0x240C
#define QSTATE_CTRL_DECON1_ECLK_1 0x2410
#define QSTATE_CTRL_HPM_APBIF_DISP1 0x241C
#define QSTATE_CTRL_PROMISE_DISP1 0x2420

PNAME(disp1_mux_aclk_disp1_0_400_user_p) = { "oscclk",
					     "top_gate_aclk_disp1_0_400" };
PNAME(disp1_mux_aclk_disp1_1_400_user_p) = { "oscclk",
					     "top_gate_aclk_disp1_1_400" };
PNAME(disp1_mux_sclk_disp1_decon1_eclk0_user_p) = {
	"oscclk", "top_gate_sclk_disp1_decon1_eclk0"
};
PNAME(disp1_mux_sclk_disp1_decon1_eclk1_user_p) = {
	"oscclk", "top_gate_sclk_disp1_decon1_eclk1"
};
PNAME(disp1_mux_sclk_disp1_600_user_p) = { "oscclk",
					   "disp0_gate_sclk_disp1_400" };
PNAME(disp1_mux_phyclk_mipidphy0_bitclkdiv2_user_p) = {
	"oscclk", "phyclk_mipidphy0_bitclkdiv2_phy"
};
PNAME(disp1_mux_phyclk_mipidphy1_bitclkdiv2_user_p) = {
	"oscclk", "phyclk_mipidphy1_bitclkdiv2_phy"
};
PNAME(disp1_mux_phyclk_mipidphy2_bitclkdiv2_user_p) = {
	"oscclk", "phyclk_mipidphy2_bitclkdiv2_phy"
};
PNAME(disp1_mux_phyclk_disp1_hdmiphy_pixel_clko_user_p) = {
	"oscclk", "phyclk_disp1_hdmiphy_pixel_clko_phy"
};
PNAME(disp1_mux_aclk_disp1_1_400_p) = { "disp1_mux_aclk_disp1_0_400_user",
					"disp1_mux_aclk_disp1_1_400_user" };
PNAME(disp1_mux_sclk_disp1_decon1_eclk0_p) = {
	"disp1_mux_sclk_disp1_decon1_eclk0_user",
	"oscclk",
	"disp1_mux_sclk_disp1_600_user",
	"disp1_mux_phyclk_mipidphy0_bitclkdiv2_user",
	"disp1_mux_phyclk_mipidphy1_bitclkdiv2_user",
	"disp1_mux_phyclk_mipidphy2_bitclkdiv2_user"
};
PNAME(disp1_mux_sclk_disp1_decon1_eclk1_p) = {
	"disp1_mux_sclk_disp1_decon1_eclk1_user", "oscclk",
	"disp1_mux_sclk_disp1_600_user"
};
PNAME(disp1_mux_sclk_decon1_eclk1_p) = {
	"disp1_div_sclk_decon1_eclk1",
	"disp1_mux_phyclk_disp1_hdmiphy_pixel_clko_user"
};

static const struct samsung_fixed_rate_clock disp1_fixed_clks[] __initconst = {
	FRATE(0, "phyclk_disp1_hdmiphy_pixel_clko_phy", NULL, 0, 0000000), // ??
};

static const struct samsung_mux_clock disp1_mux_clks[] __initconst = {
	MUX(0, "disp1_mux_aclk_disp1_0_400_user",
	    disp1_mux_aclk_disp1_0_400_user_p,
	    CLK_CON_MUX_ACLK_DISP1_0_400_USER, 12, 1),
	MUX(0, "disp1_mux_aclk_disp1_1_400_user",
	    disp1_mux_aclk_disp1_1_400_user_p,
	    CLK_CON_MUX_ACLK_DISP1_1_400_USER, 12, 1),
	MUX(0, "disp1_mux_sclk_disp1_decon1_eclk0_user",
	    disp1_mux_sclk_disp1_decon1_eclk0_user_p,
	    CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK0_USER, 12, 1),
	MUX(0, "disp1_mux_sclk_disp1_decon1_eclk1_user",
	    disp1_mux_sclk_disp1_decon1_eclk1_user_p,
	    CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK1_USER, 12, 1),
	MUX(0, "disp1_mux_sclk_disp1_600_user", disp1_mux_sclk_disp1_600_user_p,
	    CLK_CON_MUX_SCLK_DISP1_600_USER, 12, 1),
	MUX(0, "disp1_mux_phyclk_mipidphy0_bitclkdiv2_user",
	    disp1_mux_phyclk_mipidphy0_bitclkdiv2_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY0_BITCLKDIV2_USER_DISP1, 12, 1),
	MUX(0, "disp1_mux_phyclk_mipidphy1_bitclkdiv2_user",
	    disp1_mux_phyclk_mipidphy1_bitclkdiv2_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY1_BITCLKDIV2_USER_DISP1, 12, 1),
	MUX(0, "disp1_mux_phyclk_mipidphy2_bitclkdiv2_user",
	    disp1_mux_phyclk_mipidphy2_bitclkdiv2_user_p,
	    CLK_CON_MUX_PHYCLK_MIPIDPHY2_BITCLKDIV2_USER_DISP1, 12, 1),
	MUX(0, "disp1_mux_phyclk_disp1_hdmiphy_pixel_clko_user",
	    disp1_mux_phyclk_disp1_hdmiphy_pixel_clko_user_p,
	    CLK_CON_MUX_PHYCLK_DISP1_HDMIPHY_PIXEL_CLKO_USER, 12, 1),
	MUX(0, "disp1_mux_aclk_disp1_1_400", disp1_mux_aclk_disp1_1_400_p,
	    CLK_CON_MUX_ACLK_DISP1_1_400_DISP1, 12, 1),
	MUX(0, "disp1_mux_sclk_disp1_decon1_eclk0",
	    disp1_mux_sclk_disp1_decon1_eclk0_p,
	    CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK0_DISP1, 12, 3),
	MUX(0, "disp1_mux_sclk_disp1_decon1_eclk1",
	    disp1_mux_sclk_disp1_decon1_eclk1_p,
	    CLK_CON_MUX_SCLK_DISP1_DECON1_ECLK1_DISP1, 12, 2),
	MUX(0, "disp1_mux_sclk_decon1_eclk1", disp1_mux_sclk_decon1_eclk1_p,
	    CLK_CON_MUX_SCLK_DECON1_ECLK1, 12, 1),
};

static const struct samsung_gate_clock disp1_gate_clks[] __initconst = {
	GATE(0, "disp1_gate_aclk_xiu_disp1x0",
	     "disp1_mux_aclk_disp1_0_400_user", CG_CTRL_VAL_ACLK_DISP1_0_400, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_axi_lh_async_si_disp1_0",
	     "disp1_mux_aclk_disp1_0_400_user", CG_CTRL_VAL_ACLK_DISP1_0_400, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_ppmu_disp1_0",
	     "disp1_mux_aclk_disp1_0_400_user", CG_CTRL_VAL_ACLK_DISP1_0_400, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_smmu_disp1_0",
	     "disp1_mux_aclk_disp1_0_400_user", CG_CTRL_VAL_ACLK_DISP1_0_400, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_vpp1_0", "disp1_mux_aclk_disp1_0_400_user",
	     CG_CTRL_VAL_ACLK_DISP1_0_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_xiu_disp1x1", "disp1_mux_aclk_disp1_1_400",
	     CG_CTRL_VAL_ACLK_DISP1_1_400, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_axi_lh_async_si_disp1_1",
	     "disp1_mux_aclk_disp1_1_400", CG_CTRL_VAL_ACLK_DISP1_1_400, 5,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_qe_disp1_wdma", "disp1_mux_aclk_disp1_1_400",
	     CG_CTRL_VAL_ACLK_DISP1_1_400, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_ppmu_disp1_1", "disp1_mux_aclk_disp1_1_400",
	     CG_CTRL_VAL_ACLK_DISP1_1_400, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_smmu_disp1_1", "disp1_mux_aclk_disp1_1_400",
	     CG_CTRL_VAL_ACLK_DISP1_1_400, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_vpp1_1", "disp1_mux_aclk_disp1_1_400",
	     CG_CTRL_VAL_ACLK_DISP1_1_400, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_sfw_disp1_0",
	     "disp1_mux_aclk_disp1_0_400_user",
	     CG_CTRL_VAL_ACLK_DISP1_0_400_SECURE_SFW_DISP1_0, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_sfw_disp1_1", "disp1_mux_aclk_disp1_1_400",
	     CG_CTRL_VAL_ACLK_DISP1_1_400_SECURE_SFW_DISP1_1, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_vpp1_1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_decon1_1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_decon1_0", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_qe_disp1_wdma", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_ppmu_disp1_1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_ppmu_disp1_0", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_smmu_disp1_1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_smmu_disp1_0", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_sysreg_disp1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_pmu_disp1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_cmu_disp1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_vpp1_0", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_axi2apb_disp1_1x",
	     "disp1_div_pclk_disp1_0_133", CG_CTRL_VAL_PCLK_DISP1_0_133, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_axi2apb_disp1_0x",
	     "disp1_div_pclk_disp1_0_133", CG_CTRL_VAL_PCLK_DISP1_0_133, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_xiu_disp1sfrx", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_aclk_axi_lh_async_mi_disp1sfr",
	     "disp1_div_pclk_disp1_0_133", CG_CTRL_VAL_PCLK_DISP1_0_133, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_hpm_apbif_disp1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133_HPM_APBIF_DISP1, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "disp1_gate_pclk_sfw_disp1_0", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133_SECURE_SFW_DISP1_0, 10,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_pclk_sfw_disp1_1", "disp1_div_pclk_disp1_0_133",
	     CG_CTRL_VAL_PCLK_DISP1_0_133_SECURE_SFW_DISP1_1, 11,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_sclk_decon1_eclk_0", "disp1_div_sclk_decon1_eclk0",
	     CG_CTRL_VAL_SCLK_DECON1_ECLK_0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_sclk_decon1_eclk_1", "disp1_div_sclk_decon1_eclk1",
	     CG_CTRL_VAL_SCLK_DECON1_ECLK_1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_gate_sclk_promise_disp1", "top_gate_sclk_promise_disp",
	     CG_CTRL_VAL_SCLK_DISP1_PROMISE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_hwacg_decon1_eclk_0", "disp1_div_sclk_decon1_eclk0",
	     QSTATE_CTRL_DECON1_ECLK_0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_hwacg_decon1_eclk_1", "disp1_div_sclk_decon1_eclk1",
	     QSTATE_CTRL_DECON1_ECLK_1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_hwacg_hpm_apbif_disp1", "disp0_div_pclk_disp0_0_133",
	     QSTATE_CTRL_HPM_APBIF_DISP1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "disp1_hwacg_promise_disp1", "top_gate_sclk_promise_int",
	     QSTATE_CTRL_PROMISE_DISP1, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock disp1_div_clks[] __initconst = {
	DIV(0, "disp1_div_pclk_disp1_0_133", "disp1_mux_aclk_disp1_0_400_user",
	    CLK_CON_DIV_PCLK_DISP1_0_133, 0, 3),
	DIV(0, "disp1_div_sclk_decon1_eclk0",
	    "disp1_mux_sclk_disp1_decon1_eclk0", CLK_CON_DIV_SCLK_DECON1_ECLK0,
	    0, 3),
	DIV(0, "disp1_div_sclk_decon1_eclk1",
	    "disp1_mux_sclk_disp1_decon1_eclk1", CLK_CON_DIV_SCLK_DECON1_ECLK1,
	    0, 3),
};

static const struct samsung_cmu_info disp1_cmu_info __initconst = {
	.mux_clks = disp1_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(disp1_mux_clks),
	.div_clks = disp1_div_clks,
	.nr_div_clks = ARRAY_SIZE(disp1_div_clks),
	.gate_clks = disp1_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(disp1_gate_clks),
	.fixed_clks = disp1_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(disp1_fixed_clks),
	.nr_clk_ids = DISP1_NR_CLK,
};

/* Register Offset definitions for CMU_CAM0_LOCAL (0x140F0000) */
#define CLK_ENABLE_ACLK_CAM0_CSIS0_414_LOCAL 0x0800
#define CLK_ENABLE_PCLK_CAM0_CSIS0_207_LOCAL 0x0804
#define CLK_ENABLE_ACLK_CAM0_CSIS1_168_LOCAL 0x080C
#define CLK_ENABLE_ACLK_CAM0_CSIS2_234_LOCAL 0x0818
#define CLK_ENABLE_ACLK_CAM0_CSIS3_132_LOCAL 0x081C
#define CLK_ENABLE_ACLK_CAM0_3AA0_414_LOCAL 0x0828
#define CLK_ENABLE_PCLK_CAM0_3AA0_207_LOCAL 0x082C
#define CLK_ENABLE_ACLK_CAM0_3AA1_414_LOCAL 0x0830
#define CLK_ENABLE_PCLK_CAM0_3AA1_207_LOCAL 0x0834
#define CLK_ENABLE_PCLK_CAM0_TREX_264_LOCAL 0x083C

static const struct samsung_gate_clock cam0_local_gate_clks[] __initconst = {
	GATE(0, "cam0_local_gate_aclk_bns", "cam0_gate_aclk_bns",
	     CLK_ENABLE_ACLK_CAM0_CSIS0_414_LOCAL, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_aclk_pxl_asbs_csis2_int",
	     "cam0_gate_aclk_pxl_asbs_csis2_int",
	     CLK_ENABLE_ACLK_CAM0_CSIS0_414_LOCAL, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_aclk_csis0", "cam0_gate_aclk_csis0",
	     CLK_ENABLE_ACLK_CAM0_CSIS0_414_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_pclk_bns", "cam0_gate_pclk_bns",
	     CLK_ENABLE_PCLK_CAM0_CSIS0_207_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_aclk_csis1", "cam0_gate_aclk_csis1",
	     CLK_ENABLE_ACLK_CAM0_CSIS1_168_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_aclk_csis2", "cam0_gate_aclk_csis2",
	     CLK_ENABLE_ACLK_CAM0_CSIS2_234_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_aclk_csis3", "cam0_gate_aclk_csis3",
	     CLK_ENABLE_ACLK_CAM0_CSIS3_132_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_aclk_3aa0", "cam0_gate_aclk_3aa0",
	     CLK_ENABLE_ACLK_CAM0_3AA0_414_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_pclk_3aa0", "cam0_gate_pclk_3aa0",
	     CLK_ENABLE_PCLK_CAM0_3AA0_207_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_aclk_3aa1", "cam0_gate_aclk_3aa1",
	     CLK_ENABLE_ACLK_CAM0_3AA1_414_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_pclk_3aa1", "cam0_gate_pclk_3aa1",
	     CLK_ENABLE_PCLK_CAM0_3AA1_207_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_pclk_csis1", "cam0_gate_pclk_csis1",
	     CLK_ENABLE_PCLK_CAM0_TREX_264_LOCAL, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_local_gate_pclk_csis0", "cam0_gate_pclk_csis0",
	     CLK_ENABLE_PCLK_CAM0_TREX_264_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info cam0_local_cmu_info __initconst = {
	.gate_clks = cam0_local_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(cam0_local_gate_clks),
	.nr_clk_ids = CAM0_LOCAL_NR_CLK,
};

/* Register Offset definitions for CMU_CAM0 (0x144D0000) */
#define CLK_CON_MUX_ACLK_CAM0_CSIS0_414_USER 0x0200
#define CLK_CON_MUX_ACLK_CAM0_CSIS1_168_USER 0x0204
#define CLK_CON_MUX_ACLK_CAM0_CSIS2_234_USER 0x0208
#define CLK_CON_MUX_ACLK_CAM0_CSIS3_132_USER 0x020C
#define CLK_CON_MUX_ACLK_CAM0_3AA0_414_USER 0x0214
#define CLK_CON_MUX_ACLK_CAM0_3AA1_414_USER 0x0218
#define CLK_CON_MUX_ACLK_CAM0_TREX_528_USER 0x021C
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS0_USER 0x0220
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS1_CSIS0_USER 0x0224
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS2_CSIS0_USER 0x0228
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS3_CSIS0_USER 0x022C
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS1_USER 0x0230
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS1_CSIS1_USER 0x0234
#define CLK_CON_DIV_PCLK_CAM0_CSIS0_207 0x0400
#define CLK_CON_DIV_PCLK_CAM0_3AA0_207 0x040C
#define CLK_CON_DIV_PCLK_CAM0_3AA1_207 0x0410
#define CLK_CON_DIV_PCLK_CAM0_TREX_264 0x0414
#define CLK_CON_DIV_PCLK_CAM0_TREX_132 0x0418
#define CLK_STAT_MUX_ACLK_CAM0_CSIS0_414_USER 0x0600
#define CLK_STAT_MUX_ACLK_CAM0_CSIS1_168_USER 0x0604
#define CLK_STAT_MUX_ACLK_CAM0_CSIS2_234_USER 0x0608
#define CLK_STAT_MUX_ACLK_CAM0_CSIS3_132_USER 0x060C
#define CLK_STAT_MUX_ACLK_CAM0_3AA0_414_USER 0x0614
#define CLK_STAT_MUX_ACLK_CAM0_3AA1_414_USER 0x0618
#define CLK_STAT_MUX_ACLK_CAM0_TREX_528_USER 0x061C
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS0_CSIS0_USER 0x0620
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS1_CSIS0_USER 0x0624
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS2_CSIS0_USER 0x0628
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS3_CSIS0_USER 0x062C
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS0_CSIS1_USER 0x0630
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS1_CSIS1_USER 0x0634
#define CLK_ENABLE_ACLK_CAM0_CSIS0_414 0x0800
#define CLK_ENABLE_PCLK_CAM0_CSIS0_207 0x0804
#define CLK_ENABLE_ACLK_CAM0_CSIS1_168_CAM0 0x080C
#define CLK_ENABLE_ACLK_CAM0_CSIS2_234_CAM0 0x0818
#define CLK_ENABLE_ACLK_CAM0_CSIS3_132_CAM0 0x081C
#define CLK_ENABLE_ACLK_CAM0_3AA0_414_CAM0 0x0828
#define CLK_ENABLE_PCLK_CAM0_3AA0_207 0x082C
#define CLK_ENABLE_ACLK_CAM0_3AA1_414_CAM0 0x0830
#define CLK_ENABLE_PCLK_CAM0_3AA1_207 0x0834
#define CLK_ENABLE_ACLK_CAM0_TREX_528_CAM0 0x0838
#define CLK_ENABLE_PCLK_CAM0_TREX_264 0x083C
#define CLK_ENABLE_PCLK_CAM0_TREX_132 0x0840
#define CLK_ENABLE_SCLK_PROMISE_CAM0 0x0844
#define CLK_ENABLE_PHYCLK_HS0_CSIS0_RX_BYTE 0x0848
#define CLK_ENABLE_PHYCLK_HS1_CSIS0_RX_BYTE 0x084C
#define CLK_ENABLE_PHYCLK_HS2_CSIS0_RX_BYTE 0x0850
#define CLK_ENABLE_PHYCLK_HS3_CSIS0_RX_BYTE 0x0854
#define CLK_ENABLE_PHYCLK_HS0_CSIS1_RX_BYTE 0x0858
#define CLK_ENABLE_PHYCLK_HS1_CSIS1_RX_BYTE 0x085C
#define CLK_ENABLE_PCLK_HPM_APBIF_CAM0 0x0860
#define CLKOUT_CMU_CAM0 0x0D00
#define CLKOUT_CMU_CAM0_DIV_STAT 0x0D04
#define CMU_CAM0_SPARE0 0x0D08
#define CMU_CAM0_SPARE1 0x0D0C
#define CLK_ENABLE_PDN_CAM0 0x0E00
#define CAM0_SFR_IGNORE_REQ_SYSCLK 0x0F28

PNAME(cam0_mux_aclk_cam0_csis0_414_user_p) = { "oscclk",
					       "top_gate_aclk_cam0_csis0_414" };
PNAME(cam0_mux_aclk_cam0_csis1_168_user_p) = { "oscclk",
					       "top_gate_aclk_cam0_csis1_168" };
PNAME(cam0_mux_aclk_cam0_csis2_234_user_p) = { "oscclk",
					       "top_gate_aclk_cam0_csis2_234" };
PNAME(cam0_mux_aclk_cam0_csis3_132_user_p) = { "oscclk",
					       "top_gate_aclk_cam0_csis3_132" };
PNAME(cam0_mux_aclk_cam0_3aa0_414_user_p) = { "oscclk",
					      "top_gate_aclk_cam0_3aa0_414" };
PNAME(cam0_mux_aclk_cam0_3aa1_414_user_p) = { "oscclk",
					      "top_gate_aclk_cam0_3aa1_414" };
PNAME(cam0_mux_aclk_cam0_trex_528_user_p) = { "oscclk",
					      "top_gate_aclk_cam0_trex_528" };
PNAME(cam0_mux_phyclk_rxbyteclkhs0_csis0_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs0_csis0"
};
PNAME(cam0_mux_phyclk_rxbyteclkhs1_csis0_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs1_csis0"
};
PNAME(cam0_mux_phyclk_rxbyteclkhs2_csis0_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs2_csis0"
};
PNAME(cam0_mux_phyclk_rxbyteclkhs3_csis0_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs3_csis0"
};
PNAME(cam0_mux_phyclk_rxbyteclkhs0_csis1_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs0_csis1"
};
PNAME(cam0_mux_phyclk_rxbyteclkhs1_csis1_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs1_csis1"
};

static const struct samsung_fixed_rate_clock cam0_fixed_clks[] __initconst = {
	FRATE(0, "phyclk_rxbyteclkhs0_csis0", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs1_csis0", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs2_csis0", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs3_csis0", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs0_csis1", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs1_csis1", NULL, 0, 250000000),
};

static const struct samsung_mux_clock cam0_mux_clks[] __initconst = {
	MUX(0, "cam0_mux_aclk_cam0_csis0_414_user",
	    cam0_mux_aclk_cam0_csis0_414_user_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS0_414_USER, 12, 1),
	MUX(0, "cam0_mux_aclk_cam0_csis1_168_user",
	    cam0_mux_aclk_cam0_csis1_168_user_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS1_168_USER, 12, 1),
	MUX(0, "cam0_mux_aclk_cam0_csis2_234_user",
	    cam0_mux_aclk_cam0_csis2_234_user_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS2_234_USER, 12, 1),
	MUX(0, "cam0_mux_aclk_cam0_csis3_132_user",
	    cam0_mux_aclk_cam0_csis3_132_user_p,
	    CLK_CON_MUX_ACLK_CAM0_CSIS3_132_USER, 12, 1),
	MUX(0, "cam0_mux_aclk_cam0_3aa0_414_user",
	    cam0_mux_aclk_cam0_3aa0_414_user_p,
	    CLK_CON_MUX_ACLK_CAM0_3AA0_414_USER, 12, 1),
	MUX(0, "cam0_mux_aclk_cam0_3aa1_414_user",
	    cam0_mux_aclk_cam0_3aa1_414_user_p,
	    CLK_CON_MUX_ACLK_CAM0_3AA1_414_USER, 12, 1),
	MUX(0, "cam0_mux_aclk_cam0_trex_528_user",
	    cam0_mux_aclk_cam0_trex_528_user_p,
	    CLK_CON_MUX_ACLK_CAM0_TREX_528_USER, 12, 1),
	MUX(0, "cam0_mux_phyclk_rxbyteclkhs0_csis0_user",
	    cam0_mux_phyclk_rxbyteclkhs0_csis0_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS0_USER, 12, 1),
	MUX(0, "cam0_mux_phyclk_rxbyteclkhs1_csis0_user",
	    cam0_mux_phyclk_rxbyteclkhs1_csis0_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS1_CSIS0_USER, 12, 1),
	MUX(0, "cam0_mux_phyclk_rxbyteclkhs2_csis0_user",
	    cam0_mux_phyclk_rxbyteclkhs2_csis0_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS2_CSIS0_USER, 12, 1),
	MUX(0, "cam0_mux_phyclk_rxbyteclkhs3_csis0_user",
	    cam0_mux_phyclk_rxbyteclkhs3_csis0_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS3_CSIS0_USER, 12, 1),
	MUX(0, "cam0_mux_phyclk_rxbyteclkhs0_csis1_user",
	    cam0_mux_phyclk_rxbyteclkhs0_csis1_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS1_USER, 12, 1),
	MUX(0, "cam0_mux_phyclk_rxbyteclkhs1_csis1_user",
	    cam0_mux_phyclk_rxbyteclkhs1_csis1_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS1_CSIS1_USER, 12, 1),
};

static const struct samsung_gate_clock cam0_gate_clks[] __initconst = {
	GATE(0, "cam0_gate_aclk_bns", "cam0_mux_aclk_cam0_csis0_414_user",
	     CLK_ENABLE_ACLK_CAM0_CSIS0_414, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_pxl_asbs_csis2_int",
	     "cam0_mux_aclk_cam0_csis0_414_user",
	     CLK_ENABLE_ACLK_CAM0_CSIS0_414, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_csis0", "cam0_mux_aclk_cam0_csis0_414_user",
	     CLK_ENABLE_ACLK_CAM0_CSIS0_414, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_bns", "cam0_div_pclk_cam0_csis0_207",
	     CLK_ENABLE_PCLK_CAM0_CSIS0_207, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_csis1", "cam0_mux_aclk_cam0_csis1_168_user",
	     CLK_ENABLE_ACLK_CAM0_CSIS1_168_CAM0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_csis2", "cam0_mux_aclk_cam0_csis2_234_user",
	     CLK_ENABLE_ACLK_CAM0_CSIS2_234_CAM0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_csis3", "cam0_mux_aclk_cam0_csis3_132_user",
	     CLK_ENABLE_ACLK_CAM0_CSIS3_132_CAM0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_3aa0", "cam0_mux_aclk_cam0_3aa0_414_user",
	     CLK_ENABLE_ACLK_CAM0_3AA0_414_CAM0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_3aa0", "cam0_div_pclk_cam0_3aa0_207",
	     CLK_ENABLE_PCLK_CAM0_3AA0_207, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_3aa1", "cam0_mux_aclk_cam0_3aa1_414_user",
	     CLK_ENABLE_ACLK_CAM0_3AA1_414_CAM0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_3aa1", "cam0_div_pclk_cam0_3aa1_207",
	     CLK_ENABLE_PCLK_CAM0_3AA1_207, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_sfw110_is_a",
	     "cam0_mux_aclk_cam0_trex_528_user",
	     CLK_ENABLE_ACLK_CAM0_TREX_528_CAM0, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_sysmmu6_is_a",
	     "cam0_mux_aclk_cam0_trex_528_user",
	     CLK_ENABLE_ACLK_CAM0_TREX_528_CAM0, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_trex_a_5x1_is_a",
	     "cam0_mux_aclk_cam0_trex_528_user",
	     CLK_ENABLE_ACLK_CAM0_TREX_528_CAM0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_lh_async_si_cam0",
	     "cam0_mux_aclk_cam0_trex_528_user",
	     CLK_ENABLE_ACLK_CAM0_TREX_528_CAM0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_pmu_cam0", "cam0_div_pclk_cam0_trex_264",
	     CLK_ENABLE_PCLK_CAM0_TREX_264, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_sysreg_cam0", "cam0_div_pclk_cam0_trex_264",
	     CLK_ENABLE_PCLK_CAM0_TREX_264, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_lh_async_mi_cam0",
	     "cam0_div_pclk_cam0_trex_264", CLK_ENABLE_PCLK_CAM0_TREX_264, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_aclk_xiuasync_mi_cam0",
	     "cam0_div_pclk_cam0_trex_264", CLK_ENABLE_PCLK_CAM0_TREX_264, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_cam0", "cam0_div_pclk_cam0_trex_264",
	     CLK_ENABLE_PCLK_CAM0_TREX_264, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_csis1", "cam0_div_pclk_cam0_trex_264",
	     CLK_ENABLE_PCLK_CAM0_TREX_264, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_csis0", "cam0_div_pclk_cam0_trex_264",
	     CLK_ENABLE_PCLK_CAM0_TREX_264, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_xiuasync_mi_cam0",
	     "cam0_div_pclk_cam0_trex_132", CLK_ENABLE_PCLK_CAM0_TREX_132, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_trex_a_5x1_is_a", "cam0_div_pclk_cam0_trex_264",
	     CLK_ENABLE_PCLK_CAM0_TREX_264, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_sysmmu6_is_a", "cam0_div_pclk_cam0_trex_132",
	     CLK_ENABLE_PCLK_CAM0_TREX_132, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_sfw110_is_a_is_a",
	     "cam0_div_pclk_cam0_trex_132", CLK_ENABLE_PCLK_CAM0_TREX_132, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_sclk_promise_cam0", "top_gate_sclk_promise_disp",
	     CLK_ENABLE_SCLK_PROMISE_CAM0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_phyclk_hs0_csis0_rx_byte",
	     "cam0_mux_phyclk_rxbyteclkhs0_csis0_user",
	     CLK_ENABLE_PHYCLK_HS0_CSIS0_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_phyclk_hs1_csis0_rx_byte",
	     "cam0_mux_phyclk_rxbyteclkhs1_csis0_user",
	     CLK_ENABLE_PHYCLK_HS1_CSIS0_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_phyclk_hs2_csis0_rx_byte",
	     "cam0_mux_phyclk_rxbyteclkhs2_csis0_user",
	     CLK_ENABLE_PHYCLK_HS2_CSIS0_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_phyclk_hs3_csis0_rx_byte",
	     "cam0_mux_phyclk_rxbyteclkhs3_csis0_user",
	     CLK_ENABLE_PHYCLK_HS3_CSIS0_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_phyclk_hs0_csis1_rx_byte",
	     "cam0_mux_phyclk_rxbyteclkhs0_csis1_user",
	     CLK_ENABLE_PHYCLK_HS0_CSIS1_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_phyclk_hs1_csis1_rx_byte",
	     "cam0_mux_phyclk_rxbyteclkhs1_csis1_user",
	     CLK_ENABLE_PHYCLK_HS1_CSIS1_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam0_gate_pclk_hpm_apbif_cam0", "cam0_div_pclk_cam0_trex_264",
	     CLK_ENABLE_PCLK_HPM_APBIF_CAM0, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock cam0_div_clks[] __initconst = {
	DIV(0, "cam0_div_pclk_cam0_csis0_207",
	    "cam0_mux_aclk_cam0_csis0_414_user",
	    CLK_CON_DIV_PCLK_CAM0_CSIS0_207, 0, 3),
	DIV(0, "cam0_div_pclk_cam0_3aa0_207",
	    "cam0_mux_aclk_cam0_3aa0_414_user", CLK_CON_DIV_PCLK_CAM0_3AA0_207,
	    0, 3),
	DIV(0, "cam0_div_pclk_cam0_3aa1_207",
	    "cam0_mux_aclk_cam0_3aa1_414_user", CLK_CON_DIV_PCLK_CAM0_3AA1_207,
	    0, 3),
	DIV(0, "cam0_div_pclk_cam0_trex_264",
	    "cam0_mux_aclk_cam0_trex_528_user", CLK_CON_DIV_PCLK_CAM0_TREX_264,
	    0, 3),
	DIV(0, "cam0_div_pclk_cam0_trex_132",
	    "cam0_mux_aclk_cam0_trex_528_user", CLK_CON_DIV_PCLK_CAM0_TREX_132,
	    0, 3),
};

static const struct samsung_cmu_info cam0_cmu_info __initconst = {
	.mux_clks = cam0_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(cam0_mux_clks),
	.div_clks = cam0_div_clks,
	.nr_div_clks = ARRAY_SIZE(cam0_div_clks),
	.gate_clks = cam0_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(cam0_gate_clks),
	.fixed_clks = cam0_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(cam0_fixed_clks),
	.nr_clk_ids = CAM0_NR_CLK,
};

/* Register Offset definitions for CMU_CAM1_LOCAL (0x141F0000) */
#define CLK_ENABLE_ACLK_CAM1_TREX_VRA_528_LOCAL 0x0808
#define CLK_ENABLE_PCLK_CAM1_TREX_VRA_264_LOCAL 0x080C
#define CLK_ENABLE_ACLK_CAM1_BUS_264_LOCAL 0x0814
#define CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL 0x081C
#define CLK_ENABLE_ACLK_CAM1_CSIS2_414_LOCAL 0x0820
#define CLK_ENABLE_ACLK_CAM1_CSIS3_132_LOCAL 0x0828
#define CLK_ENABLE_ACLK_CAM1_SCL_566_LOCAL 0x0830

static const struct samsung_gate_clock cam1_local_gate_clks[] __initconst = {
	GATE(0, "cam1_local_gate_aclk_vra", "cam1_gate_aclk_vra",
	     CLK_ENABLE_ACLK_CAM1_TREX_VRA_528_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_vra", "cam1_gate_pclk_vra",
	     CLK_ENABLE_PCLK_CAM1_TREX_VRA_264_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_csis3", "cam1_gate_pclk_csis3",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_LOCAL, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_csis2", "cam1_gate_pclk_csis2",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_LOCAL, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_wdt", "cam1_gate_pclk_wdt",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_uart", "cam1_gate_pclk_uart",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_spi1", "cam1_gate_pclk_spi1",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_spi0", "cam1_gate_pclk_spi0",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_pwm", "cam1_gate_pclk_pwm",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_mcuctl", "cam1_gate_pclk_mcuctl",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_i2c3", "cam1_gate_pclk_i2c3",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_i2c2", "cam1_gate_pclk_i2c2",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_i2c1", "cam1_gate_pclk_i2c1",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_pclk_i2c0", "cam1_gate_pclk_i2c0",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_aclk_pdma", "cam1_gate_aclk_pdma",
	     CLK_ENABLE_PCLK_CAM1_PERI_84_LOCAL, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_aclk_csis2", "cam1_gate_aclk_csis2",
	     CLK_ENABLE_ACLK_CAM1_CSIS2_414_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_aclk_csis3", "cam1_gate_aclk_csis3",
	     CLK_ENABLE_ACLK_CAM1_CSIS3_132_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_local_gate_aclk_mc_sc", "cam1_gate_aclk_mc_sc",
	     CLK_ENABLE_ACLK_CAM1_SCL_566_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info cam1_local_cmu_info __initconst = {
	.gate_clks = cam1_local_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(cam1_local_gate_clks),
	.nr_clk_ids = CAM1_LOCAL_NR_CLK,
};

/* Register Offset definitions for CMU_CAM1 (0x145D0000) */
#define CLK_CON_MUX_ACLK_CAM1_ARM_672_USER 0x0200
#define CLK_CON_MUX_ACLK_CAM1_TREX_VRA_528_USER 0x0204
#define CLK_CON_MUX_ACLK_CAM1_TREX_B_528_USER 0x0208
#define CLK_CON_MUX_ACLK_CAM1_BUS_264_USER 0x020C
#define CLK_CON_MUX_ACLK_CAM1_PERI_84_USER 0x0210
#define CLK_CON_MUX_ACLK_CAM1_CSIS2_414_USER 0x0214
#define CLK_CON_MUX_ACLK_CAM1_CSIS3_132_USER 0x0218
#define CLK_CON_MUX_ACLK_CAM1_SCL_566_USER 0x021C
#define CLK_CON_MUX_SCLK_CAM1_ISP_SPI0_USER 0x0220
#define CLK_CON_MUX_SCLK_CAM1_ISP_SPI1_USER 0x0224
#define CLK_CON_MUX_SCLK_CAM1_ISP_UART_USER 0x0228
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS2_USER 0x022C
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS1_CSIS2_USER 0x0230
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS2_CSIS2_USER 0x0234
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS3_CSIS2_USER 0x0238
#define CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS3_USER 0x023C
#define CLK_CON_DIV_PCLK_CAM1_ARM_168 0x0400
#define CLK_CON_DIV_PCLK_CAM1_TREX_VRA_264 0x0408
#define CLK_CON_DIV_PCLK_CAM1_BUS_132 0x040C
#define CLK_CON_DIV_PCLK_CAM1_SCL_283 0x0418
#define CLK_STAT_MUX_ACLK_CAM1_ARM_672_USER 0x0600
#define CLK_STAT_MUX_ACLK_CAM1_TREX_VRA_528_USER 0x0604
#define CLK_STAT_MUX_ACLK_CAM1_TREX_B_528_USER 0x0608
#define CLK_STAT_MUX_ACLK_CAM1_BUS_264_USER 0x060C
#define CLK_STAT_MUX_ACLK_CAM1_PERI_84_USER 0x0610
#define CLK_STAT_MUX_ACLK_CAM1_CSIS2_414_USER 0x0614
#define CLK_STAT_MUX_ACLK_CAM1_CSIS3_132_USER 0x0618
#define CLK_STAT_MUX_ACLK_CAM1_SCL_566_USER 0x061C
#define CLK_STAT_MUX_SCLK_CAM1_ISP_SPI0_USER 0x0620
#define CLK_STAT_MUX_SCLK_CAM1_ISP_SPI1_USER 0x0624
#define CLK_STAT_MUX_SCLK_CAM1_ISP_UART_USER 0x0628
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS0_CSIS2_USER 0x062C
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS1_CSIS2_USER 0x0630
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS2_CSIS2_USER 0x0634
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS3_CSIS2_USER 0x0638
#define CLK_STAT_MUX_PHYCLK_RXBYTECLKHS0_CSIS3_USER 0x063C
#define CLK_ENABLE_ACLK_CAM1_ARM_672_CAM1 0x0800
#define CLK_ENABLE_PCLK_CAM1_ARM_168 0x0804
#define CLK_ENABLE_ACLK_CAM1_TREX_VRA_528_CAM1 0x0808
#define CLK_ENABLE_PCLK_CAM1_TREX_VRA_264 0x080C
#define CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1 0x0810
#define CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1 0x0814
#define CLK_ENABLE_PCLK_CAM1_BUS_132 0x0818
#define CLK_ENABLE_PCLK_CAM1_PERI_84 0x081C
#define CLK_ENABLE_ACLK_CAM1_CSIS2_414_CAM1 0x0820
#define CLK_ENABLE_ACLK_CAM1_CSIS3_132_CAM1 0x0828
#define CLK_ENABLE_ACLK_CAM1_SCL_566_CAM1 0x0830
#define CLK_ENABLE_PCLK_CAM1_SCL_283 0x083C
#define CLK_ENABLE_PCLK_CAM1_MCS_132 0x083C
#define CLK_ENABLE_SCLK_CAM1_ISP_SPI0_CAM1 0x0840
#define CLK_ENABLE_SCLK_CAM1_ISP_SPI1_CAM1 0x0844
#define CLK_ENABLE_SCLK_CAM1_ISP_UART_CAM1 0x0848
#define CLK_ENABLE_SCLK_ISP_PERI_IS_B 0x084C
#define CLK_ENABLE_PHYCLK_HS0_CSIS2_RX_BYTE 0x0850
#define CLK_ENABLE_PHYCLK_HS1_CSIS2_RX_BYTE 0x0854
#define CLK_ENABLE_PHYCLK_HS2_CSIS2_RX_BYTE 0x0858
#define CLK_ENABLE_PHYCLK_HS3_CSIS2_RX_BYTE 0x085C
#define CLK_ENABLE_PHYCLK_HS0_CSIS3_RX_BYTE 0x0860
#define CLKOUT_CMU_CAM1 0x0D00
#define CLKOUT_CMU_CAM1_DIV_STAT 0x0D04
#define CMU_CAM1_SPARE0 0x0D08
#define CMU_CAM1_SPARE1 0x0D0C
#define CLK_ENABLE_PDN_CAM1 0x0E00
#define CAM1_SFR_IGNORE_REQ_SYSCLK 0x0F28

PNAME(cam1_mux_aclk_cam1_arm_672_user_p) = { "oscclk",
					     "top_gate_aclk_cam1_arm_672" };
PNAME(cam1_mux_aclk_cam1_trex_vra_528_user_p) = {
	"oscclk", "top_gate_aclk_cam1_trex_vra_528"
};
PNAME(cam1_mux_aclk_cam1_trex_b_528_user_p) = {
	"oscclk", "top_gate_aclk_cam1_trex_b_528"
};
PNAME(cam1_mux_aclk_cam1_bus_264_user_p) = { "oscclk",
					     "top_gate_aclk_cam1_bus_264" };
PNAME(cam1_mux_aclk_cam1_peri_84_user_p) = { "oscclk",
					     "top_gate_aclk_cam1_peri_84" };
PNAME(cam1_mux_aclk_cam1_csis2_414_user_p) = { "oscclk",
					       "top_gate_aclk_cam1_csis2_414" };
PNAME(cam1_mux_aclk_cam1_csis3_132_user_p) = { "oscclk",
					       "top_gate_aclk_cam1_csis3_132" };
PNAME(cam1_mux_aclk_cam1_scl_566_user_p) = { "oscclk",
					     "top_gate_aclk_cam1_scl_566" };
PNAME(cam1_mux_sclk_cam1_isp_spi0_user_p) = { "oscclk",
					      "top_gate_sclk_cam1_isp_spi0" };
PNAME(cam1_mux_sclk_cam1_isp_spi1_user_p) = { "oscclk",
					      "top_gate_sclk_cam1_isp_spi1" };
PNAME(cam1_mux_sclk_cam1_isp_uart_user_p) = { "oscclk",
					      "top_gate_sclk_cam1_isp_uart" };
PNAME(cam1_mux_phyclk_rxbyteclkhs0_csis2_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs0_csis2"
};
PNAME(cam1_mux_phyclk_rxbyteclkhs1_csis2_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs1_csis2"
};
PNAME(cam1_mux_phyclk_rxbyteclkhs2_csis2_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs2_csis2"
};
PNAME(cam1_mux_phyclk_rxbyteclkhs3_csis2_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs3_csis2"
};
PNAME(cam1_mux_phyclk_rxbyteclkhs0_csis3_user_p) = {
	"oscclk", "phyclk_rxbyteclkhs0_csis3"
};

static const struct samsung_fixed_rate_clock cam1_fixed_clks[] __initconst = {
	FRATE(0, "phyclk_rxbyteclkhs0_csis2", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs1_csis2", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs2_csis2", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs3_csis2", NULL, 0, 250000000),
	FRATE(0, "phyclk_rxbyteclkhs0_csis3", NULL, 0, 250000000),
};

static const struct samsung_mux_clock cam1_mux_clks[] __initconst = {
	MUX(0, "cam1_mux_aclk_cam1_arm_672_user",
	    cam1_mux_aclk_cam1_arm_672_user_p,
	    CLK_CON_MUX_ACLK_CAM1_ARM_672_USER, 12, 1),
	MUX(0, "cam1_mux_aclk_cam1_trex_vra_528_user",
	    cam1_mux_aclk_cam1_trex_vra_528_user_p,
	    CLK_CON_MUX_ACLK_CAM1_TREX_VRA_528_USER, 12, 1),
	MUX(0, "cam1_mux_aclk_cam1_trex_b_528_user",
	    cam1_mux_aclk_cam1_trex_b_528_user_p,
	    CLK_CON_MUX_ACLK_CAM1_TREX_B_528_USER, 12, 1),
	MUX(0, "cam1_mux_aclk_cam1_bus_264_user",
	    cam1_mux_aclk_cam1_bus_264_user_p,
	    CLK_CON_MUX_ACLK_CAM1_BUS_264_USER, 12, 1),
	MUX(0, "cam1_mux_aclk_cam1_peri_84_user",
	    cam1_mux_aclk_cam1_peri_84_user_p,
	    CLK_CON_MUX_ACLK_CAM1_PERI_84_USER, 12, 1),
	MUX(0, "cam1_mux_aclk_cam1_csis2_414_user",
	    cam1_mux_aclk_cam1_csis2_414_user_p,
	    CLK_CON_MUX_ACLK_CAM1_CSIS2_414_USER, 12, 1),
	MUX(0, "cam1_mux_aclk_cam1_csis3_132_user",
	    cam1_mux_aclk_cam1_csis3_132_user_p,
	    CLK_CON_MUX_ACLK_CAM1_CSIS3_132_USER, 12, 1),
	MUX(0, "cam1_mux_aclk_cam1_scl_566_user",
	    cam1_mux_aclk_cam1_scl_566_user_p,
	    CLK_CON_MUX_ACLK_CAM1_SCL_566_USER, 12, 1),
	MUX(0, "cam1_mux_sclk_cam1_isp_spi0_user",
	    cam1_mux_sclk_cam1_isp_spi0_user_p,
	    CLK_CON_MUX_SCLK_CAM1_ISP_SPI0_USER, 12, 1),
	MUX(0, "cam1_mux_sclk_cam1_isp_spi1_user",
	    cam1_mux_sclk_cam1_isp_spi1_user_p,
	    CLK_CON_MUX_SCLK_CAM1_ISP_SPI1_USER, 12, 1),
	MUX(0, "cam1_mux_sclk_cam1_isp_uart_user",
	    cam1_mux_sclk_cam1_isp_uart_user_p,
	    CLK_CON_MUX_SCLK_CAM1_ISP_UART_USER, 12, 1),
	MUX(0, "cam1_mux_phyclk_rxbyteclkhs0_csis2_user",
	    cam1_mux_phyclk_rxbyteclkhs0_csis2_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS2_USER, 12, 1),
	MUX(0, "cam1_mux_phyclk_rxbyteclkhs1_csis2_user",
	    cam1_mux_phyclk_rxbyteclkhs1_csis2_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS1_CSIS2_USER, 12, 1),
	MUX(0, "cam1_mux_phyclk_rxbyteclkhs2_csis2_user",
	    cam1_mux_phyclk_rxbyteclkhs2_csis2_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS2_CSIS2_USER, 12, 1),
	MUX(0, "cam1_mux_phyclk_rxbyteclkhs3_csis2_user",
	    cam1_mux_phyclk_rxbyteclkhs3_csis2_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS3_CSIS2_USER, 12, 1),
	MUX(0, "cam1_mux_phyclk_rxbyteclkhs0_csis3_user",
	    cam1_mux_phyclk_rxbyteclkhs0_csis3_user_p,
	    CLK_CON_MUX_PHYCLK_RXBYTECLKHS0_CSIS3_USER, 12, 1),
};

static const struct samsung_gate_clock cam1_gate_clks[] __initconst = {
	GATE(0, "cam1_gate_aclk_arm", "cam1_mux_aclk_cam1_arm_672_user",
	     CLK_ENABLE_ACLK_CAM1_ARM_672_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_arm", "cam1_div_pclk_cam1_arm_168",
	     CLK_ENABLE_PCLK_CAM1_ARM_168, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_smmu_vra",
	     "cam1_mux_aclk_cam1_trex_vra_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_VRA_528_CAM1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_vra", "cam1_mux_aclk_cam1_trex_vra_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_VRA_528_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_vra", "cam1_div_pclk_cam1_trex_vra_264",
	     CLK_ENABLE_PCLK_CAM1_TREX_VRA_264, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_lh_si", "cam1_mux_aclk_cam1_trex_b_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_trex_cam1",
	     "cam1_mux_aclk_cam1_trex_b_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_xiu_from_isp1",
	     "cam1_mux_aclk_cam1_trex_b_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_smmu_is_b",
	     "cam1_mux_aclk_cam1_trex_b_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "CAM1_GATE_ACLK_SFW", "cam1_mux_aclk_cam1_trex_b_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_async_ca7_to_dram",
	     "cam1_mux_aclk_cam1_trex_b_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_smmu_ispcpu",
	     "cam1_mux_aclk_cam1_trex_b_528_user",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_trex_b", "CAM1_MUX_ACLK_CAM1_TREX_B_528_USER",
	     CLK_ENABLE_ACLK_CAM1_TREX_B_528_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_lh_mi", "cam1_mux_aclk_cam1_bus_264_user",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_peri", "cam1_mux_aclk_cam1_bus_264_user",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_csis3", "cam1_mux_aclk_cam1_bus_264_user",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_csis2", "cam1_mux_aclk_cam1_bus_264_user",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_xiu_to_cam0", "cam1_mux_aclk_cam1_bus_264_user",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_xiu_to_isp1", "cam1_mux_aclk_cam1_bus_264_user",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_xiu_to_isp0", "cam1_mux_aclk_cam1_bus_264_user",
	     CLK_ENABLE_ACLK_CAM1_BUS_264_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_cmu_local", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_sysreg_cam1", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_pmu_cam1", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_trex_cam1", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_xiu_from_isp1", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_peri", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_smmu_ispcpu", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_smmu_vra", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_smmu_is_b", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_sfw", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_trex_b", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_BUS_132, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_wdt", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_uart", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_spi1", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_spi0", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_pwm", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_mcuctl", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_i2c3", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_i2c2", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_i2c1", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_i2c0", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_pdma", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_bridge_peri", "cam1_mux_aclk_cam1_peri_84_user",
	     CLK_ENABLE_PCLK_CAM1_PERI_84, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_csis2", "cam1_mux_aclk_cam1_csis2_414_user",
	     CLK_ENABLE_ACLK_CAM1_CSIS2_414_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_csis3", "cam1_mux_aclk_cam1_csis3_132_user",
	     CLK_ENABLE_ACLK_CAM1_CSIS3_132_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_smmu_mc_sc", "cam1_mux_aclk_cam1_scl_566_user",
	     CLK_ENABLE_ACLK_CAM1_SCL_566_CAM1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_aclk_mc_sc", "cam1_mux_aclk_cam1_scl_566_user",
	     CLK_ENABLE_ACLK_CAM1_SCL_566_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_pclk_smmu_mc_sc", "cam1_div_pclk_cam1_bus_132",
	     CLK_ENABLE_PCLK_CAM1_MCS_132, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_isp_peri_is_b_spi0_ext_clk_isp",
	     "cam1_mux_sclk_cam1_isp_spi0_user",
	     CLK_ENABLE_SCLK_CAM1_ISP_SPI0_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_isp_peri_is_b_spi1_ext_clk_isp",
	     "cam1_mux_sclk_cam1_isp_spi1_user",
	     CLK_ENABLE_SCLK_CAM1_ISP_SPI1_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_isp_peri_is_b_uart_ext_clk_isp",
	     "cam1_mux_sclk_cam1_isp_uart_user",
	     CLK_ENABLE_SCLK_CAM1_ISP_UART_CAM1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_cam1_isp_is_b_oscclk_i2c3_isp", "oscclk",
	     CLK_ENABLE_SCLK_ISP_PERI_IS_B, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_cam1_isp_is_b_oscclk_i2c2_isp", "oscclk",
	     CLK_ENABLE_SCLK_ISP_PERI_IS_B, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_cam1_isp_is_b_oscclk_i2c1_isp", "oscclk",
	     CLK_ENABLE_SCLK_ISP_PERI_IS_B, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_cam1_isp_is_b_oscclk_i2c0_isp", "oscclk",
	     CLK_ENABLE_SCLK_ISP_PERI_IS_B, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_sclk_isp_peri_is_b_pwm_isp", "oscclk",
	     CLK_ENABLE_SCLK_ISP_PERI_IS_B, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_phyclk_hs0_csis2_rx_byte",
	     "cam1_mux_phyclk_rxbyteclkhs0_csis2_user",
	     CLK_ENABLE_PHYCLK_HS0_CSIS2_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_phyclk_hs1_csis2_rx_byte",
	     "cam1_mux_phyclk_rxbyteclkhs1_csis2_user",
	     CLK_ENABLE_PHYCLK_HS1_CSIS2_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_phyclk_hs2_csis2_rx_byte",
	     "cam1_mux_phyclk_rxbyteclkhs2_csis2_user",
	     CLK_ENABLE_PHYCLK_HS2_CSIS2_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_phyclk_hs3_csis2_rx_byte",
	     "cam1_mux_phyclk_rxbyteclkhs3_csis2_user",
	     CLK_ENABLE_PHYCLK_HS3_CSIS2_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "cam1_gate_phyclk_hs0_csis3_rx_byte",
	     "cam1_mux_phyclk_rxbyteclkhs0_csis3_user",
	     CLK_ENABLE_PHYCLK_HS0_CSIS3_RX_BYTE, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock cam1_div_clks[] __initconst = {
	DIV(0, "cam1_div_pclk_cam1_arm_168", "cam1_mux_aclk_cam1_arm_672_user",
	    CLK_CON_DIV_PCLK_CAM1_ARM_168, 0, 3),
	DIV(0, "cam1_div_pclk_cam1_trex_vra_264",
	    "cam1_mux_aclk_cam1_trex_vra_528_user",
	    CLK_CON_DIV_PCLK_CAM1_TREX_VRA_264, 0, 3),
	DIV(0, "cam1_div_pclk_cam1_bus_132", "cam1_mux_aclk_cam1_bus_264_user",
	    CLK_CON_DIV_PCLK_CAM1_BUS_132, 0, 3),
};

static const struct samsung_cmu_info cam1_cmu_info __initconst = {
	.mux_clks = cam1_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(cam1_mux_clks),
	.div_clks = cam1_div_clks,
	.nr_div_clks = ARRAY_SIZE(cam1_div_clks),
	.gate_clks = cam1_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(cam1_gate_clks),
	.fixed_clks = cam1_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(cam1_fixed_clks),
	.nr_clk_ids = CAM1_NR_CLK,
};

/* Register Offset definitions for CMU_ISP0_LOCAL (0x14290000) */
#define CLK_ENABLE_ACLK_ISP0_LOCAL 0x0800
#define CLK_ENABLE_PCLK_ISP0_LOCAL 0x0808
#define CLK_ENABLE_ACLK_ISP0_TPU_LOCAL 0x080C
#define CLK_ENABLE_PCLK_ISP0_TPU_LOCAL 0x0814
#define CLK_ENABLE_ACLK_ISP0_TREX_LOCAL 0x0818
#define CLK_ENABLE_PCLK_TREX_132_LOCAL 0x0824
#define CLK_ENABLE_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_LOCAL 0x0828

static const struct samsung_gate_clock isp0_local_gate_clks[] __initconst = {
	GATE(0, "isp0_local_gate_aclk_fimc_isp0", "isp0_gate_aclk_fimc_isp0",
	     CLK_ENABLE_ACLK_ISP0_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_local_gate_pclk_fimc_isp0", "isp0_gate_pclk_fimc_isp0",
	     CLK_ENABLE_PCLK_ISP0_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_local_gate_aclk_fimc_tpu", "isp0_gate_aclk_fimc_tpu",
	     CLK_ENABLE_ACLK_ISP0_TPU_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_local_gate_pclk_fimc_tpu", "isp0_gate_pclk_fimc_tpu",
	     CLK_ENABLE_PCLK_ISP0_TPU_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_local_gate_clk_c_trex_c", "isp0_gate_clk_c_trex_c",
	     CLK_ENABLE_ACLK_ISP0_TREX_LOCAL, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_local_gate_pclk_trex_c", "isp0_gate_pclk_trex_c",
	     CLK_ENABLE_PCLK_TREX_132_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info isp0_local_cmu_info __initconst = {
	.gate_clks = isp0_local_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(isp0_local_gate_clks),
	.nr_clk_ids = ISP0_LOCAL_NR_CLK,
};

/* Register Offset definitions for CMU_ISP0 (0x146D0000) */
#define CLK_CON_MUX_ACLK_ISP0_528_USER 0x0200
#define CLK_CON_MUX_ACLK_ISP0_TPU_400_USER 0x0204
#define CLK_CON_MUX_ACLK_ISP0_TREX_528_USER 0x0208
#define CLK_CON_MUX_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_USER 0x020C
#define CLK_CON_DIV_PCLK_ISP0 0x0400
#define CLK_CON_DIV_PCLK_ISP0_TPU 0x0404
#define CLK_CON_DIV_PCLK_ISP0_TREX_264 0x0408
#define CLK_CON_DIV_PCLK_ISP0_TREX_132 0x040C
#define CLK_STAT_MUX_ACLK_ISP0_528_USER 0x0600
#define CLK_STAT_MUX_ACLK_ISP0_TPU_400_USER 0x0604
#define CLK_STAT_MUX_ACLK_ISP0_TREX_528_USER 0x0608
#define CLK_STAT_MUX_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_USER 0x060C
#define CLK_ENABLE_ACLK_ISP0 0x0800
#define CLK_ENABLE_PCLK_ISP0 0x0808
#define CLK_ENABLE_ACLK_ISP0_TPU 0x080C
#define CLK_ENABLE_PCLK_ISP0_TPU 0x0814
#define CLK_ENABLE_ACLK_ISP0_TREX 0x0818
#define CLK_ENABLE_PCLK_TREX_264 0x081C
#define CLK_ENABLE_PCLK_HPM_APBIF_ISP0 0x0820
#define CLK_ENABLE_PCLK_TREX_132 0x0824
#define CLK_ENABLE_SCLK_PROMISE_ISP0 0x0828
#define CLK_ENABLE_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D 0x082C
#define CLKOUT_CMU_ISP0 0x0D00
#define CLKOUT_CMU_ISP0_DIV_STAT 0x0D04
#define CMU_ISP0_SPARE0 0x0D08
#define CMU_ISP0_SPARE1 0x0D0C
#define CLK_ENABLE_PDN_ISP0 0x0E00
#define ISP0_SFR_IGNORE_REQ_SYSCLK 0x0E04

PNAME(isp0_mux_aclk_isp0_528_user_p) = { "oscclk",
					 "top_gate_aclk_isp0_isp0_528" };
PNAME(isp0_mux_aclk_isp0_tpu_400_user_p) = { "oscclk",
					     "top_gate_aclk_isp0_tpu_400" };
PNAME(isp0_mux_aclk_isp0_trex_528_user_p) = { "oscclk",
					      "top_gate_aclk_isp0_trex_528" };
PNAME(isp0_mux_aclk_isp0_pxl_asbs_is_c_from_is_d_user_p) = {
	"oscclk", "top_gate_aclk_isp0_pxl_asbs_is_c_from_is_d"
};

static const struct samsung_mux_clock isp0_mux_clks[] __initconst = {
	MUX(0, "isp0_mux_aclk_isp0_528_user", isp0_mux_aclk_isp0_528_user_p,
	    CLK_CON_MUX_ACLK_ISP0_528_USER, 12, 1),
	MUX(0, "isp0_mux_aclk_isp0_tpu_400_user",
	    isp0_mux_aclk_isp0_tpu_400_user_p,
	    CLK_CON_MUX_ACLK_ISP0_TPU_400_USER, 12, 1),
	MUX(0, "isp0_mux_aclk_isp0_trex_528_user",
	    isp0_mux_aclk_isp0_trex_528_user_p,
	    CLK_CON_MUX_ACLK_ISP0_TREX_528_USER, 12, 1),
	MUX(0, "isp0_mux_aclk_isp0_pxl_asbs_is_c_from_is_d_user",
	    isp0_mux_aclk_isp0_pxl_asbs_is_c_from_is_d_user_p,
	    CLK_CON_MUX_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D_USER, 12, 1),
};

static const struct samsung_gate_clock isp0_gate_clks[] __initconst = {
	GATE(0, "isp0_gate_aclk_fimc_isp0", "isp0_mux_aclk_isp0_528_user",
	     CLK_ENABLE_ACLK_ISP0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_fimc_isp0", "isp0_div_pclk_isp0",
	     CLK_ENABLE_PCLK_ISP0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_aclk_fimc_tpu", "isp0_mux_aclk_isp0_tpu_400_user",
	     CLK_ENABLE_ACLK_ISP0_TPU, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_fimc_tpu", "isp0_div_pclk_isp0_tpu",
	     CLK_ENABLE_PCLK_ISP0_TPU, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_aclk_sysmmu601", "isp0_mux_aclk_isp0_trex_528_user",
	     CLK_ENABLE_ACLK_ISP0_TREX, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_clk_c_trex_c", "isp0_mux_aclk_isp0_trex_528_user",
	     CLK_ENABLE_ACLK_ISP0_TREX, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_clk_axi_lh_async_si_top_isp0",
	     "isp0_mux_aclk_isp0_trex_528_user", CLK_ENABLE_ACLK_ISP0_TREX, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_sysreg_isp0", "isp0_div_pclk_isp0_trex_264",
	     CLK_ENABLE_PCLK_TREX_264, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_pmu_isp0", "isp0_div_pclk_isp0_trex_264",
	     CLK_ENABLE_PCLK_TREX_264, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_aclk_xiu_n_async_mi", "isp0_div_pclk_isp0_trex_264",
	     CLK_ENABLE_PCLK_TREX_264, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_isp0", "isp0_div_pclk_isp0_trex_264",
	     CLK_ENABLE_PCLK_TREX_264, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_hpm_apbif_isp0", "isp0_div_pclk_isp0_trex_264",
	     CLK_ENABLE_PCLK_HPM_APBIF_ISP0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_sysmmu601", "isp0_div_pclk_isp0_trex_132",
	     CLK_ENABLE_PCLK_TREX_132, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_pclk_trex_c", "isp0_div_pclk_isp0_trex_132",
	     CLK_ENABLE_PCLK_TREX_132, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_sclk_promise_isp0", "top_gate_sclk_promise_int",
	     CLK_ENABLE_SCLK_PROMISE_ISP0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp0_gate_aclk_isp0_pxl_asbs_is_c_from_is_d",
	     "isp0_mux_aclk_isp0_pxl_asbs_is_c_from_is_d_user",
	     CLK_ENABLE_ACLK_ISP0_PXL_ASBS_IS_C_FROM_IS_D, 0, CLK_IGNORE_UNUSED,
	     0),
};

static const struct samsung_div_clock isp0_div_clks[] __initconst = {
	DIV(0, "isp0_div_pclk_isp0", "isp0_mux_aclk_isp0_528_user",
	    CLK_CON_DIV_PCLK_ISP0, 0, 3),
	DIV(0, "isp0_div_pclk_isp0_tpu", "isp0_mux_aclk_isp0_tpu_400_user",
	    CLK_CON_DIV_PCLK_ISP0_TPU, 0, 3),
	DIV(0, "isp0_div_pclk_isp0_trex_264",
	    "isp0_mux_aclk_isp0_trex_528_user", CLK_CON_DIV_PCLK_ISP0_TREX_264,
	    0, 3),
	DIV(0, "isp0_div_pclk_isp0_trex_132",
	    "isp0_mux_aclk_isp0_trex_528_user", CLK_CON_DIV_PCLK_ISP0_TREX_132,
	    0, 3),
};

static const struct samsung_cmu_info isp0_cmu_info __initconst = {
	.mux_clks = isp0_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(isp0_mux_clks),
	.div_clks = isp0_div_clks,
	.nr_div_clks = ARRAY_SIZE(isp0_div_clks),
	.gate_clks = isp0_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(isp0_gate_clks),
	.nr_clk_ids = ISP0_NR_CLK,
};

/* Register Offset definitions for CMU_ISP1_LOCAL (0x142F0000) */
#define CLK_ENABLE_ACLK_ISP1_LOCAL 0x0800
#define CLK_ENABLE_PCLK_ISP1_234_LOCAL 0x0808

static const struct samsung_gate_clock isp1_local_gate_clks[] __initconst = {
	GATE(0, "isp1_local_gate_aclk_fimc_isp1", "isp1_gate_aclk_fimc_isp1",
	     CLK_ENABLE_ACLK_ISP1_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_local_gate_pclk_fimc_isp1", "isp1_gate_pclk_fimc_isp1",
	     CLK_ENABLE_PCLK_ISP1_234_LOCAL, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info isp1_local_cmu_info __initconst = {
	.gate_clks = isp1_local_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(isp1_local_gate_clks),
	.nr_clk_ids = ISP1_LOCAL_NR_CLK,
};

/* Register Offset definitions for CMU_ISP1 (0x147D0000) */
#define CLK_CON_MUX_ACLK_ISP1_468_USER 0x0200
#define CLK_CON_DIV_PCLK_ISP1_234 0x0400
#define CLK_STAT_MUX_ACLK_ISP1_468_USER 0x0600
#define CLK_ENABLE_ACLK_ISP1 0x0800
#define CLK_ENABLE_PCLK_ISP1_234 0x0808
#define CLK_ENABLE_PCLK_HPM_APBIF_ISP1 0x080C
#define CLK_ENABLE_SCLK_PROMISE_ISP1 0x0810
#define CLKOUT_CMU_ISP1 0x0D00
#define CLKOUT_CMU_ISP1_DIV_STAT 0x0D04
#define CMU_ISP1_SPARE0 0x0D08
#define CMU_ISP1_SPARE1 0x0D0C
#define CLK_ENABLE_PDN_ISP1 0x0E00
#define ISP1_SFR_IGNORE_REQ_SYSCLK 0x0E04

PNAME(isp1_mux_aclk_isp1_468_user_p) = { "oscclk",
					 "top_gate_aclk_isp1_isp1_468" };

static const struct samsung_mux_clock isp1_mux_clks[] __initconst = {
	MUX(0, "isp1_mux_aclk_isp1_468_user", isp1_mux_aclk_isp1_468_user_p,
	    CLK_CON_MUX_ACLK_ISP1_468_USER, 12, 1),
};

static const struct samsung_gate_clock isp1_gate_clks[] __initconst = {
	GATE(0, "isp1_gate_aclk_xiu_n_async_si", "isp1_mux_aclk_isp1_468_user",
	     CLK_ENABLE_ACLK_ISP1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_aclk_fimc_isp1", "isp1_mux_aclk_isp1_468_user",
	     CLK_ENABLE_ACLK_ISP1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_pclk_sysreg_isp1", "isp1_div_pclk_isp1_234",
	     CLK_ENABLE_PCLK_ISP1_234, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_pclk_pmu_isp1", "isp1_div_pclk_isp1_234",
	     CLK_ENABLE_PCLK_ISP1_234, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_aclk_axi2apb_bridge_is2p", "isp1_div_pclk_isp1_234",
	     CLK_ENABLE_PCLK_ISP1_234, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_aclk_xiu_n_async_mi", "isp1_div_pclk_isp1_234",
	     CLK_ENABLE_PCLK_ISP1_234, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_pclk_fimc_isp1", "isp1_div_pclk_isp1_234",
	     CLK_ENABLE_PCLK_ISP1_234, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_pclk_hpm_apbif_isp1", "isp1_div_pclk_isp1_234",
	     CLK_ENABLE_PCLK_HPM_APBIF_ISP1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "isp1_gate_sclk_promise_isp1", "top_gate_sclk_promise_int",
	     CLK_ENABLE_SCLK_PROMISE_ISP1, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock isp1_div_clks[] __initconst = {
	DIV(0, "isp1_div_pclk_isp1_234", "isp1_mux_aclk_isp1_468_user",
	    CLK_CON_DIV_PCLK_ISP1_234, 0, 3),
};

static const struct samsung_cmu_info isp1_cmu_info __initconst = {
	.mux_clks = isp1_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(isp1_mux_clks),
	.div_clks = isp1_div_clks,
	.nr_div_clks = ARRAY_SIZE(isp1_div_clks),
	.gate_clks = isp1_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(isp1_gate_clks),
	.nr_clk_ids = ISP1_NR_CLK,
};

/* Register Offset definitions for CMU_G3D (0x14AA0000) */
#define CLK_CON_MUX_G3D_PLL_USER 0x0200
#define CLK_CON_MUX_BUS_PLL_USER_G3D 0x0204
#define CLK_CON_MUX_G3D 0x020C
#define CLK_CON_MUX_ACLK_G3D 0x0210
#define CLK_CON_MUX_PCLK_G3D 0x0214
#define CLK_CON_DIV_ACLK_G3D 0x0400
#define CLK_CON_DIV_PCLK_G3D 0x0404
#define CLK_CON_DIV_SCLK_HPM_G3D 0x0408
#define CLK_CON_DIV_SCLK_ATE_G3D 0x040C
#define CLK_STAT_MUX_G3D_PLL_USER 0x0600
#define CLK_STAT_MUX_BUS_PLL_USER_G3D 0x0604
#define CLK_STAT_MUX_G3D 0x060C
#define CLK_STAT_MUX_ACLK_G3D 0x0610
#define CLK_STAT_MUX_PCLK_G3D 0x0614
#define CLK_ENABLE_ACLK_G3D 0x0800
#define CLK_ENABLE_ACLK_G3D_BUS 0x0804
#define CLK_ENABLE_PCLK_G3D 0x0900
#define CLK_ENABLE_SCLK_HPM_G3D 0x0A00
#define CLK_ENABLE_SCLK_ATE_G3D 0x0A04
#define CLKOUT_CMU_G3D 0x0C00
#define CLKOUT_CMU_G3D_DIV_STAT 0x0C04
#define CLK_ENABLE_PDN_G3D 0x0E00
#define G3D_SFR_IGNORE_REQ_SYSCLK 0x0F28
#define CLK_STOPCTRL_G3D 0x1000
#define CMU_G3D_SPARE0 0x1100
#define CMU_G3D_SPARE1 0x1104

PNAME(g3d_mux_g3d_pll_user_p) = { "oscclk", "g3d_pll" };
PNAME(g3d_mux_bus_pll_user_p) = { "oscclk", "top_gate_sclk_bus_pll_g3d" };
PNAME(g3d_mux_g3d_p) = { "g3d_mux_g3d_pll_user", "g3d_mux_bus_pll_user" };

static const struct samsung_mux_clock g3d_mux_clks[] __initconst = {
	MUX(0, "g3d_mux_g3d_pll_user", g3d_mux_g3d_pll_user_p,
	    CLK_CON_MUX_G3D_PLL_USER, 12, 1),
	MUX(0, "g3d_mux_bus_pll_user", g3d_mux_bus_pll_user_p,
	    CLK_CON_MUX_BUS_PLL_USER_G3D, 12, 1),
	MUX(0, "g3d_mux_g3d", g3d_mux_g3d_p, CLK_CON_MUX_G3D, 12, 1),
};

static const struct samsung_gate_clock g3d_gate_clks[] __initconst = {
	GATE(0, "g3d_gate_aclk_g3d", "g3d_div_aclk_g3d", CLK_ENABLE_ACLK_G3D, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_gray_dec", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_sfw100_acel_g3d1", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_sfw100_acel_g3d0", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_xiu_g3d", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_ppmu_g3d1", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_ppmu_g3d0", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_asyncapbm_g3d", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_asyncaxi_g3d", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_axi_ds_g3d", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_acel_lh_async_si_g3d1", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_acel_lh_async_si_g3d0", "g3d_div_aclk_g3d",
	     CLK_ENABLE_ACLK_G3D_BUS, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_pclk_sfw100_acel_g3d1", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_pclk_sfw100_acel_g3d0", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_pclk_hpm_g3d", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_pclk_ppmu_g3d1", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_pclk_ppmu_g3d0", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_pclk_pmu_g3d", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_asyncapbs_g3d", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_pclk_sysreg_g3d", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_axi2apb_g3dp", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_aclk_axi_lh_async_mi_g3dp", "g3d_div_pclk_g3d",
	     CLK_ENABLE_PCLK_G3D, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_sclk_hpm2_g3d", "g3d_div_sclk_hpm_g3d",
	     CLK_ENABLE_SCLK_HPM_G3D, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_sclk_hpm1_g3d", "g3d_div_sclk_hpm_g3d",
	     CLK_ENABLE_SCLK_HPM_G3D, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_sclk_hpm0_g3d", "g3d_div_sclk_hpm_g3d",
	     CLK_ENABLE_SCLK_HPM_G3D, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_sclk_axi_lh_async_si_g3diram", "g3d_div_sclk_ate_g3d",
	     CLK_ENABLE_SCLK_ATE_G3D, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "g3d_gate_sclk_asyncaxi_g3d", "g3d_div_sclk_ate_g3d",
	     CLK_ENABLE_SCLK_ATE_G3D, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock g3d_div_clks[] __initconst = {
	DIV(0, "g3d_div_aclk_g3d", "g3d_mux_g3d", CLK_CON_DIV_ACLK_G3D, 0, 3),
	DIV(0, "g3d_div_pclk_g3d", "g3d_div_aclk_g3d", CLK_CON_DIV_PCLK_G3D, 0,
	    3),
	DIV(0, "g3d_div_sclk_hpm_g3d", "g3d_mux_g3d", CLK_CON_DIV_SCLK_HPM_G3D,
	    0, 2),
	DIV(0, "g3d_div_sclk_ate_g3d", "g3d_mux_g3d", CLK_CON_DIV_SCLK_ATE_G3D,
	    0, 4),
};

static const struct samsung_cmu_info g3d_cmu_info __initconst = {
	.mux_clks = g3d_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(g3d_mux_clks),
	.div_clks = g3d_div_clks,
	.nr_div_clks = ARRAY_SIZE(g3d_div_clks),
	.gate_clks = g3d_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(g3d_gate_clks),
	.nr_clk_ids = G3D_NR_CLK,
};

/* Register Offset definitions for CMU_PERIC1 (0x14C80000) */
#define CLK_CON_MUX_ACLK_PERIC1_66_USER 0x0200
#define CLK_CON_MUX_SCLK_SPI0_USER 0x0204
#define CLK_CON_MUX_SCLK_SPI1_USER 0x0208
#define CLK_CON_MUX_SCLK_SPI2_USER 0x020C
#define CLK_CON_MUX_SCLK_SPI3_USER 0x0210
#define CLK_CON_MUX_SCLK_SPI4_USER 0x0214
#define CLK_CON_MUX_SCLK_SPI5_USER 0x0218
#define CLK_CON_MUX_SCLK_SPI6_USER 0x021C
#define CLK_CON_MUX_SCLK_SPI7_USER 0x0220
#define CLK_CON_MUX_SCLK_UART1_USER 0x0224
#define CLK_CON_MUX_SCLK_UART2_USER 0x0228
#define CLK_CON_MUX_SCLK_UART3_USER 0x022C
#define CLK_CON_MUX_SCLK_UART4_USER 0x0230
#define CLK_CON_MUX_SCLK_UART5_USER 0x0234
#define CLK_STAT_MUX_ACLK_PERIC1_66_USER 0x0600
#define CLK_STAT_MUX_SCLK_SPI0_USER 0x0604
#define CLK_STAT_MUX_SCLK_SPI1_USER 0x0608
#define CLK_STAT_MUX_SCLK_SPI2_USER 0x060C
#define CLK_STAT_MUX_SCLK_SPI3_USER 0x0610
#define CLK_STAT_MUX_SCLK_SPI4_USER 0x0614
#define CLK_STAT_MUX_SCLK_SPI5_USER 0x0618
#define CLK_STAT_MUX_SCLK_SPI6_USER 0x061C
#define CLK_STAT_MUX_SCLK_SPI7_USER 0x0620
#define CLK_STAT_MUX_SCLK_UART1_USER 0x0624
#define CLK_STAT_MUX_SCLK_UART2_USER 0x0628
#define CLK_STAT_MUX_SCLK_UART3_USER 0x062C
#define CLK_STAT_MUX_SCLK_UART4_USER 0x0630
#define CLK_STAT_MUX_SCLK_UART5_USER 0x0634
#define CG_CTRL_VAL_ACLK_PERIC1_66 0x0800
#define CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C 0x0804
#define CG_CTRL_VAL_SCLK_SPI0 0x0840
#define CG_CTRL_VAL_SCLK_SPI1 0x0844
#define CG_CTRL_VAL_SCLK_SPI2 0x0848
#define CG_CTRL_VAL_SCLK_SPI3 0x084C
#define CG_CTRL_VAL_SCLK_SPI4 0x0850
#define CG_CTRL_VAL_SCLK_SPI5 0x0854
#define CG_CTRL_VAL_SCLK_SPI6 0x0858
#define CG_CTRL_VAL_SCLK_SPI7 0x085C
#define CG_CTRL_VAL_SCLK_UART1 0x0860
#define CG_CTRL_VAL_SCLK_UART2 0x0864
#define CG_CTRL_VAL_SCLK_UART3 0x0868
#define CG_CTRL_VAL_SCLK_UART4 0x086C
#define CG_CTRL_VAL_SCLK_UART5 0x0870
#define CLKOUT_CMU_PERIC1 0x0C00
#define CLKOUT_CMU_PERIC1_DIV_STAT 0x0C04
#define PERIC1_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_PERIC1_SPARE0 0x0D08
#define CMU_PERIC1_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_PERIC1_66 0x1800
#define CG_CTRL_MAN_ACLK_PERIC1_66_HSI2C 0x1804
#define CG_CTRL_MAN_SCLK_SPI0 0x1840
#define CG_CTRL_MAN_SCLK_SPI1 0x1844
#define CG_CTRL_MAN_SCLK_SPI2 0x1848
#define CG_CTRL_MAN_SCLK_SPI3 0x184C
#define CG_CTRL_MAN_SCLK_SPI4 0x1850
#define CG_CTRL_MAN_SCLK_SPI5 0x1854
#define CG_CTRL_MAN_SCLK_SPI6 0x1858
#define CG_CTRL_MAN_SCLK_SPI7 0x185C
#define CG_CTRL_MAN_SCLK_UART1 0x1860
#define CG_CTRL_MAN_SCLK_UART2 0x1864
#define CG_CTRL_MAN_SCLK_UART3 0x1868
#define CG_CTRL_MAN_SCLK_UART4 0x186C
#define CG_CTRL_MAN_SCLK_UART5 0x1870
#define CG_CTRL_STAT_ACLK_PERIC1_66_0 0x1C00
#define CG_CTRL_STAT_ACLK_PERIC1_66_1 0x1C04
#define CG_CTRL_STAT_ACLK_PERIC1_66_2 0x1C08
#define CG_CTRL_STAT_ACLK_PERIC1_66_3 0x1C0C
#define CG_CTRL_STAT_ACLK_PERIC1_66_HSI2C 0x1C10
#define CG_CTRL_STAT_SCLK_SPI0 0x1C40
#define CG_CTRL_STAT_SCLK_SPI1 0x1C44
#define CG_CTRL_STAT_SCLK_SPI2 0x1C48
#define CG_CTRL_STAT_SCLK_SPI3 0x1C4C
#define CG_CTRL_STAT_SCLK_SPI4 0x1C50
#define CG_CTRL_STAT_SCLK_SPI5 0x1C54
#define CG_CTRL_STAT_SCLK_SPI6 0x1C58
#define CG_CTRL_STAT_SCLK_SPI7 0x1C5C
#define CG_CTRL_STAT_SCLK_UART1 0x1C60
#define CG_CTRL_STAT_SCLK_UART2 0x1C64
#define CG_CTRL_STAT_SCLK_UART3 0x1C68
#define CG_CTRL_STAT_SCLK_UART4 0x1C6C
#define CG_CTRL_STAT_SCLK_UART5 0x1C70
#define QCH_CTRL_AXILHASYNCM_PERIC1 0x2000
#define QCH_CTRL_CMU_PERIC1 0x2004
#define QCH_CTRL_PMU_PERIC1 0x2008
#define QCH_CTRL_SYSREG_PERIC1 0x200C
#define QSTATE_CTRL_GPIO_PERIC1 0x2410
#define QSTATE_CTRL_GPIO_NFC 0x2414
#define QSTATE_CTRL_GPIO_TOUCH 0x2418
#define QSTATE_CTRL_GPIO_FF 0x241C
#define QSTATE_CTRL_GPIO_ESE 0x2420
#define QSTATE_CTRL_UART1 0x2424
#define QSTATE_CTRL_UART2 0x2428
#define QSTATE_CTRL_UART3 0x242C
#define QSTATE_CTRL_UART4 0x2430
#define QSTATE_CTRL_UART5 0x2434
#define QSTATE_CTRL_SPI0 0x2438
#define QSTATE_CTRL_SPI1 0x243C
#define QSTATE_CTRL_SPI2 0x2440
#define QSTATE_CTRL_SPI3 0x2444
#define QSTATE_CTRL_SPI4 0x2448
#define QSTATE_CTRL_SPI5 0x244C
#define QSTATE_CTRL_SPI6 0x2450
#define QSTATE_CTRL_SPI7 0x2454
#define QSTATE_CTRL_HSI2C2 0x2458
#define QSTATE_CTRL_HSI2C3 0x245C
#define QSTATE_CTRL_HSI2C6 0x2460
#define QSTATE_CTRL_HSI2C7 0x2464
#define QSTATE_CTRL_HSI2C8 0x2468
#define QSTATE_CTRL_HSI2C12 0x246C
#define QSTATE_CTRL_HSI2C13 0x2470
#define QSTATE_CTRL_HSI2C14 0x2474

PNAME(peric1_mux_aclk_peric1_66_user_p) = { "oscclk",
					    "top_gate_aclk_peric1_66" };
PNAME(peric1_mux_sclk_spi0_user_p) = { "oscclk", "top_gate_sclk_peric1_spi0" };
PNAME(peric1_mux_sclk_spi1_user_p) = { "oscclk", "top_gate_sclk_peric1_spi1" };
PNAME(peric1_mux_sclk_spi2_user_p) = { "oscclk", "top_gate_sclk_peric1_spi2" };
PNAME(peric1_mux_sclk_spi3_user_p) = { "oscclk", "top_gate_sclk_peric1_spi3" };
PNAME(peric1_mux_sclk_spi4_user_p) = { "oscclk", "top_gate_sclk_peric1_spi4" };
PNAME(peric1_mux_sclk_spi5_user_p) = { "oscclk", "top_gate_sclk_peric1_spi5" };
PNAME(peric1_mux_sclk_spi6_user_p) = { "oscclk", "top_gate_sclk_peric1_spi6" };
PNAME(peric1_mux_sclk_spi7_user_p) = { "oscclk", "top_gate_sclk_peric1_spi7" };
PNAME(peric1_mux_sclk_uart1_user_p) = { "oscclk",
					"top_gate_sclk_peric1_uart1" };
PNAME(peric1_mux_sclk_uart2_user_p) = { "oscclk",
					"top_gate_sclk_peric1_uart2" };
PNAME(peric1_mux_sclk_uart3_user_p) = { "oscclk",
					"top_gate_sclk_peric1_uart3" };
PNAME(peric1_mux_sclk_uart4_user_p) = { "oscclk",
					"top_gate_sclk_peric1_uart4" };
PNAME(peric1_mux_sclk_uart5_user_p) = { "oscclk",
					"top_gate_sclk_peric1_uart5" };

static const struct samsung_mux_clock peric1_mux_clks[] __initconst = {
	MUX(0, "peric1_mux_aclk_peric1_66_user",
	    peric1_mux_aclk_peric1_66_user_p, CLK_CON_MUX_ACLK_PERIC1_66_USER,
	    12, 1),
	MUX(0, "peric1_mux_sclk_spi0_user", peric1_mux_sclk_spi0_user_p,
	    CLK_CON_MUX_SCLK_SPI0_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_spi1_user", peric1_mux_sclk_spi1_user_p,
	    CLK_CON_MUX_SCLK_SPI1_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_spi2_user", peric1_mux_sclk_spi2_user_p,
	    CLK_CON_MUX_SCLK_SPI2_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_spi3_user", peric1_mux_sclk_spi3_user_p,
	    CLK_CON_MUX_SCLK_SPI3_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_spi4_user", peric1_mux_sclk_spi4_user_p,
	    CLK_CON_MUX_SCLK_SPI4_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_spi5_user", peric1_mux_sclk_spi5_user_p,
	    CLK_CON_MUX_SCLK_SPI5_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_spi6_user", peric1_mux_sclk_spi6_user_p,
	    CLK_CON_MUX_SCLK_SPI6_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_spi7_user", peric1_mux_sclk_spi7_user_p,
	    CLK_CON_MUX_SCLK_SPI7_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_uart1_user", peric1_mux_sclk_uart1_user_p,
	    CLK_CON_MUX_SCLK_UART1_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_uart2_user", peric1_mux_sclk_uart2_user_p,
	    CLK_CON_MUX_SCLK_UART2_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_uart3_user", peric1_mux_sclk_uart3_user_p,
	    CLK_CON_MUX_SCLK_UART3_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_uart4_user", peric1_mux_sclk_uart4_user_p,
	    CLK_CON_MUX_SCLK_UART4_USER, 12, 1),
	MUX(0, "peric1_mux_sclk_uart5_user", peric1_mux_sclk_uart5_user_p,
	    CLK_CON_MUX_SCLK_UART5_USER, 12, 1),
};

static const struct samsung_gate_clock peric1_gate_clks[] __initconst = {
	GATE(0, "peric1_gate_pclk_spi7", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 25, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_spi6", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 24, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_spi5", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 23, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_spi4", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 22, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_spi3", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_spi2", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 20, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_spi1", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 19, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_spi0", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 18, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_uart5", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 17, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_uart4", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_uart3", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_uart2", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_uart1", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_gpio_ese", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_gpio_ff", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_gpio_touch", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_gpio_nfc", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_gpio_peric1",
	     "peric1_mux_aclk_peric1_66_user", CG_CTRL_VAL_ACLK_PERIC1_66, 8,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_sysreg_peric1",
	     "peric1_mux_aclk_peric1_66_user", CG_CTRL_VAL_ACLK_PERIC1_66, 7,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_pmu_peric1", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_cmu_peric1", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_aclk_axi2apb_peric1_2p",
	     "peric1_mux_aclk_peric1_66_user", CG_CTRL_VAL_ACLK_PERIC1_66, 4,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_aclk_axi2apb_peric1_1p",
	     "peric1_mux_aclk_peric1_66_user", CG_CTRL_VAL_ACLK_PERIC1_66, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_aclk_axi2apb_peric1_0p",
	     "peric1_mux_aclk_peric1_66_user", CG_CTRL_VAL_ACLK_PERIC1_66, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_aclk_xiu_peric1sfrx",
	     "peric1_mux_aclk_peric1_66_user", CG_CTRL_VAL_ACLK_PERIC1_66, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_aclk_axilhasyncm_peric1",
	     "peric1_mux_aclk_peric1_66_user", CG_CTRL_VAL_ACLK_PERIC1_66, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c14", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c13", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c12", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c8", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c7", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c6", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c3", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_pclk_hsi2c2", "peric1_mux_aclk_peric1_66_user",
	     CG_CTRL_VAL_ACLK_PERIC1_66_HSI2C, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi0", "peric1_mux_sclk_spi0_user",
	     CG_CTRL_VAL_SCLK_SPI0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi1", "peric1_mux_sclk_spi1_user",
	     CG_CTRL_VAL_SCLK_SPI1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi2", "peric1_mux_sclk_spi2_user",
	     CG_CTRL_VAL_SCLK_SPI2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi3", "peric1_mux_sclk_spi3_user",
	     CG_CTRL_VAL_SCLK_SPI3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi4", "peric1_mux_sclk_spi4_user",
	     CG_CTRL_VAL_SCLK_SPI4, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi5", "peric1_mux_sclk_spi5_user",
	     CG_CTRL_VAL_SCLK_SPI5, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi6", "peric1_mux_sclk_spi6_user",
	     CG_CTRL_VAL_SCLK_SPI6, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_spi7", "peric1_mux_sclk_spi7_user",
	     CG_CTRL_VAL_SCLK_SPI7, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_uart1", "peric1_mux_sclk_uart1_user",
	     CG_CTRL_VAL_SCLK_UART1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_uart2", "peric1_mux_sclk_uart2_user",
	     CG_CTRL_VAL_SCLK_UART2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_uart3", "peric1_mux_sclk_uart3_user",
	     CG_CTRL_VAL_SCLK_UART3, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_uart4", "peric1_mux_sclk_uart4_user",
	     CG_CTRL_VAL_SCLK_UART4, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_gate_sclk_uart5", "peric1_mux_sclk_uart5_user",
	     CG_CTRL_VAL_SCLK_UART5, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_gpio_peric1", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_GPIO_PERIC1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_gpio_nfc", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_GPIO_NFC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_gpio_touch", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_GPIO_TOUCH, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_gpio_ff", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_GPIO_FF, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_gpio_ese", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_GPIO_ESE, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_uart1", "PERIC1_MUX_ACLK_PERIC1_66_USER",
	     QSTATE_CTRL_UART1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_uart2", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_UART2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_uart3", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_UART3, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_uart4", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_UART4, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_uart5", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_UART5, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi0", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi1", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi2", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi3", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI3, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi4", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI4, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi5", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI5, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi6", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI6, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_spi7", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_SPI7, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c2", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C2, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c3", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C3, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c6", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C6, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c7", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C7, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c8", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C8, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c12", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C12, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c13", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C13, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "peric1_hwacg_hsi2c14", "peric1_mux_aclk_peric1_66_user",
	     QSTATE_CTRL_HSI2C14, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_cmu_info peric1_cmu_info __initconst = {
	.mux_clks = peric1_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(peric1_mux_clks),
	.gate_clks = peric1_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(peric1_gate_clks),
	.nr_clk_ids = PERIC1_NR_CLK,
};

/* Register Offset definitions for CMU_MSCL (0x150D0000) */
#define CLK_CON_MUX_ACLK_MSCL0_528_USER 0x0200
#define CLK_CON_MUX_ACLK_MSCL1_528_USER 0x0204
#define CLK_CON_MUX_ACLK_MSCL1_528_MSCL 0x0210
#define CLK_CON_DIV_PCLK_MSCL 0x0400
#define CLK_STAT_MUX_ACLK_MSCL0_528_USER 0x0600
#define CLK_STAT_MUX_ACLK_MSCL1_528_USER 0x0604
#define CLK_STAT_MUX_ACLK_MSCL1_528_MSCL 0x0610
#define CG_CTRL_VAL_ACLK_MSCL0_528 0x0800
#define CG_CTRL_VAL_ACLK_MSCL0_528_SECURE_SFW_MSCL_0 0x0804
#define CG_CTRL_VAL_ACLK_MSCL1_528 0x0808
#define CG_CTRL_VAL_ACLK_MSCL1_528_SECURE_SFW_MSCL_1 0x080C
#define CG_CTRL_VAL_PCLK_MSCL 0x0820
#define CG_CTRL_VAL_PCLK_MSCL_SECURE_SFW_MSCL_0 0x0824
#define CG_CTRL_VAL_PCLK_MSCL_SECURE_SFW_MSCL_1 0x0828
#define CLKOUT_CMU_MSCL 0x0C00
#define CLKOUT_CMU_MSCL_DIV_STAT 0x0C04
#define MSCL_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_MSCL_SPARE0 0x0D08
#define CMU_MSCL_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_MSCL0_528 0x1800
#define CG_CTRL_MAN_ACLK_MSCL0_528_SECURE_SFW_MSCL_0 0x1804
#define CG_CTRL_MAN_ACLK_MSCL1_528 0x1808
#define CG_CTRL_MAN_ACLK_MSCL1_528_SECURE_SFW_MSCL_1 0x180C
#define CG_CTRL_MAN_PCLK_MSCL 0x1820
#define CG_CTRL_MAN_PCLK_MSCL_SECURE_SFW_MSCL_0 0x1824
#define CG_CTRL_MAN_PCLK_MSCL_SECURE_SFW_MSCL_1 0x1828
#define CG_CTRL_STAT_ACLK_MSCL0_528_0 0x1C00
#define CG_CTRL_STAT_ACLK_MSCL0_528_1 0x1C04
#define CG_CTRL_STAT_ACLK_MSCL0_528_SECURE_SFW_MSCL_0 0x1C08
#define CG_CTRL_STAT_ACLK_MSCL1_528_0 0x1C0C
#define CG_CTRL_STAT_ACLK_MSCL1_528_1 0x1C10
#define CG_CTRL_STAT_ACLK_MSCL1_528_SECURE_SFW_MSCL_1 0x1C14
#define CG_CTRL_STAT_PCLK_MSCL_0 0x1C20
#define CG_CTRL_STAT_PCLK_MSCL_1 0x1C24
#define CG_CTRL_STAT_PCLK_MSCL_2 0x1C28
#define CG_CTRL_STAT_PCLK_MSCL_SECURE_SFW_MSCL_0 0x1C2C
#define CG_CTRL_STAT_PCLK_MSCL_SECURE_SFW_MSCL_1 0x1C30
#define QCH_CTRL_LH_ASYNC_MI_MSCLSFR 0x2000
#define QCH_CTRL_CMU_MSCL 0x2004
#define QCH_CTRL_PMU_MSCL 0x2008
#define QCH_CTRL_SYSREG_MSCL 0x200C
#define QCH_CTRL_MSCL_0 0x2010
#define QCH_CTRL_MSCL_1 0x2014
#define QCH_CTRL_JPEG 0x2018
#define QCH_CTRL_G2D 0x201C
#define QCH_CTRL_SMMU_MSCL_0 0x2020
#define QCH_CTRL_SMMU_MSCL_1 0x2024
#define QCH_CTRL_SMMU_JPEG 0x2028
#define QCH_CTRL_SMMU_G2D 0x202C
#define QCH_CTRL_PPMU_MSCL_0 0x2030
#define QCH_CTRL_PPMU_MSCL_1 0x2034
#define QCH_CTRL_SFW_MSCL_0 0x2038
#define QCH_CTRL_SFW_MSCL_1 0x203C
#define QCH_CTRL_LH_ASYNC_SI_MSCL_0 0x2040
#define QCH_CTRL_LH_ASYNC_SI_MSCL_1 0x2044

PNAME(mscl_mux_aclk_mscl0_528_user_p) = { "oscclk", "top_gate_aclk_mscl0_528" };
PNAME(mscl_mux_aclk_mscl1_528_user_p) = { "oscclk", "top_gate_aclk_mscl1_528" };
PNAME(mscl_mux_aclk_mscl1_528_p) = { "mscl_mux_aclk_mscl0_528_user",
				     "mscl_mux_aclk_mscl1_528_user" };

static const struct samsung_mux_clock mscl_mux_clks[] __initconst = {
	MUX(0, "mscl_mux_aclk_mscl0_528_user", mscl_mux_aclk_mscl0_528_user_p,
	    CLK_CON_MUX_ACLK_MSCL0_528_USER, 12, 1),
	MUX(0, "mscl_mux_aclk_mscl1_528_user", mscl_mux_aclk_mscl1_528_user_p,
	    CLK_CON_MUX_ACLK_MSCL1_528_USER, 12, 1),
	MUX(0, "mscl_mux_aclk_mscl1_528", mscl_mux_aclk_mscl1_528_p,
	    CLK_CON_MUX_ACLK_MSCL1_528_MSCL, 12, 1),
};

static const struct samsung_gate_clock mscl_gate_clks[] __initconst = {
	GATE(0, "mscl_gate_aclk_asyncapb_jpeg", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_ppmu_mscl_0", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_smmu_jpeg", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_smmu_mscl_0", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_qe_jpeg", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_qe_mscl_0", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_xiu_msclx_0", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_lh_async_si_mscl_0",
	     "mscl_mux_aclk_mscl0_528_user", CG_CTRL_VAL_ACLK_MSCL0_528, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_jpeg", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_mscl_0", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_sfw_mscl_0", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL0_528_SECURE_SFW_MSCL_0, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "mscl_gate_aclk_asyncapb_g2d", "mscl_mux_aclk_mscl1_528",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_ppmu_mscl_1", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_smmu_g2d", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_smmu_mscl_1", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_qe_g2d", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_qe_mscl_1", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_axi2acel", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_xiu_msclx_1", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_lh_async_si_mscl_1",
	     "mscl_mux_aclk_mscl0_528_user", CG_CTRL_VAL_ACLK_MSCL1_528, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_g2d", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_mscl_1", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_sfw_mscl_1", "mscl_mux_aclk_mscl0_528_user",
	     CG_CTRL_VAL_ACLK_MSCL1_528_SECURE_SFW_MSCL_1, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "mscl_gate_aclk_lh_async_mi_msclsfr", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 20, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_pmu_mscl", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 19, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_sysreg_mscl", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 18, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_cmu_mscl", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 17, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_ppmu_mscl_1", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_ppmu_mscl_0", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 15, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_smmu_g2d", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 14, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_smmu_jpeg", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 13, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_smmu_mscl_1", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 12, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_smmu_mscl_0", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_qe_g2d", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 10, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_qe_jpeg", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_qe_mscl_1", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_qe_mscl_0", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_asyncapb_g2d", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_asyncapb_jpeg", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_mscl_1", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_mscl_0", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_axi2apb_msclsfr_1p", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_axi2apb_msclsfr_0p", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_aclk_xiu_msclsfrx", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_sfw_mscl_0", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL_SECURE_SFW_MSCL_0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mscl_gate_pclk_sfw_mscl_1", "mscl_div_pclk_mscl",
	     CG_CTRL_VAL_PCLK_MSCL_SECURE_SFW_MSCL_1, 0, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock mscl_div_clks[] __initconst = {
	DIV(0, "mscl_div_pclk_mscl", "mscl_mux_aclk_mscl0_528_user",
	    CLK_CON_DIV_PCLK_MSCL, 0, 3),
};

static const struct samsung_cmu_info mscl_cmu_info __initconst = {
	.mux_clks = mscl_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(mscl_mux_clks),
	.div_clks = mscl_div_clks,
	.nr_div_clks = ARRAY_SIZE(mscl_div_clks),
	.gate_clks = mscl_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(mscl_gate_clks),
	.nr_clk_ids = MSCL_NR_CLK,
};

/* Register Offset definitions for CMU_MFC (0x15280000) */
#define CLK_CON_MUX_ACLK_MFC_600_USER 0x0200
#define CLK_CON_DIV_PCLK_MFC_150 0x0400
#define CLK_STAT_MUX_ACLK_MFC_600_USER 0x0600
#define CG_CTRL_VAL_ACLK_MFC_600 0x0800
#define CG_CTRL_VAL_ACLK_MFC_600_SECURE_SFW_MFC_0 0x0804
#define CG_CTRL_VAL_ACLK_MFC_600_SECURE_SFW_MFC_1 0x0808
#define CG_CTRL_VAL_PCLK_MFC_150 0x0820
#define CG_CTRL_VAL_PCLK_MFC_150_HPM_APBIF_MFC 0x0824
#define CG_CTRL_VAL_PCLK_MFC_150_SECURE_SFW_MFC_0 0x0828
#define CG_CTRL_VAL_PCLK_MFC_150_SECURE_SFW_MFC_1 0x082C
#define CG_CTRL_VAL_SCLK_MFC_PROMISE 0x0840
#define CLKOUT_CMU_MFC 0x0C00
#define CLKOUT_CMU_MFC_DIV_STAT 0x0C04
#define MFC_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_MFC_SPARE0 0x0D08
#define CMU_MFC_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_MFC_600 0x1800
#define CG_CTRL_MAN_ACLK_MFC_600_SECURE_SFW_MFC_0 0x1804
#define CG_CTRL_MAN_ACLK_MFC_600_SECURE_SFW_MFC_1 0x1808
#define CG_CTRL_MAN_PCLK_MFC_150 0x1820
#define CG_CTRL_MAN_PCLK_MFC_150_HPM_APBIF_MFC 0x1824
#define CG_CTRL_MAN_PCLK_MFC_150_SECURE_SFW_MFC_0 0x1828
#define CG_CTRL_MAN_PCLK_MFC_150_SECURE_SFW_MFC_1 0x182C
#define CG_CTRL_MAN_SCLK_MFC_PROMISE 0x1840
#define CG_CTRL_STAT_ACLK_MFC_600 0x1C00
#define CG_CTRL_STAT_ACLK_MFC_600_SECURE_SFW_MFC_0 0x1C04
#define CG_CTRL_STAT_ACLK_MFC_600_SECURE_SFW_MFC_1 0x1C08
#define CG_CTRL_STAT_PCLK_MFC_150_0 0x1C20
#define CG_CTRL_STAT_PCLK_MFC_150_1 0x1C24
#define CG_CTRL_STAT_PCLK_MFC_150_HPM_APBIF_MFC 0x1C28
#define CG_CTRL_STAT_PCLK_MFC_150_SECURE_SFW_MFC_0 0x1C2C
#define CG_CTRL_STAT_PCLK_MFC_150_SECURE_SFW_MFC_1 0x1C30
#define CG_CTRL_STAT_SCLK_MFC_PROMISE 0x1C40
#define QCH_CTRL_MFC 0x2000
#define QCH_CTRL_LH_M_MFC 0x2004
#define QCH_CTRL_CMU_MFC 0x2008
#define QCH_CTRL_PMU_MFC 0x200C
#define QCH_CTRL_SYSREG_MFC 0x2010
#define QCH_CTRL_PPMU_MFC_0 0x2014
#define QCH_CTRL_PPMU_MFC_1 0x2018
#define QCH_CTRL_SFW_MFC_0 0x201C
#define QCH_CTRL_SFW_MFC_1 0x2020
#define QCH_CTRL_SMMU_MFC_0 0x2024
#define QCH_CTRL_SMMU_MFC_1 0x2028
#define QCH_CTRL_LH_S_MFC_0 0x202C
#define QCH_CTRL_LH_S_MFC_1 0x2030
#define QSTATE_CTRL_HPM_APBIF_MFC 0x2404
#define QSTATE_CTRL_PROMISE_MFC 0x2408

PNAME(mfc_mux_aclk_mfc_600_user_p) = { "oscclk", "top_gate_aclk_mfc_600" };

static const struct samsung_mux_clock mfc_mux_clks[] __initconst = {
	MUX(0, "mfc_mux_aclk_mfc_600_user", mfc_mux_aclk_mfc_600_user_p,
	    CLK_CON_MUX_ACLK_MFC_600_USER, 12, 1),
};

static const struct samsung_gate_clock mfc_gate_clks[] __initconst = {
	GATE(0, "mfc_gate_aclk_asyncapb_mfc", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_smmu_mfc_1", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_smmu_mfc_0", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_mfc", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_ppmu_mfc_1", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_ppmu_mfc_0", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_lh_s_mfc_1", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_lh_s_mfc_0", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_sfw_mfc_0", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600_SECURE_SFW_MFC_0, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "mfc_gate_aclk_sfw_mfc_1", "mfc_mux_aclk_mfc_600_user",
	     CG_CTRL_VAL_ACLK_MFC_600_SECURE_SFW_MFC_1, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "mfc_gate_pclk_sysreg_mfc", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 9, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_smmu_mfc_1", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 8, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_smmu_mfc_0", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 7, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_ppmu_mfc_1", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 6, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_ppmu_mfc_0", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_pmu_mfc", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_cmu_mfc", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 3, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_asyncapb_mfc", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 2, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_axi2apb_mfcsfr", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_aclk_lh_m_mfc", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_hpm_apbif_mfc", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150_HPM_APBIF_MFC, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_gate_pclk_sfw_mfc_0", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150_SECURE_SFW_MFC_0, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "mfc_gate_pclk_sfw_mfc_1", "mfc_div_pclk_mfc_150",
	     CG_CTRL_VAL_PCLK_MFC_150_SECURE_SFW_MFC_1, 0, CLK_IGNORE_UNUSED,
	     0),
	GATE(0, "mfc_gate_sclk_promise_mfc", "top_gate_sclk_promise_int",
	     CG_CTRL_VAL_SCLK_MFC_PROMISE, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_hwacg_hpm_apbif_mfc", "mfc_div_pclk_mfc_150",
	     QSTATE_CTRL_HPM_APBIF_MFC, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "mfc_hwacg_promise_mfc", "top_gate_sclk_promise_int",
	     QSTATE_CTRL_PROMISE_MFC, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock mfc_div_clks[] __initconst = {
	DIV(0, "mfc_div_pclk_mfc_150", "mfc_mux_aclk_mfc_600_user",
	    CLK_CON_DIV_PCLK_MFC_150, 0, 2),
};

static const struct samsung_cmu_info mfc_cmu_info __initconst = {
	.mux_clks = mfc_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(mfc_mux_clks),
	.div_clks = mfc_div_clks,
	.nr_div_clks = ARRAY_SIZE(mfc_div_clks),
	.gate_clks = mfc_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(mfc_gate_clks),
	.nr_clk_ids = MFC_NR_CLK,
};

/* Register Offset definitions for CMU_FSYS1 (0x156E0000) */
#define PCIE_PLL_LOCK 0x0000
#define PCIE_PLL_CON0 0x0100
#define PCIE_PLL_CON1 0x0104
#define CLK_CON_MUX_ACLK_FSYS1_200_USER 0x0200
#define CLK_CON_MUX_SCLK_FSYS1_MMC2_USER 0x0204
#define CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO_SDCARD_USER 0x020C
#define CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO_SDCARD_CFG_USER 0x0210
#define CLK_CON_MUX_SCLK_FSYS1_PCIE_PHY_USER 0x0214
#define CLK_CON_MUX_PCIE_PLL 0x0218
#define CLK_CON_MUX_PHYCLK_UFS_LINK_SDCARD_TX0_SYMBOL_USER 0x0220
#define CLK_CON_MUX_PHYCLK_UFS_LINK_SDCARD_RX0_SYMBOL_USER 0x0224
#define CLK_CON_MUX_PHYCLK_PCIE_WIFI0_TX0_USER 0x0230
#define CLK_CON_MUX_PHYCLK_PCIE_WIFI0_RX0_USER 0x0234
#define CLK_CON_MUX_PHYCLK_PCIE_WIFI1_TX0_USER 0x0238
#define CLK_CON_MUX_PHYCLK_PCIE_WIFI1_RX0_USER 0x023C
#define CLK_CON_MUX_PHYCLK_PCIE_WIFI0_DIG_REFCLK_USER 0x0240
#define CLK_CON_MUX_PHYCLK_PCIE_WIFI1_DIG_REFCLK_USER 0x0244
#define CLK_CON_MUX_PHYCLK_UFS_LINK_SDCARD_RX_PWM_CLK_USER 0x0248
#define CLK_CON_MUX_PHYCLK_UFS_LINK_SDCARD_TX_PWM_CLK_USER 0x024C
#define CLK_CON_MUX_PHYCLK_UFS_LINK_SDCARD_REFCLK_OUT_SOC_USER 0x0250
#define CLK_CON_DIV_PCLK_COMBO_PHY_WIFI 0x0400
#define CLK_STAT_MUX_ACLK_FSYS1_200_USER 0x0600
#define CLK_STAT_MUX_SCLK_FSYS1_MMC2_USER 0x0604
#define CLK_STAT_MUX_SCLK_FSYS1_UFSUNIPRO_SDCARD_USER 0x060C
#define CLK_STAT_MUX_SCLK_FSYS1_UFSUNIPRO_SDCARD_CFG_USER 0x0610
#define CLK_STAT_MUX_SCLK_FSYS1_PCIE_PHY_USER 0x0614
#define CLK_STAT_MUX_PCIE_PLL 0x0618
#define CLK_STAT_MUX_PHYCLK_UFS_LINK_SDCARD_TX0_SYMBOL_USER 0x0620
#define CLK_STAT_MUX_PHYCLK_UFS_LINK_SDCARD_RX0_SYMBOL_USER 0x0624
#define CLK_STAT_MUX_PHYCLK_PCIE_WIFI0_TX0_USER 0x0630
#define CLK_STAT_MUX_PHYCLK_PCIE_WIFI0_RX0_USER 0x0634
#define CLK_STAT_MUX_PHYCLK_PCIE_WIFI1_TX0_USER 0x0638
#define CLK_STAT_MUX_PHYCLK_PCIE_WIFI1_RX0_USER 0x063C
#define CLK_STAT_MUX_PHYCLK_PCIE_WIFI0_DIG_REFCLK_USER 0x0640
#define CLK_STAT_MUX_PHYCLK_PCIE_WIFI1_DIG_REFCLK_USER 0x0644
#define CLK_STAT_MUX_PHYCLK_UFS_LINK_SDCARD_RX_PWM_CLK_USER 0x0648
#define CLK_STAT_MUX_PHYCLK_UFS_LINK_SDCARD_TX_PWM_CLK_USER 0x064C
#define CLK_STAT_MUX_PHYCLK_UFS_LINK_SDCARD_REFCLK_OUT_SOC_USER 0x0650
#define CG_CTRL_VAL_ACLK_FSYS1_200 0x0800
#define CG_CTRL_VAL_PCLK_HPM_APBIF_FSYS1 0x0820
#define CG_CTRL_VAL_PCLK_COMBO_PHY_WIFI 0x0824
#define CG_CTRL_VAL_SCLK_MMC2 0x0840
#define CG_CTRL_VAL_SCLK_UFSUNIPRO_SDCARD 0x0844
#define CG_CTRL_VAL_SCLK_UFSUNIPRO_SDCARD_CFG 0x0848
#define CG_CTRL_VAL_SCLK_FSYS1_PCIE0_PHY 0x084C
#define CG_CTRL_VAL_SCLK_FSYS1_PCIE1_PHY 0x0850
#define CG_CTRL_VAL_SCLK_PCIE_LINK_WIFI0 0x0854
#define CG_CTRL_VAL_SCLK_PCIE_LINK_WIFI1 0x0858
#define CG_CTRL_VAL_PHYCLK_UFS_LINK_SDCARD_TX0_SYMBOL 0x085C
#define CG_CTRL_VAL_PHYCLK_UFS_LINK_SDCARD_RX0_SYMBOL 0x0860
#define CG_CTRL_VAL_PHYCLK_PCIE_WIFI0_TX0 0x086C
#define CG_CTRL_VAL_PHYCLK_PCIE_WIFI0_RX0 0x0870
#define CG_CTRL_VAL_PHYCLK_PCIE_WIFI1_TX0 0x0874
#define CG_CTRL_VAL_PHYCLK_PCIE_WIFI1_RX0 0x0878
#define CG_CTRL_VAL_PHYCLK_PCIE_WIFI0_DIG_REFCLK 0x087C
#define CG_CTRL_VAL_PHYCLK_PCIE_WIFI1_DIG_REFCLK 0x0880
#define CG_CTRL_VAL_PHYCLK_UFS_LINK_SDCARD_RX_PWM_CLK 0x0884
#define CG_CTRL_VAL_PHYCLK_UFS_LINK_SDCARD_TX_PWM_CLK 0x0888
#define CG_CTRL_VAL_PHYCLK_UFS_LINK_SDCARD_REFCLK_OUT_SOC 0x088C
#define CG_CTRL_VAL_SCLK_PROMISE_FSYS1 0x0890
#define CLKOUT_CMU_FSYS1 0x0C00
#define CLKOUT_CMU_FSYS1_DIV_STAT 0x0C04
#define FSYS1_SFR_IGNORE_REQ_SYSCLK 0x0D04
#define CMU_FSYS1_SPARE0 0x0D08
#define CMU_FSYS1_SPARE1 0x0D0C
#define CG_CTRL_MAN_ACLK_FSYS1_200 0x1800
#define CG_CTRL_MAN_PCLK_HPM_APBIF_FSYS1 0x1820
#define CG_CTRL_MAN_PCLK_COMBO_PHY_WIFI 0x1824
#define CG_CTRL_MAN_SCLK_MMC2 0x1840
#define CG_CTRL_MAN_SCLK_UFSUNIPRO_SDCARD 0x1844
#define CG_CTRL_MAN_SCLK_UFSUNIPRO_SDCARD_CFG 0x1848
#define CG_CTRL_MAN_SCLK_FSYS1_PCIE0_PHY 0x184C
#define CG_CTRL_MAN_SCLK_FSYS1_PCIE1_PHY 0x1850
#define CG_CTRL_MAN_SCLK_PCIE_LINK_WIFI0 0x1854
#define CG_CTRL_MAN_SCLK_PCIE_LINK_WIFI1 0x1858
#define CG_CTRL_MAN_PHYCLK_UFS_LINK_SDCARD_TX0_SYMBOL 0x185C
#define CG_CTRL_MAN_PHYCLK_UFS_LINK_SDCARD_RX0_SYMBOL 0x1860
#define CG_CTRL_MAN_PHYCLK_PCIE_WIFI0_TX0 0x186C
#define CG_CTRL_MAN_PHYCLK_PCIE_WIFI0_RX0 0x1870
#define CG_CTRL_MAN_PHYCLK_PCIE_WIFI1_TX0 0x1874
#define CG_CTRL_MAN_PHYCLK_PCIE_WIFI1_RX0 0x1878
#define CG_CTRL_MAN_PHYCLK_PCIE_WIFI0_DIG_REFCLK 0x187C
#define CG_CTRL_MAN_PHYCLK_PCIE_WIFI1_DIG_REFCLK 0x1880
#define CG_CTRL_MAN_PHYCLK_UFS_LINK_SDCARD_RX_PWM_CLK 0x1884
#define CG_CTRL_MAN_PHYCLK_UFS_LINK_SDCARD_TX_PWM_CLK 0x1888
#define CG_CTRL_MAN_PHYCLK_UFS_LINK_SDCARD_REFCLK_OUT_SOC 0x188C
#define CG_CTRL_MAN_SCLK_PROMISE_FSYS1 0x1890
#define CG_CTRL_STAT_ACLK_FSYS1_200_0 0x1C00
#define CG_CTRL_STAT_ACLK_FSYS1_200_1 0x1C04
#define CG_CTRL_STAT_ACLK_FSYS1_200_2 0x1C08
#define CG_CTRL_STAT_ACLK_FSYS1_200_3 0x1C0C
#define CG_CTRL_STAT_ACLK_FSYS1_200_4 0x1C10
#define CG_CTRL_STAT_PCLK_HPM_APBIF_FSYS1 0x1C20
#define CG_CTRL_STAT_PCLK_COMBO_PHY_WIFI 0x1C24
#define CG_CTRL_STAT_SCLK_MMC2 0x1C40
#define CG_CTRL_STAT_SCLK_UFSUNIPRO_SDCARD 0x1C44
#define CG_CTRL_STAT_SCLK_UFSUNIPRO_SDCARD_CFG 0x1C48
#define CG_CTRL_STAT_SCLK_FSYS1_PCIE0_PHY 0x1C4C
#define CG_CTRL_STAT_SCLK_FSYS1_PCIE1_PHY 0x1C50
#define CG_CTRL_STAT_SCLK_PCIE_LINK_WIFI0 0x1C54
#define CG_CTRL_STAT_SCLK_PCIE_LINK_WIFI1 0x1C58
#define CG_CTRL_STAT_PHYCLK_UFS_LINK_SDCARD_TX0_SYMBOL 0x1C5C
#define CG_CTRL_STAT_PHYCLK_UFS_LINK_SDCARD_RX0_SYMBOL 0x1C60
#define CG_CTRL_STAT_PHYCLK_PCIE_WIFI0_TX0 0x1C6C
#define CG_CTRL_STAT_PHYCLK_PCIE_WIFI0_RX0 0x1C70
#define CG_CTRL_STAT_PHYCLK_PCIE_WIFI1_TX0 0x1C74
#define CG_CTRL_STAT_PHYCLK_PCIE_WIFI1_RX0 0x1C78
#define CG_CTRL_STAT_PHYCLK_PCIE_WIFI0_DIG_REFCLK 0x1C7C
#define CG_CTRL_STAT_PHYCLK_PCIE_WIFI1_DIG_REFCLK 0x1C80
#define CG_CTRL_STAT_PHYCLK_UFS_LINK_SDCARD_RX_PWM_CLK 0x1C84
#define CG_CTRL_STAT_PHYCLK_UFS_LINK_SDCARD_TX_PWM_CLK 0x1C88
#define CG_CTRL_STAT_PHYCLK_UFS_LINK_SDCARD_REFCLK_OUT_SOC 0x1C8C
#define CG_CTRL_STAT_SCLK_PROMISE_FSYS1 0x1C90
#define QCH_CTRL_AXI_LH_ASYNC_MI_TOP_FSYS1 0x2000
#define QCH_CTRL_CMU_FSYS1 0x2004
#define QCH_CTRL_PMU_FSYS1 0x2008
#define QCH_CTRL_SYSREG_FSYS1 0x200C
#define QCH_CTRL_MMC2 0x2010
#define QCH_CTRL_UFS_LINK_SDCARD 0x2014
#define QCH_CTRL_PPMU_FSYS1 0x2018
#define QCH_CTRL_ACEL_LH_ASYNC_SI_TOP_FSYS1 0x201C
#define QCH_CTRL_PCIE_RC_LINK_WIFI0_SLV 0x2020
#define QCH_CTRL_PCIE_RC_LINK_WIFI0_DBI 0x2024
#define QCH_CTRL_PCIE_RC_LINK_WIFI1_SLV 0x2028
#define QCH_CTRL_PCIE_RC_LINK_WIFI1_DBI 0x202C
#define QSTATE_CTRL_SROMC_FSYS1 0x241C
#define QSTATE_CTRL_GPIO_FSYS1 0x2420
#define QSTATE_CTRL_HPM_APBIF_FSYS1 0x2424
#define QSTATE_CTRL_PROMISE_FSYS1 0x2428
#define QSTATE_CTRL_PCIE_RC_LINK_WIFI0 0x242C
#define QSTATE_CTRL_PCIE_RC_LINK_WIFI1 0x2430
#define QSTATE_CTRL_PCIE_PCS_WIFI0 0x2434
#define QSTATE_CTRL_PCIE_PCS_WIFI1 0x2438
#define QSTATE_CTRL_PCIE_PHY_FSYS1_WIFI0 0x243C
#define QSTATE_CTRL_PCIE_PHY_FSYS1_WIFI1 0x2440
#define QSTATE_CTRL_UFS_LINK_SDCARD 0x2444

/* PCIE_PLL */
static const struct samsung_pll_rate_table
	exynos8890_pcie_pll_rates[] __initconst = {
		PLL_36XX_RATE(26 * MHZ, 100000000U, 800, 13, 4, 0),
		{ /* sentinel */ }
	};

static const struct samsung_pll_clock fsys1_pll_clks[] __initconst = {
	PLL(pll_1431x, 0, "pcie_pll", "oscclk", PCIE_PLL_LOCK, PCIE_PLL_CON0,
	    exynos8890_pcie_pll_rates),
};

static const struct samsung_fixed_rate_clock fsys1_fixed_clks[] __initconst = {
	FRATE(0, "phyclk_ufs_link_sdcard_tx0_symbol_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_ufs_link_sdcard_rx0_symbol_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_pcie_wifi0_tx0_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_pcie_wifi0_rx0_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_pcie_wifi1_tx0_phy", NULL, 0, 250000000),
	FRATE(0, "phyclk_pcie_wifi1_rx0_phy", NULL, 0, 250000000),
	FRATE(0, "phyclk_pcie_wifi0_dig_refclk_phy", NULL, 0, 30000000),
	FRATE(0, "phyclk_pcie_wifi1_dig_refclk_phy", NULL, 0, 30000000),
};

PNAME(fsys1_mux_aclk_fsys1_200_user_p) = { "oscclk",
					   "top_gate_aclk_fsys1_200" };
PNAME(fsys1_mux_sclk_fsys1_mmc2_user_p) = { "oscclk",
					    "top_gate_sclk_fsys1_mmc2" };
PNAME(fsys1_mux_sclk_fsys1_ufsunipro_sdcard_user_p) = {
	"oscclk", "top_gate_sclk_fsys1_ufsunipro20"
};
PNAME(fsys1_mux_sclk_fsys1_ufsunipro_sdcard_cfg_user_p) = {
	"oscclk", "top_gate_sclk_fsys1_ufsunipro_cfg"
};
PNAME(fsys1_mux_sclk_fsys1_pcie_phy_user_p) = {
	"oscclk", "top_gate_sclk_fsys1_pcie_phy"
};
PNAME(fsys1_mux_pcie_pll_p) = { "oscclk", "pcie_pll" };
PNAME(fsys1_mux_phyclk_ufs_link_sdcard_tx0_symbol_user_p) = {
	"oscclk", "phyclk_ufs_link_sdcard_tx0_symbol_phy"
};
PNAME(fsys1_mux_phyclk_ufs_link_sdcard_rx0_symbol_user_p) = {
	"oscclk", "phyclk_ufs_link_sdcard_rx0_symbol_phy"
};
PNAME(fsys1_mux_phyclk_pcie_wifi0_tx0_user_p) = { "oscclk",
						  "phyclk_pcie_wifi0_tx0_phy" };
PNAME(fsys1_mux_phyclk_pcie_wifi0_rx0_user_p) = { "oscclk",
						  "phyclk_pcie_wifi0_rx0_phy" };
PNAME(fsys1_mux_phyclk_pcie_wifi1_tx0_user_p) = { "oscclk",
						  "phyclk_pcie_wifi1_tx0_phy" };
PNAME(fsys1_mux_phyclk_pcie_wifi1_rx0_user_p) = { "oscclk",
						  "phyclk_pcie_wifi1_rx0_phy" };
PNAME(fsys1_mux_phyclk_pcie_wifi0_dig_refclk_user_p) = {
	"oscclk", "phyclk_pcie_wifi0_dig_refclk_phy"
};
PNAME(fsys1_mux_phyclk_pcie_wifi1_dig_refclk_user_p) = {
	"oscclk", "phyclk_pcie_wifi1_dig_refclk_phy"
};

static const struct samsung_mux_clock fsys1_mux_clks[] __initconst = {
	MUX(0, "fsys1_mux_aclk_fsys1_200_user", fsys1_mux_aclk_fsys1_200_user_p,
	    CLK_CON_MUX_ACLK_FSYS1_200_USER, 12, 1),
	MUX(0, "fsys1_mux_sclk_fsys1_mmc2_user",
	    fsys1_mux_sclk_fsys1_mmc2_user_p, CLK_CON_MUX_SCLK_FSYS1_MMC2_USER,
	    12, 1),
	MUX(0, "fsys1_mux_sclk_fsys1_ufsunipro_sdcard_user",
	    fsys1_mux_sclk_fsys1_ufsunipro_sdcard_user_p,
	    CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO_SDCARD_USER, 12, 1),
	MUX(0, "fsys1_mux_sclk_fsys1_ufsunipro_sdcard_cfg_user",
	    fsys1_mux_sclk_fsys1_ufsunipro_sdcard_cfg_user_p,
	    CLK_CON_MUX_SCLK_FSYS1_UFSUNIPRO_SDCARD_CFG_USER, 12, 1),
	MUX(0, "fsys1_mux_sclk_fsys1_pcie_phy_user",
	    fsys1_mux_sclk_fsys1_pcie_phy_user_p,
	    CLK_CON_MUX_SCLK_FSYS1_PCIE_PHY_USER, 12, 1),
	MUX(0, "fsys1_mux_pcie_pll", fsys1_mux_pcie_pll_p, CLK_CON_MUX_PCIE_PLL,
	    12, 1),
	MUX(0, "fsys1_mux_phyclk_ufs_link_sdcard_tx0_symbol_user",
	    fsys1_mux_phyclk_ufs_link_sdcard_tx0_symbol_user_p,
	    CLK_CON_MUX_PHYCLK_UFS_LINK_SDCARD_TX0_SYMBOL_USER, 12, 1),
	MUX(0, "fsys1_mux_phyclk_ufs_link_sdcard_rx0_symbol_user",
	    fsys1_mux_phyclk_ufs_link_sdcard_rx0_symbol_user_p,
	    CLK_CON_MUX_PHYCLK_UFS_LINK_SDCARD_RX0_SYMBOL_USER, 12, 1),
	MUX(0, "fsys1_mux_phyclk_pcie_wifi0_tx0_user",
	    fsys1_mux_phyclk_pcie_wifi0_tx0_user_p,
	    CLK_CON_MUX_PHYCLK_PCIE_WIFI0_TX0_USER, 12, 1),
	MUX(0, "fsys1_mux_phyclk_pcie_wifi0_rx0_user",
	    fsys1_mux_phyclk_pcie_wifi0_rx0_user_p,
	    CLK_CON_MUX_PHYCLK_PCIE_WIFI0_RX0_USER, 12, 1),
	MUX(0, "fsys1_mux_phyclk_pcie_wifi1_tx0_user",
	    fsys1_mux_phyclk_pcie_wifi1_tx0_user_p,
	    CLK_CON_MUX_PHYCLK_PCIE_WIFI1_TX0_USER, 12, 1),
	MUX(0, "fsys1_mux_phyclk_pcie_wifi1_rx0_user",
	    fsys1_mux_phyclk_pcie_wifi1_rx0_user_p,
	    CLK_CON_MUX_PHYCLK_PCIE_WIFI1_RX0_USER, 12, 1),
	MUX(0, "fsys1_mux_phyclk_pcie_wifi0_dig_refclk_user",
	    fsys1_mux_phyclk_pcie_wifi0_dig_refclk_user_p,
	    CLK_CON_MUX_PHYCLK_PCIE_WIFI0_DIG_REFCLK_USER, 12, 1),
	MUX(0, "fsys1_mux_phyclk_pcie_wifi1_dig_refclk_user",
	    fsys1_mux_phyclk_pcie_wifi1_dig_refclk_user_p,
	    CLK_CON_MUX_PHYCLK_PCIE_WIFI1_DIG_REFCLK_USER, 12, 1),
};

static const struct samsung_gate_clock fsys1_gate_clks[] __initconst = {
	GATE(0, "fsys1_gate_aclk_axi2acel_fsys1x",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 31,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_cmu_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 30, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_combo_phy_pcs_pclk_wifi1",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 29,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_combo_phy_pcs_pclk_wifi0",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 28,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_pmu_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 26, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_ppmu_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 25, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_gpio_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 24, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_sysreg_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 23, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_sromc_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 22, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_pcie_wifi1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 21, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_pcie_wifi1_dbi",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 20,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_pcie_wifi1_slv",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 19,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_pcie_wifi1_mstr",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 18,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_ahb2axi_pcie_wifi1",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 17,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_pcie_wifi0", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 16, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_pcie_wifi0_dbi",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 15,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_pcie_wifi0_slv",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 14,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_pcie_wifi0_mstr",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 13,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_ahb2axi_pcie_wifi0",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 12,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_ppmu_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 11, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_ahb_bridge_fsys1_s4",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 10,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_axi2ahb_fsys1_s4",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 9,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_axi2apb_fsys1_s1",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 8,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_axi2ahb_fsys1_s0",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 7,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_xiu_fsys1sfrx",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 6,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_xiu_sdcardx", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 5, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_xiu_fsys1x", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 4, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_axi_lh_async_mi_top_fsys1",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 3,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_acel_lh_async_si_top_fsys1",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 2,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_ufs_link_sdcard",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_ACLK_FSYS1_200, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_aclk_mmc2", "fsys1_mux_aclk_fsys1_200_user",
	     CG_CTRL_VAL_ACLK_FSYS1_200, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_hpm_apbif_fsys1",
	     "fsys1_mux_aclk_fsys1_200_user", CG_CTRL_VAL_PCLK_HPM_APBIF_FSYS1,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_combo_phy_wifi1",
	     "fsys1_div_pclk_combo_phy_wifi", CG_CTRL_VAL_PCLK_COMBO_PHY_WIFI,
	     1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_pclk_combo_phy_wifi0",
	     "fsys1_div_pclk_combo_phy_wifi", CG_CTRL_VAL_PCLK_COMBO_PHY_WIFI,
	     0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_sclk_mmc2", "fsys1_mux_sclk_fsys1_mmc2_user",
	     CG_CTRL_VAL_SCLK_MMC2, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_sclk_ufsunipro_sdcard",
	     "fsys1_mux_sclk_fsys1_ufsunipro_sdcard_user",
	     CG_CTRL_VAL_SCLK_UFSUNIPRO_SDCARD, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_sclk_ufsunipro_sdcard_cfg",
	     "fsys1_mux_sclk_fsys1_ufsunipro_sdcard_cfg_user",
	     CG_CTRL_VAL_SCLK_UFSUNIPRO_SDCARD_CFG, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_sclk_pcie_link_wifi0", "oscclk",
	     CG_CTRL_VAL_SCLK_PCIE_LINK_WIFI0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_sclk_pcie_link_wifi1", "oscclk",
	     CG_CTRL_VAL_SCLK_PCIE_LINK_WIFI1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_ufs_link_sdcard_tx0_symbol",
	     "fsys1_mux_phyclk_ufs_link_sdcard_tx0_symbol_user",
	     CG_CTRL_VAL_PHYCLK_UFS_LINK_SDCARD_TX0_SYMBOL, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_ufs_link_sdcard_rx0_symbol",
	     "fsys1_mux_phyclk_ufs_link_sdcard_rx0_symbol_user",
	     CG_CTRL_VAL_PHYCLK_UFS_LINK_SDCARD_RX0_SYMBOL, 0,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_pcie_wifi0_tx0",
	     "fsys1_mux_phyclk_pcie_wifi0_tx0_user",
	     CG_CTRL_VAL_PHYCLK_PCIE_WIFI0_TX0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_pcie_wifi0_rx0",
	     "fsys1_mux_phyclk_pcie_wifi0_rx0_user",
	     CG_CTRL_VAL_PHYCLK_PCIE_WIFI0_RX0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_pcie_wifi1_tx0",
	     "fsys1_mux_phyclk_pcie_wifi1_tx0_user",
	     CG_CTRL_VAL_PHYCLK_PCIE_WIFI1_TX0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_pcie_wifi1_rx0",
	     "fsys1_mux_phyclk_pcie_wifi1_rx0_user",
	     CG_CTRL_VAL_PHYCLK_PCIE_WIFI1_RX0, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_pcie_wifi0_dig_refclk",
	     "fsys1_mux_phyclk_pcie_wifi0_dig_refclk_user",
	     CG_CTRL_VAL_PHYCLK_PCIE_WIFI0_DIG_REFCLK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_phyclk_pcie_wifi1_dig_refclk",
	     "fsys1_mux_phyclk_pcie_wifi1_dig_refclk_user",
	     CG_CTRL_VAL_PHYCLK_PCIE_WIFI1_DIG_REFCLK, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_gate_sclk_promise_fsys1", "top_gate_sclk_promise_int",
	     CG_CTRL_VAL_SCLK_PROMISE_FSYS1, 0, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_sromc_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     QSTATE_CTRL_SROMC_FSYS1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_gpio_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     QSTATE_CTRL_GPIO_FSYS1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_hpm_apbif_fsys1", "fsys1_mux_aclk_fsys1_200_user",
	     QSTATE_CTRL_HPM_APBIF_FSYS1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_promise_fsys1", "top_gate_sclk_promise_int",
	     QSTATE_CTRL_PROMISE_FSYS1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_pcie_rc_link_wifi0",
	     "fsys1_mux_aclk_fsys1_200_user", QSTATE_CTRL_PCIE_RC_LINK_WIFI0, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_pcie_rc_link_wifi1",
	     "fsys1_mux_aclk_fsys1_200_user", QSTATE_CTRL_PCIE_RC_LINK_WIFI1, 1,
	     CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_pcie_pcs_wifi0", "fsys1_mux_aclk_fsys1_200_user",
	     QSTATE_CTRL_PCIE_PCS_WIFI0, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_pcie_pcs_wifi1", "fsys1_mux_aclk_fsys1_200_user",
	     QSTATE_CTRL_PCIE_PCS_WIFI1, 1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_pcie_phy_fsys1_wifi0",
	     "fsys1_mux_aclk_fsys1_200_user", QSTATE_CTRL_PCIE_PHY_FSYS1_WIFI0,
	     1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_pcie_phy_fsys1_wifi1",
	     "fsys1_mux_aclk_fsys1_200_user", QSTATE_CTRL_PCIE_PHY_FSYS1_WIFI1,
	     1, CLK_IGNORE_UNUSED, 0),
	GATE(0, "fsys1_hwacg_ufs_link_sdcard", "fsys1_mux_aclk_fsys1_200_user",
	     QSTATE_CTRL_UFS_LINK_SDCARD, 1, CLK_IGNORE_UNUSED, 0),
};

static const struct samsung_div_clock fsys1_div_clks[] __initconst = {
	DIV(0, "fsys1_div_pclk_combo_phy_wifi", "fsys1_mux_aclk_fsys1_200_user",
	    CLK_CON_DIV_PCLK_COMBO_PHY_WIFI, 0, 2),
};

static const struct samsung_cmu_info fsys1_cmu_info __initconst = {
	.pll_clks = fsys1_pll_clks,
	.nr_pll_clks = ARRAY_SIZE(fsys1_pll_clks),
	.mux_clks = fsys1_mux_clks,
	.nr_mux_clks = ARRAY_SIZE(fsys1_mux_clks),
	.div_clks = fsys1_div_clks,
	.nr_div_clks = ARRAY_SIZE(fsys1_div_clks),
	.gate_clks = fsys1_gate_clks,
	.nr_gate_clks = ARRAY_SIZE(fsys1_gate_clks),
	.fixed_clks = fsys1_fixed_clks,
	.nr_fixed_clks = ARRAY_SIZE(fsys1_fixed_clks),
	.nr_clk_ids = FSYS1_NR_CLK,
};

struct exynos8890_cmu_data {
	struct samsung_clk_reg_dump *clk_save;
	unsigned int nr_clk_save;
	const struct samsung_clk_reg_dump *clk_suspend;
	unsigned int nr_clk_suspend;

	struct clk *clk;
	struct clk **pclks;
	int nr_pclks;

	/* must be the last entry */
	struct samsung_clk_provider ctx;
};

static int __maybe_unused exynos8890_cmu_suspend(struct device *dev)
{
	struct exynos8890_cmu_data *data = dev_get_drvdata(dev);
	int i;

	samsung_clk_save(data->ctx.reg_base, data->clk_save, data->nr_clk_save);

	for (i = 0; i < data->nr_pclks; i++)
		clk_prepare_enable(data->pclks[i]);

	/* for suspend some registers have to be set to certain values */
	samsung_clk_restore(data->ctx.reg_base, data->clk_suspend,
			    data->nr_clk_suspend);

	for (i = 0; i < data->nr_pclks; i++)
		clk_disable_unprepare(data->pclks[i]);

	clk_disable_unprepare(data->clk);

	return 0;
}

static int __maybe_unused exynos8890_cmu_resume(struct device *dev)
{
	struct exynos8890_cmu_data *data = dev_get_drvdata(dev);
	int i;

	clk_prepare_enable(data->clk);

	for (i = 0; i < data->nr_pclks; i++)
		clk_prepare_enable(data->pclks[i]);

	samsung_clk_restore(data->ctx.reg_base, data->clk_save,
			    data->nr_clk_save);

	for (i = 0; i < data->nr_pclks; i++)
		clk_disable_unprepare(data->pclks[i]);

	return 0;
}

static int __init exynos8890_cmu_probe(struct platform_device *pdev)
{
	const struct samsung_cmu_info *info;
	struct exynos8890_cmu_data *data;
	struct samsung_clk_provider *ctx;
	struct device *dev = &pdev->dev;
	struct resource *res;
	void __iomem *reg_base;
	int i;

	info = of_device_get_match_data(dev);

	data = devm_kzalloc(
		dev, struct_size(data, ctx.clk_data.hws, info->nr_clk_ids),
		GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	ctx = &data->ctx;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	reg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(reg_base))
		return PTR_ERR(reg_base);

	for (i = 0; i < info->nr_clk_ids; ++i)
		ctx->clk_data.hws[i] = ERR_PTR(-ENOENT);

	ctx->clk_data.num = info->nr_clk_ids;
	ctx->reg_base = reg_base;
	ctx->dev = dev;
	spin_lock_init(&ctx->lock);

	data->clk_save =
		samsung_clk_alloc_reg_dump(info->clk_regs, info->nr_clk_regs);
	if (!data->clk_save)
		return -ENOMEM;
	data->nr_clk_save = info->nr_clk_regs;
	data->clk_suspend = info->suspend_regs;
	data->nr_clk_suspend = info->nr_suspend_regs;
	data->nr_pclks = of_clk_get_parent_count(dev->of_node);

	if (data->nr_pclks > 0) {
		data->pclks = devm_kcalloc(dev, sizeof(struct clk *),
					   data->nr_pclks, GFP_KERNEL);
		if (!data->pclks) {
			kfree(data->clk_save);
			return -ENOMEM;
		}
		for (i = 0; i < data->nr_pclks; i++) {
			struct clk *clk = of_clk_get(dev->of_node, i);

			if (IS_ERR(clk)) {
				kfree(data->clk_save);
				while (--i >= 0)
					clk_put(data->pclks[i]);
				return PTR_ERR(clk);
			}
			data->pclks[i] = clk;
		}
	}

	if (info->clk_name)
		data->clk = clk_get(dev, info->clk_name);
	clk_prepare_enable(data->clk);

	platform_set_drvdata(pdev, data);

	/*
	 * Enable runtime PM here to allow the clock core using runtime PM
	 * for the registered clocks. Additionally, we increase the runtime
	 * PM usage count before registering the clocks, to prevent the
	 * clock core from runtime suspending the device.
	 */
	pm_runtime_get_noresume(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	if (info->pll_clks)
		samsung_clk_register_pll(ctx, info->pll_clks, info->nr_pll_clks,
					 reg_base);
	if (info->mux_clks)
		samsung_clk_register_mux(ctx, info->mux_clks,
					 info->nr_mux_clks);
	if (info->div_clks)
		samsung_clk_register_div(ctx, info->div_clks,
					 info->nr_div_clks);
	if (info->gate_clks)
		samsung_clk_register_gate(ctx, info->gate_clks,
					  info->nr_gate_clks);
	if (info->fixed_clks)
		samsung_clk_register_fixed_rate(ctx, info->fixed_clks,
						info->nr_fixed_clks);
	if (info->fixed_factor_clks)
		samsung_clk_register_fixed_factor(ctx, info->fixed_factor_clks,
						  info->nr_fixed_factor_clks);

	samsung_clk_of_add_provider(dev->of_node, ctx);
	pm_runtime_put_sync(dev);

	return 0;
}

static const struct of_device_id exynos8890_cmu_of_match[] = {
	{
		.compatible = "samsung,exynos8890-cmu-aud",
		.data = &aud_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-apollo",
		.data = &apollo_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-bus0",
		.data = &bus0_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-bus1",
		.data = &bus1_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-cam0-local",
		.data = &cam0_local_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-cam0",
		.data = &cam0_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-cam1-local",
		.data = &cam1_local_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-cam1",
		.data = &cam1_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-ccore",
		.data = &ccore_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-disp0",
		.data = &disp0_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-disp1",
		.data = &disp1_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-fsys0",
		.data = &fsys0_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-fsys1",
		.data = &fsys1_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-g3d",
		.data = &g3d_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-imem",
		.data = &imem_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-isp0-local",
		.data = &isp0_local_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-isp0",
		.data = &isp0_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-isp1-local",
		.data = &isp1_local_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-isp1",
		.data = &isp1_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-mfc",
		.data = &mfc_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-mif0",
		.data = &mif0_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-mif1",
		.data = &mif1_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-mif2",
		.data = &mif2_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-mif3",
		.data = &mif3_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-mngs",
		.data = &mngs_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-mscl",
		.data = &mscl_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-peric0",
		.data = &peric0_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-peric1",
		.data = &peric1_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-peris",
		.data = &peris_cmu_info,
	},
	{
		.compatible = "samsung,exynos8890-cmu-top",
		.data = &top_cmu_info,
	},
	{},
};

static const struct dev_pm_ops exynos8890_cmu_pm_ops = {
	SET_RUNTIME_PM_OPS(exynos8890_cmu_suspend, exynos8890_cmu_resume, NULL)
		SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(pm_runtime_force_suspend,
					      pm_runtime_force_resume)
};

static struct platform_driver exynos8890_cmu_driver __refdata = {
	.driver	= {
		.name = "exynos8890-cmu",
		.of_match_table = exynos8890_cmu_of_match,
		.suppress_bind_attrs = true,
		.pm = &exynos8890_cmu_pm_ops,
	},
	.probe = exynos8890_cmu_probe,
};

static int __init exynos8890_cmu_init(void)
{
	return platform_driver_register(&exynos8890_cmu_driver);
}
core_initcall(exynos8890_cmu_init);