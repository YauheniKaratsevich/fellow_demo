#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void thermostat_task(void* pvParams);
void cli_cmds_console_init(void);

void app_main(void)
{
    cli_cmds_console_init();
    xTaskCreate(thermostat_task, "thermostat_task", 4096, NULL, 2, NULL);

    vTaskDelete(NULL);
}
