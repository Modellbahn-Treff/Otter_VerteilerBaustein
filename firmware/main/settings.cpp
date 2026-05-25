#include "settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "settings";
static const char *NVS_NS = "settings";

// ---------------------------------------------------------------------------
// Default values (compiled-in fallbacks)
// ---------------------------------------------------------------------------

uint8_t AbschNummer       = 1;
uint8_t VerteilerBaustein = 1;

char ssid[SETTINGS_STR_LEN]        = "SSID";
char password[SETTINGS_STR_LEN]    = "PASSWORD";
char mqtt_server[SETTINGS_STR_LEN] = "10.1.0.5";
char client_name[SETTINGS_STR_LEN] = "test_client";

uint8_t networkByte1 = 10;
uint8_t networkByte2 = 1;
uint8_t gatewayByte3 = 0;
uint8_t gatewayByte4 = 1;

int AusSchaltZeitWeiche = 50;

bool TM_active[5] = {false, false, false, false, true};
bool SM_active[5] = {true, true, false, false, false};
bool WM_active[5] = {false, false, true, true, false};

char MqttTMT[20][SETTINGS_TOPIC_LEN] = {
    "otter/TM/0",  "otter/TM/1",  "otter/TM/2",  "otter/TM/3",
    "otter/TM/4",  "otter/TM/5",  "otter/TM/6",  "otter/TM/7",
    "otter/TM/8",  "otter/TM/9",  "otter/TM/10", "otter/TM/11",
    "otter/TM/12", "otter/TM/13", "otter/TM/14", "otter/TM/15",
    "otter/TM/16", "otter/TM/17", "otter/TM/18", "otter/TM/19",
};

char MqttSMS[8][SETTINGS_TOPIC_LEN] = {
    "otter/SM/0", "otter/SM/1", "otter/SM/2", "otter/SM/3",
    "otter/SM/4", "otter/SM/5", "otter/SM/6", "otter/SM/7",
};

char MqttWMW[8][SETTINGS_TOPIC_LEN] = {
    "otter/WM/0", "otter/WM/1", "otter/WM/2", "otter/WM/3",
    "otter/WM/4", "otter/WM/5", "otter/WM/6", "otter/WM/7",
};

// ---------------------------------------------------------------------------
// Helpers: serialize/deserialize bool array to/from JSON string
// ---------------------------------------------------------------------------

static void bool_array_to_json_str(const bool *arr, int n, char *out, size_t out_len) {
    cJSON *a = cJSON_CreateArray();
    for (int i = 0; i < n; i++) {
        cJSON_AddItemToArray(a, cJSON_CreateBool(arr[i]));
    }
    char *s = cJSON_PrintUnformatted(a);
    snprintf(out, out_len, "%s", s);
    cJSON_free(s);
    cJSON_Delete(a);
}

static void json_str_to_bool_array(const char *json_str, bool *arr, int n) {
    cJSON *a = cJSON_Parse(json_str);
    if (!a || !cJSON_IsArray(a)) { cJSON_Delete(a); return; }
    int cnt = cJSON_GetArraySize(a);
    for (int i = 0; i < n && i < cnt; i++) {
        cJSON *item = cJSON_GetArrayItem(a, i);
        if (cJSON_IsBool(item)) arr[i] = cJSON_IsTrue(item);
    }
    cJSON_Delete(a);
}

static void str_array_to_json_str(char arr[][SETTINGS_TOPIC_LEN], int n, char *out, size_t out_len) {
    cJSON *a = cJSON_CreateArray();
    for (int i = 0; i < n; i++) {
        cJSON_AddItemToArray(a, cJSON_CreateString(arr[i]));
    }
    char *s = cJSON_PrintUnformatted(a);
    snprintf(out, out_len, "%s", s);
    cJSON_free(s);
    cJSON_Delete(a);
}

static void json_str_to_str_array(const char *json_str, char arr[][SETTINGS_TOPIC_LEN], int n) {
    cJSON *a = cJSON_Parse(json_str);
    if (!a || !cJSON_IsArray(a)) { cJSON_Delete(a); return; }
    int cnt = cJSON_GetArraySize(a);
    for (int i = 0; i < n && i < cnt; i++) {
        cJSON *item = cJSON_GetArrayItem(a, i);
        if (cJSON_IsString(item)) {
            snprintf(arr[i], SETTINGS_TOPIC_LEN, "%s", item->valuestring);
        }
    }
    cJSON_Delete(a);
}

// ---------------------------------------------------------------------------
// NVS load — falls back to compiled defaults for any missing key
// ---------------------------------------------------------------------------

void settings_load_from_nvs(void) {
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READONLY, &h);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No saved settings, using defaults");
        return;
    }

    // uint8 scalars
    nvs_get_u8(h, "absch_nr", &AbschNummer);
    nvs_get_u8(h, "vb",       &VerteilerBaustein);
    nvs_get_u8(h, "net_b1",   &networkByte1);
    nvs_get_u8(h, "net_b2",   &networkByte2);
    nvs_get_u8(h, "gw_b3",    &gatewayByte3);
    nvs_get_u8(h, "gw_b4",    &gatewayByte4);

    // int32 scalar
    int32_t weiche = AusSchaltZeitWeiche;
    if (nvs_get_i32(h, "weiche_ms", &weiche) == ESP_OK) {
        AusSchaltZeitWeiche = (int)weiche;
    }

    // strings
    size_t len;
    len = SETTINGS_STR_LEN; nvs_get_str(h, "ssid",      ssid,        &len);
    len = SETTINGS_STR_LEN; nvs_get_str(h, "password",  password,    &len);
    len = SETTINGS_STR_LEN; nvs_get_str(h, "mqtt_srv",  mqtt_server, &len);
    len = SETTINGS_STR_LEN; nvs_get_str(h, "client_nm", client_name, &len);

    // bool arrays stored as JSON strings — use a temporary buffer
    char buf[512];
    len = sizeof(buf); if (nvs_get_str(h, "tm_active", buf, &len) == ESP_OK) json_str_to_bool_array(buf, TM_active, 5);
    len = sizeof(buf); if (nvs_get_str(h, "sm_active", buf, &len) == ESP_OK) json_str_to_bool_array(buf, SM_active, 5);
    len = sizeof(buf); if (nvs_get_str(h, "wm_active", buf, &len) == ESP_OK) json_str_to_bool_array(buf, WM_active, 5);

    // topic arrays stored as JSON strings
    char big[1600];
    size_t blen;
    blen = sizeof(big); if (nvs_get_str(h, "mqtt_tmt", big, &blen) == ESP_OK) json_str_to_str_array(big, MqttTMT, 20);
    blen = sizeof(big); if (nvs_get_str(h, "mqtt_sms", big, &blen) == ESP_OK) json_str_to_str_array(big, MqttSMS, 8);
    blen = sizeof(big); if (nvs_get_str(h, "mqtt_wmw", big, &blen) == ESP_OK) json_str_to_str_array(big, MqttWMW, 8);

    nvs_close(h);
    ESP_LOGI(TAG, "Settings loaded from NVS");
}

// ---------------------------------------------------------------------------
// NVS save — all non-WiFi settings
// ---------------------------------------------------------------------------

void settings_save_to_nvs(void) {
    nvs_handle_t h;
    ESP_ERROR_CHECK(nvs_open(NVS_NS, NVS_READWRITE, &h));

    nvs_set_u8(h, "absch_nr", AbschNummer);
    nvs_set_u8(h, "vb",       VerteilerBaustein);
    nvs_set_u8(h, "net_b1",   networkByte1);
    nvs_set_u8(h, "net_b2",   networkByte2);
    nvs_set_u8(h, "gw_b3",    gatewayByte3);
    nvs_set_u8(h, "gw_b4",    gatewayByte4);
    nvs_set_i32(h, "weiche_ms", (int32_t)AusSchaltZeitWeiche);
    nvs_set_str(h, "mqtt_srv",  mqtt_server);
    nvs_set_str(h, "client_nm", client_name);

    char buf[512];
    bool_array_to_json_str(TM_active, 5, buf, sizeof(buf)); nvs_set_str(h, "tm_active", buf);
    bool_array_to_json_str(SM_active, 5, buf, sizeof(buf)); nvs_set_str(h, "sm_active", buf);
    bool_array_to_json_str(WM_active, 5, buf, sizeof(buf)); nvs_set_str(h, "wm_active", buf);

    char big[1600];
    str_array_to_json_str(MqttTMT, 20, big, sizeof(big)); nvs_set_str(h, "mqtt_tmt", big);
    str_array_to_json_str(MqttSMS,  8, big, sizeof(big)); nvs_set_str(h, "mqtt_sms", big);
    str_array_to_json_str(MqttWMW,  8, big, sizeof(big)); nvs_set_str(h, "mqtt_wmw", big);

    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "Settings saved to NVS");
}

// ---------------------------------------------------------------------------
// NVS save — WiFi credentials only
// ---------------------------------------------------------------------------

void settings_save_wifi_to_nvs(void) {
    nvs_handle_t h;
    ESP_ERROR_CHECK(nvs_open(NVS_NS, NVS_READWRITE, &h));
    nvs_set_str(h, "ssid",     ssid);
    nvs_set_str(h, "password", password);
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "WiFi credentials saved to NVS");
}
