#include <string.h>

#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "cli_cmds.h"

static const char* TAG = "cli";

static xQueueHandle queue_cli = NULL;

static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();

    return 0;
}

static void register_restart(void)
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int set_temperature(int argc, char **argv)
{
    if (argc != 2) {
        return -1;
    }

    struct cli_message_t msg;
    msg.action = SET_TEMP;
    sscanf(argv[1], "%lf", &msg.data);

    xQueueSend(queue_cli, &msg, portMAX_DELAY);

    return 0;
}

static void register_set_temperature(void)
{
    const esp_console_cmd_t cmd = {
        .command = "T_set",
        .help = "Temperature to be achieved and maintained.",
        .hint = NULL,
        .func = &set_temperature,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int set_hysteresis(int argc, char **argv)
{
    if (argc != 2) {
        return -1;
    }

    struct cli_message_t msg;
    msg.action = SET_HYST;
    sscanf(argv[1], "%lf", &msg.data);

    xQueueSend(queue_cli, &msg, portMAX_DELAY);

    return 0;
}

static void register_set_hysteresis(void)
{
    const esp_console_cmd_t cmd = {
        .command = "T_hyst",
        .help = "Temperature hysteresis.",
        .hint = NULL,
        .func = &set_hysteresis,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int get_temperature(int argc, char **argv)
{
    if (argc != 1) {
        return -1;
    }

    struct cli_message_t msg;
    msg.action = GET_TEMP;

    xQueueSend(queue_cli, &msg, portMAX_DELAY);

    return 0;
}

static void register_get_temperature(void)
{
    const esp_console_cmd_t cmd = {
        .command = "T_get",
        .help = "Get current temperature in C.",
        .hint = NULL,
        .func = &get_temperature,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int get_thermostat_status(int argc, char **argv)
{
    if (argc != 1) {
        return -1;
    }

    struct cli_message_t msg;
    msg.action = GET_THERMO_STATUS;

    xQueueSend(queue_cli, &msg, portMAX_DELAY);

    return 0;
}

static void register_get_thermostat_status(void)
{
    const esp_console_cmd_t cmd = {
        .command = "T_stat",
        .help = "Get thermostat status - on or off.",
        .hint = NULL,
        .func = &get_thermostat_status,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int set_thermostat(int argc, char **argv)
{
    if (argc != 2) {
        return -1;
    }

    struct cli_message_t msg;
    msg.action = SET_THERMO;
    if (strcmp(argv[1], "on") == 0)
    {
        // +1.0 is used as flag means "true"
        msg.data = +1.0;
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        // -1.0 is used as flag means "false"
        msg.data = -1.0;
    }
    else
    {
        printf("Wrong argument!\r\n");
        return -2;
    }

    xQueueSend(queue_cli, &msg, portMAX_DELAY);

    return 0;
}

static void register_set_thermostat(void)
{
    const esp_console_cmd_t cmd = {
        .command = "Thermostat",
        .help = "Set thermostat",
        .hint = NULL,
        .func = &set_thermostat,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void cli_cmds_get_msg(struct cli_message_t *msg)
{
    xQueueReceive(queue_cli, msg, 0);
}

static void register_cli_cmds(void)
{
    queue_cli = xQueueCreate(1, sizeof(struct cli_message_t));

    register_get_temperature();
    register_set_temperature();
    register_set_hysteresis();
    register_get_thermostat_status();
    register_set_thermostat();
    register_restart();
}

void cli_cmds_console_init(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = ">>";
    repl_config.max_cmdline_length = 1024;

    /* Register commands */
    esp_console_register_help_command();
    register_cli_cmds();

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
