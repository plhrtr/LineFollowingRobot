#include "services/touch_sensor_service.h"
#include "main.h"
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdint.h>

#define MAX_SUBSCRIPTIONS sizeof(uint32_t) * 8

static touch_sensor_subscription_t subscriptions[MAX_SUBSCRIPTIONS];
static uint32_t subscriptions_bitmap = 0;

static const uint8_t JITTER_THRESHOLD = 50;

static uint32_t last_left_sensor_press = 0;
static uint32_t last_middle_sensor_press = 0;
static uint32_t last_right_sensor_press = 0;

static uint32_t get_sensor_last_pressed(touch_sensor_t sensor) {
  switch (sensor) {
  case TOUCH_SENSOR_LEFT:
    return last_left_sensor_press;
  case TOUCH_SENSOR_MIDDLE:
    return last_middle_sensor_press;
  case TOUCH_SENSOR_RIGHT:
    return last_right_sensor_press;
  default:
    return 0;
  }
}

static void set_sensor_last_pressed(touch_sensor_t sensor, uint32_t new_time) {
  switch (sensor) {
  case TOUCH_SENSOR_LEFT:
    last_left_sensor_press = new_time;
    break;
  case TOUCH_SENSOR_MIDDLE:
    last_middle_sensor_press = new_time;
    break;
  case TOUCH_SENSOR_RIGHT:
    last_right_sensor_press = new_time;
    break;
  default:
    break;
  }
}

void touch_sensor_init() {
  // Set priorities for the interrupts
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);

  // Enable the interrupts
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  // Set the switches to interrupt call interrupts
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = switch_right_Pin | switch_middle_Pin | switch_left_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

int touch_sensor_subscribe(touch_sensor_subscription_t subscription) {
  if (subscriptions_bitmap == ~(uint32_t)0) {
    return -1;
  }

  uint16_t index = __builtin_ctz(~subscriptions_bitmap);

  if (index >= MAX_SUBSCRIPTIONS) {
    return -1;
  }

  // Mark the task as occupied
  uint32_t mask = (uint32_t)1 << index;
  subscriptions_bitmap |= mask;

  // Insert the task into the array
  subscriptions[index] = subscription;

  return index;
}

void touch_sensor_unsubscribe(int id) {
  if (id < 0 || id >= MAX_SUBSCRIPTIONS)
    return;

  uint32_t mask = (uint32_t)1 << id;

  subscriptions_bitmap &= ~mask;
}

void touch_sensor_handle(touch_sensor_t sensor) {
  for (size_t i = 0; i < MAX_SUBSCRIPTIONS; i++) {
    uint32_t mask = (uint32_t)1 << i;

    if (!(subscriptions_bitmap & mask)) {
      continue;
    }

    touch_sensor_subscription_t *subscription = &subscriptions[i];

    if (subscription->sensors & sensor) {
      uint32_t current_tick = HAL_GetTick();

      if (current_tick - get_sensor_last_pressed(sensor) > JITTER_THRESHOLD) {
        subscription->callback();
        set_sensor_last_pressed(sensor, current_tick);
      }
    }
  }
}
