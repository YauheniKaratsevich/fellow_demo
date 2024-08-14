set(SRCS_FILES
    "main.c"
    "thermostat.c"
    "cli_cmds/cli_cmds.c"
    "heater/heater.c"
)

set(INC_DIRS
    "."
    "cli_cmds"
    "heater"
)

idf_component_register(SRCS "${SRCS_FILES}"
                       INCLUDE_DIRS "${INC_DIRS}"
                       PRIV_REQUIRES esp_adc_cal console
                       )
