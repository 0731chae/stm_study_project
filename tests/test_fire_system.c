#include "fire_system.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Checks stay active in release builds as well. */
#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); return 1; \
} } while (0)

static fire_output_t step(fire_system_t *system, uint32_t now, bool alarm, int16_t angle)
{
    fire_sample_t sample = {true, alarm, true, angle, now};
    return fire_system_step(system, &sample, now, false, false);
}

static fire_output_t begin_spray(fire_system_t *system, uint32_t start)
{
    fire_output_t output = {0};
    uint32_t elapsed;
    for (elapsed = 0U; elapsed <= 600U; elapsed += 50U) {
        output = step(system, start + elapsed, true, 90);
    }
    return output;
}

static fire_output_t keep_alarm(fire_system_t *system, uint32_t from, uint32_t to)
{
    fire_output_t output = {0};
    uint32_t now;
    for (now = from; now <= to; now += 50U) {
        output = step(system, now, true, 90);
    }
    return output;
}

static int run_case(int id)
{
    fire_system_t system;
    fire_config_t config = fire_config_default();
    fire_output_t output;
    fire_sample_t sample = {true, true, true, 90, 0U};
    uint32_t now;
    fire_system_init(&system, &config);

    switch (id) {
    case 1: /* Startup: no alarm, no output. */
        output = step(&system, 0U, false, 90);
        CHECK(output.state == FIRE_IDLE && !output.pump_on && !output.motor_enable);
        break;
    case 2: /* Third distinct alarm sample confirms, without starting the pump. */
        CHECK(step(&system, 0U, true, 90).state == FIRE_IDLE);
        CHECK(step(&system, 50U, true, 90).state == FIRE_IDLE);
        output = step(&system, 100U, true, 90);
        CHECK(output.state == FIRE_AIMING && output.motor_enable && !output.pump_on);
        break;
    case 3: /* Noise resets consecutive count. */
        (void)step(&system, 0U, true, 90);
        (void)step(&system, 50U, true, 90);
        (void)step(&system, 100U, false, 90);
        CHECK(step(&system, 150U, true, 90).state == FIRE_IDLE);
        CHECK(step(&system, 200U, true, 90).state == FIRE_IDLE);
        CHECK(step(&system, 250U, true, 90).state == FIRE_AIMING);
        break;
    case 4: /* Duplicate timestamps do not count as new samples. */
        for (now = 0U; now <= 200U; now += 10U) {
            output = fire_system_step(&system, &sample, now, false, false);
            CHECK(output.state == FIRE_IDLE && !output.pump_on);
        }
        break;
    case 5: /* Aim settle lower boundary. */
        for (now = 0U; now <= 550U; now += 50U) {
            (void)step(&system, now, true, 90);
        }
        CHECK(!step(&system, 599U, true, 90).pump_on);
        output = step(&system, 600U, true, 90);
        CHECK(output.state == FIRE_SPRAYING && output.pump_on);
        break;
    case 6: /* Max spray boundary stops and cooldown cannot re-trigger early. */
        CHECK(begin_spray(&system, 0U).pump_on);
        (void)keep_alarm(&system, 650U, 3550U);
        CHECK(step(&system, 3599U, true, 90).pump_on);
        output = step(&system, 3600U, true, 90);
        CHECK(output.state == FIRE_COOLDOWN && !output.pump_on);
        (void)keep_alarm(&system, 3650U, 8550U);
        CHECK(step(&system, 8599U, true, 90).state == FIRE_COOLDOWN);
        CHECK(step(&system, 8600U, true, 90).state == FIRE_IDLE);
        CHECK(step(&system, 8650U, true, 90).state == FIRE_AIMING);
        break;
    case 7: /* Fire clear ends spraying immediately. */
        (void)begin_spray(&system, 0U);
        output = step(&system, 650U, false, 90);
        CHECK(output.state == FIRE_COOLDOWN && !output.pump_on);
        break;
    case 8: /* Sensor failure during spray latches and holds outputs off. */
        (void)begin_spray(&system, 0U);
        sample.valid = false;
        output = fire_system_step(&system, &sample, 650U, false, false);
        CHECK(output.state == FIRE_FAULT && output.error == FIRE_ERROR_SENSOR);
        CHECK(!output.pump_on && !output.motor_enable);
        CHECK(step(&system, 700U, false, 90).state == FIRE_FAULT);
        break;
    case 9: /* Freshness accepts 250 ms, rejects 251 ms. */
        CHECK(fire_system_step(&system, &sample, 250U, false, false).state == FIRE_IDLE);
        output = fire_system_step(&system, &sample, 251U, false, false);
        CHECK(output.error == FIRE_ERROR_STALE && !output.pump_on);
        break;
    case 10: /* Range endpoints accepted, both outside endpoints rejected. */
        CHECK(step(&system, 0U, true, 0).error == FIRE_ERROR_NONE);
        CHECK(step(&system, 50U, true, 180).error == FIRE_ERROR_NONE);
        CHECK(step(&system, 100U, true, -1).error == FIRE_ERROR_TARGET);
        fire_system_init(&system, &config);
        CHECK(step(&system, 0U, true, 181).error == FIRE_ERROR_TARGET);
        break;
    case 11: /* Confirmed alarm with no direction must not spray. */
        sample.target_valid = false;
        for (now = 0U; now <= 100U; now += 50U) {
            sample.captured_ms = now;
            output = fire_system_step(&system, &sample, now, false, false);
            CHECK(!output.pump_on);
        }
        CHECK(output.state == FIRE_FAULT && output.error == FIRE_ERROR_TARGET);
        break;
    case 12: /* Emergency stop overrides spray and simultaneous reset. */
        (void)begin_spray(&system, 0U);
        sample.captured_ms = 650U;
        output = fire_system_step(&system, &sample, 650U, true, true);
        CHECK(output.state == FIRE_ESTOP && !output.pump_on && !output.motor_enable);
        CHECK(step(&system, 700U, false, 90).state == FIRE_ESTOP);
        break;
    case 13: /* Recovery requires fresh normal input plus explicit reset. */
        (void)fire_system_step(&system, NULL, 0U, false, false);
        sample.captured_ms = 50U;
        CHECK(fire_system_step(&system, &sample, 50U, false, true).state == FIRE_FAULT);
        sample.alarm = false;
        CHECK(fire_system_step(&system, &sample, 400U, false, true).state == FIRE_FAULT);
        sample.captured_ms = 450U;
        output = fire_system_step(&system, &sample, 450U, false, true);
        CHECK(output.state == FIRE_IDLE && !output.pump_on);
        CHECK(step(&system, 500U, true, 90).state == FIRE_IDLE);
        break;
    case 14: /* Changing the target during spray stops pump and re-aims. */
        (void)begin_spray(&system, 0U);
        output = step(&system, 650U, true, 120);
        CHECK(output.state == FIRE_AIMING && !output.pump_on && output.target_deg == 120);
        for (now = 700U; now <= 1100U; now += 50U) {
            CHECK(!step(&system, now, true, 120).pump_on);
        }
        CHECK(!step(&system, 1149U, true, 120).pump_on);
        CHECK(step(&system, 1150U, true, 120).pump_on);
        break;
    case 15: /* Tick rollover while aiming and spraying. */
        CHECK(begin_spray(&system, UINT32_MAX - 300U).pump_on);
        output = step(&system, 349U, false, 90);
        CHECK(output.state == FIRE_COOLDOWN && !output.pump_on);
        break;
    case 16: /* Invalid config cannot be cleared with reset. */
        config.confirm_samples = 0U;
        fire_system_init(&system, &config);
        sample.alarm = false;
        output = fire_system_step(&system, &sample, 0U, false, true);
        CHECK(output.error == FIRE_ERROR_CONFIG && !output.pump_on);
        config = fire_config_default();
        config.max_spray_ms = 0U;
        fire_system_init(&system, &config);
        CHECK(step(&system, 0U, false, 90).error == FIRE_ERROR_CONFIG);
        config.max_spray_ms = UINT32_C(0x80000000);
        fire_system_init(&system, &config);
        CHECK(step(&system, 0U, false, 90).error == FIRE_ERROR_CONFIG);
        break;
    case 17: /* No sample, and future timestamps are rejected. */
        CHECK(fire_system_step(&system, NULL, 0U, false, false).error == FIRE_ERROR_SENSOR);
        fire_system_init(&system, &config);
        sample.captured_ms = 1U;
        CHECK(fire_system_step(&system, &sample, 0U, false, false).error == FIRE_ERROR_STALE);
        break;
    case 18: /* Clear before aiming completes cancels all motion. */
        (void)step(&system, 0U, true, 90);
        (void)step(&system, 50U, true, 90);
        (void)step(&system, 100U, true, 90);
        output = step(&system, 150U, false, 90);
        CHECK(output.state == FIRE_IDLE && !output.motor_enable && !output.pump_on);
        break;
    case 19: /* Out-of-order samples do not contribute to confirmation. */
        (void)step(&system, 100U, true, 90);
        sample.captured_ms = 50U;
        output = fire_system_step(&system, &sample, 150U, false, false);
        CHECK(output.error == FIRE_ERROR_STALE && !output.pump_on);
        break;
    case 20: /* A long acquisition gap restarts confirmation. */
        (void)step(&system, 0U, true, 90);
        (void)step(&system, 50U, true, 90);
        CHECK(step(&system, 350U, true, 90).state == FIRE_IDLE);
        CHECK(step(&system, 400U, true, 90).state == FIRE_IDLE);
        CHECK(step(&system, 450U, true, 90).state == FIRE_AIMING);
        break;
    default:
        return 2;
    }
    printf("TC-%03d Pass (PC logic only)\n", id);
    return 0;
}
int main(int argc, char **argv)
{
    return argc == 2 ? run_case(atoi(argv[1])) : 2;
}
