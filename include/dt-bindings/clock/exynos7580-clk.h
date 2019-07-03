#ifndef _DT_BINDINGS_CLOCK_EXYNOS7580_H
#define _DT_BINDINGS_CLOCK_EXYNOS7580_H

/* APL */
#define CLK_APL_PLL         1

#define APL_NR_CLK			2

/* AUD */
#define CLK_MOUT_AUD_PLL_USER   1
#define CLK_MOUT_SCLK_MI2S      2
#define CLK_MOUT_SCLK_PCM       3
#define CLK_DOUT_ACLK_133       4
#define CLK_DOUT_SCLK_MI2S      5
#define CLK_DOUT_SCLK_PCM       6
#define CLK_DOUT_SCLK_AUD_UART  7
#define CLK_DOUT_SCLK_AUDMIXER  8
#define CLK_LPASS_DMAC          9
#define CLK_LPASS_MEM           10
#define CLK_PCLK_MI2S           11
#define CLK_PUART3              12
#define CLK_SCLK_MI2S           13
#define CLK_SCLK_PCM            14
#define CLK_SUART3              15
#define CLK_AUDMIXER_SYSCLK     16
#define CLK_AUDMIXER_BCLK0      17
#define CLK_AUDMIXER_BCLK1      18
#define CLK_AUDMIXER_BCLK2      19
#define CLK_MI2S_AUD_BCLK       20
#define CLK_IOCLK_AUDIOCDCLK1   21

#define AUD_NR_CLK			    22


/* BUS0 */
#define BUS0_NR_CLK     1

/* BUS1 */
#define BUS1_NR_CLK     1

/* BUS2 */
#define BUS2_NR_CLK     1

/* CPU */
#define CLK_CPU_PLL     1

#define CPU_NR_CLK      2

/* DISP */
#define CLK_MUX_ACLK_DISP_200_USER          1
#define CLK_MUX_SCLK_DECON_INT_ECLK_USER    2
#define CLK_MUX_SCLK_DECON_INT_VCLK_USER    3
#define CLK_MUX_PHYCLK_BITCLKDIV8_USER      4
#define CLK_MUX_PHYCLK_RXCLKESC0_USER       5
#define CLK_MUX_SCLK_DECON_INT_ECLK         6
#define CLK_MUX_SCLK_DECON_INT_VCLK         7
#define CLK_DIV_PCLK_DISP_100               8
#define CLK_DIV_SCLK_DECON_INT_ECLK         9
#define CLK_DIV_SCLK_DECON_INT_VCLK         10
#define CLK_ACLK_DECON0                     11
#define CLK_ACLK_XIU_DISP1                  12
#define CLK_ACLK_SMMU_DISP_MMU              13
#define CLK_PCLK_DECON_INT                  14
#define CLK_PCLK_DSI_LINK0                  15
#define CLK_PCLK_SMMU_DISP_MMU              16
#define CLK_SCLK_DECON_INT_ECLK             17
#define CLK_SCLK_DECON_INT_VCLK             18
#define CLK_SCLK_DSI_LINK0_I_RGB_VCLK       19
#define CLK_PHYCLK_BITCLKDIV8               20
#define CLK_PHYCLK_RXCLKESC0                21
#define CLK_DISP_PLL                        22

#define DISP_NR_CLK                         23

/* FSYS */
#define CLK_FREECLK                 1
#define CLK_PHYCLK                  2
#define CLK_CLK48MOHCI              3
#define CLK_PHY_OTG                 4
#define CLK_SCLK_MMC0               5
#define CLK_SCLK_MMC1               6
#define CLK_SCLK_MMC2               7
#define CLK_USB_PLL                 8
#define CLK_PDMA0                   9
#define CLK_PDMA1                   10
#define CLK_USBHOST20               11
#define CLK_ACLK_MMC0               12
#define CLK_ACLK_MMC1               13
#define CLK_ACLK_MMC2               14
#define CLK_OTG_ACLK                15
#define CLK_OTG_HCLK                16
#define CLK_UPSIZER_OTG             17
#define CLK_XIU_D_FSYS1             18
#define CLK_AHB_USBHS               19
#define CLK_AHB2AXI_USBHS           20
#define CLK_UPSIZER_FSYS1           21
#define CLK_UPSIZER_AHB_USBHS       22

#define FSYS_NR_CLK                 23

/* G3D */
#define CLK_G3D_PLL         1
#define CLK_MOUT_G3D        2
#define CLK_DOUT_ACLK_G3D   3
#define CLK_G3D             4

#define G3D_NR_CLK          5

/* IMEM */
#define IMEM_NR_CLK         1

/* ISP */
#define CLK_MOUT_SCLK_CPU_ISP_CLKIN_A           1
#define CLK_MOUT_SCLK_CPU_ISP_CLKIN_B           2
#define CLK_MOUT_ACLK_CSI_LINK1_75              3
#define CLK_MOUT_ACLK_CSI_LINK1_75_B            4
#define CLK_MOUT_ACLK_FIMC_ISP_450_A            5
#define CLK_MOUT_ACLK_FIMC_ISP_450_B            6
#define CLK_MOUT_ACLK_FIMC_ISP_450_C            7
#define CLK_MOUT_ACLK_FIMC_ISP_450_D            8
#define CLK_MOUT_ACLK_FIMC_FD_300               9
#define CLK_MOUT_ACLK_LINK_DATA_A               10
#define CLK_MOUT_ACLK_LINK_DATA_B               11
#define CLK_MOUT_ACLK_LINK_DATA_C               12
#define CLK_MOUT_ACLK_ISP_400_USER              13
#define CLK_MOUT_ACLK_ISP_333_USER              14
#define CLK_MOUT_ACLK_ISP_266_USER              15
#define CLK_MOUT_SCLK_SPI0_ISP_EXT_CLK_USER     16
#define CLK_MOUT_SCLK_SPI1_ISP_EXT_CLK_USER     17
#define CLK_MOUT_SCLK_UART_ISP_EXT_CLK_USER     18
#define CLK_MOUT_PHYCLK_CSI_LINK0_RX_USER       19
#define CLK_MOUT_PHYCLK_CSI_LINK1_RX_USER       20
#define CLK_DOUT_ISP_PLL_DIV2                   21
#define CLK_DOUT_ISP_PLL_DIV3                   22
#define CLK_DOUT_SCLK_CPU_ISP_CLKIN             23
#define CLK_DOUT_SCLK_CPU_ISP_ATCLKIN           24
#define CLK_DOUT_SCLK_CPU_ISP_PCLKDBG           25
#define CLK_DOUT_PCLK_CSI_LINK0_225             26
#define CLK_DOUT_ACLK_CSI_LINK1_75              27
#define CLK_DOUT_PCLK_CSI_LINK1_37              28
#define CLK_DOUT_ACLK_FIMC_ISP_450              29
#define CLK_DOUT_PCLK_FIMC_ISP_225              30
#define CLK_DOUT_ACLK_FIMC_FD_300               31
#define CLK_DOUT_PCLK_FIMC_FD_150               32
#define CLK_DOUT_ACLK_ISP_266                   33
#define CLK_DOUT_ACLK_ISP_133                   34
#define CLK_DOUT_ACLK_ISP_67                    35
#define CLK_DOUT_ACLK_LINK_DATA                 36
#define CLK_ACLK_CSI_LINK0_1_WRAP_CLK           37
#define CLK_ACLK_FIMC_BNS_ACLK                  38
#define CLK_ACLK_CSI_LINK1_1_WRAP_CLK           39
#define CLK_ACLK_FIMC_BNS_L_ACLK                40
#define CLK_ACLK_ISP_1                          41
#define CLK_ACLK_SASYNC_FIMC_BNS_M_TO_BUS       42
#define CLK_ACLK_MASYNC_FIMC_BNS_M_TO_BUS       43
#define CLK_ACLK_SASYNC_FIMC_BNS_L_TO_BUS       44
#define CLK_ACLK_MASYNC_FIMC_BNS_L_TO_BUS       45
#define CLK_ISP_PLL                             46

#define ISP_NR_CLK                              47

/* MFCMSCL */
#define CLK_MOUT_ACLK_MSCL_400_USER     1
#define CLK_MOUT_ACLK_MFC_266_USER      2
#define CLK_GATE_ACLK_M2M_SCALER0       3
#define CLK_GATE_ACLK_M2M_SCALER1       4
#define CLK_ACLK_JPEG                   5
#define CLK_ACLK_SMMU_MSCL_MMU          6
#define CLK_ACLK_AHB2APB_MFC            7
#define CLK_PCLK_AHB_JPEG               8
#define CLK_GATE_PCLK_M2M_SCALER0       9
#define CLK_GATE_PCLK_M2M_SCALER1       10
#define CLK_PCLK_SMMU_MSCL_MMU          11
#define CLK_PCLK_SMMU_MFC_MMU           12
#define CLK_PCLK_SASYNC_AHB2APB_MFC     13
#define CLK_PCLK_MFC                    14
#define CLK_ACLK_MFC                    15
#define CLK_ACLK_BUS_D_MFC              16
#define CLK_ACLK_UPSIZER_MFC            17
#define CLK_ACLK_SMMU_MFC_MMU           18
#define CLK_ACLK_SASYNC_AHB2APB_MFC     19

#define MFCMSCL_NR_CLK                  20

/* MIF */
#define CLK_MUX_ACLK_DISP_200               1
#define CLK_MUX_SCLK_DISP_DECON_INT_ECLK_A  2
#define CLK_MUX_SCLK_DISP_DECON_INT_ECLK_B  3
#define CLK_MUX_SCLK_DISP_DECON_INT_VCLK_A  4
#define CLK_MUX_SCLK_DISP_DECON_INT_VCLK_B  5
#define CLK_DIV_ACLK_DISP_200               6
#define CLK_DIV_SCLK_DISP_DECON_INT_ECLK    7
#define CLK_DIV_SCLK_DISP_DECON_INT_VCLK    8
#define CLK_ACLK_DISP_200                   9
#define CLK_SCLK_DISP_DECON_INT_ECLK        10
#define CLK_SCLK_DISP_DECON_INT_VCLK        11
#define CLK_HSI2C3                          12
#define CLK_MEM0_PLL                        13
#define CLK_BUS_PLL                         14
#define CLK_SCLK_MEDIA_PLL_TOP              15

#define MIF_NR_CLK                          16

/* TOP */
#define CLK_MOUT_SCLK_ISP_SPI0_EXT_CLK      1
#define CLK_MOUT_SCLK_ISP_SPI1_EXT_CLK      2
#define CLK_MOUT_SCLK_ISP_UART_EXT_UCLK     3
#define CLK_MOUT_SCLK_ISP_SENSOR0           4
#define CLK_MOUT_SCLK_ISP_SENSOR1           5
#define CLK_MUX_BUS_PLL_TOP_USER            6
#define CLK_DOUT_ACLK_MFCMSCL_266           7
#define CLK_DOUT_SCLK_ISP_SPI0_EXT_CLK_A    8
#define CLK_DOUT_SCLK_ISP_SPI0_EXT_CLK_B    9
#define CLK_DOUT_SCLK_ISP_SPI1_EXT_CLK_A    10
#define CLK_DOUT_SCLK_ISP_SPI1_EXT_CLK_B    11
#define CLK_DOUT_SCLK_ISP_UART_EXT_UCLK     12
#define CLK_DOUT_SCLK_ISP_SENSOR0_A         13
#define CLK_DOUT_SCLK_ISP_SENSOR0_B         14
#define CLK_DOUT_SCLK_ISP_SENSOR1_A         15
#define CLK_DOUT_SCLK_ISP_SENSOR1_B         16
#define CLK_DOUT_MMC0_A                     17
#define CLK_DOUT_MMC0_B                     18
#define CLK_DOUT_MMC1_A                     19
#define CLK_DOUT_MMC1_B                     20
#define CLK_DOUT_MMC2_A                     21
#define CLK_DOUT_MMC2_B                     22
#define CLK_BAUD0                           23
#define CLK_BAUD1                           24
#define CLK_BAUD2                           25
#define CLK_ACLK_G3D_400                    26
#define CLK_ACLK_MFCMSCL_266                27
#define CLK_ACLK_MFCMSCL_400                28
#define CLK_SCLK_FSYS_MMC0                  29
#define CLK_SCLK_FSYS_MMC1                  30
#define CLK_SCLK_FSYS_MMC2                  31
#define CLK_SCLK_ISP_SENSOR0                32
#define CLK_SCLK_ISP_SENSOR1                33
#define CLK_SCLK_SPI0                       34
#define CLK_SCLK_SPI1                       35
#define CLK_SCLK_SPI2                       36
#define CLK_AUD_PLL                         37
#define CLK_DOUT_ACLK_BUS1_400              38
#define CLK_DOUT_ACLK_BUS0_400              39
#define CLK_DOUT_ACLK_BUS2_400              40
#define CLK_ACLK_FSYS_200                   41
#define CLK_SCLK_PERI_I2S_I2SCODCLKI        42
#define CLK_DOUT_ACLK_PERI_66               43
#define CLK_SCLK_PERI_UART0_EXT_UCLK        44
#define CLK_SCLK_PERI_UART1_EXT_UCLK        45
#define CLK_SCLK_PERI_UART2_EXT_UCLK        46
#define CLK_ACLK_IMEM_266                   47
#define CLK_ACLK_IMEM_200                   48
#define CLK_ACLK_ISP_400                    49
#define CLK_ACLK_ISP_333                    50
#define CLK_ACLK_ISP_266                    51
#define CLK_SCLK_ISP_SPI0_EXT_CLK           52
#define CLK_SCLK_ISP_SPI1_EXT_CLK           53
#define CLK_SCLK_ISP_UART_EXT_CLK           54

#define TOP_NR_CLK                          55

/* PERI */
#define CLK_MCT         1
#define CLK_PCLK_WDT    2
#define CLK_GATE_RTC    3
#define CLK_I2C1        4
#define CLK_I2C2        5
#define CLK_I2C3        6
#define CLK_PUART0      7
#define CLK_PUART1      8
#define CLK_PUART2      9
#define CLK_PCLK_ADCIF  10
#define CLK_PCLK_SPI0   11
#define CLK_PCLK_SPI1   12
#define CLK_PCLK_SPI2   13
#define CLK_PCLK_PWM    14
#define CLK_HSI2C0      15
#define CLK_HSI2C1      16
#define CLK_HSI2C2      17
#define CLK_I2C0        18
#define CLK_SUART0      19
#define CLK_SUART1      20
#define CLK_SUART2      21

#define PERI_NR_CLK     22

#endif /* _DT_BINDINGS_CLOCK_EXYNOS7580_H */
