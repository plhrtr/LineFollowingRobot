#include "mission_control/handlers/line_following.h"
#include "mission_control/handlers/line_searching.h"
#include "mission_control/mission_control.h"
#include "services/line_sensor_service.h"
#include "services/motor_service.h"
#include "services/touch_sensor_service.h"
#include "services/wheel_encoder_service.h"
#include <stdint.h>

#define P_VALUE 100

static const uint8_t minimum_speed = 40;

// Variables to handle gaps in the line
static uint16_t start_distance = 0;
static const uint16_t gap_distance = 25;
static bool is_in_gap = false;

static int touch_sensor_unsubscribe_id = 0;

static int clamp_int(int value) {
  if (value < -100) {
    return -100;
  } else if (value > 100) {
    return 100;
  } else {
    return value;
  }
}

/**
 * Callback that gets executed when an obstacle is detected
 */
static void on_obstacle_detected() {
  touch_sensor_unsubscribe(touch_sensor_unsubscribe_id);
  mission_control_set_state(OBSTACLE_AVOIDANCE);
}

void line_following_init() {
  touch_sensor_subscription_t sub = {.sensors = TOUCH_SENSOR_MIDDLE,
                                     .callback = &on_obstacle_detected};
  touch_sensor_unsubscribe_id = touch_sensor_subscribe(sub);
}

void line_following_run(void) {
  // error in [-1,1]: negative = too far right, positive = too far left
  float error = line_sensor_get_error();
  int control_size = 0;
  uint8_t speed = 0;

  float scaled = P_VALUE * error;

  if (scaled > -0.5f && scaled < 0.5f) {
    // Avoid jitter for very small errors
    control_size = 0;
    speed = 100;
  } else {
    control_size = (int)(scaled > 0.0f ? scaled + 0.5f : scaled - 0.5f);
    control_size = clamp_int(control_size);
    speed = 100 - (error > 0 ? error : -error) * 100;
  }
  speed = speed > minimum_speed ? speed : minimum_speed;

  if (line_sensor_line_abruptly_ended()) {
    if (!is_in_gap) {
      is_in_gap = true;
      start_distance = wheel_encoder_get_current_distance().distance_right;
    }
    // Switch to line searching after the gap distance is travelled
    else if (wheel_encoder_get_current_distance().distance_right -
                 start_distance >=
             gap_distance) {
      is_in_gap = false;
      line_searching_reset();
      touch_sensor_unsubscribe(touch_sensor_unsubscribe_id);
      mission_control_set_state(LINE_SEARCHING);
    }
    // Drive straight when in a gap
    control_size = 0;
    speed = minimum_speed;
  } else {
    is_in_gap = false;
  }

  if (control_size > 0) {
    motors_drive_curve(speed, control_size, RIGHT);
  } else {
    motors_drive_curve(speed, -control_size, LEFT);
  }
}
