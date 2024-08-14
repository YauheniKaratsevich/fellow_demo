#include "driver/gpio.h"

#define HEATER_PIN GPIO_NUM_14
void heater_init(void)
{
    gpio_config_t io_conf = {0};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT | GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = BIT64(HEATER_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf);

    gpio_set_level(HEATER_PIN, 1); // Dout = 1 - heater off
}

void heater_set_on(void)
{
    gpio_set_level(HEATER_PIN, 0); // Dout = 0 - heater on 
}

void heater_set_off(void)
{
    gpio_set_level(HEATER_PIN, 1); // Dout = 1 - heater off 
}

int heater_get_status(void)
{
    return !gpio_get_level(HEATER_PIN);
}
