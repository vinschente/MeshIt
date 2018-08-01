#ifndef _MICROBIT_LEDMATRIX_H_
#define _MICROBIT_LEDMATRIX_H_

#include <stdint.h>

void Matrix_Init(void);

void Matrix_Reset(void);
void Matrix_SetDot(uint8_t x, uint8_t y);
void Matrix_ResetDot(uint8_t x, uint8_t y);

void Matrix_ToggleDot(uint8_t x, uint8_t y);

#endif /* _MICROBIT_LEDMATRIX_H_ */
