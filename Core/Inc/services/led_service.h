#ifndef LED_SERVICE_H
#define LED_SERVICE_H

/**
 * Enum of all LEDs for the armuro robot
 */
typedef enum {
  // The LED left of the nucleo board
  LED_LEFT,
  // The left right of the nucleo borad
  LED_RIGHT,
  // The LD3 on the nucleo board
  LD_3
} led_t;

/**
 * Toggle the given LED.
 * @param led - the LED you want to toggle.
 */
void led_toggle(led_t led);

/**
 * Turn on the given LED.
 * @param led - the LED you want to turn on.
 */
void led_on(led_t led);

/**
 * Turn off the given LED.
 * @param led - The LED you want to turn off.
 */
void led_off(led_t led);

#endif // !LED_SERVICE_H
