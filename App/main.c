/**
 ******************************************************************************
 * RTOS-based Embedded Driver Simulation (FreeRTOS + STM32)
 *
 * This application simulates a small STM32 firmware stack running under
 * FreeRTOS, without any physical board attached. It demonstrates:
 *
 *   - GPIO driver   : LED blink task + button-driven event
 *   - UART driver   : interrupt-style RX (queue) + mutex-protected TX
 *   - ADC driver    : periodic polled sampling of a simulated sensor
 *   - Task sync     : mutex (shared UART), queue (button events),
 *                     binary semaphore (ADC "conversion complete" signal)
 *   - System task   : reports free heap / task count, like a watchdog/
 *                     diagnostics task on a real product
 *
 * Build/run as a normal Linux process via the FreeRTOS POSIX simulator
 * port -- every FreeRTOS task maps to one pthread, the tick is a POSIX
 * timer. Driver code (Drivers/) is written exactly as it would be for
 * real STM32 silicon; only main()'s startup differs from a real target.
 ******************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stm32_regmap.h"
#include "gpio_driver.h"
#include "uart_driver.h"
#include "adc_driver.h"
#include "hardware_sim.h"

/* ------------------------------------------------------------------------ */
/*  Shared RTOS objects                                                     */
/* ------------------------------------------------------------------------ */
static SemaphoreHandle_t xUartMutex;        /* protects the shared UART TX line   */
static QueueHandle_t     xButtonEventQueue; /* GPIO ISR-ish -> LED task            */
static SemaphoreHandle_t xAdcReadySem;      /* "ADC conversion complete" signal    */

/* Convenience: thread-safe printf-over-UART used by every task */
static void UART_Printf(const char *tag, const char *msg)
{
    if (xSemaphoreTake(xUartMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        char line[128];
        snprintf(line, sizeof(line), "[%-10s] %s\r\n", tag, msg);
        UART_SendString(USART2, line);
        xSemaphoreGive(xUartMutex);
    }
}

/* ------------------------------------------------------------------------ */
/*  Task: LED blink driven by GPIO driver, rate controlled by button events */
/* ------------------------------------------------------------------------ */
static void vTask_LedBlink(void *pv)
{
    (void)pv;
    TickType_t period = pdMS_TO_TICKS(500);
    BaseType_t event;

    GPIO_Init(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT);

    for (;;) {
        /* Non-blocking peek at button-event queue to adjust blink speed */
        uint8_t dummy;
        event = xQueueReceive(xButtonEventQueue, &dummy, 0);
        if (event == pdTRUE) {
            period = (period == pdMS_TO_TICKS(500)) ? pdMS_TO_TICKS(150)
                                                     : pdMS_TO_TICKS(500);
            UART_Printf("LED", period == pdMS_TO_TICKS(150)
                                 ? "Fast-blink mode (button pressed)"
                                 : "Normal-blink mode");
        }

        GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        UART_Printf("LED", GPIO_ReadPin(GPIOA, GPIO_PIN_5) ? "ON" : "OFF");
        vTaskDelay(period);
    }
}

/* ------------------------------------------------------------------------ */
/*  Task: polls the button pin (simulated GPIO input) and posts an event    */
/* ------------------------------------------------------------------------ */
static void vTask_ButtonMonitor(void *pv)
{
    (void)pv;
    GPIO_Init(GPIOA, GPIO_PIN_13, GPIO_MODE_INPUT);
    GPIO_PinState_t last = GPIO_PIN_RESET;

    for (;;) {
        GPIO_PinState_t now = GPIO_ReadPin(GPIOA, GPIO_PIN_13);
        if (now == GPIO_PIN_SET && last == GPIO_PIN_RESET) {
            uint8_t evt = 1;
            xQueueSend(xButtonEventQueue, &evt, 0);
            UART_Printf("BUTTON", "Press detected -> event queued");
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(50)); /* debounce-style poll rate */
    }
}

/* ------------------------------------------------------------------------ */
/*  Task: UART RX handling - blocks on the ISR-fed queue, echoes commands   */
/* ------------------------------------------------------------------------ */
static void vTask_UartRx(void *pv)
{
    (void)pv;
    char buf[64];
    size_t idx = 0;
    char c;

    for (;;) {
        if (UART_ReceiveChar(USART2, &c, pdMS_TO_TICKS(1000))) {
            if (c == '\n' || idx >= sizeof(buf) - 1) {
                buf[idx] = '\0';
                char resp[96];
                snprintf(resp, sizeof(resp), "RX cmd: \"%s\" -> ACK", buf);
                UART_Printf("UART-RX", resp);
                idx = 0;
            } else if (c != '\r') {
                buf[idx++] = c;
            }
        }
    }
}

/* ------------------------------------------------------------------------ */
/*  Task: periodic ADC sampling (simulated sensor), signals via semaphore   */
/* ------------------------------------------------------------------------ */
static void vTask_AdcSample(void *pv)
{
    (void)pv;
    ADC_Init(ADC1);

    for (;;) {
        uint16_t raw = ADC_ReadChannel(ADC1);
        double voltage = (raw / 4095.0) * 3.3;

        xSemaphoreGive(xAdcReadySem); /* signal "data ready" to anyone listening */

        char msg[64];
        snprintf(msg, sizeof(msg), "raw=%4u  voltage=%.2fV", raw, voltage);
        UART_Printf("ADC", msg);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ------------------------------------------------------------------------ */
/*  Task: waits on the ADC-ready semaphore to demonstrate sync between      */
/*  a producer (ADC task) and a consumer (e.g. a data-logging task)        */
/* ------------------------------------------------------------------------ */
static void vTask_DataLogger(void *pv)
{
    (void)pv;
    uint32_t sample_count = 0;

    for (;;) {
        if (xSemaphoreTake(xAdcReadySem, portMAX_DELAY) == pdTRUE) {
            sample_count++;
            if (sample_count % 5 == 0) {
                char msg[64];
                snprintf(msg, sizeof(msg), "Logged %lu ADC samples so far",
                         (unsigned long)sample_count);
                UART_Printf("LOGGER", msg);
            }
        }
    }
}

/* ------------------------------------------------------------------------ */
/*  Task: system diagnostics - free heap, task count (like a watchdog task) */
/* ------------------------------------------------------------------------ */
static void vTask_SystemMonitor(void *pv)
{
    (void)pv;
    for (;;) {
        char msg[96];
        snprintf(msg, sizeof(msg),
                 "Free heap: %zu bytes | Active tasks: %lu | Uptime: %lu ticks",
                 (size_t)xPortGetFreeHeapSize(),
                 (unsigned long)uxTaskGetNumberOfTasks(),
                 (unsigned long)xTaskGetTickCount());
        UART_Printf("SYSMON", msg);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/* ------------------------------------------------------------------------ */
/*  Entry point                                                              */
/* ------------------------------------------------------------------------ */
int main(void)
{
    printf("=======================================================\r\n");
    printf(" RTOS-based Embedded Driver Simulation (FreeRTOS+STM32)\r\n");
    printf(" Host: Linux (POSIX simulator port) -- Ctrl+C to stop\r\n");
    printf("=======================================================\r\n\n");

    UART_Init(USART2, 115200);

    xUartMutex        = xSemaphoreCreateMutex();
    xButtonEventQueue = xQueueCreate(8, sizeof(uint8_t));
    xAdcReadySem      = xSemaphoreCreateBinary();

    xTaskCreate(vTask_LedBlink,      "LED_Task",    configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vTask_ButtonMonitor, "BTN_Task",    configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vTask_UartRx,        "UART_RX",     configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(vTask_AdcSample,     "ADC_Task",    configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vTask_DataLogger,    "Logger_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTask_SystemMonitor, "SysMon_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTask_HardwareSim,   "HW_Sim",      configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    /* Only reached if there isn't enough heap to start the scheduler */
    for (;;) { }
    return 0;
}

/* ------------------------------------------------------------------------ */
/*  Required FreeRTOS hook functions                                        */
/* ------------------------------------------------------------------------ */
void vApplicationMallocFailedHook(void)
{
    fprintf(stderr, "FATAL: FreeRTOS malloc failed\n");
    exit(1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    fprintf(stderr, "FATAL: Stack overflow in task %s\n", pcTaskName);
    exit(1);
}
