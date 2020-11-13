#include "I2C.h"
// #include "esp_common/include/esp_err.h"
//this uses the i2c interface with the ESP32 as the master device

//default constructor required because for the IMU unit, it has 2 I2C addresses, which prevents a direct inheritence.
//possibly, it could all be done in 1 object, but I prefer to have each I2C object be representative of a connection, and only have 1 address
I2C::I2C(){};

I2C::I2C(uint8_t adress)
{
    if (adress <= 0xEF) //127
    {
        slaveaddress = adress;
    }//otherwise error, future implementation

    if (!initialized){
    I2C::configureMaster();
    initialized=true;
    }
};

//For I2C, SCL is GPIO 22, pin 36, and SDA is GPIO 21, pin 33
void I2C::configureMaster()
{

    i2c_config_t config;

    //using the device as Master
    config.mode = I2C_MODE_MASTER;
    //For I2C, SCL is GPIO 22, pin 36, and SDA is GPIO 21, pin 33
    config.sda_io_num = 21;
    config.scl_io_num = 22;
    //For the actual device, using external pull-up resistors are recommended
    config.sda_pullup_en = true;
    config.scl_pullup_en = true;
    //I2C clock speed is currently maxed at 1MHz.  Using slightly reduced value for safety/reliability
    config.master.clk_speed = 1 * 100000;
    //x1 is the arduino max
    //x10 works. (x10 is the max listed as working in ESP32 docs)
    //x12 works as well.  
    //x25 seems to cause some errors, but can still go through without problems.

    //using port 0 as master (port 1 of 2)
    

    std::cout<<"Config status: "<<(int)i2c_param_config(I2C_NUM_0, &config)<<std::endl;
    //default parameters are used. See docs for how to change these

    //install i2c driver as a master
    //set buffers and values to 0
    std::cout<<"Driver status: "<<(int)i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0)<<std::endl;

    //values return 0, meaning both configs are working properly.
};

esp_err_t I2C::i2c_master_read_slave(uint8_t *data_rd, size_t size, uint8_t register2get)
{
    //*data_rd is the pointer to n bytes of data, and size is the size in bytes
    if (size == 0)
    {
        return ESP_OK;
    }
    if (register2get > 0xEF)
    {
        return ESP_FAIL;
    }

    //create start signal
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    //Write to slave address
    //slave address.  You can set this to any 7 bit value (read/write bit is | onto the end)
    //this also checks for an ack back from the device
    i2c_master_write_byte(cmd, (slaveaddress << 1) | I2C_MASTER_READ, ACK_CHECK_EN);

    //I'm also thinking this method needs to also send a register value to access data, although the example this was taken from did not use a register (perhaps it was too simple?)
    if (!register2get)
    {
        std::cout<<"accessing register writing"<<std::endl;
        std::cout<<"registerwrite status: "<<i2c_master_write_byte(cmd, (register2get << 1) | I2C_MASTER_READ, ACK_CHECK_EN)<<std::endl;
    }

    if (size > 1)
    {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
};

esp_err_t I2C::i2c_master_write_slave(uint8_t *data_wr, size_t size, uint8_t register2set)
{ //see above for comments
    if (size == 0)
    {
        return ESP_OK;
    }
    if (register2set > 0xEF)
    {
        return ESP_FAIL;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    std::cout<<"slavewrite status: "<<i2c_master_write_byte(cmd, (slaveaddress << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN)<<std::endl;
    if (!register2set)
    {
        std::cout<<"registerwrite status: "<<i2c_master_write_byte(cmd, (register2set << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN)<<std::endl;
    }
    std::cout<<"data status: "<<i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN)<<std::endl;
    std::cout<<"stop status: "<<i2c_master_stop(cmd)<<std::endl;
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// esp_err_t I2C::i2c_master_write_slave_byte(uint8_t data_wr)
// { //writes a single byte
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (slaveaddress << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
//     i2c_master_write_byte(cmd, (data_wr<<1)|I2C_MASTER_WRITE, ACK_CHECK_EN);
//     i2c_master_stop(cmd);
//     esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
//     i2c_cmd_link_delete(cmd);
//     return ret;
// }
