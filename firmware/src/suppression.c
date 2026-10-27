#include "suppression.h"

void suppression_init(suppression_t *controller)
{
    controller->state = FIRE_IDLE;
    controller->entered_ms = 0U;
    controller->target_deg = 0;
}

void suppression_update(suppression_t *controller, const fire_config_t *config,
    const fire_event_t *event, uint32_t now_ms, fire_output_t *output)
{
    uint32_t elapsed = (uint32_t)(now_ms - controller->entered_ms);
    output->pump_on = false;
    output->motor_enable = false;
    output->error = FIRE_ERROR_NONE;
    switch (controller->state) {
    case FIRE_IDLE:
        if (event->confirmed) {
            controller->state = FIRE_AIMING;
            controller->entered_ms = now_ms;
            controller->target_deg = event->target_deg;
        }
        break;
    case FIRE_AIMING:
        if (!event->confirmed) {
            controller->state = FIRE_IDLE;
        } else if (controller->target_deg != event->target_deg) {
            controller->target_deg = event->target_deg;
            controller->entered_ms = now_ms;
        } else if (elapsed >= config->aim_settle_ms) {
            controller->state = FIRE_SPRAYING;
            controller->entered_ms = now_ms;
        }
        break;
    case FIRE_SPRAYING:
        if (!event->confirmed || elapsed >= config->max_spray_ms) {
            controller->state = FIRE_COOLDOWN;
            controller->entered_ms = now_ms;
        } else if (controller->target_deg != event->target_deg) {
            controller->state = FIRE_AIMING;
            controller->target_deg = event->target_deg;
            controller->entered_ms = now_ms;
        }
        break;
    case FIRE_COOLDOWN:
        if (elapsed >= config->cooldown_ms) {
            controller->state = FIRE_IDLE;
        }
        break;
    default:
        break;
    }
    output->state = controller->state;
    output->target_deg = controller->target_deg;
    output->motor_enable = controller->state == FIRE_AIMING || controller->state == FIRE_SPRAYING;
    output->pump_on = controller->state == FIRE_SPRAYING;
}
