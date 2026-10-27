#include "fire_system.h"
#include <stdio.h>
static const char *state_name(fire_state_t state)
{
    switch (state) {
    case FIRE_IDLE: return "IDLE";
    case FIRE_AIMING: return "AIMING";
    case FIRE_SPRAYING: return "SPRAYING";
    case FIRE_COOLDOWN: return "COOLDOWN";
    case FIRE_FAULT: return "FAULT";
    case FIRE_ESTOP: return "ESTOP";
    default: return "UNKNOWN";
    }
}
int main(void)
{
    fire_system_t system;
    fire_config_t config = fire_config_default();
    fire_state_t previous = FIRE_ESTOP;
    uint32_t now;
    fire_system_init(&system, &config);
    puts("PC simulation only: no physical outputs");
    for (now = 0U; now <= 10000U; now += 50U) {
        fire_sample_t sample = {true, now >= 500U && now < 5000U, true, 90, now};
        fire_output_t output = fire_system_step(&system, &sample, now, false, false);
        if (output.state != previous) {
            printf("%5lu ms %-8s pump=%u motor=%u angle=%d error=%d\n",
                (unsigned long)now, state_name(output.state), (unsigned)output.pump_on,
                (unsigned)output.motor_enable, (int)output.target_deg, (int)output.error);
            previous = output.state;
        }
    }
    return 0;
}
