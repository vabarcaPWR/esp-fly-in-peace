#include "lk8ex1_simulation_profile.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define LK8EX1_PROFILE_COMMAND_MAX_LEN 64U

const char *lk8ex1_simulation_profile_to_name(lk8ex1_sim_profile_e profile)
{
    switch (profile)
    {
    case LK8EX1_SIM_PROFILE_NOMINAL:
        return "nominal";
    case LK8EX1_SIM_PROFILE_CLIMB:
        return "climb";
    case LK8EX1_SIM_PROFILE_SINK:
        return "sink";
    case LK8EX1_SIM_PROFILE_EDGE:
        return "edge";
    case LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM:
        return "malformed-checksum";
    case LK8EX1_SIM_PROFILE_MALFORMED_SHAPE:
        return "malformed-shape";
    default:
        return "unknown";
    }
}

static int32_t lk8ex1_simulation_battery_percent_for_frame(uint32_t frame_index)
{
    int32_t battery = 96 - (int32_t)(frame_index / 240U);
    if (battery < 15)
    {
        battery = 15;
    }

    return battery;
}

static void lk8ex1_simulation_build_nominal_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    static const int32_t pressure_offsets[8] = {0, -3, -2, -1, 0, 1, 2, 1};
    static const int32_t altitude_offsets[8] = {0, 1, 1, 0, 0, -1, -1, 0};
    static const int32_t vario_values[8] = {5, 8, 3, 0, -2, -4, -1, 2};
    static const int32_t temperature_offsets[8] = {0, 1, 1, 0, 0, -1, -1, 0};
    uint32_t sample = frame_index % 8U;

    data->pressure_pa = 100900 + pressure_offsets[sample];
    data->altitude_m = 1035 + altitude_offsets[sample];
    data->vario_cms = vario_values[sample];
    data->temperature_dc = 235 + temperature_offsets[sample];
    data->battery_mv = lk8ex1_simulation_battery_percent_for_frame(frame_index);
}

static void lk8ex1_simulation_build_climb_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    uint32_t trend_sample = frame_index % 320U;
    int32_t altitude = 920 + (int32_t)(trend_sample / 2U);
    int32_t pressure = 101325 - (altitude * 12);

    data->pressure_pa = pressure;
    data->altitude_m = altitude;
    data->vario_cms = 180 + (int32_t)(frame_index % 6U);
    data->temperature_dc = 228;
    data->battery_mv = lk8ex1_simulation_battery_percent_for_frame(frame_index);
}

static void lk8ex1_simulation_build_sink_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    uint32_t trend_sample = frame_index % 320U;
    int32_t altitude = 1260 - (int32_t)(trend_sample / 2U);
    int32_t pressure = 101325 - (altitude * 12);

    data->pressure_pa = pressure;
    data->altitude_m = altitude;
    data->vario_cms = -180 - (int32_t)(frame_index % 6U);
    data->temperature_dc = 224;
    data->battery_mv = lk8ex1_simulation_battery_percent_for_frame(frame_index);
}

static void lk8ex1_simulation_build_edge_data(uint32_t frame_index, lk8ex1_data_t *data)
{
    uint32_t sample = frame_index % 6U;

    if (sample == 0U)
    {
        data->pressure_pa = 100840;
        data->altitude_m = 99999;
        data->vario_cms = 0;
        data->temperature_dc = 230;
        data->battery_mv = 93;
        return;
    }

    if (sample == 1U)
    {
        data->pressure_pa = 100860;
        data->altitude_m = 1010;
        data->vario_cms = -5;
        data->temperature_dc = 232;
        data->battery_mv = 999;
        return;
    }

    if (sample == 2U)
    {
        data->pressure_pa = 30000;
        data->altitude_m = -500;
        data->vario_cms = 2500;
        data->temperature_dc = -200;
        data->battery_mv = 100;
        return;
    }

    if (sample == 3U)
    {
        data->pressure_pa = 120000;
        data->altitude_m = 9000;
        data->vario_cms = -2500;
        data->temperature_dc = 600;
        data->battery_mv = 5;
        return;
    }

    if (sample == 4U)
    {
        data->pressure_pa = 101325;
        data->altitude_m = 0;
        data->vario_cms = 0;
        data->temperature_dc = -400;
        data->battery_mv = 50;
        return;
    }

    data->pressure_pa = 101325;
    data->altitude_m = 0;
    data->vario_cms = 0;
    data->temperature_dc = 850;
    data->battery_mv = 50;
}

bool lk8ex1_simulation_parse_profile_command(const uint8_t *data, uint16_t len, lk8ex1_sim_profile_e *profile)
{
    if (!data || !profile || (len == 0U))
    {
        return false;
    }

    size_t copy_len = len;
    if (copy_len > (LK8EX1_PROFILE_COMMAND_MAX_LEN - 1U))
    {
        copy_len = LK8EX1_PROFILE_COMMAND_MAX_LEN - 1U;
    }

    char command[LK8EX1_PROFILE_COMMAND_MAX_LEN];
    memset(command, 0, sizeof(command));
    memcpy(command, data, copy_len);

    for (size_t i = 0; i < copy_len; i++)
    {
        command[i] = (char)toupper((unsigned char)command[i]);
    }

    if (strstr(command, "NOMINAL"))
    {
        *profile = LK8EX1_SIM_PROFILE_NOMINAL;
        return true;
    }

    if (strstr(command, "CLIMB"))
    {
        *profile = LK8EX1_SIM_PROFILE_CLIMB;
        return true;
    }

    if (strstr(command, "SINK"))
    {
        *profile = LK8EX1_SIM_PROFILE_SINK;
        return true;
    }

    if (strstr(command, "EDGE"))
    {
        *profile = LK8EX1_SIM_PROFILE_EDGE;
        return true;
    }

    if (strstr(command, "CHECKSUM"))
    {
        *profile = LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM;
        return true;
    }

    if (strstr(command, "SHAPE"))
    {
        *profile = LK8EX1_SIM_PROFILE_MALFORMED_SHAPE;
        return true;
    }

    return false;
}

static bool lk8ex1_simulation_corrupt_sentence_checksum(char *sentence)
{
    if (!sentence)
    {
        return false;
    }

    char *asterisk = strchr(sentence, '*');
    if (!asterisk || (asterisk[1] == '\0'))
    {
        return false;
    }

    asterisk[1] = (asterisk[1] == 'A') ? 'B' : 'A';
    return true;
}

static bool lk8ex1_simulation_build_malformed_shape_sentence(char *sentence, size_t sentence_size)
{
    if (!sentence || (sentence_size == 0U))
    {
        return false;
    }

    int32_t written = snprintf(sentence, sentence_size, "$LK8EX1,101325,1000,120*00\r\n");
    return (written > 0) && ((size_t)written < sentence_size);
}

bool lk8ex1_simulation_build_profile_sentence(const lk8ex1_simulation_state_t *state, char *sentence,
                                              size_t sentence_size)
{
    if (!state || !sentence)
    {
        return false;
    }

    if (state->profile == LK8EX1_SIM_PROFILE_MALFORMED_SHAPE)
    {
        return lk8ex1_simulation_build_malformed_shape_sentence(sentence, sentence_size);
    }

    lk8ex1_data_t lk8ex1_data = {
        .pressure_pa = 0,
        .altitude_m = 0,
        .vario_cms = 0,
        .temperature_dc = 0,
        .battery_mv = 0,
    };

    switch (state->profile)
    {
    case LK8EX1_SIM_PROFILE_NOMINAL:
        lk8ex1_simulation_build_nominal_data(state->frame_index, &lk8ex1_data);
        break;
    case LK8EX1_SIM_PROFILE_CLIMB:
        lk8ex1_simulation_build_climb_data(state->frame_index, &lk8ex1_data);
        break;
    case LK8EX1_SIM_PROFILE_SINK:
        lk8ex1_simulation_build_sink_data(state->frame_index, &lk8ex1_data);
        break;
    case LK8EX1_SIM_PROFILE_EDGE:
        lk8ex1_simulation_build_edge_data(state->frame_index, &lk8ex1_data);
        break;
    case LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM:
        lk8ex1_simulation_build_nominal_data(state->frame_index, &lk8ex1_data);
        break;
    default:
        return false;
    }

    if (lk8ex1_format(&lk8ex1_data, sentence, sentence_size) != ESP_OK)
    {
        return false;
    }

    if (state->profile == LK8EX1_SIM_PROFILE_MALFORMED_CHECKSUM)
    {
        return lk8ex1_simulation_corrupt_sentence_checksum(sentence);
    }

    return true;
}

void lk8ex1_simulation_advance_state(lk8ex1_simulation_state_t *state)
{
    if (!state)
    {
        return;
    }

    state->frame_index++;
}
