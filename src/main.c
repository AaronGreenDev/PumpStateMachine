#include <stdio.h>
#include "driver/uart.h"
#include "esp_system.h"

#define UART_PORT UART_NUM_0
#define BUFFER_SIZE 1024

typedef enum{
    EVENT_START,
    EVENT_STOP,
    EVENT_FAILURE,
    EVENT_RETURN_TO_NORMAL
} pump_event_t;

typedef enum{
    STATE_STOPPED,
    STATE_RUNNING,
    STATE_FAULT
} pump_state_t;

static pump_state_t current_state = STATE_STOPPED;

void uart_init() {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, BUFFER_SIZE * 2, 0, 0, NULL, 0);
 }

  pump_event_t parse_input(char c){
    switch (c) {
        case 's':
            return EVENT_START;
        case 't':
            return EVENT_STOP;
        case 'f':
            return EVENT_FAILURE;
        case 'r':
            return EVENT_RETURN_TO_NORMAL;
        default:
            return -1; // Invalid Input
    }
 }

 void handle_event(pump_event_t event) 
{
    switch (current_state)
     {
        case STATE_STOPPED:
            if (event == EVENT_START) 
            {
                current_state = STATE_RUNNING;
                printf("Pump started.\n");
            }
             else if (event == EVENT_FAILURE) 
            {
                current_state = STATE_FAULT;
                printf("Pump failure detected.\n");
            }
            break;
        case STATE_RUNNING:
            if (event == EVENT_STOP)
            {
                current_state = STATE_STOPPED;
                printf("Pump stopped.\n");
            } else if (event == EVENT_FAILURE) 
            {
                current_state = STATE_FAULT;
                printf("Pump failure detected.\n");
            }
            break;
        case STATE_FAULT:
            if (event == EVENT_RETURN_TO_NORMAL) 
            {
                current_state = STATE_STOPPED;
                printf("Pump returned to normal operation.\n");
            }
            else if(event == EVENT_START)
            {
                printf("Cannot start pump. It is in FAULT state.\n");
            }
            break;
    }
}

void app_main(void)
{
    uart_init();
    char data;
    while(1)
    {
        int len = uart_read_bytes(UART_PORT, &data,1 ,20 / portTICK_PERIOD_MS);
        if(len > 0)
        {
            if(data == '?')
            {
                switch(current_state) 
                {
                    case STATE_STOPPED:
                        printf("Current State: STOPPED\n");
                        break;
                    case STATE_RUNNING:
                        printf("Current State: RUNNING\n");
                        break;
                    case STATE_FAULT:
                        printf("Current State: FAULT\n");
                        break;
                }
            }
            else
            {
                pump_event_t event = parse_input(data);
                if(event == -1){
                    printf("Invalid command received\n");
                } else {
                    handle_event(event);
                }
            }
        }
    }
}
