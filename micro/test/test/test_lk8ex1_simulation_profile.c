#include "lk8ex1_simulation_profile.h"
#include "unity.h"

TEST_SOURCE_FILE("../main/lk8ex1_simulation_profile.c")
TEST_SOURCE_FILE("../components/lk8ex1/src/lk8ex1.c")
TEST_SOURCE_FILE("../components/lk8ex1/src/lk8ex1_model.c")
TEST_SOURCE_FILE("../components/lk8ex1/src/lk8ex1_hardware.c")

#include <string.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_parse_profile_command_detects_climb(void)
{
    const char command[] = "profile:climb";
    lk8ex1_sim_profile_e profile = LK8EX1_SIM_PROFILE_NOMINAL;

    TEST_ASSERT_TRUE(
        lk8ex1_simulation_parse_profile_command((const uint8_t *)command, (uint16_t)strlen(command), &profile));
    TEST_ASSERT_EQUAL_INT(LK8EX1_SIM_PROFILE_CLIMB, profile);
}

void test_parse_profile_command_rejects_unknown_command(void)
{
    const char command[] = "profile:unknown";
    lk8ex1_sim_profile_e profile = LK8EX1_SIM_PROFILE_NOMINAL;

    TEST_ASSERT_FALSE(
        lk8ex1_simulation_parse_profile_command((const uint8_t *)command, (uint16_t)strlen(command), &profile));
}

void test_build_profile_sentence_nominal_is_valid_lk8ex1(void)
{
    lk8ex1_simulation_state_t state = {
        .profile = LK8EX1_SIM_PROFILE_NOMINAL,
        .frame_index = 0,
    };
    char sentence[LK8EX1_MAX_SENTENCE_LEN];

    TEST_ASSERT_TRUE(lk8ex1_simulation_build_profile_sentence(&state, sentence, sizeof(sentence)));
    TEST_ASSERT_TRUE(lk8ex1_validate(sentence));
}

void test_build_profile_sentence_malformed_checksum_is_invalid_lk8ex1(void)
{
    lk8ex1_simulation_state_t state = {
        .profile = LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM,
        .frame_index = 1,
    };
    char sentence[LK8EX1_MAX_SENTENCE_LEN];

    TEST_ASSERT_TRUE(lk8ex1_simulation_build_profile_sentence(&state, sentence, sizeof(sentence)));
    TEST_ASSERT_FALSE(lk8ex1_validate(sentence));
}

void test_advance_state_increments_frame_index(void)
{
    lk8ex1_simulation_state_t state = {
        .profile = LK8EX1_SIM_PROFILE_NOMINAL,
        .frame_index = 5,
    };

    lk8ex1_simulation_advance_state(&state);

    TEST_ASSERT_EQUAL_UINT32(6U, state.frame_index);
}
