#include "kt_stepper.h"
#include "kt_config.h"
#include "kt_log.h"
#include "stm32f1xx_hal.h"

static const uint8_t seq[8][4] = {
    {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
    {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
};

static void write_phase(uint8_t index)
{
    HAL_GPIO_WritePin(KT_STEPPER_IN1_PORT, KT_STEPPER_IN1_PIN, seq[index][0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN2_PORT, KT_STEPPER_IN2_PIN, seq[index][1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN3_PORT, KT_STEPPER_IN3_PIN, seq[index][2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN4_PORT, KT_STEPPER_IN4_PIN, seq[index][3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void run_steps(uint8_t reverse)
{
    uint16_t step;
    for (step = 0; step < 512U; step++) {
        uint8_t idx = (uint8_t)(step & 0x07U);
        if (reverse) {
            idx = (uint8_t)(7U - idx);
        }
        write_phase(idx);
        HAL_Delay(KT_STEPPER_STEP_DELAY_MS);
    }
    kt_stepper_stop();
}

void kt_stepper_forward_test(void)
{
    KT_LOG_INFO("Stepper forward test start");
    run_steps(0);
    KT_LOG_INFO("Stepper forward test done");
}

void kt_stepper_reverse_test(void)
{
    KT_LOG_INFO("Stepper reverse test start");
    run_steps(1);
    KT_LOG_INFO("Stepper reverse test done");
}

void kt_stepper_stop(void)
{
    HAL_GPIO_WritePin(KT_STEPPER_IN1_PORT, KT_STEPPER_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN2_PORT, KT_STEPPER_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN3_PORT, KT_STEPPER_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN4_PORT, KT_STEPPER_IN4_PIN, GPIO_PIN_RESET);
    KT_LOG_INFO("Stepper stopped");
}

void kt_stepper_init(void)
{
    HAL_GPIO_WritePin(KT_STEPPER_IN1_PORT, KT_STEPPER_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN2_PORT, KT_STEPPER_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN3_PORT, KT_STEPPER_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN4_PORT, KT_STEPPER_IN4_PIN, GPIO_PIN_RESET);
    KT_LOG_INFO("Stepper init: STOP, coils off");
}
