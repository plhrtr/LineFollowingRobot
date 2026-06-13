#include "mission_control/handlers/line_following.h"
#include "mission_control/handlers/line_searching.h"
#include "mission_control/mission_control.h"
#include "services/line_sensor_service.h"
#include "services/motor_service.h"
#include "services/wheel_encoder_service.h"
#include <stdint.h>

#define P_VALUE 100

static const uint16_t gap_distance = 25;

static uint8_t minimum_speed = 40;
static bool is_in_gap = false;
static uint16_t start_distance = 0;

static int clamp_int(int value) {
  if (value < -100) {
    return -100;
  } else if (value > 100) {
    return 100;
  } else {
    return value;
  }
}

void line_following_run(void) {
  // TODO: Check whether line ended abruptly

  // error in [-1,1]: negative = too far right, positive = too far left
  float error = line_sensor_get_error();
  int control_size = 0;
  uint8_t speed = 0;

  float scaled = P_VALUE * error;

  // Deadband to avoid actuator chatter for very small errors
  if (scaled > -0.5f && scaled < 0.5f) {
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
    } else if (wheel_encoder_get_current_distance().distance_right -
                   start_distance >=
               gap_distance) {
      is_in_gap = false;
      line_searching_reset();
      mission_control_set_state(LINE_SEARCHING);
    }
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
