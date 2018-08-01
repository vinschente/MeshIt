#ifndef __MICROBIT_H__
#define __MICROBIT_H__

#define LEDS_NUMBER    0

#define BUTTONS_NUMBER 2

#define BUTTON_1       17
#define BUTTON_2       26

#define BUTTON_PULL    NRF_GPIO_PIN_NOPULL

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2}

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2


#define BSP_DISPLAY_COL_1 4
#define BSP_DISPLAY_COL_2 5
#define BSP_DISPLAY_COL_3 6
#define BSP_DISPLAY_COL_4 7
#define BSP_DISPLAY_COL_5 8
#define BSP_DISPLAY_COL_6 9
#define BSP_DISPLAY_COL_7 10
#define BSP_DISPLAY_COL_8 11
#define BSP_DISPLAY_COL_9 12

#define BSP_DISPLAY_ROW_1 13
#define BSP_DISPLAY_ROW_2 14
#define BSP_DISPLAY_ROW_3 15

/* ### ADC ### */
#define BSP_ADC


#define RX_PIN_NUMBER 25
#define TX_PIN_NUMBER 24

/* I2C */
#define BSP_I2C_SDA 30
#define BSP_I2C_SCL 0

//nrf_clock_lf_cfg_t clock_lf_cfg = 
//{
//    .source = NRF_CLOCK_LF_SRC_RC,
//    .rc_ctiv = 16, // Interval in 0.25 s, 16 * 0.25 = 4 sec
//    .rc_temp_ctiv = 2, // Check temperature every .rc_ctiv, but calibrate every .rc_temp_ctiv 
//    .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_250_PPM,
//};

#define NRF_CLOCK_LFCLKSRC  {.source        = NRF_CLOCK_LF_SRC_XTAL,       \
                             .rc_ctiv       = 0,                                \
                             .rc_temp_ctiv  = 0,                                \
                             .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#endif /* __MICROBIT_H__ */
