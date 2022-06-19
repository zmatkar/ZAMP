/**
 * @file      scheduler.h
 * @brief     Simple scheduler (header file)
 * @author    Tomáš Košan
 * @date      21. 8. 2017
 * @version   1.0
 *
 */

#ifndef SHARED_SRC_SCHEDULER_H_
#define SHARED_SRC_SCHEDULER_H_

#ifdef __cplusplus
extern "C"
{
#endif

  /*!
   * Maximum number of tasks
   */
#define TASKS_MAX (16)

  /*!
   * Set to 1 to enable timing measurements.
   */
#define DEBUG_TIMING (0)

#define MEASURE_LOAD (0)

  typedef void
  (*task_fnc_ptr_t)(void);

  /*!
   *  Structure defining task.
   */
  typedef struct
  {
    uint32_t timestamp; /*!< Internal use, last run timestamp. */
    uint32_t period; /*!< Period (in number of sys_ticks periods) when function will be run. */
    uint32_t task_time; /*!< Time spend in task. */
    task_fnc_ptr_t task_fptr; /*!< Task/function to run. */
    uint16_t is_running; /*!< Flag of running task - avoid recursive calls. */
#if DEBUG_TIMING == 1
    uint32_t elapsed_ns; /*!< Elapsed time in scheduler ticks. */
    uint32_t ts_delta; /*!< Timestamp delta, i.e. delay of task execution. */
#endif
  } task_t;

  /*!
   * Main scheduler function. It will run tasks according to system ticks.
   */
  void
  SchedulerRun(void);

  /**
   * Setup HW to be used by scheduler. It uses TIMER0.
   * @param cpu_freq Frequency of CPU in MHz.
   * @param tick_period Required basic scheduler period in us.
   */
  void
  SchedulerInit(float tick_period);

  /**
   * Setup task to call passed function with provided period in scheduler ticks.
   * @param fptr Pointer to function to be called.
   * @param period Period of function calls (in scheduler ticks).
   */
  task_t*
  SchedulerAddTask(task_fnc_ptr_t fptr, uint32_t period);

  /**
   * Scheduler wait function, it allows other functions to run while waiting for some action.
   */
  void
  SchedulerWaitMs(uint16_t wait_ms);

  /**
   * Get scheduler ticks, overlaps after 49 days.
   */
  uint32_t SchedulerGetTicks(void);

#ifdef __cplusplus
}
#endif

#endif /* SHARED_SRC_SCHEDULER_H_ */
