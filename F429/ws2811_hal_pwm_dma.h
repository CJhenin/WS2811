#ifndef __WS2811_HAL_PWM_DMA_H__
#define __WS2811_HAL_PWM_DMA_H__

#include "main.h"
#include <stdint.h>
#include <string.h>
#include <math.h>

#define USE_FREERTOS

#ifdef USE_FREERTOS
    #include "FreeRTOS.h"
    #define MY_MALLOC(size)    pvPortMalloc(size)
    #define MY_FREE(ptr)       vPortFree(ptr)
#else
    #include <stdlib.h>
    #define MY_MALLOC(size)    malloc(size)
    #define MY_FREE(ptr)       free(ptr)
#endif


#define MAX_WS2811_INSTANCES 4

#define WS2811_MAX_RGB_NUM  64  
#define WS2811_BITS_PER_LED 24	// a rgb have 24bit

#define WS2811_RING_DIR_S_TO_B 0
#define WS2811_RING_DIR_B_TO_S 1

#define RGB_NONE      0x000000
#define RGB_WHITE     0xFFFFFF
#define RGB_RED       0xFF0000
#define RGB_GREEN     0x006400
#define RGB_BLUE      0x0000FF
#define RGB_YELLOW    0x646400	//0xFFFF00
#define RGB_PINK      0xFF69B4
#define RGB_CYAN      0x00FFFF

typedef enum
{
    WS2811_STATUS_OK,
    WS2811_STATUS_ERROR
} WS2811_Status_e;

typedef struct WS2811_Info_t WS2811_Info_t;
typedef struct WS2811_Info_t
{
    uint8_t rgb_num;
    uint8_t (*rgb_arr)[3]; // 3 bytes for each rgb
    uint16_t* pwm_buf;

    TIM_HandleTypeDef* tim;
    uint32_t tim_channel;

    uint16_t code_0;
    uint16_t code_1;
    
    uint8_t twinkle_state;
    uint8_t twinkle_count;

    float fade_step;

    uint16_t ring_index;
    
    WS2811_Status_e (*init)(WS2811_Info_t* const self, uint8_t rgb_num,
                                   TIM_HandleTypeDef* htim, const uint32_t tim_channel);
    WS2811_Status_e (*deinit)(WS2811_Info_t* self);

    WS2811_Status_e (*update_rgb)(WS2811_Info_t* const self);

    WS2811_Status_e (*set_one_pixel)(WS2811_Info_t* const self, uint8_t index, uint32_t rgb);
    WS2811_Status_e (*set_all_same_color)(WS2811_Info_t* const self, uint32_t rgb);
    WS2811_Status_e (*effect_twinkle)(WS2811_Info_t* const self, \
					const uint8_t start_index, uint8_t end_index, \
					const uint8_t freq_scaler, const uint32_t rgb);
    WS2811_Status_e (*effect_fade)(WS2811_Info_t* const self, \
					const uint8_t start_index, uint8_t end_index, \
					const uint32_t rgb);
    WS2811_Status_e (*effect_ring)(WS2811_Info_t* const self, \
					const uint8_t start_index, uint8_t end_index, \
					const uint8_t dir, const uint32_t rgb);  
} WS2811_Info_t;

extern WS2811_Info_t ws2811_create(uint8_t rgb_num, TIM_HandleTypeDef* htim, uint32_t channel);

extern WS2811_Status_e ws2811_register(WS2811_Info_t* const self);
extern void ws2811_handle_dma_callback(TIM_HandleTypeDef *htim);  

extern WS2811_Status_e ws2811_init(WS2811_Info_t* const self, uint8_t rgb_num,
                                   TIM_HandleTypeDef* htim, const uint32_t tim_channel);
extern WS2811_Status_e ws2811_deinit(WS2811_Info_t* self);

extern WS2811_Status_e update_rgb(WS2811_Info_t* const self);

extern WS2811_Status_e set_one_pixel(WS2811_Info_t* const self, uint8_t index, uint32_t rgb);
extern WS2811_Status_e set_all_same_color(WS2811_Info_t* const self, uint32_t rgb);
extern WS2811_Status_e effect_twinkle(WS2811_Info_t* const self, \
                                      const uint8_t start_index, uint8_t end_index, \
                                      const uint8_t freq_scaler, const uint32_t rgb);
extern WS2811_Status_e effect_fade(WS2811_Info_t* const self, \
                                   const uint8_t start_index, uint8_t end_index, \
                                   const uint32_t rgb);
extern WS2811_Status_e effect_ring(WS2811_Info_t* const self, \
                                   const uint8_t start_index, uint8_t end_index, \
                                   const uint8_t dir, const uint32_t rgb);
#endif
