#include "mission_control/mission_control.h"
#include "mission_control/handlers/line_following.h"
#include "mission_control/handlers/line_searching.h"
#include "mission_control/handlers/obstacle_avoidance.h"
#include "mission_control/handlers/overcome_gap.h"
#include "mission_control/handlers/yellow_line_following.h"
#include <stdbool.h>
#include <stdint.h>

const uint16_t MISSION_CONTROL_PERIOD = 2;

// The current state the mission control is in
static mission_control_state_t current_state = YELLOW_LINE_FOLLOWING;

void mission_control_init() { yellow_line_following_init(); }

void mission_control_run() {
  switch (current_state) {
  case YELLOW_LINE_FOLLOWING:
    // Always transitions to [LINE_SEARCHING]
    yellow_line_following_run();
    break;
  case LINE_SEARCHING:
    // If a line is found -> Switch to [LINE_FOLLOWING]
    // If not transitions to [OVERCOME_GAP]
    line_searching_run();
    break;
  case LINE_FOLLOWING:
    // If the line is lost transitions to [LINE_SEARCHING]
    // If a touch sensor press is detected transitions to [OBSTACLE_AVOIDANCE]
    line_following_run();
    break;
  case OBSTACLE_AVOIDANCE:
    // Always transitions to [LINE_SEARCHING]
    obstacle_avoidance_run();
    break;
  case OVERCOME_GAP:
    // If a line is found -> Switch to [LINE_FOLLOWING]
    // If not transitions after specified travel distance to [LINE_SEARCHING]
    overcome_gap_run();
    break;
  default:
    mission_control_set_state(LINE_SEARCHING);
    break;
  }
}

void mission_control_set_state(mission_control_state_t state) {
  current_state = state;
}

mission_control_state_t mission_control_get_state() { return current_state; }
