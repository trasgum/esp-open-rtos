/* Very basic example that just demonstrates we can run at all!
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "espressif/esp_common.h"
#include "esp8266.h"

#include <string.h>

#include "esp/uart.h"
#include "ds18b20/ds18b20.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "ssid_config.h"

#include "utils.h"
#include "home_iot.h"

#define WEB_SERVER "192.168.2.102"
#define WEB_PORT 8000

#define LED_GPIO 4
#define SENSOR_GPIO 13
#define MAX_SENSORS 2
#define RESCAN_INTERVAL 10
#define TEMP_DELAY_MS 2500

//static xQueueHandle mainqueue;

void read_temperature(void *pvParameters) 
{
    ds18b20_addr_t addrs[MAX_SENSORS];
    float temps[MAX_SENSORS];
    int sensor_count;
    ds18_read sensor;
    xQueueHandle *q_temp = (xQueueHandle *)pvParameters;

    gpio_set_pullup(SENSOR_GPIO, true, true);
    while(1) {
        sensor_count = ds18b20_scan_devices(SENSOR_GPIO, addrs, MAX_SENSORS);
        if (sensor_count < 1) {
            printf("\nNo sensors detected!\n");
        }else {
            printf("\n%d sensors detected.\n", sensor_count);
            if (sensor_count > MAX_SENSORS) sensor_count = MAX_SENSORS;

            // Do as much as temperature samples and send the results to print task.
            for (int i = 0; i < RESCAN_INTERVAL; i++) {
                ds18b20_measure_and_read_multi(SENSOR_GPIO, addrs, sensor_count, temps);
                for (int j = 0; j < sensor_count; j++){
                    // The DS18B20 address is a 64-bit integer, but newlib-nano
                    // printf does not support printing 64-bit values, so we
                    // split it up into two 32-bit integers and print them
                    // back-to-back to make it look like one big hex number.
                    uint32_t addr0 = addrs[j] >> 32;
                    uint32_t addr1 = addrs[j];
                    float temp_c = temps[j];
                    float temp_f = (temp_c * 1.8) + 32;

                    sensor.addr0 = addr0;
                    sensor.addr1 = addr1;
                    sensor.temp_c = temp_c;
                    sensor.temp_f = temp_f;

                    // TODO: create a structure with: addr0, addr1, temp_c to send to the queue
                    //printf("  Sensor %08x%08x reports %f deg C (%f deg F)\n", addr0, addr1, temp_c, temp_f);
                    //xQueueSend(*q_temp, &temp_c, 0);
                    xQueueSend(*q_temp, &sensor, 0);
                }
                // Wait for a little bit between each sample (note that the
                // ds18b20_measure_and_read_multi operation already takes at
                // least 750ms to run, so this is on top of that delay).
                vTaskDelay(TEMP_DELAY_MS / portTICK_RATE_MS);
            }
        }
    }
}

void print_temperature(void *pvParameters) 
{
    xQueueHandle *q_temp = (xQueueHandle *)pvParameters;
    int time;
    while(1){
        //float temp_c;
        ds18_read recv_temp;
        int port;
        port = WEB_PORT;  

        //if(xQueueReceive(*q_temp, &temp_c, 500)){
        if(xQueueReceive(*q_temp, &recv_temp, 500)){
            time = http_get_time(WEB_SERVER, &port);
            printf("\n[%d] Received temp: %f from: %x-%x", time, 
                    recv_temp.temp_c, recv_temp.addr0, recv_temp.addr1);
        }else {
            printf("\nNo temp received");
        }
    }
}

void blinkled(void *pvParameters)
{
    gpio_enable(LED_GPIO, GPIO_OUTPUT);
    while(1) {
        gpio_write(LED_GPIO, 1);
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_write(LED_GPIO, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

/*
void task1(void *pvParameters)
{
    xQueueHandle *queue = (xQueueHandle *)pvParameters;
    printf("Hello from task1!\r\n");
    uint32_t count = 0;
        while(1) {
            vTaskDelay(100);
        xQueueSend(*queue, &count, 0);
        count++;
    }
}

void task2(void *pvParameters)
{
    printf("Hello from task 2!\r\n");
    xQueueHandle *queue = (xQueueHandle *)pvParameters;
    while(1) {
        uint32_t count;
        if(xQueueReceive(*queue, &count, 1001)) {
            printf("Got %u\n", count);
        } else {
            printf("No msg :(\n");
        }
    }
}
*/

static xQueueHandle tempqueue;

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    //Config WIFI conn
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    //Queue management
    tempqueue = xQueueCreate(10, sizeof(ds18_read));
    //mainqueue = xQueueCreate(10, sizeof(uint32_t));
    
    //Task Management
    //xTaskCreate(task1, (signed char *)"tsk1", 256, &mainqueue, 2, NULL);
    //xTaskCreate(task2, (signed char *)"tsk2", 256, &mainqueue, 2, NULL);
    xTaskCreate(blinkled, (signed char *)"blink_led", 256, NULL, 2, NULL);
    xTaskCreate(read_temperature, (signed char *)"read_temp", 256, &tempqueue, 2, NULL);
    xTaskCreate(print_temperature, (signed char *)"print_temp", 256, &tempqueue, 2, NULL);
    //xTaskCreate(post_http_temperature, (signed char *)"send_temp", 256, &tempqueue, 2, NULL);

}
