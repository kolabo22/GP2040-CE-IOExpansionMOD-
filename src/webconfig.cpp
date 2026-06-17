#include "config.pb.h"
#include "base64.h"
#include "hardware/adc.h"
#include "helper.h"

#include "drivermanager.h"
#include "storagemanager.h"
#include "eventmanager.h"
#include "layoutmanager.h"
#include "peripheralmanager.h"
#include "animationstorage.h"
#include "system.h"
#include "config_utils.h"
#include "types.h"
#include "version.h"

#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <set>

#include <pico/types.h>

// for hall-effect calibration
#include "hardware/adc.h"

// HTTPD Includes
#include <ArduinoJson.h>
#include "rndis.h"
#include "fs.h"
#include "fscustom.h"
#include "fsdata.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "addons/input_macro.h"

#include "tusb.h"
#include "hardware/watchdog.h"
#include "pico/time.h"

#define PATH_CGI_ACTION "/cgi/action"

#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN (1024 * 16)

extern struct fsdata_file file__index_html[];

const static char* spaPaths[] = { "/backup", "/display-config", "/led-config", "/pin-mapping", "/settings", "/reset-settings", "/add-ons", "/custom-theme", "/macro", "/peripheral-mapping" };
const static char* excludePaths[] = { "/css", "/images", "/js", "/static" };
const static uint32_t rebootDelayMs = 500;
static string http_post_uri;
static char http_post_payload[LWIP_HTTPD_POST_MAX_PAYLOAD_LEN];
static uint16_t http_post_payload_len = 0;

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K>
static void __attribute__((noinline)) readDoc(T& var, const DynamicJsonDocument& doc, const K& key)
{
    var = doc[key];
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1>
static void __attribute__((noinline)) readDoc(T& var, const DynamicJsonDocument& doc, const K0& key0, const K1& key1)
{
    var = doc[key0][key1];
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1, typename K2>
static void __attribute__((noinline)) readDoc(T& var, const DynamicJsonDocument& doc, const K0& key0, const K1& key1, const K2& key2)
{
    var = doc[key0][key1][key2];
}

// Don't inline this function, we do not want to consume stack space in the calling function
static bool __attribute__((noinline)) hasValue(const DynamicJsonDocument& doc, const char* key0, const char* key1)
{
    return doc[key0][key1] != nullptr;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T>
static void __attribute__((noinline)) docToValue(T& value, const DynamicJsonDocument& doc, const char* key)
{
    if (doc[key] != nullptr)
    {
        value = doc[key];
    }
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T>
static void __attribute__((noinline)) docToValue(T& value, const DynamicJsonDocument& doc, const char* key0, const char* key1)
{
    if (doc[key0][key1] != nullptr)
    {
        value = doc[key0][key1];
    }
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T>
static void __attribute__((noinline)) docToValue(T& value, const DynamicJsonDocument& doc, const char* key0, const char* key1, const char* key2)
{
    if (doc[key0][key1][key2] != nullptr)
    {
        value = doc[key0][key1][key2];
    }
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) cleanAddonGpioMappings(Pin_t& addonPin, Pin_t oldAddonPin)
{
    GpioMappingInfo* gpioMappings = Storage::getInstance().getGpioMappings().pins;
    ProfileOptions& profiles = Storage::getInstance().getProfileOptions();

    // if the new addon pin value is valid, mark it assigned in GpioMappings
    if (isValidPin(addonPin))
    {
        gpioMappings[addonPin].action = GpioAction::ASSIGNED_TO_ADDON;
        profiles.gpioMappingsSets[0].pins[addonPin].action = GpioAction::ASSIGNED_TO_ADDON;
        profiles.gpioMappingsSets[1].pins[addonPin].action = GpioAction::ASSIGNED_TO_ADDON;
        profiles.gpioMappingsSets[2].pins[addonPin].action = GpioAction::ASSIGNED_TO_ADDON;
    } else {
        // -1 is our de facto value for "not assigned" in addons
        addonPin = -1;
    }

    // either way now, the addon's pin config is set to its real value, if the
    // old value is a real pin (and different), we should unset it
    if (isValidPin(oldAddonPin) && oldAddonPin != addonPin)
    {
        gpioMappings[oldAddonPin].action = GpioAction::NONE;
        profiles.gpioMappingsSets[0].pins[oldAddonPin].action = GpioAction::NONE;
        profiles.gpioMappingsSets[1].pins[oldAddonPin].action = GpioAction::NONE;
        profiles.gpioMappingsSets[2].pins[oldAddonPin].action = GpioAction::NONE;
    }
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) docToPin(Pin_t& pin, const DynamicJsonDocument& doc, const char* key)
{
    Pin_t oldPin = pin;
    if (doc.containsKey(key))
    {
        pin = doc[key];
        cleanAddonGpioMappings(pin, oldPin);
    }
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) docToPin(Pin_t& pin, const DynamicJsonDocument& doc, const char* key0, const char* key1)
{
    Pin_t oldPin = pin;
    if (doc.containsKey(key0) && doc[key0].containsKey(key1))
    {
        pin = doc[key0][key1];
        cleanAddonGpioMappings(pin, oldPin);
    }
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) docToPin(Pin_t& pin, const DynamicJsonDocument& doc, const char* key0, const char* key1, const char* key2)
{
    Pin_t oldPin = pin;
    if (doc.containsKey(key0) && doc[key0].containsKey(key1) && doc[key0][key1].containsKey(key2))
    {
        pin = doc[key0][key1][key2];
        cleanAddonGpioMappings(pin, oldPin);
    }
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K& key, const T& var)
{
    doc[key] = var;
}

// Don't inline this function, we do not want to consume stack space in the calling function
// Web-config frontend compatibility workaround
template <typename K>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K& key, const bool& var)
{
    doc[key] = var ? 1 : 0;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K0& key0, const K1& key1, const T& var)
{
    doc[key0][key1] = var;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1, typename K2>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K0& key0, const K1& key1, const K2& key2, const T& var)
{
    doc[key0][key1][key2] = var;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1, typename K2, typename K3>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K0& key0, const K1& key1, const K2& key2, const K3& key3, const T& var)
{
    doc[key0][key1][key2][key3] = var;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1, typename K2, typename K3, typename K4>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K0& key0, const K1& key1, const K2& key2, const K3& key3, const K4& key4, const T& var)
{
    doc[key0][key1][key2][key3][key4] = var;
}

static int32_t cleanPin(int32_t pin) { return isValidPin(pin) ? pin : -1; }

enum class HttpStatusCode
{
    _200,
    _400,
    _500,
};

struct DataAndStatusCode
{
    DataAndStatusCode(string&& data, HttpStatusCode statusCode) :
        data(std::move(data)),
        statusCode(statusCode)
    {}

    string data;
    HttpStatusCode statusCode;
};

// **** WEB SERVER Overrides and Special Functionality ****
int set_file_data(fs_file* file, const DataAndStatusCode& dataAndStatusCode)
{
    std::string* returnData = new std::string();

    const char* statusCodeStr = "";
    switch (dataAndStatusCode.statusCode)
    {
        case HttpStatusCode::_200: statusCodeStr = "200 OK"; break;
        case HttpStatusCode::_400: statusCodeStr = "400 Bad Request"; break;
        case HttpStatusCode::_500: statusCodeStr = "500 Internal Server Error"; break;
    }

    returnData->clear();
    returnData->append("HTTP/1.0 ");
    returnData->append(statusCodeStr);
    returnData->append("\r\n");
    returnData->append(
        "Server: GP2040-CE " GP2040VERSION "\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: "
    );

    returnData->append(std::to_string(dataAndStatusCode.data.length()));
    returnData->append("\r\n\r\n");
    returnData->append(dataAndStatusCode.data);
    
    file->data = returnData->c_str();
    file->len = returnData->size();
    file->index = file->len;
    file->http_header_included = true;
    file->pextension = returnData;  // store for cleanup
    file->is_custom_file = 1;

    return 1;
}

int set_file_data(fs_file *file, string&& data)
{
    if (data.empty())
        return 0;
    return set_file_data(file, DataAndStatusCode(std::move(data), HttpStatusCode::_200));
}

DynamicJsonDocument get_post_data()
{
    DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
    deserializeJson(doc, http_post_payload, http_post_payload_len);
    return doc;
}

void save_hotkey(HotkeyEntry* hotkey, const DynamicJsonDocument& doc, const string hotkey_key)
{
    readDoc(hotkey->auxMask, doc, hotkey_key, "auxMask");
    uint32_t buttonsMask = doc[hotkey_key]["buttonsMask"];
    uint32_t dpadMask = 0;
    if (buttonsMask & GAMEPAD_MASK_DU) {
        dpadMask |= GAMEPAD_MASK_UP;
    }
    if (buttonsMask & GAMEPAD_MASK_DD) {
        dpadMask |= GAMEPAD_MASK_DOWN;
    }
    if (buttonsMask & GAMEPAD_MASK_DL) {
        dpadMask |= GAMEPAD_MASK_LEFT;
    }
    if (buttonsMask & GAMEPAD_MASK_DR) {
        dpadMask |= GAMEPAD_MASK_RIGHT;
    }
    buttonsMask &= ~(GAMEPAD_MASK_DU | GAMEPAD_MASK_DD | GAMEPAD_MASK_DL | GAMEPAD_MASK_DR);
    hotkey->dpadMask = dpadMask;
    hotkey->buttonsMask = buttonsMask;
    readDoc(hotkey->action, doc, hotkey_key, "action");
}

void load_hotkey(const HotkeyEntry* hotkey, DynamicJsonDocument& doc, const string hotkey_key)
{
    writeDoc(doc, hotkey_key, "auxMask", hotkey->auxMask);
    uint32_t buttonsMask = hotkey->buttonsMask;
    if (hotkey->dpadMask & GAMEPAD_MASK_UP) {
        buttonsMask |= GAMEPAD_MASK_DU;
    }
    if (hotkey->dpadMask & GAMEPAD_MASK_DOWN) {
        buttonsMask |= GAMEPAD_MASK_DD;
    }
    if (hotkey->dpadMask & GAMEPAD_MASK_LEFT) {
        buttonsMask |= GAMEPAD_MASK_DL;
    }
    if (hotkey->dpadMask & GAMEPAD_MASK_RIGHT) {
        buttonsMask |= GAMEPAD_MASK_DR;
    }
    writeDoc(doc, hotkey_key, "buttonsMask", buttonsMask);
    writeDoc(doc, hotkey_key, "action", hotkey->action);
}

// LWIP callback on HTTP POST to validate the URI

// ==============================================================================
// 🛠️ LWIP POST 開始コールバック (URI判定の水際でフック)
// ==============================================================================
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       uint16_t http_request_len, int content_len, char *response_uri,
                       uint16_t response_uri_len, uint8_t *post_auto_wnd)
{
    LWIP_UNUSED_ARG(http_request);
    LWIP_UNUSED_ARG(http_request_len);
    LWIP_UNUSED_ARG(content_len);
    LWIP_UNUSED_ARG(response_uri);
    LWIP_UNUSED_ARG(response_uri_len);
    LWIP_UNUSED_ARG(post_auto_wnd);

    if (!uri || strncmp(uri, "/api", 4) != 0) {
        return ERR_ARG;
    }

    // 🎯 バックアップAPI：4MB目へRAW転送してセーブ、ブラウザには通常終了を返す
    if (strcmp(uri, "/api/backup") == 0) {
        Storage::getInstance().save(true);
        strncpy(response_uri, "/reboot.html", response_uri_len);
        response_uri[response_uri_len - 1] = '\0';
        return ERR_OK;
    }

    // 🎯 リストアまたは初期化API：実機内完結で読み込み・同期
    if (strcmp(uri, "/api/restore") == 0 || strcmp(uri, "/api/resetSettings") == 0) {
        Storage::getInstance().ResetSettings();
        strncpy(response_uri, "/reboot.html", response_uri_len);
        response_uri[response_uri_len - 1] = '\0';
        return ERR_OK;
    }

    // それ以外の通常APIはバニラ通り
    http_post_uri = uri;
    http_post_payload_len = 0;
    memset(http_post_payload, 0, LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
    return ERR_OK;
}

// ==============================================================================
// 🛠️ LWIP POST データ受信コールバック (巨大パケットを読まずに即廃棄)
// ==============================================================================
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
    LWIP_UNUSED_ARG(connection);
    
    // 🎯 通信パケットの中身を読まずにその場で即時解放（バッファ溢れを完全に防ぐ）
    if (http_post_uri == "/api/backup" || http_post_uri == "/api/restore" || http_post_uri == "/api/resetSettings") {
        pbuf_free(p); 
        return ERR_OK;
    }

    // 通常のAPI（設定保存等）は従来通り
    while (p != NULL)
    {
        if (http_post_payload_len + p->len <= LWIP_HTTPD_POST_MAX_PAYLOAD_LEN)
        {
            MEMCPY(http_post_payload + http_post_payload_len, p->payload, p->len);
            http_post_payload_len += p->len;
        }
        else 
        {
            http_post_payload_len = 0xffff;
            break;
        }
        p = p->next;
    }
    pbuf_free(p);

    if (http_post_payload_len == 0xffff) {
        return ERR_BUF;
    }
    return ERR_OK;
}

// ==============================================================================
// 🛠️ LWIP POST 完了コールバック (重複なし・完全一本化)
// ==============================================================================
void httpd_post_finished(void *connection, char *response_uri, uint16_t response_uri_len)
{
    LWIP_UNUSED_ARG(connection);
    
    if (http_post_uri == "/api/backup" || http_post_uri == "/api/restore" || http_post_uri == "/api/resetSettings") {
        strncpy(response_uri, "/reboot.html", response_uri_len);
        response_uri[response_uri_len - 1] = '\0';
        return;
    }

    if (http_post_payload_len != 0xffff) {
        strncpy(response_uri, http_post_uri.c_str(), response_uri_len);
        response_uri[response_uri_len - 1] = '\0';
    }
}

