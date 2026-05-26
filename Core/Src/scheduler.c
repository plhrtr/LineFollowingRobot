#include "scheduler.h"
#include "logger.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

// -------------------------------
// LOGGING SETTINGS FOR THIS FILE
// -------------------------------
static char scheduler_logging_enabled = 0;

static const log_module_t scheduler_log_module = {"scheduler_log_module",
                                                  &scheduler_logging_enabled};

#define MAX_TASKS sizeof(uint32_t)

static const float scheduler_tolerance = 10;

static uint32_t task_bitmap = 0;
static task_t tasks[MAX_TASKS];

int scheduler_schedule(task_t task) {
  if (task_bitmap == ~(uint32_t)0) {
    return -1;
  }

  uint16_t index = __builtin_ctz(~task_bitmap);

  // Mark the task as occupied
  uint32_t mask = (uint32_t)1 << index;
  task_bitmap |= mask;

  // Insert the task into the array
  tasks[index] = task;

  return index;
}

void scheduler_unschedule(uint16_t task_index) {
  if (task_index >= MAX_TASKS)
    return;

  uint32_t mask = (size_t)1 << task_index;

  task_bitmap &= ~mask;
};

void scheduler_run() {
  while (1) {

    uint32_t current_tick = HAL_GetTick();

    for (size_t i = 0; i < MAX_TASKS; i++) {
      uint32_t mask = (uint32_t)1 << i;

      if (!(task_bitmap & mask)) {
        continue;
      }

      task_t *task = &tasks[i];

      uint32_t elapsed = current_tick - task->next_run;

      if ((elapsed & 0x80000000UL) == 0) {
        if (elapsed > scheduler_tolerance) {
          LOGGER_LOG(LOG_WARNING, scheduler_log_module,
                     "Task %s: delayed by %u ms (should: %u, ran: %u)",
                     task->name, elapsed, task->next_run, current_tick);
        }

        task->task_run();

        if (task->period != UINT32_MAX) {
          task->next_run = current_tick + task->period;
        } else {
          scheduler_unschedule(i);
        }
      }
    }
  }
}
