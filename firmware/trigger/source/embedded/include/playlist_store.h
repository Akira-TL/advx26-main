#ifndef PLAYLIST_STORE_H
#define PLAYLIST_STORE_H

#include <stdint.h>
#include <stdbool.h>

#define PLIST_MAX_ITEMS     32
#define PLIST_LABEL_MAX     48
#define PLIST_ID_MAX        33

/* item state */
#define PLIST_ITEM_UNKNOWN     0
#define PLIST_ITEM_READY       1
#define PLIST_ITEM_PROCESSING  2
#define PLIST_ITEM_FAILED      3

typedef struct {
    char label[PLIST_LABEL_MAX];
    char content_id[PLIST_ID_MAX];
    uint32_t duration_ms;
    uint8_t state;
} plist_item_t;

typedef enum {
    PLIST_STORE_IDLE = 0,
    PLIST_STORE_LOADING,
    PLIST_STORE_READY,
    PLIST_STORE_NOT_PAIRED,
    PLIST_STORE_TOKEN_EXPIRED,
    PLIST_STORE_NET_ERROR,
} plist_store_status_t;

/* Trigger an async refresh from the cloud (no-op while already loading). */
void playlist_store_fetch(void);

plist_store_status_t playlist_store_status(void);

int playlist_store_count(void);

const plist_item_t *playlist_store_get(int idx);

/* Incremented after every completed fetch; UIs poll this to refresh. */
uint32_t playlist_store_generation(void);

/* Cloud server base URL ("http://host:port") from the last fetch, or "". */
const char *playlist_store_server(void);

/*
 * Send a {"type":"load",...} NDJSON command for item idx to the playback
 * board over tcp_link. Returns false when not connected or item invalid.
 */
bool playlist_send_load(int idx);

/* Find item index by 32-hex content id. Returns -1 when not present. */
int playlist_store_find_by_id(const char *content_id);

#endif /* PLAYLIST_STORE_H */
