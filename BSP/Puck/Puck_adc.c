#include "nrf_drv_adc.h"
#include "app_error.h"

#include "nrf_delay.h"
#include "nrf_log.h"

#define NRF_DRV_ADC_CHANNEL()                  \
 {{{                                                       \
    .resolution = NRF_ADC_CONFIG_RES_8BIT,                 \
    .input      = NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD, \
    .reference  = NRF_ADC_CONFIG_REF_VBG,                  \
    .ain        = NRF_ADC_CONFIG_INPUT_DISABLED            \
 }}, NULL}
 
static const nrf_drv_adc_channel_t adc_channel_VDD = NRF_DRV_ADC_CHANNEL(); /**< Channel instance. Default configuration used. */

/**
 * @brief ADC initialization.
 */
uint16_t adc_GetVDD(void)
{
	nrf_adc_value_t adc_value;
	static uint16_t voltage_mV;

	ret_code_t ret_code;
	nrf_drv_adc_config_t config = NRF_DRV_ADC_DEFAULT_CONFIG;

	ret_code = nrf_drv_adc_init(&config, NULL);
	APP_ERROR_CHECK(ret_code);	
	
	nrf_drv_adc_sample_convert(&adc_channel_VDD, &adc_value);
	
	nrf_drv_adc_uninit();
	
	voltage_mV = 1.2*(float)adc_value*1000*3.0/255;
	
	return voltage_mV;
}

