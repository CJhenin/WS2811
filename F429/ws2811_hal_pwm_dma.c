#include "ws2811_hal_pwm_dma.h"

// TODO: function ws2811_handle_dma_callback return instances index \
         
extern TIM_HandleTypeDef htim8;

static WS2811_Info_t* ws2811_instances[MAX_WS2811_INSTANCES];
static uint8_t ws2811_instance_count = 0;

WS2811_Status_e ws2811_register(WS2811_Info_t* const self)
{
    if ((self != NULL) && (ws2811_instance_count < MAX_WS2811_INSTANCES)) 
	{

		for (uint8_t i = 0; i < ws2811_instance_count; ++i)
		{
			if (ws2811_instances[i] == self)
			{
				// Instance already registered
				return WS2811_STATUS_OK;
			}
		}
        ws2811_instances[ws2811_instance_count++] = self;

		return WS2811_STATUS_OK;
    }

	else 
	{
		// Instance limit reached
		//ws2811_deinit(self);	// deinit the instance in outer code
		return WS2811_STATUS_ERROR;
	}
}

// TODO: return i
void ws2811_handle_dma_callback(TIM_HandleTypeDef *htim)
{
    for (uint8_t i = 0; i < ws2811_instance_count; ++i) 
	{
        if (ws2811_instances[i]->tim->Instance == htim->Instance) 
		{
            __HAL_TIM_SET_COMPARE(htim, ws2811_instances[i]->tim_channel, 0);
            HAL_TIM_PWM_Stop_DMA(htim, ws2811_instances[i]->tim_channel);
        }
    }
}

WS2811_Status_e ws2811_init(WS2811_Info_t* const self, uint8_t rgb_num,
                            TIM_HandleTypeDef* htim, const uint32_t tim_channel)
{
	if ((self == NULL) || (htim == NULL))
	{
		return WS2811_STATUS_ERROR;
	}

	if (rgb_num > WS2811_MAX_RGB_NUM)
	{
		rgb_num = WS2811_MAX_RGB_NUM; 
	}

	self->tim = htim;
	self->tim_channel = tim_channel;
	self->code_0 = __HAL_TIM_GetAutoreload(htim) * 0.2f; // 20% duty cycle for code 0
	self->code_1 = __HAL_TIM_GetAutoreload(htim) * 0.5f; // 50% duty cycle for code 1

	self->rgb_num = rgb_num;
	self->rgb_arr = MY_MALLOC(sizeof(uint8_t[3]) * rgb_num);
	self->pwm_buf = MY_MALLOC(sizeof(uint16_t) * WS2811_BITS_PER_LED * rgb_num);
	if ((self->rgb_arr == NULL) || (self->pwm_buf == NULL))
	{
		MY_FREE(self->rgb_arr);
		MY_FREE(self->pwm_buf);
		self->rgb_arr = NULL;
		self->pwm_buf = NULL;
		self->rgb_num = 0;
		return WS2811_STATUS_ERROR;
	}

	self->twinkle_state = 1;
	self->twinkle_count = 0;
	self->fade_step = 1.57f;
	self->ring_index = 0;

	// self->update_rgb = update_rgb;
	// self->set_one_pixel = set_one_pixel;
	// self->set_all_same_color = set_all_same_color;
	// self->effect_twinkle = effect_twinkle;
	// self->effect_fade = effect_fade;
	// self->effect_ring = effect_ring;

	ws2811_register(self);

	return WS2811_STATUS_OK;
}

WS2811_Status_e ws2811_deinit(WS2811_Info_t* self)
{
	if (self == NULL)
	{
		return WS2811_STATUS_ERROR;
	}

	if (self->rgb_arr)
	{
		MY_FREE(self->rgb_arr);
		self->rgb_arr = NULL;
	}
	
	if (self->pwm_buf)
	{
		MY_FREE(self->pwm_buf);
		self->pwm_buf = NULL;
	}

	self->rgb_num = 0;
	
	self->tim = NULL;
	self->tim_channel = 0;

	return WS2811_STATUS_OK;
}

WS2811_Info_t ws2811_create(uint8_t rgb_num, TIM_HandleTypeDef* htim, uint32_t channel)
{
    WS2811_Info_t self = {0};

    self.init   = ws2811_init;
    self.deinit = ws2811_deinit;

    self.update_rgb         = update_rgb;
    self.set_all_same_color = set_all_same_color;
    self.set_one_pixel      = set_one_pixel;
    self.effect_twinkle     = effect_twinkle;
    self.effect_fade        = effect_fade;
    self.effect_ring        = effect_ring;

 
    if (self.init(&self, rgb_num, htim, channel) != WS2811_STATUS_OK) 
	{
        self.rgb_num = 0;
    }

    return self;
}

static WS2811_Status_e ws2811_encode(WS2811_Info_t* const self)
{
	if (self == NULL)
	{
		return WS2811_STATUS_ERROR;
	}

    uint32_t i, j, k = 0;
    for (i = 0; i < self->rgb_num; ++i)
    {
		for (j = 0; j < 3; ++j)
		{
			uint8_t byte = self->rgb_arr[i][j];

			for (uint8_t b = 0; b < 8; ++b)
			{
				self->pwm_buf[k++] = (byte & 0x80) ? self->code_1 : self->code_0;
				byte <<= 1;
			}
		}
    }

	return WS2811_STATUS_OK;
}

WS2811_Status_e update_rgb(WS2811_Info_t* const self)
{
	if ((self == NULL) || (self->tim == NULL) || \
	    (self->pwm_buf == NULL) || (self->rgb_num == 0))
	{
		return WS2811_STATUS_ERROR;
	}

	ws2811_encode(self);

	HAL_TIM_PWM_Start_DMA(self->tim, self->tim_channel, \
		                  (uint32_t *)self->pwm_buf, self->rgb_num * WS2811_BITS_PER_LED);

	return WS2811_STATUS_OK;
}

// index starts from 0
WS2811_Status_e set_one_pixel(WS2811_Info_t* const self, uint8_t index, uint32_t rgb)
{
	if ((self == NULL) || (index >= self->rgb_num))
	{
		return WS2811_STATUS_ERROR;
	}

    self->rgb_arr[index][0] = (rgb >> 16) & 0xFF;
    self->rgb_arr[index][1] = (rgb >> 8) & 0xFF;
    self->rgb_arr[index][2] = (rgb) & 0xFF;

	return WS2811_STATUS_OK;
}

WS2811_Status_e set_all_same_color(WS2811_Info_t* const self, uint32_t rgb)
{
    if (self == NULL)
    {
        return WS2811_STATUS_ERROR;
    }

	for (uint8_t i = 0; i < self->rgb_num; ++i)
	{
		if (set_one_pixel(self, i, rgb) != WS2811_STATUS_OK)
		{
			return WS2811_STATUS_ERROR;
		}
	}

	return WS2811_STATUS_OK;
}

WS2811_Status_e effect_twinkle(WS2811_Info_t* const self, \
					const uint8_t start_index, uint8_t end_index, \
					const uint8_t freq_scaler, const uint32_t rgb)
{
    if ((self == NULL) || (start_index > end_index))
	{
		return WS2811_STATUS_ERROR;
	}

	if (end_index >= self->rgb_num)
	{
		end_index = self->rgb_num - 1;
	}

	if (self->twinkle_count > freq_scaler)
	{
		self->twinkle_state = !self->twinkle_state;
		self->twinkle_count = 0;
	}

	self->twinkle_count++;

	if (self->twinkle_state)
	{
		for (uint8_t i = start_index; i <= end_index; ++i)
		{
			if (set_one_pixel(self, i, rgb) != WS2811_STATUS_OK)
			{
				return WS2811_STATUS_ERROR;
			}
		}
	}
	else
	{
		for (uint8_t i = start_index; i <= end_index; ++i)
		{
			if (set_one_pixel(self, i, 0) != WS2811_STATUS_OK)
			{
				return WS2811_STATUS_ERROR;
			}
		}
	}

	return WS2811_STATUS_OK;
}

WS2811_Status_e effect_fade(WS2811_Info_t* const self, \
					const uint8_t start_index, uint8_t end_index, \
					const uint32_t rgb)
{
	if ((self == NULL) || (start_index > end_index))
	{
		return WS2811_STATUS_ERROR;
	}

	if (end_index >= self->rgb_num)
	{
		end_index = self->rgb_num - 1;
	}

	float brightness = 0.5f * (1.0f + sinf(self->fade_step));  // use value table to avoid calculation
	if (brightness < 0.0f)
	{
		brightness = 0.0f;
	}

	if (brightness > 1.0f)
	{
		brightness = 1.0f;
	}

    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8)  & 0xFF;
    uint8_t b = rgb & 0xFF;

	r = (uint8_t)roundf(r * brightness);
    g = (uint8_t)roundf(g * brightness);
    b = (uint8_t)roundf(b * brightness);

	for (uint8_t i = start_index; i <= end_index; ++i)
	{
		if(set_one_pixel(self, i, ((r << 16) | (g << 8) | b)) != WS2811_STATUS_OK)
		{
			return WS2811_STATUS_ERROR;
		}
	}

	self->fade_step += 0.05f;
	if (self->fade_step > 6.28f)
	{
		self->fade_step = 0.0f;
	}

	return WS2811_STATUS_OK;
}

/*
ring mode:
000...000 -> 100...000 -> 010...000 -> 001...000 -> ... -> 000...100 -> 000...010 -> 000...001
*/
WS2811_Status_e effect_ring(WS2811_Info_t* const self, \
					const uint8_t start_index, uint8_t end_index, \
					const uint8_t dir, const uint32_t rgb)
{
	if ((self == NULL) || (start_index > end_index))
	{
		return WS2811_STATUS_ERROR;
	}

	if (end_index >= self->rgb_num)
	{
		end_index = self->rgb_num - 1;
	}

	if ((self->ring_index < start_index) || (self->ring_index > end_index))
	{
        self->ring_index = (dir == WS2811_RING_DIR_S_TO_B) ? start_index : end_index;
	}
	
	// set_all_same_color(self, RGB_NONE); // Clear all pixels firstly
	if (set_one_pixel(self, self->ring_index, rgb) != WS2811_STATUS_OK)
	{
		return WS2811_STATUS_ERROR;
	}

	if (dir == WS2811_RING_DIR_S_TO_B)
    {
      self->ring_index = (self->ring_index >= end_index) ? start_index : (self->ring_index + 1);
    }
    else
    {
      self->ring_index = (self->ring_index <= start_index) ? end_index : (self->ring_index - 1);
    }

	return WS2811_STATUS_OK;
}

 void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
 {
 	 ws2811_handle_dma_callback(htim);
 }

