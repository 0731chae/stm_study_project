#ifndef FIRE_PORT_H
#define FIRE_PORT_H
#include "fire_types.h"

/* Contract only: board-specific implementations do not exist yet.
 * init must establish safe physical outputs before enabling any actuator.
 * acquire: return true only for a new complete sample; on failure set valid=false.
 * apply: return false on a detected IO failure, and force physical outputs off.
 * force_off: use driver-specific OFF polarity and emergency stop behavior.
 * All calls are from the main loop, not an ISR. Never fabricate timestamps.
 */
bool fire_port_init(void);
bool fire_port_acquire(fire_sample_t *sample);
bool fire_port_apply(const fire_output_t *output);
void fire_port_force_off(void);

#endif
