#ifndef FELICA_H
#define FELICA_H

#include "pn532_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FELICA_BLOCK_LEN 16

#define FELICA_SC_SUICA   0x0003U
#define FELICA_SC_OCTOPUS 0x8008U
#define FELICA_SC_SZT     0x8005U

#define FELICA_SVC_SUICA_HISTORY 0x090FU
#define FELICA_SVC_OCTOPUS       0x0117U
#define FELICA_SVC_SZT           0x0118U

/* Request System Code (0x0C). systems[] is host-endian; *count in/out. */
OPERATE_RET felica_request_system_code(const uint8_t *idm, uint16_t *systems,
                                       uint8_t *count);

/* Read Without Encryption (0x06), single block. out gets 16 bytes. */
OPERATE_RET felica_read_block(const uint8_t *idm, uint16_t service,
                              uint8_t addr, uint8_t out[FELICA_BLOCK_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* FELICA_H */
