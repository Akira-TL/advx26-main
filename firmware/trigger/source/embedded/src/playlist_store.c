#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tal_api.h"
#include "http_client_interface.h"
#include "cJSON.h"

#include "playlist_store.h"
#include "tcp_link.h"

#define PLIST_HTTP_STACK    (1024 * 12)
#define PLIST_HTTP_TIMEOUT  10000

#define PLIST_KV_TOKEN      "sp_cloud_token"
#define PLIST_KV_SERVER     "sp_cloud_server"
#define PLIST_TOKEN_MAX     2048
#define PLIST_SERVER_MAX    128

static plist_item_t sg_items[PLIST_MAX_ITEMS];
static int sg_item_count;
static char sg_server[PLIST_SERVER_MAX];

static volatile bool sg_fetch_requested;
static volatile plist_store_status_t sg_status = PLIST_STORE_IDLE;
static volatile uint32_t sg_generation;

static THREAD_HANDLE sg_thread;
static bool sg_thread_started;

static void plist_parse_url(const char *url, char *host_out, size_t host_max,
                            uint16_t *port_out)
{
    const char *p = url;
    const char *host_start;
    const char *host_end;
    size_t host_len;

    *port_out = 80;

    if (strncmp(p, "http://", 7) == 0) {
        p += 7;
    }
    host_start = p;

    host_end = strchr(p, ':');
    if (host_end != NULL) {
        *port_out = (uint16_t)atoi(host_end + 1);
    } else {
        host_end = strchr(p, '/');
        if (host_end == NULL) {
            host_end = p + strlen(p);
        }
    }

    host_len = (size_t)(host_end - host_start);
    if (host_len >= host_max) {
        host_len = host_max - 1;
    }
    memcpy(host_out, host_start, host_len);
    host_out[host_len] = '\0';
}

static uint8_t plist_parse_state(const char *s)
{
    if (s == NULL) return PLIST_ITEM_UNKNOWN;
    if (strcmp(s, "READY") == 0) return PLIST_ITEM_READY;
    if (strcmp(s, "PROCESSING") == 0 || strcmp(s, "UPLOADED") == 0) return PLIST_ITEM_PROCESSING;
    if (strcmp(s, "FAILED") == 0) return PLIST_ITEM_FAILED;
    return PLIST_ITEM_UNKNOWN;
}

static void plist_finish(plist_store_status_t status)
{
    sg_status = status;
    sg_fetch_requested = false;
    sg_generation++;
}

static void plist_task(void *arg)
{
    /* ~4.3 KB of buffers; keep them off the 12 KB thread stack. Only this
     * single worker thread ever touches them. */
    static char token[PLIST_TOKEN_MAX];
    static char auth_hdr[PLIST_TOKEN_MAX + 16];

    (void)arg;

    for (;;) {
        if (!sg_fetch_requested) {
            tal_system_sleep(200);
            continue;
        }

        char server[PLIST_SERVER_MAX];
        uint8_t *kv_val = NULL;
        size_t kv_len = 0;

        sg_item_count = 0;

        if (tal_kv_get(PLIST_KV_SERVER, &kv_val, &kv_len) != OPRT_OK ||
            kv_val == NULL || kv_len == 0) {
            plist_finish(PLIST_STORE_NOT_PAIRED);
            continue;
        }
        snprintf(server, sizeof(server), "%s", (const char *)kv_val);
        tal_kv_free(kv_val);
        kv_val = NULL;
        snprintf(sg_server, sizeof(sg_server), "%s", server);

        if (tal_kv_get(PLIST_KV_TOKEN, &kv_val, &kv_len) != OPRT_OK ||
            kv_val == NULL || kv_len == 0) {
            plist_finish(PLIST_STORE_NOT_PAIRED);
            continue;
        }
        if (kv_len >= sizeof(token)) {
            kv_len = sizeof(token) - 1;
        }
        memcpy(token, kv_val, kv_len);
        token[kv_len] = '\0';
        tal_kv_free(kv_val);
        kv_val = NULL;

        char host[64];
        uint16_t port;
        plist_parse_url(server, host, sizeof(host), &port);

        snprintf(auth_hdr, sizeof(auth_hdr), "Bearer %s", token);

        http_client_header_t headers[] = {
            {.key = "Authorization", .value = auth_hdr},
            {.key = "Accept", .value = "application/json"},
        };

        http_client_response_t http_response = {0};
        http_client_status_t st = http_client_request(
            &(const http_client_request_t){
                .host = host,
                .port = port,
                .path = "/api/v1/contents",
                .method = "GET",
                .headers = headers,
                .headers_count = 2,
                .body = (const uint8_t *)"",
                .body_length = 0,
                .timeout_ms = PLIST_HTTP_TIMEOUT,
            },
            &http_response);

        if (st != HTTP_CLIENT_SUCCESS) {
            PR_ERR("plist: http request failed st=%d", st);
            http_client_free(&http_response);
            plist_finish(PLIST_STORE_NET_ERROR);
            continue;
        }

        if (http_response.status_code == 401) {
            http_client_free(&http_response);
            plist_finish(PLIST_STORE_TOKEN_EXPIRED);
            continue;
        }

        if (http_response.status_code != 200 || http_response.body == NULL) {
            PR_ERR("plist: http status %u", http_response.status_code);
            http_client_free(&http_response);
            plist_finish(PLIST_STORE_NET_ERROR);
            continue;
        }

        cJSON *root = cJSON_ParseWithLength((const char *)http_response.body,
                                            http_response.body_length);
        http_client_free(&http_response);

        if (root == NULL) {
            plist_finish(PLIST_STORE_NET_ERROR);
            continue;
        }

        cJSON *items = cJSON_GetObjectItem(root, "items");
        if (cJSON_IsArray(items)) {
            int count = cJSON_GetArraySize(items);
            int i;
            if (count > PLIST_MAX_ITEMS) count = PLIST_MAX_ITEMS;
            for (i = 0; i < count; i++) {
                cJSON *item = cJSON_GetArrayItem(items, i);
                cJSON *j_label = cJSON_GetObjectItem(item, "display_label");
                cJSON *j_id = cJSON_GetObjectItem(item, "content_id");
                cJSON *j_dur = cJSON_GetObjectItem(item, "duration_ms");
                cJSON *j_state = cJSON_GetObjectItem(item, "state");
                cJSON *j_src = cJSON_GetObjectItem(item, "source");

                plist_item_t *pi = &sg_items[i];
                memset(pi, 0, sizeof(*pi));

                if (cJSON_IsString(j_label) && j_label->valuestring != NULL &&
                    j_label->valuestring[0] != '\0') {
                    snprintf(pi->label, sizeof(pi->label), "%s", j_label->valuestring);
                } else if (cJSON_IsObject(j_src)) {
                    cJSON *j_fn = cJSON_GetObjectItem(j_src, "filename");
                    if (cJSON_IsString(j_fn) && j_fn->valuestring != NULL) {
                        snprintf(pi->label, sizeof(pi->label), "%s", j_fn->valuestring);
                    } else {
                        snprintf(pi->label, sizeof(pi->label), "Sound %d", i + 1);
                    }
                } else {
                    snprintf(pi->label, sizeof(pi->label), "Sound %d", i + 1);
                }

                if (cJSON_IsString(j_id) && j_id->valuestring != NULL) {
                    snprintf(pi->content_id, sizeof(pi->content_id), "%s",
                             j_id->valuestring);
                }

                if (cJSON_IsNumber(j_dur)) {
                    pi->duration_ms = (uint32_t)j_dur->valuedouble;
                }

                pi->state = plist_parse_state(
                    (cJSON_IsString(j_state) && j_state->valuestring != NULL)
                        ? j_state->valuestring : NULL);
            }
            sg_item_count = count;
        }

        cJSON_Delete(root);
        PR_NOTICE("plist: fetched %d items", sg_item_count);
        plist_finish(PLIST_STORE_READY);
    }
}

void playlist_store_fetch(void)
{
    if (sg_fetch_requested) {
        return;
    }

    if (!sg_thread_started) {
        THREAD_CFG_T cfg = {0};
        cfg.stackDepth = PLIST_HTTP_STACK;
        cfg.priority = THREAD_PRIO_2;
        cfg.thrdname = "plist_http";
        tal_thread_create_and_start(&sg_thread, NULL, NULL, plist_task, NULL, &cfg);
        sg_thread_started = true;
    }

    sg_status = PLIST_STORE_LOADING;
    sg_fetch_requested = true;
}

plist_store_status_t playlist_store_status(void)
{
    return sg_status;
}

int playlist_store_count(void)
{
    return sg_item_count;
}

const plist_item_t *playlist_store_get(int idx)
{
    if (idx < 0 || idx >= sg_item_count) {
        return NULL;
    }
    return &sg_items[idx];
}

uint32_t playlist_store_generation(void)
{
    return sg_generation;
}

const char *playlist_store_server(void)
{
    return sg_server;
}

bool playlist_send_load(int idx)
{
    const plist_item_t *pi = playlist_store_get(idx);
    char msg[512];
    int len;

    if (pi == NULL || pi->content_id[0] == '\0' || sg_server[0] == '\0') {
        return false;
    }
    if (tcp_link_get_state() != TCP_LINK_CONNECTED) {
        return false;
    }

    len = snprintf(msg, sizeof(msg),
                   "{\"type\":\"load\","
                   "\"content_id\":\"%s\","
                   "\"mp3_url\":\"%s/api/v1/contents/%s/assets/audio\","
                   "\"mp4_url\":\"%s/api/v1/contents/%s/assets/video\"}\n",
                   pi->content_id,
                   sg_server, pi->content_id, sg_server, pi->content_id);
    if (len <= 0 || len >= (int)sizeof(msg)) {
        return false;
    }
    return tcp_link_send(msg, (uint16_t)len) == OPRT_OK;
}

int playlist_store_find_by_id(const char *content_id)
{
    int i;

    if (content_id == NULL || content_id[0] == '\0') {
        return -1;
    }
    for (i = 0; i < sg_item_count; i++) {
        if (strcmp(sg_items[i].content_id, content_id) == 0) {
            return i;
        }
    }
    return -1;
}
