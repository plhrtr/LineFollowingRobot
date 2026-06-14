#ifndef TOUCH_SENSOR_SERVICE_H
#define TOUCH_SENSOR_SERVICE_H

/**
 * All available touch sensors
 */
typedef enum {
  TOUCH_SENSOR_LEFT = 1,
  TOUCH_SENSOR_MIDDLE = 2,
  TOUCH_SENSOR_RIGHT = 4,
} touch_sensor_t;

/**
 * Defines a subscription to the touch sensor
 */
typedef struct {
  // The callback function for the subscription
  void (*callback)();
  // The sensor you want to subscribe to. Use | to subscribe to multiple
  // sensors. Subscribing to multiple sensors means at least one of them has to
  // be pressed
  touch_sensor_t sensors;
} touch_sensor_subscription_t;

/**
 * Initialize the touch sensor
 */
void touch_sensor_init();

/**
 * Subscribe a callback to the touch sensor that gets executed on press
 * @param subscribtion - The user defined subscribtion
 * @return - the id to later unsubscribe or -1 if the callback array is full
 * @warning - The callback is executed on pressing AND releasing
 */
int touch_sensor_subscribe(touch_sensor_subscription_t subscribtion);

/**
 * Unsubscribe the given id from the touch sensor
 * @param id - The id you want to unsubscribe
 */
void touch_sensor_unsubscribe(int id);

/**
 * Handle an interrupt for the given sensor
 */
void touch_sensor_handle(touch_sensor_t sensor);

#endif // !TOUCH_SENSOR_SERVICE_H
