#pragma once

enum commands_t {
    NO_ACT,
    SET_TEMP,
    SET_HYST,
    GET_TEMP,
    GET_THERMO_STATUS,
    SET_THERMO,
};

struct cli_message_t {
    enum commands_t action;
    double data;
};

typedef struct cli_message_t cli_message;

void cli_cmds_get_msg(cli_message *msg);
void cli_cmds_console_init(void);
