#ifndef FIRE_TYPES_H
#define FIRE_TYPES_H
#include <stdbool.h>
#include <stdint.h>

typedef enum { FIRE_IDLE, FIRE_AIMING, FIRE_SPRAYING, FIRE_COOLDOWN, FIRE_FAULT, FIRE_ESTOP } fire_state_t;
typedef enum {
    FIRE_ERROR_NONE, FIRE_ERROR_SENSOR, FIRE_ERROR_STALE,
    FIRE_ERROR_TARGET, FIRE_ERROR_CONFIG, FIRE_ERROR_ESTOP
} fire_error_t;

typedef struct {
    bool valid;
    bool alarm;
    bool target_valid;
    int16_t target_deg; /* logical angle: 0..180, not a PWM pulse width */
    uint32_t captured_ms; /* actual acquisition time, same clock as now_ms */
} fire_sample_t;

typedef struct {
    uint8_t confirm_samples;
    uint32_t sample_timeout_ms;
    uint32_t aim_settle_ms;
    uint32_t max_spray_ms;
    uint32_t cooldown_ms;
} fire_config_t;

typedef struct {
    bool confirmed;
    bool target_valid;
    int16_t target_deg;
} fire_event_t;

typedef struct {
    fire_state_t state;
    fire_error_t error;
    bool pump_on;
    bool motor_enable;
    int16_t target_deg;
} fire_output_t;
#endif
