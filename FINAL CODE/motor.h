/**
 * motor.h — DC Motor Driver via L298N + TIM3 PWM
 *
 * Hardware mapping:
 *   PB4  (TIM3_CH1, AF2) → L298N ENA  (Motor A / Left speed)
 *   PB5  (TIM3_CH2, AF2) → L298N ENB  (Motor B / Right speed)
 *   PE7  (GPIO OUT)       → L298N IN1  (Motor A direction)
 *   PE8  (GPIO OUT)       → L298N IN2  (Motor A direction)
 *   PE9  (GPIO OUT)       → L298N IN3  (Motor B direction)
 *   PE10 (GPIO OUT)       → L298N IN4  (Motor B direction)
 *
 * PWM: TIM3 @ 84 MHz / 4200 = 20 kHz
 * Speed range: 0–100 (mapped to 0–4199 CCR value)
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f4xx.h"
#include <stdint.h>

#define MOTOR_PWM_PERIOD    4199
#define MOTOR_SPEED_MIN     0
#define MOTOR_SPEED_MAX     100

typedef enum {
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_BRAKE,
    MOTOR_COAST
} Motor_Direction_t;

typedef enum {
    MOTOR_LEFT,
    MOTOR_RIGHT
} Motor_Select_t;

void Motor_Init(void);
void Motor_SetSpeed(Motor_Select_t motor, uint8_t speed);
void Motor_SetDirection(Motor_Select_t motor, Motor_Direction_t dir);
void Motor_Drive(Motor_Select_t motor, Motor_Direction_t dir, uint8_t speed);
void Motor_DriveBoth(int8_t speed_left, int8_t speed_right);
void Motor_Stop(void);

#endif
