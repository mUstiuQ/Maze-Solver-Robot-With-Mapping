/**
 * motor.c — DC Motor Driver implementation (register-level)
 *
 * TIM3 CH1/CH2 PWM Mode 1, GPIO AF2 on PB4/PB5,
 * push-pull outputs on PE7/PE8/PE9/PE10.
 */

#include "motor.h"

/* ── Map 0–100% to 0–4199 CCR ─────────────────────────── */
static uint32_t speed_to_ccr(uint8_t speed)
{
    if (speed > MOTOR_SPEED_MAX) speed = MOTOR_SPEED_MAX;
    return (uint32_t)speed * MOTOR_PWM_PERIOD / MOTOR_SPEED_MAX;
}

/* ── PB4/PB5 as AF2 (TIM3_CH1/CH2) ────────────────────── */
static void gpio_init_pwm_pins(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* PB4: AF mode, push-pull, 50 MHz, no pull */
    GPIOB->MODER   &= ~(3U << (4 * 2));
    GPIOB->MODER   |=  (2U << (4 * 2));
    GPIOB->OTYPER  &= ~(1U << 4);
    GPIOB->OSPEEDR &= ~(3U << (4 * 2));
    GPIOB->OSPEEDR |=  (2U << (4 * 2));
    GPIOB->PUPDR   &= ~(3U << (4 * 2));
    GPIOB->AFR[0]  &= ~(0xFU << (4 * 4));
    GPIOB->AFR[0]  |=  (2U   << (4 * 4));   /* AF2 = TIM3 */

    /* PB5: same config */
    GPIOB->MODER   &= ~(3U << (5 * 2));
    GPIOB->MODER   |=  (2U << (5 * 2));
    GPIOB->OTYPER  &= ~(1U << 5);
    GPIOB->OSPEEDR &= ~(3U << (5 * 2));
    GPIOB->OSPEEDR |=  (2U << (5 * 2));
    GPIOB->PUPDR   &= ~(3U << (5 * 2));
    GPIOB->AFR[0]  &= ~(0xFU << (5 * 4));
    GPIOB->AFR[0]  |=  (2U   << (5 * 4));   /* AF2 = TIM3 */
}

/* ── PE7-PE10 as push-pull outputs (L298N IN1-IN4) ────── */
static void gpio_init_direction_pins(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    uint32_t pins[] = {7, 8, 9, 10};
    for (int i = 0; i < 4; i++) {
        uint32_t p = pins[i];
        GPIOE->MODER   &= ~(3U << (p * 2));
        GPIOE->MODER   |=  (1U << (p * 2));   /* Output */
        GPIOE->OTYPER  &= ~(1U << p);          /* Push-pull */
        GPIOE->OSPEEDR &= ~(3U << (p * 2));    /* Low speed */
        GPIOE->PUPDR   &= ~(3U << (p * 2));    /* No pull */
    }

    /* All LOW = coast (motors off) */
    GPIOE->BSRR = (1U << (7+16)) | (1U << (8+16))
                 | (1U << (9+16)) | (1U << (10+16));
}

/* ── TIM3 PWM: 84 MHz / 4200 = 20 kHz ─────────────────── */
static void tim3_pwm_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->PSC  = 0;                 /* 84 MHz count rate    */
    TIM3->ARR  = MOTOR_PWM_PERIOD;  /* 4199 → 20 kHz       */
    TIM3->CR1 &= ~TIM_CR1_DIR;     /* Up-counting          */

    /* CH1 — PWM Mode 1, preload on */
    TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE);
    TIM3->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM3->CCER  &= ~TIM_CCER_CC1P;
    TIM3->CCER  |=  TIM_CCER_CC1E;
    TIM3->CCR1   = 0;

    /* CH2 — PWM Mode 1, preload on */
    TIM3->CCMR1 &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_OC2PE);
    TIM3->CCMR1 |= (6U << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
    TIM3->CCER  &= ~TIM_CCER_CC2P;
    TIM3->CCER  |=  TIM_CCER_CC2E;
    TIM3->CCR2   = 0;

    /* Start timer */
    TIM3->CR1 |= TIM_CR1_ARPE;
    TIM3->EGR |= TIM_EGR_UG;
    TIM3->SR  &= ~TIM_SR_UIF;
    TIM3->CR1 |= TIM_CR1_CEN;
}

/* ══════════════════════════════════════════════════════════
 *  PUBLIC API
 * ══════════════════════════════════════════════════════════ */

void Motor_Init(void)
{
    gpio_init_pwm_pins();
    gpio_init_direction_pins();
    tim3_pwm_init();
}

void Motor_SetSpeed(Motor_Select_t motor, uint8_t speed)
{
    uint32_t ccr = speed_to_ccr(speed);
    if (motor == MOTOR_LEFT)
        TIM3->CCR1 = ccr;
    else
        TIM3->CCR2 = ccr;
}

void Motor_SetDirection(Motor_Select_t motor, Motor_Direction_t dir)
{
    uint32_t pin_fwd, pin_rev;

    if (motor == MOTOR_LEFT) {
        pin_fwd = 7;  pin_rev = 8;     /* PE7=IN1, PE8=IN2 */
    } else {
        pin_fwd = 9;  pin_rev = 10;    /* PE9=IN3, PE10=IN4 */
    }

    switch (dir) {
        case MOTOR_FORWARD:
            GPIOE->BSRR = (1U << pin_fwd) | (1U << (pin_rev+16));
            break;
        case MOTOR_BACKWARD:
            GPIOE->BSRR = (1U << (pin_fwd+16)) | (1U << pin_rev);
            break;
        case MOTOR_BRAKE:
            GPIOE->BSRR = (1U << pin_fwd) | (1U << pin_rev);
            break;
        case MOTOR_COAST:
            GPIOE->BSRR = (1U << (pin_fwd+16)) | (1U << (pin_rev+16));
            break;
    }
}

void Motor_Drive(Motor_Select_t motor, Motor_Direction_t dir, uint8_t speed)
{
    Motor_SetDirection(motor, dir);
    Motor_SetSpeed(motor, speed);
}

void Motor_DriveBoth(int8_t speed_left, int8_t speed_right)
{
    if (speed_left >= 0) {
        Motor_SetDirection(MOTOR_LEFT, MOTOR_FORWARD);
        Motor_SetSpeed(MOTOR_LEFT, (uint8_t)speed_left);
    } else {
        Motor_SetDirection(MOTOR_LEFT, MOTOR_BACKWARD);
        Motor_SetSpeed(MOTOR_LEFT, (uint8_t)(-speed_left));
    }

    if (speed_right >= 0) {
        Motor_SetDirection(MOTOR_RIGHT, MOTOR_FORWARD);
        Motor_SetSpeed(MOTOR_RIGHT, (uint8_t)speed_right);
    } else {
        Motor_SetDirection(MOTOR_RIGHT, MOTOR_BACKWARD);
        Motor_SetSpeed(MOTOR_RIGHT, (uint8_t)(-speed_right));
    }
}

void Motor_Stop(void)
{
    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;
    Motor_SetDirection(MOTOR_LEFT, MOTOR_BRAKE);
    Motor_SetDirection(MOTOR_RIGHT, MOTOR_BRAKE);
}
