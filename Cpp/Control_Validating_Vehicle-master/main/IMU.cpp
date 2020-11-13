#include "IMU.h"

//For the MPU9265 9DOF unit:
// #define    MPU9250_ADDRESS            0x68
// #define    MAG_ADDRESS                0x0C

// #define    GYRO_FULL_SCALE_250_DPS    0x00
// #define    GYRO_FULL_SCALE_500_DPS    0x08
// #define    GYRO_FULL_SCALE_1000_DPS   0x10
// #define    GYRO_FULL_SCALE_2000_DPS   0x18

// #define    ACC_FULL_SCALE_2_G        0x00
// #define    ACC_FULL_SCALE_4_G        0x08
// #define    ACC_FULL_SCALE_8_G        0x10
// #define    ACC_FULL_SCALE_16_G       0x18

IMU::IMU()//(uint8_t adress):I2C(adress)//cannot be used as the IMU has 2 addresses, while I2C has only 1 allowed
{

    I2C Magnetometer(MAG_ADDRESS);
    I2C MPU9250(MPU9250_ADDRESS);

    //configure the device initially



    //I'm not sure if it's possible to write to multiple registers before ending a transmission in i2c
    //according to the docs, i2c_master_cmd_begin can send more than 1 qued message (messages in byte order), but I'm not sure if you need to break transmissions to do so each time

    //This appears to have failed  Not sure why.  It seems everything is working except for the final i2c_master_cmd_begin
    //it appears to have worked once, but never again.  Frequency doesn't appear to do anything.  Perhaps this is silently failing in the arduino code
    uint8_t mag2 = 0x01;
    std::cout<<"Mag2 status: "<<(int)Magnetometer.i2c_master_write_slave(&mag2, 1, 0x0A)<<std::endl;

    uint8_t accscale = ACC_FULL_SCALE_16_G;
    std::cout<<"acc status: "<<(int)MPU9250.i2c_master_write_slave(&accscale, 1, 28)<<std::endl;
    uint8_t gyroscale = GYRO_FULL_SCALE_2000_DPS;
    std::cout<<"gyro status: "<<(int)MPU9250.i2c_master_write_slave(&gyroscale, 1, 27)<<std::endl;
    uint8_t mag1 = 0x02;
    std::cout<<"Mag1 status: "<<(int)MPU9250.i2c_master_write_slave(&mag1, 1, 0x37)<<std::endl;


    //Taken from:

    //      // Configurar acelerometro
    //    I2CwriteByte(MPU9250_ADDRESS, 28, ACC_FULL_SCALE_16_G);
    //    // Configurar giroscopio
    //    I2CwriteByte(MPU9250_ADDRESS, 27, GYRO_FULL_SCALE_2000_DPS);
    //    // Configurar magnetometro
    //    I2CwriteByte(MPU9250_ADDRESS, 0x37, 0x02);
    //    I2CwriteByte(MAG_ADDRESS, 0x0A, 0x01);
}

void IMU::getData()
{   //There is clearly a problem with the IMU data that it is receiving back.  Both buffers are basically empty and likely out of sync

    //Buf is the pointer to the length 14 buffer
    uint8_t Buf[14];
    MPU9250.i2c_master_read_slave(Buf, 14, 0x3B);

    int i=0;
    std::cout<<"Buf contains: "<<std::endl;
    for(;i<14;i++){std::cout<<Buf[i]<<std::endl;};

    // Not sure how this is being converted.  Check documentation, might be quaternions
    ax = -(Buf[0] << 8 | Buf[1]);
    ay = -(Buf[2] << 8 | Buf[3]);
    az = Buf[4] << 8 | Buf[5];

    gx = -(Buf[8] << 8 | Buf[9]);
    gy = -(Buf[10] << 8 | Buf[11]);
    gz = Buf[12] << 8 | Buf[13];

    uint8_t ST1;
    do
    {
        Magnetometer.i2c_master_read_slave(&ST1, 1, 0x02);
    } while (!(ST1 & 0x01));

    uint8_t Mag[7];
    Magnetometer.i2c_master_read_slave(Mag, 7, 0x03);

    std::cout<<"Mag contains: "<<std::endl;
    for(i=0;i<7;i++){std::cout<<Mag[i]<<std::endl;};

    mx = -(Mag[3] << 8 | Mag[2]);
    my = -(Mag[1] << 8 | Mag[0]);
    mz = -(Mag[5] << 8 | Mag[4]);

    //Taken from:

    //     // ---  Lectura acelerometro y giroscopio ---
    //    uint8_t Buf[14];
    //    I2Cread(MPU9250_ADDRESS, 0x3B, 14, Buf);

    //    // Convertir registros acelerometro
    //    int16_t ax = -(Buf[0] << 8 | Buf[1]);
    //    int16_t ay = -(Buf[2] << 8 | Buf[3]);
    //    int16_t az = Buf[4] << 8 | Buf[5];

    //    // Convertir registros giroscopio
    //    int16_t gx = -(Buf[8] << 8 | Buf[9]);
    //    int16_t gy = -(Buf[10] << 8 | Buf[11]);
    //    int16_t gz = Buf[12] << 8 | Buf[13];

    //    // ---  Lectura del magnetometro ---
    //    uint8_t ST1;
    //    do
    //    {
    //       I2Cread(MAG_ADDRESS, 0x02, 1, &ST1);
    //    } while (!(ST1 & 0x01));

    //    uint8_t Mag[7];
    //    I2Cread(MAG_ADDRESS, 0x03, 7, Mag);

    //    // Convertir registros magnetometro
    //    int16_t mx = -(Mag[3] << 8 | Mag[2]);
    //    int16_t my = -(Mag[1] << 8 | Mag[0]);
    //    int16_t mz = -(Mag[5] << 8 | Mag[4]);
}