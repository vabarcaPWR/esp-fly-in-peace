#include "max98357_model.h"
#include "unity.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

TEST_SOURCE_FILE("../components/sound/src/max98357/src/max98357_model.c")

#define TEST_SAMPLE_RATE 16000
#define TEST_BUF_SIZE 800

static synth_state_t s_state;
static int16_t s_buf[TEST_BUF_SIZE];

void setUp(void)
{
    memset(&s_state, 0, sizeof(s_state));
    memset(s_buf, 0xAA, sizeof(s_buf));
}

void tearDown(void)
{
}

void test_synth_init_returns_ok(void)
{
    TEST_ASSERT_EQUAL(ESP_OK, max98357_mdl_init(&s_state, TEST_SAMPLE_RATE));
}

void test_synth_init_null_returns_invalid_arg(void)
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, max98357_mdl_init(NULL, TEST_SAMPLE_RATE));
}

void test_synth_init_sets_sample_rate(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    TEST_ASSERT_EQUAL_UINT32(TEST_SAMPLE_RATE, s_state.sample_rate);
}

void test_synth_init_resets_phase(void)
{
    s_state.phase_accumulator = 12345;
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    TEST_ASSERT_EQUAL_UINT32(0, s_state.phase_accumulator);
}

void test_sine_table_at_0_degrees_is_zero(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    TEST_ASSERT_INT_WITHIN(100, 0, s_state.sine_table[0]);
}

void test_sine_table_at_90_degrees_is_positive_max(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    int16_t val = s_state.sine_table[SYNTH_SINE_TABLE_SIZE / 4];
    TEST_ASSERT_INT_WITHIN(1, 32767, val);
}

void test_sine_table_at_180_degrees_is_zero(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    int16_t val = s_state.sine_table[SYNTH_SINE_TABLE_SIZE / 2];
    TEST_ASSERT_INT_WITHIN(100, 0, val);
}

void test_sine_table_at_270_degrees_is_negative_max(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    int16_t val = s_state.sine_table[3 * SYNTH_SINE_TABLE_SIZE / 4];
    TEST_ASSERT_INT_WITHIN(1, -32767, val);
}

void test_fill_tone_produces_nonzero_samples(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    max98357_mdl_fill_tone(&s_state, s_buf, TEST_BUF_SIZE, 440, 100);

    int nonzero = 0;
    for (size_t i = 0; i < TEST_BUF_SIZE; i++)
    {
        if (s_buf[i] != 0)
            nonzero++;
    }
    TEST_ASSERT_TRUE(nonzero > TEST_BUF_SIZE / 2);
}

void test_fill_silence_produces_all_zeros(void)
{
    memset(s_buf, 0xFF, sizeof(s_buf));
    max98357_mdl_fill_silence(s_buf, TEST_BUF_SIZE);

    for (size_t i = 0; i < TEST_BUF_SIZE; i++)
    {
        TEST_ASSERT_EQUAL_INT16(0, s_buf[i]);
    }
}

void test_fill_tone_zero_freq_produces_silence(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    max98357_mdl_fill_tone(&s_state, s_buf, TEST_BUF_SIZE, 0, 100);

    for (size_t i = 0; i < TEST_BUF_SIZE; i++)
    {
        TEST_ASSERT_EQUAL_INT16(0, s_buf[i]);
    }
}

void test_fill_tone_volume_50_halves_amplitude(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);

    int16_t buf_full[TEST_BUF_SIZE];
    int16_t buf_half[TEST_BUF_SIZE];

    max98357_mdl_fill_tone(&s_state, buf_full, TEST_BUF_SIZE, 1000, 100);

    s_state.phase_accumulator = 0;
    max98357_mdl_fill_tone(&s_state, buf_half, TEST_BUF_SIZE, 1000, 50);

    int16_t max_full = 0;
    int16_t max_half = 0;
    for (size_t i = 0; i < TEST_BUF_SIZE; i++)
    {
        if (abs(buf_full[i]) > max_full)
            max_full = abs(buf_full[i]);
        if (abs(buf_half[i]) > max_half)
            max_half = abs(buf_half[i]);
    }

    TEST_ASSERT_TRUE(max_full > 0);
    TEST_ASSERT_TRUE(max_half > 0);
    float ratio = (float)max_half / (float)max_full;
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 0.5f, ratio);
}

void test_fill_tone_phase_continuity(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);

    int16_t buf1[256];
    int16_t buf2[256];
    max98357_mdl_fill_tone(&s_state, buf1, 256, 440, 100);
    max98357_mdl_fill_tone(&s_state, buf2, 256, 440, 100);

    int16_t last_of_first = buf1[255];
    int16_t first_of_second = buf2[0];
    int16_t jump = abs(first_of_second - last_of_first);

    TEST_ASSERT_TRUE(jump < 6000);
}

void test_fill_tone_null_state_does_not_crash(void)
{
    max98357_mdl_fill_tone(NULL, s_buf, TEST_BUF_SIZE, 440, 100);
}

void test_fill_tone_null_buf_does_not_crash(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    max98357_mdl_fill_tone(&s_state, NULL, TEST_BUF_SIZE, 440, 100);
}

void test_fill_silence_null_buf_does_not_crash(void)
{
    max98357_mdl_fill_silence(NULL, TEST_BUF_SIZE);
}

void test_fill_tone_volume_zero_produces_silence(void)
{
    max98357_mdl_init(&s_state, TEST_SAMPLE_RATE);
    max98357_mdl_fill_tone(&s_state, s_buf, TEST_BUF_SIZE, 440, 0);

    for (size_t i = 0; i < TEST_BUF_SIZE; i++)
    {
        TEST_ASSERT_EQUAL_INT16(0, s_buf[i]);
    }
}
