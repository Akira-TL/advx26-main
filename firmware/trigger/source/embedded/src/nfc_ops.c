#include <stdio.h>
#include <string.h>

#include "tal_api.h"

#include "nfc_ops.h"
#include "pn532_i2c.h"
#include "mifare_classic.h"
#include "ntag.h"
#include "felica.h"

#define M1_DUMP_MAX_ROWS   (NFC_DUMP_MAX_LEN / M1_BLOCK_LEN) /* 64 */
#define NTAG_UL_PAGES      16   /* plain Ultralight: 64 bytes */
#define M1_WRITE_FIRST_BLK 4    /* sector 1 data blocks 4..6 */
#define M1_WRITE_BLOCKS    3
#define POLL_RETRY         3
#define POLL_GAP_MS        120

static const uint8_t sg_m1_keys[][6] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5},
    {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

const char *nfc_card_type_name(nfc_card_type_e type)
{
    switch (type) {
    case NFC_CARD_MIFARE_1K:  return "Mifare Classic 1K";
    case NFC_CARD_MIFARE_4K:  return "Mifare Classic 4K";
    case NFC_CARD_NTAG:       return "NTAG21x";
    case NFC_CARD_ULTRALIGHT: return "Mifare Ultralight";
    case NFC_CARD_ISO_DEP:    return "ISO-DEP (CPU)";
    case NFC_CARD_FELICA:     return "FeliCa";
    case NFC_CARD_TYPE_B:     return "ISO14443-B";
    default:                  return "Unknown card";
    }
}

static OPERATE_RET m1_auth_any(uint8_t first_block, const uint8_t *uid,
                               uint8_t uid_len)
{
    size_t k;

    for (k = 0; k < sizeof(sg_m1_keys) / sizeof(sg_m1_keys[0]); k++) {
        if (m1_auth_a(first_block, sg_m1_keys[k], uid, uid_len) == OPRT_OK) {
            return OPRT_OK;
        }
    }
    return OPRT_COM_ERROR;
}

static void m1_dump(nfc_card_t *card, uint16_t total_blocks)
{
    uint16_t blk = 0;

    while (blk < total_blocks) {
        uint16_t sector_blocks = (blk < 128U) ? 4U : 16U;
        uint16_t j;
        OPERATE_RET rt = m1_auth_any((uint8_t)blk, card->uid, card->uid_len);

        for (j = 0; j < sector_blocks && blk + j < total_blocks; j++) {
            uint16_t row = blk + j;

            if (row >= M1_DUMP_MAX_ROWS) {
                return; /* dump buffer full; size_bytes still reports full size */
            }
            if (rt == OPRT_OK &&
                m1_read_block((uint8_t)(blk + j),
                              card->dump + row * M1_BLOCK_LEN) == OPRT_OK) {
                card->readable[row] = 1;
            }
            if (row + 1U > card->dump_rows) {
                card->dump_rows = row + 1U;
            }
        }
        blk += sector_blocks;
    }
}

static const char *uri_prefix_str(uint8_t code)
{
    switch (code) {
    case 0x01: return "http://www.";
    case 0x02: return "https://www.";
    case 0x03: return "http://";
    case 0x04: return "https://";
    default:   return "";
    }
}

static void ndef_extract_text(const uint8_t *mem, size_t len, char *out,
                              size_t out_size)
{
    size_t i = 0;

    out[0] = '\0';
    while (i < len) {
        uint8_t tlv = mem[i];

        if (tlv == 0x00U) { i++; continue; }
        if (tlv == 0xFEU) { return; }
        if (tlv != 0x03U || i + 1U >= len) { return; }

        {
            uint8_t msg_len = mem[i + 1U];
            const uint8_t *msg = &mem[i + 2U];
            const uint8_t *type;
            const uint8_t *payload;
            uint8_t type_len;
            uint8_t plen;

            if (msg_len < 5U || i + 2U + msg_len > len) { return; }
            /* short record: hdr, type_len, plen, type[tlen], payload[plen] */
            type_len = msg[1];
            plen = msg[2];
            if ((size_t)type_len + plen + 3U > msg_len) { return; }
            type = &msg[3];
            payload = &msg[3 + type_len];

            if (type_len == 1U && type[0] == 0x54U && plen >= 1U) {
                /* Text: status byte (lang len in low 6 bits) + lang + text */
                uint8_t lang_len = payload[0] & 0x3FU;
                size_t text_len;

                if ((size_t)lang_len + 1U > plen) { return; }
                text_len = (size_t)plen - 1U - lang_len;
                if (text_len >= out_size) { text_len = out_size - 1U; }
                memcpy(out, &payload[1U + lang_len], text_len);
                out[text_len] = '\0';
            } else if (type_len == 1U && type[0] == 0x55U && plen >= 1U) {
                /* URI: prefix code + body */
                snprintf(out, out_size, "%s%.*s", uri_prefix_str(payload[0]),
                         (int)(plen - 1U), (const char *)&payload[1]);
            } else if (type_len == 15U &&
                       memcmp(type, "android.com:pkg", 15) == 0) {
                snprintf(out, out_size, "APP: %.*s", (int)plen,
                         (const char *)payload);
            }
        }
        return;
    }
}

static void ndef_extract_content_id(const char *url, char *out, size_t out_size)
{
    const char *p;
    size_t i;

    out[0] = '\0';
    if (out_size < 33U) {
        return;
    }
    p = strstr(url, "/c/");
    if (p == NULL) {
        return;
    }
    p += 3U;
    for (i = 0; i < 32U; i++) {
        char c = p[i];
        int is_hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                     (c >= 'A' && c <= 'F');

        if (!is_hex) {
            return;
        }
        out[i] = c;
    }
    out[32] = '\0';
}

static uint16_t ndef_build_text(const char *text, uint8_t *out,
                                uint16_t out_cap)
{
    size_t text_len = strlen(text);
    size_t plen = 3U + text_len;        /* status + "en" + text */
    size_t msg_len = 4U + plen;         /* hdr + typelen + plen + type */
    size_t total = 2U + msg_len + 1U;   /* TLV hdr + msg + terminator */

    if (text_len == 0U || plen > 0xFFU || msg_len > 0xFFU ||
        total > out_cap) {
        return 0;
    }
    out[0] = 0x03;
    out[1] = (uint8_t)msg_len;
    out[2] = 0xD1;
    out[3] = 0x01;
    out[4] = (uint8_t)plen;
    out[5] = 0x54;
    out[6] = 0x02; /* UTF-8, lang len 2 */
    out[7] = 'e';
    out[8] = 'n';
    memcpy(&out[9], text, text_len);
    out[9 + text_len] = 0xFE;
    return (uint16_t)total;
}

static const struct {
    const char *prefix;
    uint8_t code;
} sg_uri_prefixes[] = {
    {"http://www.", 0x01},
    {"https://www.", 0x02},
    {"http://", 0x03},
    {"https://", 0x04},
};

static uint16_t ndef_build_uri(const char *uri, uint8_t *out,
                               uint16_t out_cap)
{
    uint8_t code = 0x00; /* no prefix */
    const char *body = uri;
    size_t body_len;
    size_t plen;
    size_t msg_len;
    size_t total;
    size_t i;

    for (i = 0; i < sizeof(sg_uri_prefixes) / sizeof(sg_uri_prefixes[0]); i++) {
        size_t pfx_len = strlen(sg_uri_prefixes[i].prefix);

        if (strncmp(uri, sg_uri_prefixes[i].prefix, pfx_len) == 0) {
            code = sg_uri_prefixes[i].code;
            body = uri + pfx_len;
            break;
        }
    }
    body_len = strlen(body);
    plen = 1U + body_len;               /* prefix code + body */
    msg_len = 4U + plen;
    total = 2U + msg_len + 1U;
    if (body_len == 0U || plen > 0xFFU || msg_len > 0xFFU ||
        total > out_cap) {
        return 0;
    }
    out[0] = 0x03;
    out[1] = (uint8_t)msg_len;
    out[2] = 0xD1;
    out[3] = 0x01;
    out[4] = (uint8_t)plen;
    out[5] = 0x55; /* 'U' */
    out[6] = code;
    memcpy(&out[7], body, body_len);
    out[7 + body_len] = 0xFE;
    return (uint16_t)total;
}

#define AAR_TYPE     "android.com:pkg"
#define AAR_TYPE_LEN 15U

static uint16_t ndef_build_aar(const char *pkg, uint8_t *out,
                               uint16_t out_cap)
{
    size_t pkg_len = strlen(pkg);
    size_t msg_len = 3U + AAR_TYPE_LEN + pkg_len;
    size_t total = 2U + msg_len + 1U;

    if (pkg_len == 0U || pkg_len > 0xFFU || msg_len > 0xFFU ||
        total > out_cap) {
        return 0;
    }
    out[0] = 0x03;
    out[1] = (uint8_t)msg_len;
    out[2] = 0xD4; /* MB|ME|SR, TNF=external */
    out[3] = AAR_TYPE_LEN;
    out[4] = (uint8_t)pkg_len;
    memcpy(&out[5], AAR_TYPE, AAR_TYPE_LEN);
    memcpy(&out[5 + AAR_TYPE_LEN], pkg, pkg_len);
    out[5 + AAR_TYPE_LEN + pkg_len] = 0xFE;
    return (uint16_t)total;
}

static uint16_t ndef_build(nfc_ndef_type_e type, const char *text,
                           uint8_t *out, uint16_t out_cap)
{
    switch (type) {
    case NFC_NDEF_URI: return ndef_build_uri(text, out, out_cap);
    case NFC_NDEF_AAR: return ndef_build_aar(text, out, out_cap);
    default:           return ndef_build_text(text, out, out_cap);
    }
}

static OPERATE_RET ntag_dump(nfc_card_t *card, ntag_type_e nt)
{
    uint8_t last = ntag_user_last_page(nt);
    uint16_t total_pages = (uint16_t)last + 1U;
    uint16_t rows = (total_pages + 3U) / 4U;
    uint16_t r;

    card->block_count = total_pages;
    card->size_bytes = total_pages * NTAG_PAGE_LEN;
    for (r = 0; r < rows && r < M1_DUMP_MAX_ROWS; r++) {
        if (ntag_read((uint8_t)(r * 4U),
                      card->dump + r * NTAG_READ_LEN) != OPRT_OK) {
            break;
        }
        card->readable[r] = 1;
        card->dump_rows = r + 1U;
    }
    /* user memory starts at page 4 = byte offset 16 in the dump */
    if (card->dump_rows > 1U) {
        ndef_extract_text(card->dump + 16,
                          (size_t)card->dump_rows * 16U - 16U,
                          card->ndef_text, sizeof(card->ndef_text));
        ndef_extract_content_id(card->ndef_text, card->content_id,
                                sizeof(card->content_id));
    }
    return (card->dump_rows > 0U) ? OPRT_OK : OPRT_COM_ERROR;
}

static OPERATE_RET ultralight_dump(nfc_card_t *card)
{
    uint16_t total_pages = NTAG_UL_PAGES;
    uint16_t rows;
    uint16_t r;

    if (ntag_read(0, card->dump) != OPRT_OK) {
        return OPRT_COM_ERROR;
    }
    card->readable[0] = 1;
    card->dump_rows = 1U;

    /* Capability Container page 3 byte 2 = NDEF area size / 8. Trust it so
     * an NTAG whose GET_VERSION failed (misdetected as Ultralight) still
     * gets its full user memory dumped instead of only 16 pages. */
    if (card->dump[14] != 0U) {
        total_pages = (uint16_t)(4U + ((uint16_t)card->dump[14] * 8U) / 4U);
    }
    rows = (total_pages + 3U) / 4U;

    card->block_count = total_pages;
    card->size_bytes = total_pages * NTAG_PAGE_LEN;
    for (r = 1; r < rows && r < M1_DUMP_MAX_ROWS; r++) {
        if (ntag_read((uint8_t)(r * 4U),
                      card->dump + r * NTAG_READ_LEN) != OPRT_OK) {
            break;
        }
        card->readable[r] = 1;
        card->dump_rows = r + 1U;
    }
    if (card->dump_rows > 1U) {
        ndef_extract_text(card->dump + 16,
                          (size_t)card->dump_rows * 16U - 16U,
                          card->ndef_text, sizeof(card->ndef_text));
        ndef_extract_content_id(card->ndef_text, card->content_id,
                                sizeof(card->content_id));
    }
    return (card->dump_rows > 0U) ? OPRT_OK : OPRT_COM_ERROR;
}

static void fill_target_id(nfc_card_t *card, const PN532_TARGET_T *t)
{
    card->uid_len = (t->uid_len > NFC_UID_MAX_LEN) ? NFC_UID_MAX_LEN : t->uid_len;
    memcpy(card->uid, t->uid, card->uid_len);
    card->sak = t->sak;
    memcpy(card->atqa, t->atqa, sizeof(card->atqa));
}

static OPERATE_RET read_type_a(nfc_card_t *card, const PN532_TARGET_T *t)
{
    fill_target_id(card, t);

    if (t->sak == 0x08U || t->sak == 0x18U) {
        uint16_t blocks = (t->sak == 0x08U) ? M1_1K_BLOCKS : M1_4K_BLOCKS;

        card->type = (t->sak == 0x08U) ? NFC_CARD_MIFARE_1K : NFC_CARD_MIFARE_4K;
        card->block_count = blocks;
        card->size_bytes = blocks * M1_BLOCK_LEN;
        m1_dump(card, blocks);
        return OPRT_OK;
    }
    if (t->sak == 0x00U) {
        ntag_type_e nt = ntag_get_version();

        if (NTAG_TYPE_UNKNOWN != nt) {
            card->type = NFC_CARD_NTAG;
            ntag_dump(card, nt);
        } else {
            card->type = NFC_CARD_ULTRALIGHT;
            ultralight_dump(card);
        }
        return OPRT_OK;
    }
    if (t->sak & 0x20U) {
        card->type = NFC_CARD_ISO_DEP;
        return OPRT_OK;
    }
    card->type = NFC_CARD_UNKNOWN;
    return OPRT_OK;
}

OPERATE_RET nfc_ops_detect(nfc_card_t *card)
{
    PN532_TARGET_T t;
    OPERATE_RET rt;

    if (card == NULL) {
        return OPRT_INVALID_PARM;
    }
    memset(card, 0, sizeof(*card));
    card->type = NFC_CARD_NONE;

    rt = pn532_poll_target(PN532_POLL_ISO14443A, &t);
    if (rt != OPRT_OK) {
        return rt;
    }
    fill_target_id(card, &t);
    return OPRT_OK;
}

OPERATE_RET nfc_ops_read_detected(nfc_card_t *card)
{
    PN532_TARGET_T t;

    if (card == NULL || card->uid_len == 0U) {
        return OPRT_INVALID_PARM;
    }
    /* The card is still selected from the detect poll; re-selecting it
     * back-to-back leaves NTAG GET_VERSION failing, so reuse the target. */
    memset(&t, 0, sizeof(t));
    t.uid_len = card->uid_len;
    memcpy(t.uid, card->uid, card->uid_len);
    t.sak = card->sak;
    memcpy(t.atqa, card->atqa, sizeof(t.atqa));

    return read_type_a(card, &t);
}

OPERATE_RET nfc_ops_read_card(nfc_card_t *card)
{
    PN532_TARGET_T t;
    OPERATE_RET rt;
    uint8_t attempt;

    if (card == NULL) {
        return OPRT_INVALID_PARM;
    }
    memset(card, 0, sizeof(*card));
    card->type = NFC_CARD_NONE;

    for (attempt = 0; attempt < POLL_RETRY; attempt++) {
        rt = pn532_poll_target(PN532_POLL_ISO14443A, &t);
        if (rt == OPRT_OK) {
            return read_type_a(card, &t);
        }
        if (rt != OPRT_NOT_FOUND) {
            return rt;
        }
        tal_system_sleep(POLL_GAP_MS);
    }

    rt = pn532_poll_target(PN532_POLL_FELICA_212, &t);
    if (rt == OPRT_OK) {
        uint16_t systems[8];
        uint8_t count = sizeof(systems) / sizeof(systems[0]);

        card->type = NFC_CARD_FELICA;
        fill_target_id(card, &t);
        if (felica_request_system_code(t.uid, systems, &count) == OPRT_OK) {
            size_t off = 0;
            uint8_t i;

            off += (size_t)snprintf(card->ndef_text + off,
                                    sizeof(card->ndef_text) - off, "SC:");
            for (i = 0; i < count && off + 6U < sizeof(card->ndef_text); i++) {
                off += (size_t)snprintf(card->ndef_text + off,
                                        sizeof(card->ndef_text) - off,
                                        " %04X", systems[i]);
            }
        }
        return OPRT_OK;
    }
    if (rt != OPRT_NOT_FOUND) {
        return rt;
    }

    rt = pn532_poll_target(PN532_POLL_ISO14443B, &t);
    if (rt == OPRT_OK) {
        card->type = NFC_CARD_TYPE_B;
        fill_target_id(card, &t);
        return OPRT_OK;
    }
    return rt;
}

static OPERATE_RET m1_write_text(const uint8_t *uid, uint8_t uid_len,
                                 const char *text)
{
    uint8_t buf[M1_WRITE_BLOCKS * M1_BLOCK_LEN];
    size_t len = strlen(text);
    uint8_t b;
    OPERATE_RET rt;

    if (len > sizeof(buf) - 1U) {
        len = sizeof(buf) - 1U;
    }
    memset(buf, 0, sizeof(buf));
    memcpy(buf, text, len);

    rt = m1_auth_any(M1_WRITE_FIRST_BLK, uid, uid_len);
    if (rt != OPRT_OK) {
        return rt;
    }
    for (b = 0; b < M1_WRITE_BLOCKS; b++) {
        rt = m1_write_block((uint8_t)(M1_WRITE_FIRST_BLK + b),
                            buf + (size_t)b * M1_BLOCK_LEN);
        if (rt != OPRT_OK) {
            return rt;
        }
    }
    return OPRT_OK;
}

static OPERATE_RET ntag_write_ndef(nfc_ndef_type_e ndef_type,
                                   const char *text, uint8_t last)
{
    uint8_t tlv[192];
    uint16_t total = ndef_build(ndef_type, text, tlv, sizeof(tlv));
    uint16_t pages;
    uint16_t p;

    if (total == 0U) {
        return OPRT_INVALID_PARM;
    }
    pages = (total + NTAG_PAGE_LEN - 1U) / NTAG_PAGE_LEN;
    if ((uint16_t)NTAG_USER_START + pages - 1U > (uint16_t)last) {
        return OPRT_INVALID_PARM; /* does not fit on this tag */
    }
    for (p = 0; p < pages; p++) {
        uint8_t data[NTAG_PAGE_LEN];

        memset(data, 0, sizeof(data));
        if ((size_t)p * NTAG_PAGE_LEN < total) {
            size_t chunk = total - (size_t)p * NTAG_PAGE_LEN;
            if (chunk > NTAG_PAGE_LEN) {
                chunk = NTAG_PAGE_LEN;
            }
            memcpy(data, tlv + (size_t)p * NTAG_PAGE_LEN, chunk);
        }
        if (ntag_write_page((uint8_t)(NTAG_USER_START + p), data) != OPRT_OK) {
            return OPRT_COM_ERROR;
        }
    }
    return OPRT_OK;
}

OPERATE_RET nfc_ops_write_record(nfc_ndef_type_e ndef_type, const char *text,
                                 nfc_card_type_e *out_type)
{
    PN532_TARGET_T t;
    OPERATE_RET rt;
    uint8_t attempt;

    if (text == NULL || text[0] == '\0') {
        return OPRT_INVALID_PARM;
    }
    if (out_type != NULL) {
        *out_type = NFC_CARD_NONE;
    }

    for (attempt = 0; attempt < POLL_RETRY; attempt++) {
        rt = pn532_poll_target(PN532_POLL_ISO14443A, &t);
        if (rt == OPRT_OK) {
            break;
        }
        if (rt != OPRT_NOT_FOUND) {
            return rt;
        }
        tal_system_sleep(POLL_GAP_MS);
    }
    if (rt != OPRT_OK) {
        rt = pn532_poll_target(PN532_POLL_FELICA_212, &t);
        if (rt == OPRT_OK) {
            if (out_type != NULL) { *out_type = NFC_CARD_FELICA; }
            return OPRT_NOT_SUPPORTED;
        }
        if (rt != OPRT_NOT_FOUND) {
            return rt;
        }
        rt = pn532_poll_target(PN532_POLL_ISO14443B, &t);
        if (rt == OPRT_OK) {
            if (out_type != NULL) { *out_type = NFC_CARD_TYPE_B; }
            return OPRT_NOT_SUPPORTED;
        }
        return (rt == OPRT_OK) ? OPRT_NOT_SUPPORTED : rt;
    }

    if (t.sak == 0x08U || t.sak == 0x18U) {
        if (out_type != NULL) {
            *out_type = (t.sak == 0x08U) ? NFC_CARD_MIFARE_1K
                                         : NFC_CARD_MIFARE_4K;
        }
        return m1_write_text(t.uid, t.uid_len, text);
    }
    if (t.sak == 0x00U) {
        ntag_type_e nt = ntag_get_version();
        uint8_t last = NTAG_UL_LAST_PAGE;

        if (NTAG_TYPE_UNKNOWN != nt) {
            if (out_type != NULL) { *out_type = NFC_CARD_NTAG; }
            return ntag_write_ndef(ndef_type, text, ntag_user_last_page(nt));
        }
        if (out_type != NULL) { *out_type = NFC_CARD_ULTRALIGHT; }
        /* GET_VERSION often fails on NTAG21x here (bus timing), which would
         * cap capacity at Ultralight's 16 pages and reject long URLs. The
         * Capability Container (page 3 byte 2 = NDEF area size / 8) is
         * factory-programmed on NTAG21x, so size the tag from it instead. */
        {
            uint8_t hdr[NTAG_READ_LEN];

            if (ntag_read(0, hdr) == OPRT_OK && hdr[14] != 0U) {
                uint16_t user_pages = ((uint16_t)hdr[14] * 8U) / 4U;
                uint16_t cc_last = (uint16_t)NTAG_USER_START + user_pages - 1U;

                if (cc_last > 255U) { cc_last = 255U; }
                last = (uint8_t)cc_last;
            }
        }
        return ntag_write_ndef(ndef_type, text, last);
    }
    if (t.sak & 0x20U) {
        if (out_type != NULL) { *out_type = NFC_CARD_ISO_DEP; }
        return OPRT_NOT_SUPPORTED;
    }
    if (out_type != NULL) { *out_type = NFC_CARD_UNKNOWN; }
    return OPRT_NOT_SUPPORTED;
}
