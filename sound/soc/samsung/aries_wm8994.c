// SPDX-License-Identifier: GPL-2.0+
//
// Wolfson wm8994 machine driver for Aries board

#include <linux/extcon.h>
#include <linux/iio/consumer.h>
#include <linux/input-event-codes.h>
#include <linux/mfd/wm8994/registers.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <sound/jack.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include "i2s.h"
#include "../codecs/wm8994.h"

/* Aries has a 24MHZ crystal attached to WM8994 */
#define ARIES_MCLK1_FREQ 24000000
#define ARIES_MCLK2_FREQ 32768

/* Support up to 5 different jack detection zones */
#define MAX_ZONES 5

struct jack_zone {
	u32 adc_high;
	u32 delay_ms;
	u32 check_count;
	u32 jack_type;
};

struct aries_wm8994_data {
	struct extcon_dev *usb_extcon;
	struct gpio_desc *gpio_earpath_sel;
	struct regulator *reg_main_micbias;
	struct regulator *reg_headset_micbias;
	struct gpio_desc *gpio_headset_detect;
	struct gpio_desc *gpio_headset_key;
	struct iio_channel *adc;
	struct jack_zone zones[MAX_ZONES];
	int num_zones;
	bool aif2_slave;
};

/* USB dock */
static struct snd_soc_jack aries_dock;

static struct snd_soc_jack_pin dock_pins[] = {
	{
		.pin = "LINE",
		.mask = SND_JACK_LINEOUT,
	},
};

static int aries_extcon_notifier(struct notifier_block *this,
				 unsigned long connected, void *_cmd)
{
	if (connected)
		snd_soc_jack_report(&aries_dock, SND_JACK_LINEOUT,
				SND_JACK_LINEOUT);
	else
		snd_soc_jack_report(&aries_dock, 0, SND_JACK_LINEOUT);

	return NOTIFY_DONE;
}

static struct notifier_block aries_extcon_notifier_block = {
	.notifier_call = aries_extcon_notifier,
};

/* Headset jack */
static struct snd_soc_jack aries_headset;

static struct snd_soc_jack_pin jack_pins[] = {
	{
		.pin = "HP",
		.mask = SND_JACK_HEADPHONE,
	}, {
		.pin = "Headset Mic",
		.mask = SND_JACK_MICROPHONE,
	},
};

static int jack_adc_check(void *data)
{
	struct aries_wm8994_data *priv = (struct aries_wm8994_data *) data;
	int adc, ret, i;
	int count[MAX_ZONES] = { 0 };

	while (gpiod_get_value_cansleep(priv->gpio_headset_detect)) {
		ret = iio_read_channel_raw(priv->adc, &adc);
		if (ret < 0) {
			pr_err("%s failed to read adc: %d", __func__, ret);
			return 0;
		}

		/* determine the type of headset based on the
		 * adc value.  An adc value can fall in various
		 * ranges or zones.  Within some ranges, the type
		 * can be returned immediately.  Within others, the
		 * value is considered unstable and we need to sample
		 * a few more types (up to the limit determined by
		 * the range) before we return the type for that range.
		 */
		for (i = 0; i < priv->num_zones; i++) {
			if (adc <= priv->zones[i].adc_high) {
				if (++count[i] > priv->zones[i].check_count)
					return priv->zones[i].jack_type;

				msleep(priv->zones[i].delay_ms);
				break;
			}
		}
	}

	/* jack was unplugged */
	return 0;
}

static int jack_button_check(void *data)
{
	struct aries_wm8994_data *priv = (struct aries_wm8994_data *) data;

	if (gpiod_get_value_cansleep(priv->gpio_headset_key) &&
			aries_headset.status & SND_JACK_MICROPHONE)
		return SND_JACK_BTN_0;

	return 0;
}

static struct snd_soc_jack_gpio jack_gpios[] = {
	{
		.name = "Headset Detect",
		.report = SND_JACK_HEADPHONE | SND_JACK_MICROPHONE,
		.debounce_time = 200,
		.jack_status_check = jack_adc_check,
	},
	{
		.name = "Media Button",
		.report = SND_JACK_BTN_0,
		.debounce_time  = 30,
		.jack_status_check = jack_button_check,
	},
};

static int aries_main_bias(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_card *card = w->dapm->card;
	struct aries_wm8994_data *priv = snd_soc_card_get_drvdata(card);
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = regulator_enable(priv->reg_main_micbias);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = regulator_disable(priv->reg_main_micbias);
		break;
	}

	return ret;
}

static int aries_headset_bias(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_card *card = w->dapm->card;
	struct aries_wm8994_data *priv = snd_soc_card_get_drvdata(card);
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		gpiod_set_value_cansleep(priv->gpio_earpath_sel, 1);
		ret = regulator_enable(priv->reg_headset_micbias);
		break;
	case SND_SOC_DAPM_POST_PMD:
		gpiod_set_value_cansleep(priv->gpio_earpath_sel, 0);
		ret = regulator_disable(priv->reg_headset_micbias);
	}

	return ret;
}

static const struct snd_kcontrol_new aries_controls[] = {
	SOC_DAPM_PIN_SWITCH("HP"),
	SOC_DAPM_PIN_SWITCH("SPK"),
	SOC_DAPM_PIN_SWITCH("RCV"),
	SOC_DAPM_PIN_SWITCH("LINE"),

	SOC_DAPM_PIN_SWITCH("Main Mic"),
	SOC_DAPM_PIN_SWITCH("Headset Mic"),

	SOC_DAPM_PIN_SWITCH("FM In"),
};

static const struct snd_soc_dapm_widget aries_dapm_widgets[] = {
	SND_SOC_DAPM_HP("HP", NULL),

	SND_SOC_DAPM_SPK("SPK", NULL),
	SND_SOC_DAPM_SPK("RCV", NULL),

	SND_SOC_DAPM_LINE("LINE", NULL),

	SND_SOC_DAPM_MIC("Main Mic", aries_main_bias),
	SND_SOC_DAPM_MIC("Headset Mic", aries_headset_bias),

	SND_SOC_DAPM_LINE("FM In", NULL),
};

static int aries_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int pll_out;
	int ret;

	/* AIF1CLK should be >=3MHz for optimal performance */
	if (params_width(params) == 24)
		pll_out = params_rate(params) * 384;
	else if (params_rate(params) == 8000 || params_rate(params) == 11025)
		pll_out = params_rate(params) * 512;
	else
		pll_out = params_rate(params) * 256;

	/* set the codec FLL */
	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL1, WM8994_FLL_SRC_MCLK1,
			ARIES_MCLK1_FREQ, pll_out);
	if (ret < 0)
		return ret;

	/* set the codec system clock */
	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_FLL1,
			pll_out, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	return 0;
}

static int aries_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret;

	/* set system clock to MCLK1 as it is always on */
	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK1,
			ARIES_MCLK1_FREQ, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	/* disable FLL1 */
	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL1, WM8994_SYSCLK_MCLK1,
				    0, 0);

	return ret;
}

static struct snd_soc_ops aries_ops = {
	.hw_params = aries_hw_params,
	.hw_free = aries_hw_free,
};

static int aries_modem_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct aries_wm8994_data *priv = snd_soc_card_get_drvdata(card);
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int pll_in, pll_out;
	int mclk, fmt, ret;

	if (params_rate(params) != 8000)
		return -EINVAL;

	pll_out = params_rate(params) * 256;

	if (priv->aif2_slave) {
		mclk = WM8994_FLL_SRC_MCLK2;
		pll_in = ARIES_MCLK2_FREQ;
		fmt = SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_IB_NF |
				SND_SOC_DAIFMT_CBS_CFS;
	} else {
		mclk = WM8994_FLL_SRC_MCLK1;
		pll_in = ARIES_MCLK1_FREQ;
		fmt = SND_SOC_DAIFMT_LEFT_J | SND_SOC_DAIFMT_IB_IF |
			SND_SOC_DAIFMT_CBM_CFM;
	}

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (ret < 0)
		return ret;

	/* set the codec FLL */
	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL2, mclk,
				  pll_in, pll_out);
	if (ret < 0)
		return ret;

	/* set the codec system clock */
	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_FLL2,
				     pll_out, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	return 0;
}

static int aries_modem_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret;

	/* set system clock to MCLK1 as it is always on */
	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK1,
			ARIES_MCLK1_FREQ, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	/* disable FLL2 */
	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL2, WM8994_SYSCLK_MCLK1,
				    0, 0);

	return ret;
}

static struct snd_soc_ops aries_modem_ops = {
	.hw_params = aries_modem_hw_params,
	.hw_free = aries_modem_hw_free,
};

static int aries_late_probe(struct snd_soc_card *card)
{
	struct aries_wm8994_data *priv = snd_soc_card_get_drvdata(card);
	int ret;

	if (priv->usb_extcon) {
		ret = devm_extcon_register_notifier(card->dev,
				priv->usb_extcon, EXTCON_JACK_LINE_OUT,
				&aries_extcon_notifier_block);
		if (ret)
			return ret;

		ret = snd_soc_card_jack_new(card, "Dock", SND_JACK_LINEOUT,
				&aries_dock, dock_pins, ARRAY_SIZE(dock_pins));
		if (ret)
			return ret;

		if (extcon_get_state(priv->usb_extcon,
				EXTCON_JACK_LINE_OUT) > 0)
			snd_soc_jack_report(&aries_dock, SND_JACK_LINEOUT,
					SND_JACK_LINEOUT);
		else
			snd_soc_jack_report(&aries_dock, 0, SND_JACK_LINEOUT);
	}

	if (priv->adc) {
		ret = snd_soc_card_jack_new(card, "Headset",
				SND_JACK_HEADSET | SND_JACK_BTN_0,
				&aries_headset,
				jack_pins, ARRAY_SIZE(jack_pins));
		if (ret)
			return ret;

		jack_gpios[0].data = priv;
		jack_gpios[0].desc = priv->gpio_headset_detect;

		jack_gpios[1].data = priv;
		jack_gpios[1].desc = priv->gpio_headset_key;

		snd_jack_set_key(aries_headset.jack, SND_JACK_BTN_0,
				KEY_MEDIA);

		ret = snd_soc_jack_add_gpios(&aries_headset,
				ARRAY_SIZE(jack_gpios), jack_gpios);
		if (ret)
			return ret;
	}

	return 0;
}

static struct snd_soc_dai_driver aries_ext_dai[] = {
	{
		.name = "aries-modem-dai",
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 8000,
			.rates = SNDRV_PCM_RATE_8000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 8000,
			.rates = SNDRV_PCM_RATE_8000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
};

static const struct snd_soc_component_driver aries_component = {
	.name	= "aries-audio",
};

static struct snd_soc_dai_link aries_dai[] = {
	{
		.name = "WM8994 AIF1",
		.stream_name = "Pri_Dai",
		.codec_dai_name = "wm8994-aif1",
		.cpu_dai_name = SAMSUNG_I2S_DAI,
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
			SND_SOC_DAIFMT_CBM_CFM,
		.ops = &aries_ops,
	},
	{
		.name = "WM8994 AIF2",
		.stream_name = "Voice",
		.codec_dai_name = "wm8994-aif2",
		.cpu_dai_name = "aries-modem-dai",
		.ops = &aries_modem_ops,
		.ignore_suspend = 1,
	},
};

static struct snd_soc_card aries = {
	.name = "Aries-I2S",
	.owner = THIS_MODULE,
	.dai_link = aries_dai,
	.num_links = ARRAY_SIZE(aries_dai),
	.fully_routed = true,
	.controls = aries_controls,
	.num_controls = ARRAY_SIZE(aries_controls),
	.dapm_widgets = aries_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(aries_dapm_widgets),
	.late_probe = aries_late_probe,
};

static const struct of_device_id samsung_wm8994_of_match[] = {
	{ .compatible = "samsung,aries-wm8994" },
	{},
};
MODULE_DEVICE_TABLE(of, samsung_wm8994_of_match);

static int aries_audio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_dai_np, *codec_dai_np, *extcon_np;
	struct snd_soc_card *card = &aries;
	struct snd_soc_dai_link *dai_link;
	struct aries_wm8994_data *priv;
	unsigned int buffer[MAX_ZONES];
	int ret, i;

	card->dev = dev;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	snd_soc_card_set_drvdata(card, priv);

	priv->aif2_slave = of_property_read_bool(np, "aif2-slave");

	priv->gpio_earpath_sel = devm_gpiod_get_optional(dev, "earpath-sel",
			GPIOD_OUT_HIGH);
	if (IS_ERR(priv->gpio_earpath_sel)) {
		dev_err(dev, "Failed to get earpath select GPIO\n");
		return PTR_ERR(priv->gpio_earpath_sel);
	}

	priv->reg_main_micbias = devm_regulator_get(dev, "main-micbias");
	if (IS_ERR(priv->reg_main_micbias)) {
		dev_err(dev, "Failed to get main micbias regulator\n");
		return PTR_ERR(priv->reg_main_micbias);
	}

	priv->reg_headset_micbias = devm_regulator_get(dev, "headset-micbias");
	if (IS_ERR(priv->reg_headset_micbias)) {
		dev_err(dev, "Failed to get headset micbias regulator\n");
		return PTR_ERR(priv->reg_headset_micbias);
	}

	extcon_np = of_parse_phandle(np, "dock-extcon", 0);
	if (extcon_np) {
		priv->usb_extcon = extcon_find_edev_by_node(extcon_np);
		if (IS_ERR(priv->usb_extcon)) {
			dev_warn(dev, "Couldn't get extcon device");
			priv->usb_extcon = NULL;
		}
		of_node_put(extcon_np);
	}

	priv->adc = devm_iio_channel_get(dev, "headset-detect");
	if (IS_ERR(priv->adc)) {
		if (PTR_ERR(priv->adc) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
	}

	if (priv->adc) {
		priv->gpio_headset_key = devm_gpiod_get(dev, "headset-key",
				GPIOD_IN);
		if (IS_ERR(priv->gpio_headset_detect)) {
			dev_err(dev, "Failed to get headset key gpio");
			return PTR_ERR(priv->gpio_headset_detect);
		}

		priv->gpio_headset_detect = devm_gpiod_get(dev,
				"headset-detect", GPIOD_IN);
		if (IS_ERR(priv->gpio_headset_detect)) {
			dev_err(dev, "Failed to get headset detect GPIO");
			return PTR_ERR(priv->gpio_headset_detect);
		}

		ret = of_property_read_variable_u32_array(np, "jack-det-adcs",
				buffer, 1, MAX_ZONES);
		priv->num_zones = ret;
		if (ret < 0) {
			dev_err(dev, "Couldn't read jack detect ADC values");
			return ret;
		}

		for (i = 0; i < priv->num_zones; i++)
			priv->zones[i].adc_high = buffer[i];

		ret = of_property_read_variable_u32_array(np,
				"jack-det-delays", buffer, 1, MAX_ZONES);
		if (ret != priv->num_zones) {
			dev_err(dev, "Invalid/missing jack delay values");
			return ret < 0 ? ret : -EINVAL;
		}

		for (i = 0; i < priv->num_zones; i++)
			priv->zones[i].delay_ms = buffer[i];

		ret = of_property_read_variable_u32_array(np,
				"jack-det-counts", buffer, 1, MAX_ZONES);
		if (ret != priv->num_zones) {
			dev_err(dev, "Invalid/missing jack detect counts");
			return ret < 0 ? ret : -EINVAL;
		}

		for (i = 0; i < priv->num_zones; i++)
			priv->zones[i].check_count = buffer[i];

		ret = of_property_read_variable_u32_array(np, "jack-types",
				buffer, 1, MAX_ZONES);
		if (ret != priv->num_zones) {
			dev_err(dev, "Invalid/missing jack detect types");
			return ret < 0 ? ret : -EINVAL;
		}

		for (i = 0; i < priv->num_zones; i++)
			priv->zones[i].jack_type = buffer[i];
	}

	ret = snd_soc_of_parse_card_name(card, "model");
	if (ret < 0) {
		dev_err(dev, "Card name is not specified\n");
		return ret;
	}

	ret = snd_soc_of_parse_audio_routing(card, "samsung,audio-routing");
	if (ret < 0) {
		dev_err(dev, "Audio routing invalid/unspecified\n");
		return ret;
	}

	cpu_dai_np = of_parse_phandle(dev->of_node,
			"samsung,i2s-controller", 0);
	if (!cpu_dai_np) {
		dev_err(dev, "i2s-controller property invalid/missing\n");
		return -EINVAL;
	}

	codec_dai_np = of_parse_phandle(dev->of_node,
			"samsung,audio-codec", 0);
	if (!codec_dai_np) {
		dev_err(dev, "samsung,audio-codec property invalid/missing\n");
		return -EINVAL;
	}

	for_each_card_prelinks(card, i, dai_link)
		card->dai_link[i].codec_of_node = codec_dai_np;
	card->dai_link[0].cpu_of_node = cpu_dai_np;
	card->dai_link[0].platform_of_node = cpu_dai_np;

	ret = devm_snd_soc_register_component(dev, &aries_component,
			aries_ext_dai, ARRAY_SIZE(aries_ext_dai));
	if (ret < 0) {
		dev_err(dev, "Failed to register component: %d\n", ret);
		return ret;
	}

	ret = devm_snd_soc_register_card(dev, card);
	if (ret < 0) {
		dev_err(dev, "Failed to register card: %d\n", ret);
		return ret;
	}

	return ret;
}

static struct platform_driver aries_audio_driver = {
	.driver		= {
		.name	= "aries-audio-wm8994",
		.of_match_table = of_match_ptr(samsung_wm8994_of_match),
		.pm	= &snd_soc_pm_ops,
	},
	.probe		= aries_audio_probe,
};

module_platform_driver(aries_audio_driver);

MODULE_DESCRIPTION("ALSA SoC Aries WM8994");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:aries-audio-wm8994");
