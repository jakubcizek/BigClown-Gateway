#include <application.h>

uint8_t read_fifo_buffer[1024];
bc_fifo_t read_fifo;
uint8_t write_fifo_buffer[1024];
bc_fifo_t write_fifo;
uint64_t peer_id;

void uart_init()
{
    bc_fifo_init(&read_fifo, read_fifo_buffer, sizeof(read_fifo_buffer));
    bc_fifo_init(&write_fifo, write_fifo_buffer, sizeof(write_fifo_buffer));
    bc_uart_init(BC_UART_UART2, BC_UART_BAUDRATE_115200, BC_UART_SETTING_8N1);
    bc_uart_set_async_fifo(BC_UART_UART2, &write_fifo, &read_fifo);
}

static void stop_pairing(void* param)
{
    (void) param;

    bc_radio_pairing_mode_start();
    /*
    uint64_t devices_address[BC_RADIO_MAX_DEVICES];
    bc_radio_get_peer_id(devices_address, BC_RADIO_MAX_DEVICES);
    bc_uart_async_write(BC_UART_UART2, "Sparovane peery:\r\n", 18);
    for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
	{
		if (devices_address[i] != 0)
		{
            sprintf(buffer, "Peer ID:  %lld\r\n", devices_address[i]); 
            bc_uart_async_write(BC_UART_UART2, buffer, strlen(buffer));
            bc_radio_peer_device_remove(devices_address[i]);
		}
	}
    */
}

/*
void radio_event(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    if(event == BC_RADIO_EVENT_ATTACH){;}
    else if(event == BC_RADIO_EVENT_DETACH){;}
    else if(event == BC_RADIO_EVENT_ATTACH_FAILURE){;}
    else if(event == BC_RADIO_EVENT_INIT_DONE){;}
}
*/

void bc_radio_pub_on_int(uint64_t *id, char *subtopic, int *value)
{
    char bf[100];
    if(strcmp(subtopic, "bat-pct") == 0)
    {
        sprintf(bf, "BCD;ID:%" PRIu64 ";C:%d\r\n", *id, *value); 
        bc_uart_async_write(BC_UART_UART2, bf, strlen(bf));
    }
    else if(strcmp(subtopic, "bat-alarm") == 0)
    {
        sprintf(bf, "BCA;ID:%" PRIu64 ";ALRBAT:%d\r\n", *id, *value); 
        bc_uart_async_write(BC_UART_UART2, bf, strlen(bf));
    }
}

void bc_radio_pub_on_buffer(uint64_t *id, uint8_t *buffer, size_t length)
{
    (void)length;

    char bf[100];
    int16_t temperature_c;
    float temperature;
    uint16_t light;
    uint8_t humidity;
    uint16_t pressure_c;
    float pressure;

    memcpy(&temperature_c, &buffer[0], 2);
    memcpy(&humidity, &buffer[2], 1);
    memcpy(&light, &buffer[3], 2);
    memcpy(&pressure_c, &buffer[5], 2);

    temperature = temperature_c / 100.0f;
    pressure = (pressure_c / 100.0f) + 900;

    sprintf(bf, "BCD;ID:%" PRIu64 ";T:%.2f;H:%d;L:%d;P:%.2f\r\n", *id, temperature, humidity, light, pressure); 
    bc_uart_async_write(BC_UART_UART2, bf, strlen(bf));
}

void bc_radio_pub_on_battery(uint64_t *id, float *voltage)
{
    char bf[100];
    sprintf(bf, "BCD;ID:%" PRIu64 ";V:%.2f\r\n", *id, *voltage); 
    bc_uart_async_write(BC_UART_UART2, bf, strlen(bf));
}

void application_init(void)
{
    uart_init();
    bc_radio_init(BC_RADIO_MODE_GATEWAY);
    bc_radio_automatic_pairing_start();
    bc_radio_pairing_mode_start();
    bc_scheduler_register(stop_pairing, NULL, bc_tick_get() + 3000);
    //bc_radio_set_event_handler(radio_event, NULL);
}