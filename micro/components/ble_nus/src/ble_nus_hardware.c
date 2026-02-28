#include "ble_nus_hardware.h"

#include <stddef.h>
#include <string.h>

#include "ble_nus_model.h"
#include "esp_log.h"
#include "host/ble_att.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define BLE_NUS_MTU_REQUESTED 256U
#define BLE_NUS_ADV_MIN_INTERVAL_UNITS 32U
#define BLE_NUS_ADV_MAX_INTERVAL_UNITS 16384U
#define BLE_NUS_RX_MAX_LEN 512U
#define BLE_NUS_DEFAULT_CONN_HANDLE 0xFFFFU
#define BLE_NUS_DEFAULT_MTU BLE_ATT_MTU_DFLT
#define BLE_NUS_ADV_SHORT_NAME_LEN 6U

static const char *TAG = "ble_nus_hw";
static uint16_t s_tx_value_handle;

static const ble_uuid128_t NUS_SERVICE_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);
static const ble_uuid128_t NUS_RX_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);
static const ble_uuid128_t NUS_TX_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e);

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

static int ble_nus_hardware_set_advertising_data(void)
{
    const char *device_name = ble_nus_model_get_device_name();
    uint8_t device_name_len = (uint8_t)strlen(device_name);
    uint8_t short_name_len = device_name_len;
    if (short_name_len > BLE_NUS_ADV_SHORT_NAME_LEN)
    {
        short_name_len = BLE_NUS_ADV_SHORT_NAME_LEN;
    }

    struct ble_hs_adv_fields adv_fields;
    memset(&adv_fields, 0, sizeof(adv_fields));
    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.uuids128 = (ble_uuid128_t *)&NUS_SERVICE_UUID;
    adv_fields.num_uuids128 = 1;
    adv_fields.uuids128_is_complete = 1;
    adv_fields.name = (uint8_t *)device_name;
    adv_fields.name_len = short_name_len;
    adv_fields.name_is_complete = (short_name_len == device_name_len);

    int field_result = ble_gap_adv_set_fields(&adv_fields);
    if (field_result)
    {
        ESP_LOGE(TAG, "ble_gap_adv_set_fields failed: rc=%d", field_result);
        return field_result;
    }

    struct ble_hs_adv_fields scan_rsp_fields;
    memset(&scan_rsp_fields, 0, sizeof(scan_rsp_fields));
    scan_rsp_fields.tx_pwr_lvl_is_present = 1;
    scan_rsp_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    scan_rsp_fields.name = (uint8_t *)device_name;
    scan_rsp_fields.name_len = device_name_len;
    scan_rsp_fields.name_is_complete = 1;

    int scan_rsp_result = ble_gap_adv_rsp_set_fields(&scan_rsp_fields);
    if (scan_rsp_result)
    {
        ESP_LOGE(TAG, "ble_gap_adv_rsp_set_fields failed: rc=%d", scan_rsp_result);
        return scan_rsp_result;
    }

    return 0;
}

static int ble_nus_hardware_start_advertising(int (*event_cb)(struct ble_gap_event *event, void *arg))
{
    int field_result = ble_nus_hardware_set_advertising_data();
    if (field_result)
    {
        return field_result;
    }

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    uint16_t interval_units = adv_interval_ms_to_units(ble_nus_model_get_adv_interval_ms());
    adv_params.itvl_min = interval_units;
    adv_params.itvl_max = interval_units;

    int adv_result =
        ble_gap_adv_start(ble_nus_model_get_own_addr_type(), NULL, BLE_HS_FOREVER, &adv_params, event_cb, NULL);
    if (adv_result)
    {
        ESP_LOGE(TAG, "ble_gap_adv_start failed: rc=%d", adv_result);
        return adv_result;
    }

    return 0;
}

static int ble_nus_hardware_gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                           struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle;
    (void)attr_handle;

    const ble_uuid_t *characteristic_uuid = (const ble_uuid_t *)arg;
    if (!ble_uuid_cmp(characteristic_uuid, &NUS_RX_UUID.u))
    {
        if (BLE_GATT_ACCESS_OP_WRITE_CHR != ctxt->op)
            return BLE_ATT_ERR_UNLIKELY;

        uint16_t payload_len = OS_MBUF_PKTLEN(ctxt->om);
        if (!payload_len)
            return 0;

        if (payload_len > BLE_NUS_RX_MAX_LEN)
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

        uint8_t rx_data[BLE_NUS_RX_MAX_LEN];
        int copy_result = ble_hs_mbuf_to_flat(ctxt->om, rx_data, payload_len, NULL);
        if (copy_result)
            return BLE_ATT_ERR_UNLIKELY;

        ble_nus_rx_cb_t callback = ble_nus_model_get_rx_callback();
        if (callback)
        {
            callback(rx_data, payload_len);
        }

        return 0;
    }

    if (!ble_uuid_cmp(characteristic_uuid, &NUS_TX_UUID.u))
    {
        if (BLE_GATT_ACCESS_OP_READ_CHR == ctxt->op)
            return 0;

        return BLE_ATT_ERR_UNLIKELY;
    }

    return BLE_ATT_ERR_UNLIKELY;
}

static int ble_nus_hardware_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT: {
        if (!event->connect.status)
        {
            ble_nus_model_set_conn_state(true, event->connect.conn_handle, BLE_NUS_DEFAULT_CONN_HANDLE,
                                         BLE_NUS_DEFAULT_MTU);
            ESP_LOGI(TAG, "BLE connected: conn_handle=%u", event->connect.conn_handle);

            ble_nus_state_cb_t callback = ble_nus_model_get_state_callback();
            if (callback)
            {
                callback(true, event->connect.conn_handle);
            }
        }
        else
        {
            ESP_LOGW(TAG, "BLE connect failed: status=%d", event->connect.status);
            return ble_nus_hardware_start_advertising(ble_nus_hardware_gap_event_cb);
        }
        return 0;
    }

    case BLE_GAP_EVENT_DISCONNECT: {
        uint16_t disconnected_handle = event->disconnect.conn.conn_handle;
        ble_nus_model_set_conn_state(false, disconnected_handle, BLE_NUS_DEFAULT_CONN_HANDLE, BLE_NUS_DEFAULT_MTU);
        ESP_LOGI(TAG, "BLE disconnected: conn_handle=%u reason=%d", disconnected_handle, event->disconnect.reason);

        ble_nus_state_cb_t callback = ble_nus_model_get_state_callback();
        if (callback)
        {
            callback(false, disconnected_handle);
        }

        return ble_nus_hardware_start_advertising(ble_nus_hardware_gap_event_cb);
    }

    case BLE_GAP_EVENT_MTU: {
        ble_nus_model_set_mtu(event->mtu.value);
        ESP_LOGI(TAG, "BLE MTU updated: conn_handle=%u mtu=%u", event->mtu.conn_handle, event->mtu.value);
        return 0;
    }

    case BLE_GAP_EVENT_SUBSCRIBE: {
        uint16_t tx_value_handle = ble_nus_model_get_tx_value_handle();
        bool notify_enabled = event->subscribe.cur_notify != 0U;
        if (event->subscribe.attr_handle == tx_value_handle || notify_enabled || event->subscribe.prev_notify != 0U)
        {
            if (notify_enabled && !tx_value_handle && event->subscribe.attr_handle)
            {
                ble_nus_model_set_tx_value_handle(event->subscribe.attr_handle);
                tx_value_handle = event->subscribe.attr_handle;
                ESP_LOGI(TAG, "NUS TX value handle synchronized from subscribe event: handle=%u", tx_value_handle);
            }

            ble_nus_model_set_notify_enabled(notify_enabled);
            ESP_LOGI(TAG, "BLE notify subscription: conn_handle=%u attr_handle=%u tx_handle=%u enabled=%u prev=%u",
                     event->subscribe.conn_handle, event->subscribe.attr_handle, tx_value_handle,
                     event->subscribe.cur_notify, event->subscribe.prev_notify);
        }
        return 0;
    }

    default: {
        return 0;
    }
    }
}

static const struct ble_gatt_svc_def s_gatt_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &NUS_SERVICE_UUID.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &NUS_RX_UUID.u,
                    .access_cb = ble_nus_hardware_gatt_access_cb,
                    .arg = (void *)&NUS_RX_UUID.u,
                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                },
                {
                    .uuid = &NUS_TX_UUID.u,
                    .access_cb = ble_nus_hardware_gatt_access_cb,
                    .arg = (void *)&NUS_TX_UUID.u,
                    .val_handle = &s_tx_value_handle,
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

static void ble_nus_hardware_on_sync(void)
{
    uint8_t own_addr_type = 0U;
    int infer_result = ble_hs_id_infer_auto(0, &own_addr_type);
    if (infer_result)
    {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto failed: rc=%d", infer_result);
        return;
    }

    ble_nus_model_set_own_addr_type(own_addr_type);

    int mtu_result = ble_att_set_preferred_mtu(BLE_NUS_MTU_REQUESTED);
    if (mtu_result)
    {
        ESP_LOGW(TAG, "ble_att_set_preferred_mtu failed: rc=%d", mtu_result);
    }

    int adv_result = ble_nus_hardware_start_advertising(ble_nus_hardware_gap_event_cb);
    if (adv_result)
    {
        return;
    }

    ESP_LOGI(TAG, "BLE advertising started: name=%s interval_ms=%u", ble_nus_model_get_device_name(),
             ble_nus_model_get_adv_interval_ms());
}

static void ble_nus_hardware_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_nus_hardware_start(void)
{
    esp_err_t nvs_result = nvs_flash_init();
    if (ESP_ERR_NVS_NO_FREE_PAGES == nvs_result || ESP_ERR_NVS_NEW_VERSION_FOUND == nvs_result)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_result = nvs_flash_init();
    }
    if (ESP_OK != nvs_result)
    {
        ESP_LOGE(TAG, "nvs_flash_init failed: err=0x%x", nvs_result);
        return nvs_result;
    }

    int nimble_init_result = nimble_port_init();
    if (nimble_init_result)
    {
        ESP_LOGE(TAG, "nimble_port_init failed: rc=%d", nimble_init_result);
        return ESP_FAIL;
    }

    ble_hs_cfg.sync_cb = ble_nus_hardware_on_sync;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    int gap_name_result = ble_svc_gap_device_name_set(ble_nus_model_get_device_name());
    if (gap_name_result)
    {
        ESP_LOGE(TAG, "ble_svc_gap_device_name_set failed: rc=%d", gap_name_result);
        nimble_port_deinit();
        return ESP_FAIL;
    }

    int count_result = ble_gatts_count_cfg(s_gatt_services);
    if (count_result)
    {
        ESP_LOGE(TAG, "ble_gatts_count_cfg failed: rc=%d", count_result);
        nimble_port_deinit();
        return ESP_FAIL;
    }

    int add_result = ble_gatts_add_svcs(s_gatt_services);
    if (add_result)
    {
        ESP_LOGE(TAG, "ble_gatts_add_svcs failed: rc=%d", add_result);
        nimble_port_deinit();
        return ESP_FAIL;
    }

    ble_nus_model_set_tx_value_handle(s_tx_value_handle);
    nimble_port_freertos_init(ble_nus_hardware_host_task);

    return ESP_OK;
}

esp_err_t ble_nus_hardware_stop(void)
{
    ble_nus_model_snapshot_t snapshot;
    if (!ble_nus_model_get_snapshot(&snapshot))
        return ESP_FAIL;

    int adv_stop_result = ble_gap_adv_stop();
    if (adv_stop_result && BLE_HS_EALREADY != adv_stop_result)
    {
        ESP_LOGW(TAG, "ble_gap_adv_stop returned rc=%d", adv_stop_result);
    }

    if (BLE_NUS_DEFAULT_CONN_HANDLE != snapshot.conn_handle)
    {
        int terminate_result = ble_gap_terminate(snapshot.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        if (terminate_result)
        {
            ESP_LOGW(TAG, "ble_gap_terminate returned rc=%d", terminate_result);
        }
    }

    int stop_result = nimble_port_stop();
    if (stop_result)
    {
        ESP_LOGW(TAG, "nimble_port_stop returned rc=%d", stop_result);
    }

    nimble_port_deinit();

    s_tx_value_handle = 0U;
    return ESP_OK;
}

esp_err_t ble_nus_hardware_send(const uint8_t *data, uint16_t len, uint16_t att_overhead)
{
    if (!data || !len)
        return ESP_ERR_INVALID_ARG;

    ble_nus_model_snapshot_t snapshot;
    if (!ble_nus_model_get_snapshot(&snapshot))
        return ESP_FAIL;

    if (!snapshot.connected || !snapshot.notify_enabled || BLE_NUS_DEFAULT_CONN_HANDLE == snapshot.conn_handle)
        return ESP_ERR_INVALID_STATE;

    uint16_t max_payload = snapshot.mtu > att_overhead ? (uint16_t)(snapshot.mtu - att_overhead) : 20U;
    uint16_t offset = 0U;

    while (offset < len)
    {
        uint16_t fragment_len = (uint16_t)(len - offset);
        if (fragment_len > max_payload)
        {
            fragment_len = max_payload;
        }

        struct os_mbuf *packet = ble_hs_mbuf_from_flat(data + offset, fragment_len);
        if (!packet)
            return ESP_ERR_NO_MEM;

        int notify_result = ble_gatts_notify_custom(snapshot.conn_handle, snapshot.tx_value_handle, packet);
        if (notify_result)
            return ESP_FAIL;

        offset = (uint16_t)(offset + fragment_len);
    }

    return ESP_OK;
}
