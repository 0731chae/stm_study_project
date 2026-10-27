#ifndef FIRE_SYSTEM_H
#define FIRE_SYSTEM_H
#include "detection.h"
#include "suppression.h"
typedef struct {
    fire_config_t config;
    detection_t detector;
    suppression_t suppression;
    fire_error_t latched_error;
    bool config_valid;
} fire_system_t;
fire_config_t fire_config_default(void);
/* Non-null system/config required. An invalid config latches CONFIG fault. */
void fire_system_init(fire_system_t *system, const fire_config_t *config);
/* Single main-loop caller. NULL sample is a sensor fault. No IO or waits. */
fire_output_t fire_system_step(fire_system_t *system, const fire_sample_t *sample,
    uint32_t now_ms, bool emergency_stop, bool reset_requested);
#endif
