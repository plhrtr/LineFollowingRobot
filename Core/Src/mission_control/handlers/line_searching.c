#include "mission_control/handlers/line_searching.h"
#include "mission_control/handlers/line_following.h"
#include "mission_control/mission_control.h"
#include "services/line_sensor_service.h"
#include "services/motor_service.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  SEARCH_STATE_INIT,
  SEARCH_STATE_FIRST,
  SEARCH_STATE_SECOND,
  SEARCH_STATE_BACK,
  SEARCH_STATE_BREAK
} search_state_t;

static const uint32_t SEARCH_DURATION = 400;
static const uint32_t BREAK_DURATION = 200;

static search_state_t search_state = SEARCH_STATE_INIT;
static search_state_t pending_state = SEARCH_STATE_INIT;
static search_direction_t start_direction = SEARCH_LEFT;
static uint32_t start_time = 0;

void line_searching_run() {
  // Check whether the line is found (only after traveling a short distance
  // forward to avoid false positives)
  if (search_state != SEARCH_STATE_INIT) {
    line_sensor_get_error();
    if (line_sensor_is_on_line()) {
      line_searching_reset(SEARCH_LEFT);
      line_following_init();
      mission_control_set_state(LINE_FOLLOWING);
      return;
    }
  }

  uint32_t now = HAL_GetTick();

  switch (search_state) {
  case SEARCH_STATE_INIT:
    start_time = now + SEARCH_DURATION;
    search_state = SEARCH_STATE_FIRST;
    switch (start_direction) {
    case SEARCH_LEFT:
      motors_drive_curve(55, 100, LEFT);
      break;
    case SEARCH_RIGHT:
      motors_drive_curve(55, 100, RIGHT);
      break;
    }
    break;
  case SEARCH_STATE_FIRST:
    if ((int32_t)(now - start_time) >= 0) {
      motors_stop();
      start_time = now + BREAK_DURATION;
      pending_state = SEARCH_STATE_SECOND;
      search_state = SEARCH_STATE_BREAK;
    }
    break;
  case SEARCH_STATE_SECOND:
    if ((int32_t)(now - start_time) >= 0) {
      motors_stop();
      start_time = now + BREAK_DURATION;
      pending_state = SEARCH_STATE_BACK;
      search_state = SEARCH_STATE_BREAK;
    }
    break;
  case SEARCH_STATE_BREAK:
    if ((int32_t)(now - start_time) >= 0) {
      if (pending_state == SEARCH_STATE_SECOND) {
        start_time = now + 1.8f * SEARCH_DURATION;
        search_state = SEARCH_STATE_SECOND;
        switch (start_direction) {
        case SEARCH_LEFT:
          motors_drive_curve(50, 100, RIGHT);
          break;
        case SEARCH_RIGHT:
          motors_drive_curve(50, 100, LEFT);
          break;
        }
      } else if (pending_state == SEARCH_STATE_BACK) {
        start_time = now + SEARCH_DURATION;
        search_state = SEARCH_STATE_BACK;
        switch (start_direction) {
        case SEARCH_LEFT:
          motors_drive_curve(50, 100, LEFT);
          break;
        case SEARCH_RIGHT:
          motors_drive_curve(50, 100, RIGHT);
          break;
        }
      } else {
        /* fallback */
        line_searching_reset(start_direction);
        mission_control_set_state(OVERCOME_GAP);
      }
    }
    break;
  case SEARCH_STATE_BACK:
    if ((int32_t)(now - start_time) >= 0) {
      line_searching_reset(start_direction);
      mission_control_set_state(OVERCOME_GAP);
    }
    break;
  }
}

void line_searching_reset(search_direction_t direction) {
  start_time = 0;
  search_state = SEARCH_STATE_INIT;
  pending_state = SEARCH_STATE_INIT;
  start_direction = direction;
}
