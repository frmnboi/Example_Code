#include <esp_log.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <iostream>

#include "Wheel.h"
#include "Encoder.h"
#include "IMU.h"

extern "C"
{
    void app_main(void);
}

static void mcpwm_example_brushed_motor_control(void *arg)
{
    MPWM asdf{16, 1000, MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM0A};
    asdf.setDuty((float)100);
    for (;;)
    {
        std::cout << "Cycle Has Repeated!" << std::endl;
        asdf.setDuty((float)0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        asdf.setDuty((float)100);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

static void wheelcontroller(void *arg)
{
    //Initialize Wheel Object

}

static void encoder_example(void *arg)
{
    Encoder testencoder{15, 2, 0};
    for (;;)
    {
        std::cout << testencoder.count() << std::endl;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

static void IMU_example(void *arg)
{
    //SDA-21
    //SCL-22
    IMU MPUIMU{}; 
    for (;;)
    {
        MPUIMU.getData();
        // vTaskDelay(1000/ portTICK_RATE_MS);
        std::cout<<MPUIMU.ax<<std::endl;
    }
}

void app_main()
{

    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_example_brushed_motor_control", 4096, NULL, 5, NULL);
    xTaskCreate(encoder_example, "Encodertest", 4096, NULL, 5, NULL);
    xTaskCreate(IMU_example, "IMU", 4096, NULL, 5, NULL);


// ESP_ERROR_CHECK(i2c_slave_init());
}
