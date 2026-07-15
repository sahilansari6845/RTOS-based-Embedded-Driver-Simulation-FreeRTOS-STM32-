#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdio.h>

/* This is the FreeRTOS configuration for the GCC/Posix "simulator" port.
 * It lets the whole RTOS + driver-simulation project run and be tested
 * as a normal Linux process (each FreeRTOS task = one pthread), while
 * the driver code itself (Drivers/) is written exactly as it would be
 * for a real STM32 target. */

#define projCOVERAGE_TEST                       0

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
#define configUSE_TICKLESS_IDLE                  0
#define configCPU_CLOCK_HZ                       ( 100000000UL )
#define configTICK_RATE_HZ                       ( 1000 )
#define configMAX_PRIORITIES                     ( 7 )
#define configMINIMAL_STACK_SIZE                  ( configSTACK_DEPTH_TYPE ) 4000
#define configMAX_TASK_NAME_LEN                  ( 16 )
#define configTICK_TYPE_WIDTH_IN_BITS             TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                   1
#define configUSE_TASK_NOTIFICATIONS              1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES     1
#define configUSE_MUTEXES                         1
#define configUSE_RECURSIVE_MUTEXES               1
#define configUSE_COUNTING_SEMAPHORES             1
#define configQUEUE_REGISTRY_SIZE                 20
#define configUSE_QUEUE_SETS                      1
#define configUSE_TIME_SLICING                    1
#define configUSE_NEWLIB_REENTRANT                0
#define configENABLE_BACKWARD_COMPATIBILITY       0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS   5
#define configSTACK_DEPTH_TYPE                    size_t
#define configMESSAGE_BUFFER_LENGTH_TYPE           size_t
#define configNUMBER_OF_CORES                     1

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION           0
#define configSUPPORT_DYNAMIC_ALLOCATION          1
#define configTOTAL_HEAP_SIZE                      ( ( size_t ) ( 1024 * 1024 ) )
#define configAPPLICATION_ALLOCATED_HEAP           0

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                       0
#define configUSE_TICK_HOOK                       0
#define configCHECK_FOR_STACK_OVERFLOW             2
#define configUSE_MALLOC_FAILED_HOOK               1
#define configUSE_DAEMON_TASK_STARTUP_HOOK         0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS              0
#define configUSE_TRACE_FACILITY                   1
#define configUSE_STATS_FORMATTING_FUNCTIONS       1

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                      0
#define configMAX_CO_ROUTINE_PRIORITIES            1

/* Software timer related definitions. */
#define configUSE_TIMERS                           1
#define configTIMER_TASK_PRIORITY                  ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                   10
#define configTIMER_TASK_STACK_DEPTH                configMINIMAL_STACK_SIZE

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                   1
#define INCLUDE_uxTaskPriorityGet                  1
#define INCLUDE_vTaskDelete                        1
#define INCLUDE_vTaskSuspend                       1
#define INCLUDE_xTaskDelayUntil                    1
#define INCLUDE_vTaskDelay                         1
#define INCLUDE_xTaskGetSchedulerState             1
#define INCLUDE_xTaskGetCurrentTaskHandle          1
#define INCLUDE_uxTaskGetStackHighWaterMark        0
#define INCLUDE_xTaskGetIdleTaskHandle             0
#define INCLUDE_eTaskGetState                      1
#define INCLUDE_xTimerPendFunctionCall             1
#define INCLUDE_xTaskAbortDelay                    1
#define INCLUDE_xTaskGetHandle                     1
#define INCLUDE_xTaskResumeFromISR                 1

/* This demo makes use of one or more example stats formatting functions which
 * make use of the sprintf() function.  sprintf() is provided by the same
 * compiler library that supplies the standard C library string functions. */
#define configASSERT( x )   if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* Map standard names to POSIX port equivalents. */
#define vPortSVCHandler        SVC_Handler
#define xPortPendSVHandler     PendSV_Handler
#define xPortSysTickHandler    SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
