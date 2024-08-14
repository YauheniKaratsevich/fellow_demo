/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "heater.h"
#include "cli_cmds.h"

#define ADC1_EXAMPLE_CHAN0          ADC1_CHANNEL_2
#define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_11
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP_FIT

#define DIV_RATIO 10.0/1000.0

#define ARRAY_LEN(x) sizeof(x)/sizeof(*x)

static const char *TAG = "thermo";

static esp_adc_cal_characteristics_t adc1_chars;

static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}

static double convert_v_to_c(uint32_t voltage_mv, double k_divider)
{
    return voltage_mv * k_divider;
}

static uint32_t filter(uint32_t voltage_mv)
{
    //TODO: implement sliding window filter

    return voltage_mv;
}
void thermostat_task(void* pvParams)
{
    (void)pvParams;

    uint32_t voltage_mv = 0;

    // Main thermostat settings
    double tempCurrent = 0.0;
    double tempSetpoint = 0.0;
    double tempHysteresis = 0.0;

    // States of thermostat state machine
    enum { MONITOR, WARM_UP, COLD_DOWN } state = COLD_DOWN;

    // Cli message container
    cli_message msg = { .action = NO_ACT };

    //ADC config
    adc_calibration_init();
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));

    // Start with heater off
    heater_init();
    heater_set_off();

    while (1)
    {
        // Get and parse cli commands
        cli_cmds_get_msg(&msg);
        switch(msg.action)
        {
        case SET_TEMP:
            printf("\rTemperature setpoint switches from %0.2fC to ", tempSetpoint);
            tempSetpoint = msg.data;
            printf("%0.2fC\r\n", tempSetpoint);
            break;

        case SET_HYST:
            printf("\rHysteresis switches from %0.2fC to ", tempHysteresis);
            tempHysteresis = msg.data;
            printf("%0.2fC\r\n", tempHysteresis);
            break;

        case GET_TEMP:
            printf("\rCurrent temperature is %0.2fC\r\n", tempCurrent);
            break;

        case GET_THERMO_STATUS:
            printf("\rThermostat is %s\r\n", state == WARM_UP || state == COLD_DOWN ? "On" : "Off");
            break;

        case SET_THERMO:
            // Command to set On
            if (msg.data > 0.0)
            {
                if (state != WARM_UP && state != COLD_DOWN)
                {
                    printf("\rTemperature setpoint is %0.2fC\r\n", tempSetpoint);
                    printf("Hysteresis is %0.2fC\r\n", tempHysteresis);
                    printf("Current temperature is %0.2fC\r\n", tempCurrent);
                    printf("Thermostat is Off, let's turn it on.\r\n");
                    state = WARM_UP;
                }
                else
                {
                    printf("\rThermostat is On already.\r\n");
                }
            }
            // Command to set Off
            else if (msg.data < 0.0)
            {
                if (state == WARM_UP || state == COLD_DOWN)
                {
                    printf("\rTemperature setpoint is %0.2fC\r\n", tempSetpoint);
                    printf("Hysteresis is %0.2fC\r\n", tempHysteresis);
                    printf("Current temperature is %0.2fC\r\n", tempCurrent);
                    printf("Thermostat is On, let's turn it Off\r\n");
                    state = MONITOR;
                }
                else
                {
                    printf("\rThermostat is Off already.\r\n");
                }
            }
            break;

        case NO_ACT:
            break;

        default:
            ESP_LOGE(TAG,"Unexpected action type.");
        }
        // Flush message container
        msg.action = NO_ACT;

        // Capture voltage voltage from thermosensor and convert it to Celcius
        voltage_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_EXAMPLE_CHAN0), &adc1_chars);
        tempCurrent = convert_v_to_c(filter(voltage_mv), DIV_RATIO);

        // Thermostat state machine
        switch(state)
        {
        case MONITOR:
            printf("\rCurrent temperature is %0.2fC\r", tempCurrent);
            break;

        case WARM_UP:
            // Current temperature is too high
            if (tempCurrent > tempSetpoint + tempHysteresis) {
                heater_set_off();
                state = COLD_DOWN;
                ESP_LOGI(TAG, "Too hot, cold down");
            }
            else
            {
                if (!heater_get_status()) {
                    ESP_LOGI(TAG, "Heater turn on");
                    heater_set_on();
                }
            }
            break;

        case COLD_DOWN:
            // Current temperature is too low
            if (tempCurrent < tempSetpoint - tempHysteresis) {
                heater_set_on();
                state = WARM_UP;
                ESP_LOGI(TAG, "Too cold, warm up");
            }
            else
            {
                if (heater_get_status()) {
                    ESP_LOGI(TAG, "Heater turn off");
                    heater_set_off();
                }
            }
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
