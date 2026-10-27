#ifndef DETECTION_H
#define DETECTION_H
#include "fire_types.h"
typedef struct {
    uint8_t consecutive;
    bool have_sample;
    uint32_t last_sample_ms;
} detection_t;
void detection_init(detection_t *detector);
fire_error_t detection_update(detection_t *detector, const fire_config_t *config,
    const fire_sample_t *sample, uint32_t now_ms, fire_event_t *event);
#endif
