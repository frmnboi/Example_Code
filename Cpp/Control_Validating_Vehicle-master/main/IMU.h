#ifndef IMU_H
#define IMU_H

#include "I2C.h"

//For the MPU9265 9DOF unit:
#define    MPU9250_ADDRESS            0x68
#define    MAG_ADDRESS                0x0C

//note the magnetometer is a seperate unit and has its own address
 
#define    GYRO_FULL_SCALE_250_DPS    0x00  
#define    GYRO_FULL_SCALE_500_DPS    0x08
#define    GYRO_FULL_SCALE_1000_DPS   0x10
#define    GYRO_FULL_SCALE_2000_DPS   0x18
 
#define    ACC_FULL_SCALE_2_G        0x00  
#define    ACC_FULL_SCALE_4_G        0x08
#define    ACC_FULL_SCALE_8_G        0x10
#define    ACC_FULL_SCALE_16_G       0x18


class IMU:public I2C
{
private:
    I2C MPU9250;
    I2C Magnetometer;


    //Accelerometer
    
    int16_t ax;
    int16_t ay;
    int16_t az;
    
    int16_t gx;
    int16_t gy;
    int16_t gz;

    //Gyroscope

    int16_t mx;
    int16_t my;
    int16_t mz;


public:
    IMU();
    void getData();

};
#endif
