#ifndef SUPPRESSION_H
#define SUPPRESSION_H
#include "fire_types.h"
typedef struct {
    fire_state_t state;
    uint32_t entered_ms;
    int16_t target_deg;
} suppression_t;
void suppression_init(suppression_t *controller);
void suppression_update(suppression_t *controller, const fire_config_t *config,
    const fire_event_t *event, uint32_t now_ms, fire_output_t *output);
#endif
