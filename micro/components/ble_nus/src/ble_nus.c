#include "ble_nus.h"

#include <stdatomic.h>
#include <stddef.h>
#include <string.h>

#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "host/ble_att.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define BLE_NUS_MAX_DEVICE_NAME_LEN 20U
#define BLE_NUS_MTU_REQUESTED 256U
#define BLE_NUS_ATT_OVERHEAD 3U
#define BLE_NUS_ADV_MIN_INTERVAL_UNITS 32U
#define BLE_NUS_ADV_MAX_INTERVAL_UNITS 16384U
#define BLE_NUS_RX_MAX_LEN 512U

static const char *TAG = "ble_nus";

static const ble_uuid128_t NUS_SERVICE_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);
static const ble_uuid128_t NUS_RX_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);
static const ble_uuid128_t NUS_TX_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e);

static SemaphoreHandle_t s_state_mutex;
static _Atomic bool s_initialized;
static _Atomic bool s_connected;
static uint16_t s_conn_handle;
static uint16_t s_nus_tx_val_handle;
static uint16_t s_mtu;
static uint16_t s_adv_interval_ms;
static uint8_t s_own_addr_type;
static bool s_notify_enabled;
static ble_nus_rx_cb_t s_rx_callback;
static ble_nus_state_cb_t s_state_callback;
static char s_device_name[BLE_NUS_MAX_DEVICE_NAME_LEN + 1];

static int ble_nus_gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt,
                                  void *arg)
{
    (void)conn_handle;
    (void)attr_handle;

    const ble_uuid_t *characteristic_uuid = (const ble_uuid_t *)arg;
    if (ble_uuid_cmp(characteristic_uuid, &NUS_RX_UUID.u) == 0)
    {
        if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR)
        {
            return BLE_ATT_ERR_UNLIKELY;
        }

        uint16_t payload_len = OS_MBUF_PKTLEN(ctxt->om);
        if (payload_len == 0)
        {
            return 0;
        }

        if (payload_len > BLE_NUS_RX_MAX_LEN)
        {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        uint8_t rx_data[BLE_NUS_RX_MAX_LEN];
        int copy_result = ble_hs_mbuf_to_flat(ctxt->om, rx_data, payload_len, NULL);
        if (copy_result != 0)
        {
            return BLE_ATT_ERR_UNLIKELY;
        }

        ble_nus_rx_cb_t callback = NULL;
        if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            callback = s_rx_callback;
            xSemaphoreGive(s_state_mutex);
        }

        if (callback != NULL)
        {
            callback(rx_data, payload_len);
        }

        return 0;
    }

    if (ble_uuid_cmp(characteristic_uuid, &NUS_TX_UUID.u) == 0)
    {
        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
        {
            return 0;
        }

        return BLE_ATT_ERR_UNLIKELY;
    }

    return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def s_gatt_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &NUS_SERVICE_UUID.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &NUS_RX_UUID.u,
                    .access_cb = ble_nus_gatt_access_cb,
                    .arg = (void *)&NUS_RX_UUID.u,
                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                },
                {
                    .uuid = &NUS_TX_UUID.u,
                    .access_cb = ble_nus_gatt_access_cb,
                    .arg = (void *)&NUS_TX_UUID.u,
                    .val_handle = &s_nus_tx_val_handle,
                    .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
                },
                {
                    0,
                },
            },
    },
    {
        0,
    },
};

static uint16_t adv_interval_ms_to_units(uint16_t interval_ms)
{
    uint32_t units = ((uint32_t)interval_ms * 1000U) / 625U;
    if (units < BLE_NUS_ADV_MIN_INTERVAL_UNITS)
    {
        units = BLE_NUS_ADV_MIN_INTERVAL_UNITS;
    }
    if (units > BLE_NUS_ADV_MAX_INTERVAL_UNITS)
    {
        units = BLE_NUS_ADV_MAX_INTERVAL_UNITS;
    }

    return (uint16_t)units;
}

static int ble_nus_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT: {
        if (event->connect.status == 0)
        {
            if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
            {
                s_conn_handle = event->connect.conn_handle;
                s_notify_enabled = false;
                s_mtu = BLE_ATT_MTU_DFLT;
                atomic_store(&s_connected, true);
                xSemaphoreGive(s_state_mutex);
            }

            ESP_LOGI(TAG, "BLE connected: conn_handle=%u", event->connect.conn_handle);

            if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
            {
                ble_nus_state_cb_t callback = s_state_callback;
                xSemaphoreGive(s_state_mutex);
                if (callback != NULL)
                {
                    callback(true, event->connect.conn_handle);
                }
            }
        }
        else
        {
            ESP_LOGW(TAG, "BLE connect failed: status=%d, restarting advertising", event->connect.status);

            struct ble_hs_adv_fields adv_fields;
            memset(&adv_fields, 0, sizeof(adv_fields));
            adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
            adv_fields.tx_pwr_lvl_is_present = 1;
            adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
            adv_fields.name = (uint8_t *)s_device_name;
            adv_fields.name_len = (uint8_t)strlen(s_device_name);
            adv_fields.name_is_complete = 1;
            adv_fields.uuids128 = (ble_uuid128_t *)&NUS_SERVICE_UUID;
            adv_fields.num_uuids128 = 1;
            adv_fields.uuids128_is_complete = 1;

            int field_result = ble_gap_adv_set_fields(&adv_fields);
            if (field_result != 0)
            {
                ESP_LOGE(TAG, "ble_gap_adv_set_fields failed: rc=%d", field_result);
                return field_result;
            }

            struct ble_gap_adv_params adv_params;
            memset(&adv_params, 0, sizeof(adv_params));
            adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
            adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
            uint16_t interval_units = adv_interval_ms_to_units(s_adv_interval_ms);
            adv_params.itvl_min = interval_units;
            adv_params.itvl_max = interval_units;

            int adv_result =
                ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_nus_gap_event_cb, NULL);
            if (adv_result != 0)
            {
                ESP_LOGE(TAG, "ble_gap_adv_start failed: rc=%d", adv_result);
                return adv_result;
            }
        }
        return 0;
    }

    case BLE_GAP_EVENT_DISCONNECT: {
        uint16_t disconnected_handle = event->disconnect.conn.conn_handle;

        if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
            s_notify_enabled = false;
            s_mtu = BLE_ATT_MTU_DFLT;
            atomic_store(&s_connected, false);
            xSemaphoreGive(s_state_mutex);
        }

        ESP_LOGI(TAG, "BLE disconnected: conn_handle=%u reason=%d", disconnected_handle, event->disconnect.reason);

        if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            ble_nus_state_cb_t callback = s_state_callback;
            xSemaphoreGive(s_state_mutex);
            if (callback != NULL)
            {
                callback(false, disconnected_handle);
            }
        }

        struct ble_hs_adv_fields adv_fields;
        memset(&adv_fields, 0, sizeof(adv_fields));
        adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
        adv_fields.tx_pwr_lvl_is_present = 1;
        adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
        adv_fields.name = (uint8_t *)s_device_name;
        adv_fields.name_len = (uint8_t)strlen(s_device_name);
        adv_fields.name_is_complete = 1;
        adv_fields.uuids128 = (ble_uuid128_t *)&NUS_SERVICE_UUID;
        adv_fields.num_uuids128 = 1;
        adv_fields.uuids128_is_complete = 1;

        int field_result = ble_gap_adv_set_fields(&adv_fields);
        if (field_result != 0)
        {
            ESP_LOGE(TAG, "ble_gap_adv_set_fields failed: rc=%d", field_result);
            return field_result;
        }

        struct ble_gap_adv_params adv_params;
        memset(&adv_params, 0, sizeof(adv_params));
        adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
        adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
        uint16_t interval_units = adv_interval_ms_to_units(s_adv_interval_ms);
        adv_params.itvl_min = interval_units;
        adv_params.itvl_max = interval_units;

        int adv_result =
            ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_nus_gap_event_cb, NULL);
        if (adv_result != 0)
        {
            ESP_LOGE(TAG, "ble_gap_adv_start failed: rc=%d", adv_result);
            return adv_result;
        }
        return 0;
    }

    case BLE_GAP_EVENT_MTU: {
        if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            s_mtu = event->mtu.value;
            xSemaphoreGive(s_state_mutex);
        }

        ESP_LOGI(TAG, "BLE MTU updated: conn_handle=%u mtu=%u", event->mtu.conn_handle, event->mtu.value);
        return 0;
    }

    case BLE_GAP_EVENT_SUBSCRIBE: {
        if (event->subscribe.attr_handle == s_nus_tx_val_handle)
        {
            if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
            {
                s_notify_enabled = event->subscribe.cur_notify != 0;
                xSemaphoreGive(s_state_mutex);
            }

            ESP_LOGI(TAG, "BLE notify subscription: conn_handle=%u enabled=%u", event->subscribe.conn_handle,
                     event->subscribe.cur_notify);
        }
        return 0;
    }

    default: {
        return 0;
    }
    }
}

static void ble_nus_on_sync(void)
{
    int infer_result = ble_hs_id_infer_auto(0, &s_own_addr_type);
    if (infer_result != 0)
    {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto failed: rc=%d", infer_result);
        return;
    }

    int mtu_result = ble_att_set_preferred_mtu(BLE_NUS_MTU_REQUESTED);
    if (mtu_result != 0)
    {
        ESP_LOGW(TAG, "ble_att_set_preferred_mtu failed: rc=%d", mtu_result);
    }

    struct ble_hs_adv_fields adv_fields;
    memset(&adv_fields, 0, sizeof(adv_fields));
    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.tx_pwr_lvl_is_present = 1;
    adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    adv_fields.name = (uint8_t *)s_device_name;
    adv_fields.name_len = (uint8_t)strlen(s_device_name);
    adv_fields.name_is_complete = 1;
    adv_fields.uuids128 = (ble_uuid128_t *)&NUS_SERVICE_UUID;
    adv_fields.num_uuids128 = 1;
    adv_fields.uuids128_is_complete = 1;

    int field_result = ble_gap_adv_set_fields(&adv_fields);
    if (field_result != 0)
    {
        ESP_LOGE(TAG, "ble_gap_adv_set_fields failed: rc=%d", field_result);
        return;
    }

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    uint16_t interval_units = adv_interval_ms_to_units(s_adv_interval_ms);
    adv_params.itvl_min = interval_units;
    adv_params.itvl_max = interval_units;

    int adv_result = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_nus_gap_event_cb, NULL);
    if (adv_result != 0)
    {
        ESP_LOGE(TAG, "ble_gap_adv_start failed: rc=%d", adv_result);
        return;
    }

    ESP_LOGI(TAG, "BLE advertising started: name=%s interval_ms=%u", s_device_name, s_adv_interval_ms);
}

static void ble_nus_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_nus_init(const ble_nus_cfg_t *cfg)
{
    if (cfg == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (atomic_load(&s_initialized))
    {
        return ESP_ERR_INVALID_STATE;
    }

    const char *requested_name = cfg->device_name != NULL ? cfg->device_name : BLE_NUS_DEFAULT_DEVICE_NAME;
    size_t device_name_len = strlen(requested_name);
    if (device_name_len == 0U || device_name_len > BLE_NUS_MAX_DEVICE_NAME_LEN)
    {
        return ESP_ERR_INVALID_ARG;
    }

    s_state_mutex = xSemaphoreCreateMutex();
    if (s_state_mutex == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    memset(s_device_name, 0, sizeof(s_device_name));
    memcpy(s_device_name, requested_name, device_name_len);

    s_adv_interval_ms = cfg->adv_interval_ms != 0U ? cfg->adv_interval_ms : BLE_NUS_DEFAULT_ADV_INTERVAL_MS;
    s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
    s_notify_enabled = false;
    s_mtu = BLE_ATT_MTU_DFLT;
    s_nus_tx_val_handle = 0;
    s_rx_callback = NULL;
    s_state_callback = NULL;
    atomic_store(&s_connected, false);

    esp_err_t nvs_result = nvs_flash_init();
    if (nvs_result == ESP_ERR_NVS_NO_FREE_PAGES || nvs_result == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_result = nvs_flash_init();
    }
    if (nvs_result != ESP_OK)
    {
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
        return nvs_result;
    }

    esp_err_t hci_result = esp_nimble_hci_init();
    if (hci_result != ESP_OK)
    {
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
        return hci_result;
    }

    nimble_port_init();

    ble_hs_cfg.sync_cb = ble_nus_on_sync;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    int gap_name_result = ble_svc_gap_device_name_set(s_device_name);
    if (gap_name_result != 0)
    {
        nimble_port_deinit();
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
        return ESP_FAIL;
    }

    int count_result = ble_gatts_count_cfg(s_gatt_services);
    if (count_result != 0)
    {
        nimble_port_deinit();
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
        return ESP_FAIL;
    }

    int add_result = ble_gatts_add_svcs(s_gatt_services);
    if (add_result != 0)
    {
        nimble_port_deinit();
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
        return ESP_FAIL;
    }

    nimble_port_freertos_init(ble_nus_host_task);

    atomic_store(&s_initialized, true);
    ESP_LOGI(TAG, "BLE NUS initialized");

    return ESP_OK;
}

esp_err_t ble_nus_deinit(void)
{
    if (!atomic_load(&s_initialized))
    {
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;
    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        conn_handle = s_conn_handle;
        xSemaphoreGive(s_state_mutex);
    }

    int adv_stop_result = ble_gap_adv_stop();
    if (adv_stop_result != 0 && adv_stop_result != BLE_HS_EALREADY)
    {
        ESP_LOGW(TAG, "ble_gap_adv_stop returned rc=%d", adv_stop_result);
    }

    if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
    {
        int terminate_result = ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        if (terminate_result != 0)
        {
            ESP_LOGW(TAG, "ble_gap_terminate returned rc=%d", terminate_result);
        }
    }

    int stop_result = nimble_port_stop();
    if (stop_result != 0)
    {
        ESP_LOGW(TAG, "nimble_port_stop returned rc=%d", stop_result);
    }

    nimble_port_deinit();

    esp_err_t controller_result = esp_nimble_hci_deinit();
    if (controller_result != ESP_OK)
    {
        ESP_LOGW(TAG, "esp_nimble_hci_deinit returned err=0x%x", controller_result);
    }

    if (s_state_mutex != NULL)
    {
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
    }

    s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
    s_notify_enabled = false;
    s_mtu = BLE_ATT_MTU_DFLT;
    s_adv_interval_ms = BLE_NUS_DEFAULT_ADV_INTERVAL_MS;
    s_nus_tx_val_handle = 0;
    s_rx_callback = NULL;
    s_state_callback = NULL;
    memset(s_device_name, 0, sizeof(s_device_name));
    atomic_store(&s_connected, false);
    atomic_store(&s_initialized, false);

    ESP_LOGI(TAG, "BLE NUS deinitialized");
    return ESP_OK;
}

esp_err_t ble_nus_send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0U)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!atomic_load(&s_initialized))
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) != pdTRUE)
    {
        return ESP_FAIL;
    }

    bool connected = atomic_load(&s_connected);
    bool notify_enabled = s_notify_enabled;
    uint16_t conn_handle = s_conn_handle;
    uint16_t mtu = s_mtu;

    if (!connected || !notify_enabled || conn_handle == BLE_HS_CONN_HANDLE_NONE)
    {
        xSemaphoreGive(s_state_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t max_payload = mtu > BLE_NUS_ATT_OVERHEAD ? (uint16_t)(mtu - BLE_NUS_ATT_OVERHEAD) : 20U;
    uint16_t offset = 0;

    while (offset < len)
    {
        uint16_t fragment_len = (uint16_t)(len - offset);
        if (fragment_len > max_payload)
        {
            fragment_len = max_payload;
        }

        struct os_mbuf *packet = ble_hs_mbuf_from_flat(data + offset, fragment_len);
        if (packet == NULL)
        {
            xSemaphoreGive(s_state_mutex);
            return ESP_ERR_NO_MEM;
        }

        int notify_result = ble_gatts_notify_custom(conn_handle, s_nus_tx_val_handle, packet);
        if (notify_result != 0)
        {
            xSemaphoreGive(s_state_mutex);
            return ESP_FAIL;
        }

        offset = (uint16_t)(offset + fragment_len);
    }

    xSemaphoreGive(s_state_mutex);
    return ESP_OK;
}

bool ble_nus_is_connected(void)
{
    return atomic_load(&s_connected);
}

void ble_nus_register_rx_callback(ble_nus_rx_cb_t callback)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_rx_callback = callback;
        xSemaphoreGive(s_state_mutex);
    }
}

void ble_nus_register_state_callback(ble_nus_state_cb_t callback)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_state_callback = callback;
        xSemaphoreGive(s_state_mutex);
    }
}
