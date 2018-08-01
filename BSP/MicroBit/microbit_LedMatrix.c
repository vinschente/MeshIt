#include "microbit_LedMatrix.h"
#include "nrf_gpio.h"
#include "boards.h"


#include "nrf_drv_clock.h"
#include "nrf_drv_timer.h"

uint8_t matrix[5] = {0};

#define DATA_BUFFER_SIZE 3
uint16_t rowData[DATA_BUFFER_SIZE] = {0x0000, 0x0000, 0x0000};

static uint8_t isTimerStarted = 0;

static int Matrix_isEmpty(void);

static void hfclk_config(void);
static void timer_init(void);
static void timer_start(void);
static void timer_stop(void);

void Matrix_Init(void)
{
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_1);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_2);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_3);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_4);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_5);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_6);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_7);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_8);
	nrf_gpio_cfg_output(BSP_DISPLAY_COL_9);

	nrf_gpio_cfg_output(BSP_DISPLAY_ROW_1);
	nrf_gpio_cfg_output(BSP_DISPLAY_ROW_2);
	nrf_gpio_cfg_output(BSP_DISPLAY_ROW_3);
	
	// Turn everthing off
	nrf_gpio_pin_write(BSP_DISPLAY_COL_1, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_2, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_3, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_4, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_5, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_6, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_7, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_8, 1);
	nrf_gpio_pin_write(BSP_DISPLAY_COL_9, 1);
	
	nrf_gpio_pin_write(BSP_DISPLAY_ROW_1, 0);
	nrf_gpio_pin_write(BSP_DISPLAY_ROW_2, 0);
	nrf_gpio_pin_write(BSP_DISPLAY_ROW_3, 0);
	
	timer_init();
}

void Matrix_Reset(void)
{
	memset(rowData, 0, sizeof(rowData));
	timer_stop();
}

static int Matrix_isEmpty(void)
{
	for(int i = 0; i < DATA_BUFFER_SIZE; i++) {
		if(rowData[i]){
			return 0;
		}
	}
	return 1;
}

const struct{
	uint8_t row;
	uint8_t col;
} MappingTab[25] = {
	{0x01, 0x01}, {0x03, 0x04}, {0x02, 0x02}, {0x01, 0x08}, {0x03, 0x03},
	{0x02, 0x04}, {0x03, 0x05}, {0x01, 0x09}, {0x01, 0x07}, {0x02, 0x07},
	{0x01, 0x02}, {0x03, 0x06}, {0x02, 0x03}, {0x01, 0x06}, {0x03, 0x01},
	{0x02, 0x05}, {0x03, 0x07}, {0x03, 0x09}, {0x01, 0x05}, {0x02, 0x06},
	{0x01, 0x03}, {0x03, 0x08}, {0x02, 0x01}, {0x01, 0x04}, {0x03, 0x02},
};

void Matrix_SetDot(uint8_t x, uint8_t y)
{
	if(x >= 5 || y >= 5){
		return;
	}
	
	uint8_t idx;
	
	idx = x+y*5;
	rowData[MappingTab[idx].row-1] |= (1<<(MappingTab[idx].col-1));
	
	if(!isTimerStarted){
		timer_start();
	}
}

void Matrix_ResetDot(uint8_t x, uint8_t y)
{
	if(x >= 5 || y >= 5){
		return;
	}

	uint8_t idx;
	
	idx = x+y*5;
	rowData[MappingTab[idx].row-1] &= ~(1<<(MappingTab[idx].col-1));
	
	if(Matrix_isEmpty()){
		timer_stop();
	}
}

void Matrix_ToggleDot(uint8_t x, uint8_t y) {
	if(x >= 5 || y >= 5){
		return;
	}

	uint8_t idx;
	
	idx = x+y*5;
	rowData[MappingTab[idx].row-1] ^= (1<<(MappingTab[idx].col-1));
	
	if(Matrix_isEmpty()){
		timer_stop();
	}else if(!isTimerStarted){
		timer_start();
	}
}

static void Update_Col(uint8_t row)
{
	if(rowData[row] & 0x01){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_1, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_1, 1);
	}
	if(rowData[row] & 0x02){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_2, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_2, 1);
	}
	if(rowData[row] & 0x04){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_3, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_3, 1);
	}
	if(rowData[row] & 0x08){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_4, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_4, 1);
	}
	if(rowData[row] & 0x10){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_5, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_5, 1);
	}
	if(rowData[row] & 0x20){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_6, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_6, 1);
	}
	if(rowData[row] & 0x40){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_7, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_7, 1);
	}
	if(rowData[row] & 0x80){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_8, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_8, 1);
	}
	if(rowData[row] & 0x100){
		nrf_gpio_pin_write(BSP_DISPLAY_COL_9, 0);
	}else {
		nrf_gpio_pin_write(BSP_DISPLAY_COL_9, 1);
	}
}

void Matrix_UpdateRow(void)
{
	static uint8_t row = 0;
	switch(row){
		case 0:
			nrf_gpio_pin_write(BSP_DISPLAY_ROW_3, 0);
			Update_Col(row);
			nrf_gpio_pin_write(BSP_DISPLAY_ROW_1, 1);
			break;
		case 1:
			nrf_gpio_pin_write(BSP_DISPLAY_ROW_1, 0);
			Update_Col(row);
			nrf_gpio_pin_write(BSP_DISPLAY_ROW_2, 1);
			break;
		case 2:
			nrf_gpio_pin_write(BSP_DISPLAY_ROW_2, 0);
			Update_Col(row);
			nrf_gpio_pin_write(BSP_DISPLAY_ROW_3, 1);
			break;
	}
	
	row++;
	if(row > 2){
		row = 0;
	}
}


const nrf_drv_timer_t mytimer1 = NRF_DRV_TIMER_INSTANCE(1); /**< Declaring an instance of nrf_drv_timer for TIMER1. */

/** @brief Function for timer events.
 */
static void timer1_handler(nrf_timer_event_t event_type, void* p_context)
{
//    uint32_t err_code;

    if (event_type == NRF_TIMER_EVENT_COMPARE0) {   // Interrupt from COMPARE0 event.
       Matrix_UpdateRow();
    }
}


/** @brief Function starting the HFCLK oscillator.
 */
static void hfclk_config(void)
{
	ret_code_t err_code = nrf_drv_clock_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_clock_hfclk_request(NULL);
}

static void timer_init(void)
{
	uint32_t time_in_ms = 4;   //Time(in miliseconds) between consecutive compare events.

	uint32_t time2ticks;
	uint32_t err_code;
	
	nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
	err_code = nrf_drv_timer_init(&mytimer1, &timer_cfg, timer1_handler);
	APP_ERROR_CHECK(err_code);

	time2ticks = nrf_drv_timer_ms_to_ticks(&mytimer1, time_in_ms);

	nrf_drv_timer_extended_compare(&mytimer1, NRF_TIMER_CC_CHANNEL0, time2ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
	
	isTimerStarted = 0;
}

static void timer_start(void)
{
	nrf_drv_timer_enable(&mytimer1);
	isTimerStarted = 1;
}

static void timer_stop(void)
{
	nrf_drv_timer_disable(&mytimer1);
	isTimerStarted = 0;
}
