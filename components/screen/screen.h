#ifndef SCREEN_SCREEN_H_
#define SCREEN_SCREEN_H_

#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * e-Paper GPIO
**/
#define EPD_RST_PIN     26
#define EPD_DC_PIN      27
#define EPD_PWR_PIN     1
#define EPD_CS_PIN      15
#define EPD_BUSY_PIN    25

/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value)  gpio_set_level(_pin, _value == 0? 0:1)
#define DEV_Digital_Read(_pin) gpio_get_level(_pin)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms) vTaskDelay(__xms);

void DEV_SPI_WriteByte(UBYTE value);

int DEV_Module_Init(void);
void DEV_Module_Exit(void);

// Display resolution
#define EPD_2in13_V4_WIDTH       122
#define EPD_2in13_V4_HEIGHT      250

void EPD_2in13_V4_Init(void);
void EPD_2in13_V4_Init_Fast(void);
void EPD_2in13_V4_Init_GUI(void);
void EPD_2in13_V4_Clear(void);
void EPD_2in13_V4_Clear_Black(void);
void EPD_2in13_V4_Display(UBYTE *Image);
void EPD_2in13_V4_Display_Fast(UBYTE *Image);
void EPD_2in13_V4_Display_Base(UBYTE *Image);
void EPD_2in13_V4_Display_Partial(UBYTE *Image);
void EPD_2in13_V4_Sleep(void);


#endif /* SCREEN_SCREEN_H_ */
