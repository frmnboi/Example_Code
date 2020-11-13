#ifndef i2c_H
#define i2c_H

#include "driver/i2c.h"
// #include "hal/i2c_types.h"

#include <iostream> //for debug


#define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0     /*!< I2C master will not check ack from slave */

#define ACK_VAL    I2C_MASTER_ACK         /*!< I2C ack value */
#define NACK_VAL   I2C_MASTER_NACK         /*!< I2C nack value */
// of i2c_ack_type_t

class I2C
{
private:
    uint8_t slaveaddress;
    //the status variable for the pwm initialization
    static bool initialized;
    void configureMaster();

public:
    I2C();
    I2C(uint8_t adress);
    esp_err_t i2c_master_read_slave(uint8_t *data_rd, size_t size, uint8_t registers=0);
    esp_err_t i2c_master_write_slave(uint8_t *data_wr, size_t size, uint8_t register2set=0);
    // esp_err_t i2c_master_write_slave_byte(uint8_t data_wr);
};
//default the initialized variable to be false
inline bool I2C::initialized {false};
#endif
