#include "mission_control/handlers/obstacle_avoidance.h"
#include "mission_control/handlers/line_following.h"
#include "mission_control/handlers/line_searching.h"
#include "mission_control/mission_control.h"
#include "services/line_sensor_service.h"
#include "services/motor_service.h"
#include "services/waypoint_navigation.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

static const uint32_t SEARCH_TIMEOUT = 3000;

typedef enum {
  OBSTACLE_AVOIDANCE_INIT,
  OBSTACLE_AVOIDANCE_WAYPOINT,
  OBSTACLE_AVOIDANCE_SEARCH,
} obstacle_avoidance_states_t;

obstacle_avoidance_states_t state = OBSTACLE_AVOIDANCE_INIT;

// List of waypoints to avoid a detected object
static waypoint_navigation_task_t obstacle_avoidance_tasks[] = {
    {DRIVE_BACKWARDS, 30},
    {TURN_RIGHT, 90},
    {DRIVE_STRAIGHT, 110},
    {TURN_LEFT, 90},
};

/**
 * Callback that gets executed when the waypoint navigation finishes
 */
static void on_finish_obstacle_avoidance() {
  motors_drive_curve(80, 10, LEFT);
  state = OBSTACLE_AVOIDANCE_SEARCH;
}
static uint32_t search_start = 0;

void obstacle_avoidance_init() {
  waypoint_navigation_set_tasks(obstacle_avoidance_tasks,
                                sizeof(obstacle_avoidance_tasks) /
                                    sizeof(waypoint_navigation_task_t),
                                &on_finish_obstacle_avoidance);
}

void obstacle_avoidance_run() {
  switch (state) {
  case OBSTACLE_AVOIDANCE_INIT:
    waypoint_navigation_set_tasks(obstacle_avoidance_tasks,
                                  sizeof(obstacle_avoidance_tasks) /
                                      sizeof(waypoint_navigation_task_t),
                                  &on_finish_obstacle_avoidance);
    state = OBSTACLE_AVOIDANCE_WAYPOINT;
    break;
  case OBSTACLE_AVOIDANCE_WAYPOINT:
    waypoint_navigation_run();
    break;
  case OBSTACLE_AVOIDANCE_SEARCH:
    if (search_start == 0) {
      search_start = HAL_GetTick();
    }
    // Check whether the robot found the line
    line_sensor_get_error();
    if (line_sensor_is_on_line()) {
      state = OBSTACLE_AVOIDANCE_INIT;
      search_start = 0;
      line_following_init();
      mission_control_set_state(LINE_FOLLOWING);
      return;
    }

    // Timeout
    if ((int32_t)(HAL_GetTick() - search_start) >= SEARCH_TIMEOUT) {
      state = OBSTACLE_AVOIDANCE_INIT;
      search_start = 0;
      line_searching_reset(SEARCH_LEFT);
      mission_control_set_state(LINE_SEARCHING);
    }
    break;
  }
}
