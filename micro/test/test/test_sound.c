#include "mock_max98357.h"
#include "mock_piezo.h"
#include "sound.h"
#include "unity.h"

TEST_SOURCE_FILE("../components/sound/src/sound.c")

static int piezo_provider_call_count;
static int max98357_provider_call_count;

static esp_err_t fake_init(void)
{
    return ESP_OK;
}

static esp_err_t fake_update(double vario_cms)
{
    (void)vario_cms;
    return ESP_OK;
}

static esp_err_t fake_play_startup(void)
{
    return ESP_OK;
}

static esp_err_t fake_set_config(const tone_config_t *cfg)
{
    (void)cfg;
    return ESP_OK;
}

static esp_err_t fake_get_config(tone_config_t *cfg)
{
    (void)cfg;
    return ESP_OK;
}

static const char *fake_piezo_get_name(void)
{
    return "piezo";
}

static const char *fake_max98357_get_name(void)
{
    return "max98357";
}

static const sound_generator_t *stub_get_piezo_sound_generator(int cmock_num_calls)
{
    (void)cmock_num_calls;
    piezo_provider_call_count++;

    static const sound_generator_t fake_gen = {
        .init = fake_init,
        .update = fake_update,
        .play_startup = fake_play_startup,
        .get_name = fake_piezo_get_name,
        .set_config = fake_set_config,
        .get_config = fake_get_config,
    };

    return &fake_gen;
}

static const sound_generator_t *stub_get_max98357_sound_generator(int cmock_num_calls)
{
    (void)cmock_num_calls;
    max98357_provider_call_count++;

    static const sound_generator_t fake_gen = {
        .init = fake_init,
        .update = fake_update,
        .play_startup = fake_play_startup,
        .get_name = fake_max98357_get_name,
        .set_config = fake_set_config,
        .get_config = fake_get_config,
    };

    return &fake_gen;
}

void setUp(void)
{
    piezo_provider_call_count = 0;
    max98357_provider_call_count = 0;
    get_piezo_sound_generator_StubWithCallback(stub_get_piezo_sound_generator);
    get_max98357_sound_generator_StubWithCallback(stub_get_max98357_sound_generator);
}

void tearDown(void)
{
}

void test_get_sound_generator_null_name_returns_null(void)
{
    TEST_ASSERT_NULL(get_sound_generator(NULL));
}

void test_get_sound_generator_unknown_name_returns_null(void)
{
    TEST_ASSERT_NULL(get_sound_generator("unknown"));
}

void test_get_sound_generator_unknown_does_not_query_piezo(void)
{
    (void)get_sound_generator("unknown");
    TEST_ASSERT_EQUAL_INT(0, piezo_provider_call_count);
}

void test_get_sound_generator_piezo_returns_non_null(void)
{
    TEST_ASSERT_NOT_NULL(get_sound_generator("piezo"));
}

void test_get_sound_generator_piezo_queries_piezo_backend(void)
{
    (void)get_sound_generator("piezo");
    TEST_ASSERT_EQUAL_INT(1, piezo_provider_call_count);
}

void test_get_sound_generator_piezo_backend_name_matches(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_EQUAL_STRING("piezo", gen->get_name());
}

void test_get_sound_generator_piezo_backend_has_play_startup(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_NOT_NULL(gen->play_startup);
}

void test_get_sound_generator_piezo_play_startup_returns_ok(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_EQUAL(ESP_OK, gen->play_startup());
}

void test_get_sound_generator_max98357_returns_non_null(void)
{
    TEST_ASSERT_NOT_NULL(get_sound_generator("max98357"));
}

void test_get_sound_generator_max98357_queries_max98357_backend(void)
{
    (void)get_sound_generator("max98357");
    TEST_ASSERT_EQUAL_INT(1, max98357_provider_call_count);
}

void test_get_sound_generator_max98357_does_not_query_piezo(void)
{
    (void)get_sound_generator("max98357");
    TEST_ASSERT_EQUAL_INT(0, piezo_provider_call_count);
}

void test_get_sound_generator_max98357_backend_name_matches(void)
{
    const sound_generator_t *gen = get_sound_generator("max98357");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_EQUAL_STRING("max98357", gen->get_name());
}

void test_get_sound_generator_max98357_backend_has_play_startup(void)
{
    const sound_generator_t *gen = get_sound_generator("max98357");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_NOT_NULL(gen->play_startup);
}

void test_get_sound_generator_max98357_play_startup_returns_ok(void)
{
    const sound_generator_t *gen = get_sound_generator("max98357");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_EQUAL(ESP_OK, gen->play_startup());
}

void test_piezo_backend_has_set_config(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_NOT_NULL(gen->set_config);
}

void test_piezo_backend_has_get_config(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_NOT_NULL(gen->get_config);
}

void test_max98357_backend_has_set_config(void)
{
    const sound_generator_t *gen = get_sound_generator("max98357");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_NOT_NULL(gen->set_config);
}

void test_max98357_backend_has_get_config(void)
{
    const sound_generator_t *gen = get_sound_generator("max98357");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_NOT_NULL(gen->get_config);
}
