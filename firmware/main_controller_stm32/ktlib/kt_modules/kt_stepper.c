#include "kt_stepper.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_system/kt_tick.h"
#include "stm32f1xx_hal.h"

typedef enum {
    STEPPER_DIR_IDLE = 0,
    STEPPER_DIR_FORWARD,
    STEPPER_DIR_REVERSE
} stepper_dir_t;

static const uint8_t seq[8][4] = {
#if (KT_STEPPER_WAVE_DRIVE_ENABLE != 0)
    {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1},
    {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}
#else
    {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
    {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
#endif
};

static stepper_dir_t stepper_dir = STEPPER_DIR_IDLE;
static uint16_t stepper_remaining;
static uint8_t stepper_phase;
static uint32_t stepper_last_ms;

static void write_phase(uint8_t index)
{
    HAL_GPIO_WritePin(KT_STEPPER_IN1_PORT, KT_STEPPER_IN1_PIN, seq[index][0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN2_PORT, KT_STEPPER_IN2_PIN, seq[index][1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN3_PORT, KT_STEPPER_IN3_PIN, seq[index][2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN4_PORT, KT_STEPPER_IN4_PIN, seq[index][3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void start_steps(stepper_dir_t dir, uint16_t steps)
{
    if (steps == 0U) {
        kt_stepper_stop();
        return;
    }

    stepper_dir = dir;
    stepper_remaining = steps;
    stepper_phase = (dir == STEPPER_DIR_REVERSE) ? 7U : 0U;
    stepper_last_ms = 0U;
}

void kt_stepper_task(void)
{
    if (stepper_dir == STEPPER_DIR_IDLE) {
        return;
    }

    if (stepper_last_ms != 0U &&
        !kt_tick_is_timeout(stepper_last_ms, KT_STEPPER_STEP_DELAY_MS)) {
        return;
    }

    write_phase(stepper_phase);
    stepper_last_ms = kt_tick_get_ms();

    if (stepper_dir == STEPPER_DIR_REVERSE) {
        stepper_phase = (stepper_phase == 0U) ? 7U : (uint8_t)(stepper_phase - 1U);
    } else {
        stepper_phase = (uint8_t)((stepper_phase + 1U) & 0x07U);
    }

    stepper_remaining--;
    if (stepper_remaining == 0U) {
        kt_stepper_stop();
    }
}

void kt_stepper_forward_test(void)
{
    KT_LOG_INFO("Stepper forward start");
    kt_stepper_start_forward(KT_STEPPER_GATE_STEPS);
}

void kt_stepper_reverse_test(void)
{
    KT_LOG_INFO("Stepper reverse start");
    kt_stepper_start_reverse(KT_STEPPER_GATE_STEPS);
}

void kt_stepper_start_forward(uint16_t steps)
{
    start_steps(STEPPER_DIR_FORWARD, steps);
}

void kt_stepper_start_reverse(uint16_t steps)
{
    start_steps(STEPPER_DIR_REVERSE, steps);
}

uint8_t kt_stepper_is_busy(void)
{
    return (stepper_dir != STEPPER_DIR_IDLE) ? 1U : 0U;
}

void kt_stepper_stop(void)
{
    stepper_dir = STEPPER_DIR_IDLE;
    stepper_remaining = 0U;
    HAL_GPIO_WritePin(KT_STEPPER_IN1_PORT, KT_STEPPER_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN2_PORT, KT_STEPPER_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN3_PORT, KT_STEPPER_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN4_PORT, KT_STEPPER_IN4_PIN, GPIO_PIN_RESET);
}

void kt_stepper_init(void)
{
    stepper_dir = STEPPER_DIR_IDLE;
    stepper_remaining = 0U;
    HAL_GPIO_WritePin(KT_STEPPER_IN1_PORT, KT_STEPPER_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN2_PORT, KT_STEPPER_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN3_PORT, KT_STEPPER_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_STEPPER_IN4_PORT, KT_STEPPER_IN4_PIN, GPIO_PIN_RESET);
    KT_LOG_INFO("Stepper init: STOP, coils off");
}
