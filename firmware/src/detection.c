#include "detection.h"
#include <stddef.h>

void detection_init(detection_t *detector)
{
    detector->consecutive = 0U;
    detector->have_sample = false;
    detector->last_sample_ms = 0U;
}

fire_error_t detection_update(detection_t *detector, const fire_config_t *config,
    const fire_sample_t *sample, uint32_t now_ms, fire_event_t *event)
{
    event->confirmed = false;
    event->target_valid = false;
    event->target_deg = 0;
    if (sample == NULL || !sample->valid) {
        detection_init(detector);
        return FIRE_ERROR_SENSOR;
    }
    /* Unsigned subtraction supports uint32_t tick rollover. Future/old data fail. */
    if ((uint32_t)(now_ms - sample->captured_ms) > config->sample_timeout_ms) {
        detection_init(detector);
        return FIRE_ERROR_STALE;
    }
    if (sample->target_valid && (sample->target_deg < 0 || sample->target_deg > 180)) {
        detection_init(detector);
        return FIRE_ERROR_TARGET;
    }
    if (detector->have_sample && sample->captured_ms != detector->last_sample_ms) {
        uint32_t gap = (uint32_t)(sample->captured_ms - detector->last_sample_ms);
        if (gap >= UINT32_C(0x80000000)) {
            detection_init(detector);
            return FIRE_ERROR_STALE;
        }
        if (gap > config->sample_timeout_ms) {
            detector->consecutive = 0U;
        }
    }
    if (!sample->alarm) {
        detector->consecutive = 0U;
    } else if (!detector->have_sample || sample->captured_ms != detector->last_sample_ms) {
        if (detector->consecutive < config->confirm_samples) {
            ++detector->consecutive;
        }
    }
    detector->have_sample = true;
    detector->last_sample_ms = sample->captured_ms;
    event->confirmed = detector->consecutive >= config->confirm_samples;
    event->target_valid = sample->target_valid;
    event->target_deg = sample->target_deg;
    return FIRE_ERROR_NONE;
}
