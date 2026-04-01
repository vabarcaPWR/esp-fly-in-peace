#include "tone_model.h"
#include "unity.h"
#include <math.h>

TEST_SOURCE_FILE("../components/sound/src/tone/src/tone_model.c")

#define FLOAT_TOL 0.01f

static tone_curve_t default_curve(void)
{
    const tone_config_t *cfg = tone_config_get_defaults();
    return cfg->curve;
}

static tone_thresholds_t default_thresholds(void)
{
    const tone_config_t *cfg = tone_config_get_defaults();
    return cfg->thresholds;
}

void setUp(void)
{
}
void tearDown(void)
{
}

void test_compute_at_exact_breakpoint_returns_breakpoint_values(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 1.0f, &out);
    TEST_ASSERT_EQUAL_UINT16(700, out.freq_hz);
    TEST_ASSERT_EQUAL_UINT16(420, out.cycle_ms);
    TEST_ASSERT_EQUAL_UINT8(40, out.duty_pct);
    TEST_ASSERT_EQUAL(TONE_ZONE_CLIMB, out.zone);
}

void test_compute_at_first_breakpoint_returns_first_values(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, -10.0f, &out);
    TEST_ASSERT_EQUAL_UINT16(200, out.freq_hz);
    TEST_ASSERT_EQUAL_UINT16(0, out.cycle_ms);
    TEST_ASSERT_EQUAL_UINT8(100, out.duty_pct);
    TEST_ASSERT_EQUAL(TONE_ZONE_SINK, out.zone);
}

void test_compute_at_last_breakpoint_returns_last_values(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 10.0f, &out);
    TEST_ASSERT_EQUAL_UINT16(1600, out.freq_hz);
    TEST_ASSERT_EQUAL_UINT16(180, out.cycle_ms);
    TEST_ASSERT_EQUAL_UINT8(55, out.duty_pct);
    TEST_ASSERT_EQUAL(TONE_ZONE_CLIMB, out.zone);
}

void test_compute_between_breakpoints_interpolates_linearly(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 1.5f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_CLIMB, out.zone);

    uint16_t expected_freq = (uint16_t)((700.0f + 950.0f) / 2.0f + 0.5f);
    uint16_t expected_cycle = (uint16_t)((420.0f + 320.0f) / 2.0f + 0.5f);
    uint8_t expected_duty = (uint8_t)((40.0f + 45.0f) / 2.0f + 0.5f);

    TEST_ASSERT_EQUAL_UINT16(expected_freq, out.freq_hz);
    TEST_ASSERT_EQUAL_UINT16(expected_cycle, out.cycle_ms);
    TEST_ASSERT_EQUAL_UINT8(expected_duty, out.duty_pct);
}

void test_compute_below_first_breakpoint_clamps_to_first(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, -15.0f, &out);
    TEST_ASSERT_EQUAL_UINT16(200, out.freq_hz);
    TEST_ASSERT_EQUAL(TONE_ZONE_SINK, out.zone);
}

void test_compute_above_last_breakpoint_clamps_to_last(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 20.0f, &out);
    TEST_ASSERT_EQUAL_UINT16(1600, out.freq_hz);
    TEST_ASSERT_EQUAL_UINT16(180, out.cycle_ms);
    TEST_ASSERT_EQUAL(TONE_ZONE_CLIMB, out.zone);
}

void test_vario_in_dead_zone_returns_silence(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 0.0f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_SILENCE, out.zone);
    TEST_ASSERT_EQUAL_UINT16(0, out.freq_hz);
}

void test_vario_in_dead_zone_negative_returns_silence(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, -1.0f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_SILENCE, out.zone);
}

void test_vario_in_pre_lift_zone_returns_pre_lift(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 0.10f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_PRE_LIFT, out.zone);
    TEST_ASSERT_EQUAL_UINT16(400, out.freq_hz);
    TEST_ASSERT_EQUAL_UINT16(1000, out.cycle_ms);
    TEST_ASSERT_EQUAL_UINT8(5, out.duty_pct);
}

void test_pre_lift_disabled_returns_silence_in_pre_lift_zone(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, false, 0.10f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_SILENCE, out.zone);
}

void test_vario_crosses_climb_on_returns_climb(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 0.15f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_CLIMB, out.zone);
    TEST_ASSERT_TRUE(out.freq_hz > 0);
}

void test_vario_just_below_climb_on_is_pre_lift(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 0.14f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_PRE_LIFT, out.zone);
}

void test_vario_at_climb_off_boundary_is_pre_lift(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 0.05f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_PRE_LIFT, out.zone);
}

void test_vario_just_below_climb_off_is_silence(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, 0.04f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_SILENCE, out.zone);
}

void test_sink_threshold_triggers_sink_zone(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, -2.0f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_SINK, out.zone);
    TEST_ASSERT_TRUE(out.freq_hz > 0);
}

void test_vario_just_above_sink_on_is_silence(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, -1.9f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_SILENCE, out.zone);
}

void test_default_config_passes_validation(void)
{
    const tone_config_t *cfg = tone_config_get_defaults();
    TEST_ASSERT_EQUAL(ESP_OK, tone_config_validate(cfg));
}

void test_validate_null_config_returns_invalid_arg(void)
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(NULL));
}

void test_validate_unsorted_breakpoints_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.curve.points[1].vario_ms = cfg.curve.points[0].vario_ms;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_validate_freq_too_high_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.curve.points[0].freq_hz = 5000;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_validate_count_below_minimum_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.curve.count = 1;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_validate_count_above_maximum_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.curve.count = TONE_MAX_POINTS + 1;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_validate_climb_on_lte_climb_off_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.thresholds.climb_on_ms = cfg.thresholds.climb_off_ms;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_validate_sink_on_gte_sink_off_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.thresholds.sink_on_ms = cfg.thresholds.sink_off_ms;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_validate_volume_over_100_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.volume_pct = 101;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_validate_duty_over_100_rejected(void)
{
    tone_config_t cfg = *tone_config_get_defaults();
    cfg.curve.points[0].duty_pct = 101;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, tone_config_validate(&cfg));
}

void test_compute_null_curve_does_not_crash(void)
{
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {.freq_hz = 999};

    tone_model_compute(NULL, &thresh, true, 1.0f, &out);
    TEST_ASSERT_EQUAL_UINT16(999, out.freq_hz);
}

void test_compute_null_output_does_not_crash(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();

    tone_model_compute(&curve, &thresh, true, 1.0f, NULL);
}

void test_sink_zone_continuous_tone_has_zero_cycle(void)
{
    tone_curve_t curve = default_curve();
    tone_thresholds_t thresh = default_thresholds();
    tone_output_t out = {0};

    tone_model_compute(&curve, &thresh, true, -5.0f, &out);
    TEST_ASSERT_EQUAL(TONE_ZONE_SINK, out.zone);
    TEST_ASSERT_EQUAL_UINT16(0, out.cycle_ms);
    TEST_ASSERT_EQUAL_UINT8(100, out.duty_pct);
}
