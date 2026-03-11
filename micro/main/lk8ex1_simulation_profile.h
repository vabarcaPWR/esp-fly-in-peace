#ifndef LK8EX1_SIMULATION_PROFILE_H
#define LK8EX1_SIMULATION_PROFILE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lk8ex1.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum lk8ex1_sim_profile_e
    {
        LK8EX1_SIM_PROFILE_NOMINAL = 0,
        LK8EX1_SIM_PROFILE_CLIMB,
        LK8EX1_SIM_PROFILE_SINK,
        LK8EX1_SIM_PROFILE_EDGE,
        LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM,
        LK8EX1_SIM_PROFILE_MALFORMED_SHAPE,
    } lk8ex1_sim_profile_e;

    typedef struct lk8ex1_simulation_state_s
    {
        lk8ex1_sim_profile_e profile;
        uint32_t frame_index;
    } lk8ex1_simulation_state_t;

    const char *lk8ex1_simulation_profile_to_name(lk8ex1_sim_profile_e profile);
    bool lk8ex1_simulation_parse_profile_command(const uint8_t *data, uint16_t len, lk8ex1_sim_profile_e *profile);
    bool lk8ex1_simulation_build_profile_sentence(const lk8ex1_simulation_state_t *state, char *sentence,
                                                  size_t sentence_size);
    void lk8ex1_simulation_advance_state(lk8ex1_simulation_state_t *state);

#ifdef __cplusplus
}
#endif

#endif
