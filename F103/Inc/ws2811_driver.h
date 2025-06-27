#ifndef __WS2811_DRIVER_H__
#define __WS2811_DRIVER_H__

#include "stm32f10x.h"
#include <stdint.h>
#include <string.h>
#include <math.h>

#define WS2811_LED_NUM          8   // number of rgb
#define WS2811_BITS_PER_LED     24  // a rgb have 24bit
#define WS2811_BUF_LEN         (WS2811_LED_NUM * WS2811_BITS_PER_LED)

#define TIM3_ARR_VALUE          89  // 72MHz / (89 + 1) = 800KHz, a bit 1.25us
#define DUTY_0                 (uint16_t)(TIM3_ARR_VALUE * 0.2f)    //the duty of ws2811 code0
#define DUTY_1                 (uint16_t)(TIM3_ARR_VALUE * 0.5f)    //the duty of ws2811 code1

#define WS2811_RING_DIR_S_TO_B 0
#define WS2811_RING_DIR_B_TO_S 1

#define RGB_NONE      0x000000
#define RGB_WHITE     0xFFFFFF
#define RGB_RED       0xFF0000
#define RGB_GREEN     0x00FF00
#define RGB_BLUE      0x0000FF
#define RGB_YELLOW    0xFFFF00
#define RGB_PINK      0xFF69B4
#define RGB_CYAN      0x00FFFF

extern uint16_t ws2811_buf[WS2811_BUF_LEN];

extern void ws2811_show(void);
extern void ws2811_set_pixel(const uint8_t index, const uint32_t rgb);
extern void ws2811_all_same_color(const uint32_t rgb);
extern void ws2811_twinkle(const uint8_t start_index, const uint8_t end_index, const uint32_t rgb);
extern void ws2811_fade(const uint8_t start_index, const uint8_t end_index, const uint32_t rgb);
extern void ws2811_ring(const uint8_t start_index, const uint8_t end_index, \
                        const uint8_t dir, const uint32_t rgb);

#endif
