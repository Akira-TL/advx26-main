#ifndef BOARD_LINK_H
#define BOARD_LINK_H

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "c" {
#endif

#define BOARD_LINK_LINE_MAX 2200
#define BOARD_LINK_RING_DEPTH 8

OPERATE_RET board_link_init(void);
OPERATE_RET board_link_send(const char *data, uint16_t len);
BOOL_T board_link_poll(char *buf, uint16_t *len);

#ifdef __cplusplus
}
#endif

#endif
