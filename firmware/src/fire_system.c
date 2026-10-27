#include "fire_system.h"

fire_config_t fire_config_default(void)
{
    const fire_config_t config = {3U, 250U, 500U, 3000U, 5000U};
    return config;
}
static bool duration_valid(uint32_t value)
{
    return value > 0U && value < UINT32_C(0x80000000);
}
void fire_system_init(fire_system_t *system, const fire_config_t *config)
{
    system->config = *config;
    system->config_valid = config->confirm_samples > 0U
        && duration_valid(config->sample_timeout_ms) && duration_valid(config->aim_settle_ms)
        && duration_valid(config->max_spray_ms) && duration_valid(config->cooldown_ms);
    detection_init(&system->detector);
    suppression_init(&system->suppression);
    system->latched_error = system->config_valid ? FIRE_ERROR_NONE : FIRE_ERROR_CONFIG;
}
fire_output_t fire_system_step(fire_system_t *system, const fire_sample_t *sample,
    uint32_t now_ms, bool emergency_stop, bool reset_requested)
{
    fire_output_t output = {FIRE_IDLE, FIRE_ERROR_NONE, false, false, 0};
    fire_event_t event;
    fire_error_t error;
    /* Emergency stop has priority over reset, sensor and configuration errors. */
    if (emergency_stop) {
        system->latched_error = FIRE_ERROR_ESTOP;
    }
    if (!system->config_valid && !emergency_stop) {
        system->latched_error = FIRE_ERROR_CONFIG;
    }
    error = detection_update(&system->detector, &system->config, sample, now_ms, &event);
    if (error == FIRE_ERROR_NONE && event.confirmed && !event.target_valid) {
        error = FIRE_ERROR_TARGET;
    }
    if (system->latched_error == FIRE_ERROR_NONE && error != FIRE_ERROR_NONE) {
        system->latched_error = error;
    }
    if (system->latched_error != FIRE_ERROR_NONE) {
        if (!emergency_stop && system->config_valid && reset_requested
            && error == FIRE_ERROR_NONE && !sample->alarm) {
            system->latched_error = FIRE_ERROR_NONE;
            detection_init(&system->detector);
            suppression_init(&system->suppression);
        } else {
            suppression_init(&system->suppression);
            output.state = system->latched_error == FIRE_ERROR_ESTOP ? FIRE_ESTOP : FIRE_FAULT;
            output.error = system->latched_error;
            return output;
        }
    }
    suppression_update(&system->suppression, &system->config, &event, now_ms, &output);
    return output;
}
