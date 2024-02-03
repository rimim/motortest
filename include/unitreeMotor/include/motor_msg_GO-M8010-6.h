#ifndef __MOTOR_MSG_GO_M8010_6_H
#define __MOTOR_MSG_GO_M8010_6_H

#include <stdint.h>
#define CRC_SIZE 2
#define CTRL_DAT_SIZE sizeof(ControlData_t) - CRC_SIZE
#define DATA_DAT_SIZE sizeof(MotorData_t) - CRC_SIZE

#pragma pack(1)

/**
 * @brief Motor mode control information
 *
 */
typedef struct
{
    uint8_t id:4; // Motor ID: 0,1...,14 15 means broadcasting data to all motors (no return at this time)
    uint8_t status :3; // Working mode: 0. Lock 1. FOC closed loop 2. Encoder calibration 3. Reserved
    uint8_t none :1; // reserved bits
} RIS_Mode_t; // Control mode 1Byte

/**
 * @brief Motor status control information
 *
 */
typedef struct
{
    int16_t tor_des; // Desired joint output torque unit: Nm (q8)
    int16_t spd_des; // Desired joint output speed unit: rad/s (q7)
    int32_t pos_des; // Desired joint output position unit: rad (q15)
    uint16_t k_pos; // Expected joint stiffness coefficient unit: 0.0-1.0 (q15)
    uint16_t k_spd; // Desired joint damping coefficient unit: 0.0-1.0 (q15)
    
} RIS_Comd_t; // Control parameter 12Byte

/**
 * @brief motor status feedback information
 *
 */
typedef struct
{
    int16_t torque; // Actual joint output torque unit: Nm (q8)
    int16_t speed; //actual joint output speed unit: rad/s (q7)
    int32_t pos; //actual joint output position unit: W (q15)
    int8_t temp; // Motor temperature: -128~127°C, temperature protection is triggered at 90°C
    uint8_t MError:3; // Motor error identification: 0. Normal 1. Overheating 2. Overcurrent 3. Overvoltage 4. Encoder failure 5-7. Reserved
    uint16_t force :12; // Foot pressure sensor data 12bit (0-4095)
    uint8_t none :1; // reserved bits
} RIS_Fbk_t; // Status data 11Byte


#pragma pack()

#pragma pack(1)

/**
 * @brief control packet format
 *
 */
typedef struct
{
    uint8_t head[2]; // Baotou 2Byte
    RIS_Mode_t mode; // Motor control mode 1Byte
    RIS_Comd_t comd; // Motor expected data 12Byte
    uint16_t CRC16; // CRC 2Byte

} ControlData_t; // Host control command 17Byte

/**
 * @brief motor feedback data packet format
 *
 */
typedef struct
{
    uint8_t head[2]; // Baotou 2Byte
    RIS_Mode_t mode; // Motor control mode 1Byte
    RIS_Fbk_t fbk; // Motor feedback data 11Byte
    uint16_t CRC16; // CRC 2Byte

} MotorData_t; // Motor returns data 16Byte

#pragma pack()

#endif

