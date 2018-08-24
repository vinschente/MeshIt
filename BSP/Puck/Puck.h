#ifndef TEUFEL_PUCK_H
#define TEUFEL_PUCK_H

#ifdef __cplusplus
extern "C" {
#endif

// LEDs definitions
#define LEDS_NUMBER    2

#define LED_1          29
#define LED_2          9

#define LEDS_ACTIVE_STATE 1
#define LED_ON          1
#define LED_OFF         0

#define LEDS_LIST { LED_1, LED_2}

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2

#define LEDS_INV_MASK  LEDS_MASK

// Configure Buttons
#define BUTTONS_NUMBER 2

#define BUTTON_1       4
#define BUTTON_2       0
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2


#define BSP_ENC_A      3
#define BSP_ENC_B      1


#define RX_PIN_NUMBER  13
#define TX_PIN_NUMBER  14
#define CTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define RTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define HWFC           false


// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}


#ifdef __cplusplus
}
#endif

#endif // TEUFEL_PUCK_H
