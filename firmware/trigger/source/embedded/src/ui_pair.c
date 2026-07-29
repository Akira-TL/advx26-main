#include <stdio.h>
#include <string.h>

#include "tal_api.h"
#include "tal_wifi.h"
#include "lvgl.h"
#include "http_host.h"
#include "cJSON.h"

#include "ui_pair.h"
#include "ui_nav.h"

#define PAIR_BG       0x071426
#define PAIR_CARD     0x173852
#define PAIR_WHITE    0xF7FBFF
#define PAIR_MUTED    0xA9C1D7
#define PAIR_CYAN     0x48D8FF
#define PAIR_GREEN    0x48E0A4
#define PAIR_YELLOW   0xFFD166
#define PAIR_RED      0xFF6B7A

#define PAIR_PORT          8787
#define PAIR_NONCE_BYTES   8                 /* 8 random bytes -> 16 hex chars */
#define PAIR_NONCE_TTL_MS  180000U           /* 3 minutes */
#define PAIR_TOKEN_MAX     2048
#define PAIR_HTTP_MAX_REQ  4096

#define PAIR_KV_TOKEN      "sp_cloud_token"
#define PAIR_KV_UID        "sp_cloud_uid"
#define PAIR_KV_EMAIL      "sp_cloud_email"
#define PAIR_EMAIL_MAX     128
#define PAIR_KV_SERVER     "sp_cloud_server"
#define PAIR_SERVER_MAX    128

static lv_obj_t *sg_scr;
static lv_obj_t *sg_qr;
static lv_obj_t *sg_status_label;
static lv_obj_t *sg_ip_label;
static lv_obj_t *sg_countdown_label;
static lv_obj_t *sg_devid_label;
static lv_obj_t *sg_email_label;
static lv_obj_t *sg_logout_btn;

static HTTP_HOST_HANDLE_T sg_http;
static MUTEX_HANDLE sg_mutex;

/* Nonce state — shared between the LVGL thread and the HTTP thread, guarded
 * by sg_mutex. */
static char sg_nonce_hex[PAIR_NONCE_BYTES * 2 + 1];
static SYS_TIME_T sg_nonce_born_ms;
static volatile bool sg_nonce_valid;

/* Cross-thread signals. The HTTP thread sets sg_pair_done; the LVGL timer
 * consumes it. sg_pairing_open gates whether the HTTP endpoint accepts. */
static volatile bool sg_pair_done;
static volatile bool sg_pairing_open;

/* Paired account email — written by the HTTP thread under sg_mutex, read by
 * the LVGL thread to render the account line. */
static char sg_paired_email[PAIR_EMAIL_MAX];

static char sg_payload[96];

static void pair_set_status(uint32_t color, const char *text)
{
    lv_obj_set_style_text_color(sg_status_label, lv_color_hex(color), LV_PART_MAIN);
    lv_label_set_text(sg_status_label, text);
}

static lv_obj_t *pair_create_label(lv_obj_t *parent, const char *text,
                                   const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    /* Decorative label — let background swipes pass through (USER_1 wins over
     * CLICKABLE in the gesture filter). */
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(label, LV_OBJ_FLAG_USER_1);
    return label;
}

/* Render the "already paired" state for the given account email. */
static void pair_apply_paired_ui(const char *email)
{
    char buf[PAIR_EMAIL_MAX + 16];

    pair_set_status(PAIR_GREEN, "Paired successfully");
    snprintf(buf, sizeof(buf), "Account: %s",
             (email != NULL && email[0] != '\0') ? email : "(unknown)");
    lv_label_set_text(sg_email_label, buf);
    lv_obj_clear_flag(sg_logout_btn, LV_OBJ_FLAG_HIDDEN);
}

/* If a token is already persisted, show the paired account; otherwise reset
 * the account widgets to the unpaired state. Runs on the LVGL thread. */
static void pair_load_paired_state(void)
{
    uint8_t *val = NULL;
    size_t len = 0;

    if (tal_kv_get(PAIR_KV_TOKEN, &val, &len) != OPRT_OK || len == 0U) {
        if (val != NULL) {
            tal_kv_free(val);
        }
        sg_paired_email[0] = '\0';
        lv_label_set_text(sg_email_label, "");
        lv_obj_add_flag(sg_logout_btn, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    tal_kv_free(val);
    val = NULL;

    sg_paired_email[0] = '\0';
    if (tal_kv_get(PAIR_KV_EMAIL, &val, &len) == OPRT_OK && val != NULL) {
        size_t n = (len < (PAIR_EMAIL_MAX - 1U)) ? len : (PAIR_EMAIL_MAX - 1U);
        memcpy(sg_paired_email, val, n);
        sg_paired_email[n] = '\0';
        tal_kv_free(val);
    }
    pair_apply_paired_ui(sg_paired_email);
}

/* Generate a fresh nonce under the mutex and copy its hex form to hex_out
 * (buffer must hold 17 bytes). Records the birth time and marks it valid. */
static void pair_gen_nonce(char hex_out[PAIR_NONCE_BYTES * 2 + 1])
{
    uint32_t i;

    tal_mutex_lock(sg_mutex);
    for (i = 0U; i < PAIR_NONCE_BYTES; i++) {
        int r = tal_system_get_random(256);
        snprintf(&sg_nonce_hex[i * 2U], 3U, "%02x", (unsigned)(r & 0xFF));
    }
    sg_nonce_hex[PAIR_NONCE_BYTES * 2U] = '\0';
    sg_nonce_born_ms = tal_system_get_millisecond();
    sg_nonce_valid = true;
    strcpy(hex_out, sg_nonce_hex);
    tal_mutex_unlock(sg_mutex);
}

/* Rebuild the QR payload from the current WiFi state. Runs on the LVGL thread.
 * If WiFi has no IP yet, shows a waiting message and leaves the QR untouched. */
static void pair_refresh_qr(void)
{
    WF_STATION_STAT_E stat = WSS_IDLE;
    NW_IP_S ip;
    NW_MAC_S mac;
    char nonce[PAIR_NONCE_BYTES * 2 + 1];
    char devid[13];

    if (tal_wifi_station_get_status(&stat) != OPRT_OK || stat != WSS_GOT_IP) {
        pair_set_status(PAIR_MUTED, "Waiting for WiFi...");
        lv_label_set_text(sg_ip_label, "IP: --");
        lv_label_set_text(sg_countdown_label, "");
        lv_label_set_text(sg_devid_label, "");
        return;
    }

    memset(&ip, 0, sizeof(ip));
    tal_wifi_get_ip(WF_STATION, &ip);

    strcpy(devid, "000000000000");
    if (tal_wifi_get_mac(WF_STATION, &mac) == OPRT_OK) {
        snprintf(devid, sizeof(devid), "%02X%02X%02X%02X%02X%02X",
                 mac.mac[0], mac.mac[1], mac.mac[2],
                 mac.mac[3], mac.mac[4], mac.mac[5]);
    }

    pair_gen_nonce(nonce);

    snprintf(sg_payload, sizeof(sg_payload), "SNDPOLA1|%s|%u|%s|%s",
             ip.ip, (unsigned)PAIR_PORT, nonce, devid);
    lv_qrcode_update(sg_qr, sg_payload, (uint32_t)strlen(sg_payload));

    pair_set_status(PAIR_CYAN, "Scan with SoundPola app");
    {
        char buf[40];
        snprintf(buf, sizeof(buf), "IP: %s", ip.ip);
        lv_label_set_text(sg_ip_label, buf);
        snprintf(buf, sizeof(buf), "Device: %s", devid);
        lv_label_set_text(sg_devid_label, buf);
    }
    lv_label_set_text(sg_countdown_label, "QR valid: 180s");
}

static void pair_reply(int fd, const char *status, const char *json)
{
    http_host_reply(fd, status, "application/json", json);
}

/* HTTP thread. Must reply synchronously — the client socket closes when this
 * returns. Never touches LVGL here; UI updates are deferred via sg_pair_done. */
static OPERATE_RET pair_on_request(const HTTP_HOST_REQUEST_T *req, void *user_ctx)
{
    cJSON *root;
    cJSON *j_nonce;
    cJSON *j_token;
    cJSON *j_uid;
    cJSON *j_email;
    const char *token;
    const char *uid;
    const char *email;
    size_t token_len;
    bool nonce_ok = false;

    (void)user_ctx;

    if (strcmp(req->path, "/pair") != 0) {
        http_host_reply_not_found(req->client_fd);
        return OPRT_OK;
    }
    if (strcmp(req->method, "POST") != 0) {
        pair_reply(req->client_fd, "405 Method Not Allowed",
                   "{\"ok\":false,\"error\":\"method_not_allowed\"}");
        return OPRT_OK;
    }
    if (!sg_pairing_open) {
        pair_reply(req->client_fd, "403 Forbidden",
                   "{\"ok\":false,\"error\":\"pairing_closed\"}");
        return OPRT_OK;
    }

    root = cJSON_ParseWithLength(req->body, req->body_len);
    if (root == NULL) {
        pair_reply(req->client_fd, "400 Bad Request",
                   "{\"ok\":false,\"error\":\"bad_request\"}");
        return OPRT_OK;
    }

    j_nonce = cJSON_GetObjectItem(root, "nonce");
    j_token = cJSON_GetObjectItem(root, "token");
    j_uid = cJSON_GetObjectItem(root, "user_id");
    if (!cJSON_IsString(j_nonce) || !cJSON_IsString(j_token) ||
        j_nonce->valuestring == NULL || j_token->valuestring == NULL) {
        cJSON_Delete(root);
        pair_reply(req->client_fd, "400 Bad Request",
                   "{\"ok\":false,\"error\":\"bad_request\"}");
        return OPRT_OK;
    }
    token = j_token->valuestring;
    token_len = strlen(token);
    uid = (cJSON_IsString(j_uid) && j_uid->valuestring != NULL)
              ? j_uid->valuestring : "";
    j_email = cJSON_GetObjectItem(root, "email");
    email = (cJSON_IsString(j_email) && j_email->valuestring != NULL)
                ? j_email->valuestring : "";
    if (token_len == 0U || token_len >= PAIR_TOKEN_MAX) {
        cJSON_Delete(root);
        pair_reply(req->client_fd, "400 Bad Request",
                   "{\"ok\":false,\"error\":\"bad_request\"}");
        return OPRT_OK;
    }

    /* Validate the nonce: pairing open, nonce valid, not expired, exact match.
     * Consume it (one-shot) before releasing the lock. */
    tal_mutex_lock(sg_mutex);
    if (sg_nonce_valid &&
        (tal_system_get_millisecond() - sg_nonce_born_ms) < PAIR_NONCE_TTL_MS &&
        strcmp(sg_nonce_hex, j_nonce->valuestring) == 0) {
        sg_nonce_valid = false;
        nonce_ok = true;
    }
    tal_mutex_unlock(sg_mutex);

    if (!nonce_ok) {
        cJSON_Delete(root);
        pair_reply(req->client_fd, "401 Unauthorized",
                   "{\"ok\":false,\"error\":\"nonce_invalid\"}");
        return OPRT_OK;
    }

    tal_kv_set(PAIR_KV_TOKEN, (const uint8_t *)token, token_len);
    tal_kv_set(PAIR_KV_UID, (const uint8_t *)uid, strlen(uid));
    tal_kv_set(PAIR_KV_EMAIL, (const uint8_t *)email, strlen(email));
    {
        cJSON *j_server = cJSON_GetObjectItem(root, "server_url");
        if (cJSON_IsString(j_server) && j_server->valuestring != NULL) {
            size_t slen = strlen(j_server->valuestring);
            if (slen > 0 && slen < PAIR_SERVER_MAX &&
                strncmp(j_server->valuestring, "http://", 7) == 0) {
                tal_kv_set(PAIR_KV_SERVER, (const uint8_t *)j_server->valuestring, slen + 1);
                PR_NOTICE("pair: server_url stored '%s'", j_server->valuestring);
            }
        }
    }
    PR_NOTICE("pair: token stored (%u bytes), uid='%s' email='%s'",
              (unsigned)token_len, uid, email);

    tal_mutex_lock(sg_mutex);
    snprintf(sg_paired_email, sizeof(sg_paired_email), "%s", email);
    tal_mutex_unlock(sg_mutex);

    cJSON_Delete(root);
    sg_pair_done = true;
    pair_reply(req->client_fd, "200 OK", "{\"ok\":true}");
    return OPRT_OK;
}

static void pair_ui_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if (sg_pair_done) {
        char email[PAIR_EMAIL_MAX];

        sg_pair_done = false;
        tal_mutex_lock(sg_mutex);
        snprintf(email, sizeof(email), "%s", sg_paired_email);
        tal_mutex_unlock(sg_mutex);
        pair_apply_paired_ui(email);
        lv_label_set_text(sg_countdown_label, "Token saved to device");
        return;
    }

    if (!sg_pairing_open || lv_screen_active() != sg_scr) {
        return;
    }

    if (!sg_nonce_valid) {
        /* No live nonce: either waiting for WiFi or never generated. Refresh
         * (which polls WiFi status and generates a nonce once an IP exists). */
        pair_refresh_qr();
        return;
    }

    {
        SYS_TIME_T elapsed = tal_system_get_millisecond() - sg_nonce_born_ms;
        char buf[32];

        if (elapsed >= PAIR_NONCE_TTL_MS) {
            pair_refresh_qr(); /* rotate to keep the displayed QR fresh */
        } else {
            snprintf(buf, sizeof(buf), "QR valid: %us",
                     (unsigned)((PAIR_NONCE_TTL_MS - elapsed) / 1000U));
            lv_label_set_text(sg_countdown_label, buf);
        }
    }
}

static void pair_refresh_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    pair_refresh_qr();
}

static void pair_logout_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    tal_kv_del(PAIR_KV_TOKEN);
    tal_kv_del(PAIR_KV_UID);
    tal_kv_del(PAIR_KV_EMAIL);
    tal_kv_del(PAIR_KV_SERVER);

    tal_mutex_lock(sg_mutex);
    sg_paired_email[0] = '\0';
    tal_mutex_unlock(sg_mutex);

    lv_obj_add_flag(sg_logout_btn, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(sg_email_label, "");
    pair_set_status(PAIR_MUTED, "Logged out");
    pair_refresh_qr();
}

static void pair_back_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    ui_nav_go_anim(UI_SCR_HOME, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250);
}

static void pair_loaded_cb(lv_event_t *e)
{
    (void)e;
    sg_pairing_open = true;
    pair_load_paired_state();
    pair_refresh_qr();
}

static void pair_unloaded_cb(lv_event_t *e)
{
    (void)e;
    sg_pairing_open = false;
}

static void pair_style_screen(lv_obj_t *scr)
{
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(PAIR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
}

void ui_pair_init(void)
{
    lv_obj_t *btn;
    lv_obj_t *label;
    HTTP_HOST_CFG_T cfg;

    sg_scr = lv_obj_create(NULL);
    pair_style_screen(sg_scr);

    btn = lv_button_create(sg_scr);
    lv_obj_set_pos(btn, 4, 4);
    lv_obj_set_size(btn, 40, 28);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(PAIR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, pair_back_cb, LV_EVENT_CLICKED, NULL);
    label = pair_create_label(btn, LV_SYMBOL_LEFT, &lv_font_montserrat_14, PAIR_WHITE);
    lv_obj_center(label);

    {
        lv_obj_t *title = pair_create_label(sg_scr, "Pair Device",
                                            &lv_font_montserrat_20, PAIR_WHITE);
        lv_obj_set_pos(title, 48, 8);
    }

    sg_qr = lv_qrcode_create(sg_scr);
    lv_qrcode_set_size(sg_qr, 180);
    lv_qrcode_set_dark_color(sg_qr, lv_color_hex(0x000000));
    lv_qrcode_set_light_color(sg_qr, lv_color_hex(0xFFFFFF));
    lv_obj_set_pos(sg_qr, 24, 56);
    /* Placeholder so the canvas is defined before the first real payload. */
    lv_qrcode_update(sg_qr, "SNDPOLA1", 8);

    sg_status_label = pair_create_label(sg_scr, "Waiting for WiFi...",
                                        &lv_font_montserrat_16, PAIR_MUTED);
    lv_obj_set_pos(sg_status_label, 224, 60);
    lv_obj_set_width(sg_status_label, 240);
    lv_label_set_long_mode(sg_status_label, LV_LABEL_LONG_WRAP);

    sg_ip_label = pair_create_label(sg_scr, "IP: --",
                                    &lv_font_montserrat_14, PAIR_WHITE);
    lv_obj_set_pos(sg_ip_label, 224, 108);

    sg_countdown_label = pair_create_label(sg_scr, "",
                                           &lv_font_montserrat_14, PAIR_YELLOW);
    lv_obj_set_pos(sg_countdown_label, 224, 134);

    sg_devid_label = pair_create_label(sg_scr, "",
                                       &lv_font_montserrat_14, PAIR_MUTED);
    lv_obj_set_pos(sg_devid_label, 224, 160);

    btn = lv_button_create(sg_scr);
    lv_obj_set_pos(btn, 224, 200);
    lv_obj_set_size(btn, 120, 32);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(PAIR_CYAN), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, pair_refresh_cb, LV_EVENT_CLICKED, NULL);
    label = pair_create_label(btn, "REFRESH", &lv_font_montserrat_14, PAIR_BG);
    lv_obj_center(label);

    sg_logout_btn = lv_button_create(sg_scr);
    lv_obj_set_pos(sg_logout_btn, 352, 200);
    lv_obj_set_size(sg_logout_btn, 112, 32);
    lv_obj_set_style_radius(sg_logout_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_logout_btn, lv_color_hex(PAIR_RED), LV_PART_MAIN);
    lv_obj_set_style_border_width(sg_logout_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(sg_logout_btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(sg_logout_btn, pair_logout_cb, LV_EVENT_CLICKED, NULL);
    label = pair_create_label(sg_logout_btn, "LOGOUT", &lv_font_montserrat_14,
                              PAIR_WHITE);
    lv_obj_center(label);
    lv_obj_add_flag(sg_logout_btn, LV_OBJ_FLAG_HIDDEN);

    sg_email_label = pair_create_label(sg_scr, "",
                                       &lv_font_montserrat_14, PAIR_GREEN);
    lv_obj_set_pos(sg_email_label, 224, 244);
    lv_obj_set_width(sg_email_label, 240);
    lv_label_set_long_mode(sg_email_label, LV_LABEL_LONG_DOT);

    {
        lv_obj_t *hint = pair_create_label(
            sg_scr, "App: Account -> Pair NFC device",
            &lv_font_montserrat_14, 0x5D7891);
        lv_obj_set_pos(hint, 0, 296);
        lv_obj_set_width(hint, 480);
        lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }

    sg_nonce_valid = false;
    sg_pair_done = false;
    sg_pairing_open = false;

    tal_mutex_create_init(&sg_mutex);

    lv_obj_add_event_cb(sg_scr, pair_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(sg_scr, pair_unloaded_cb, LV_EVENT_SCREEN_UNLOADED, NULL);

    lv_timer_create(pair_ui_timer_cb, 250, NULL);

    memset(&cfg, 0, sizeof(cfg));
    cfg.port = PAIR_PORT;
    cfg.max_request_size = PAIR_HTTP_MAX_REQ;
    cfg.stack_depth = 1024 * 6;
    cfg.thread_name = "pair_http";
    if (http_host_start(&sg_http, &cfg, pair_on_request, NULL, NULL) != OPRT_OK) {
        PR_ERR("pair http host start failed");
    }

    ui_nav_register(UI_SCR_PAIR, sg_scr);
    ui_nav_attach_gesture(sg_scr);
}
