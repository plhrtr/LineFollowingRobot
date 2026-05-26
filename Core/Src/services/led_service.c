#include "services/led_service.h"
#include "main.h"
#include "stm32l432xx.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdint.h>

static struct led_config_t {
  GPIO_TypeDef *port;
  uint16_t pin;
};
typedef struct led_config_t led_config_t;

static led_config_t get_config(led_t led) {
  led_config_t config;
  switch (led) {
  case LED_LEFT:
    config.pin = LED_left_Pin;
    config.port = LED_left_GPIO_Port;
    break;
  case LED_RIGHT:
    config.pin = LED_right_Pin;
    config.port = LED_right_GPIO_Port;
    break;
  case LD_3:
    config.pin = LD3_Pin;
    config.port = LD3_GPIO_Port;
    break;
  }
  return config;
};

void led_toggle(led_t led) {
  struct led_config_t config = get_config(led);
  HAL_GPIO_TogglePin(config.port, config.pin);
}

void led_on(led_t led) {
  struct led_config_t config = get_config(led);
  HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_SET);
}

void led_off(led_t led) {
  struct led_config_t config = get_config(led);
  HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_RESET);
}
