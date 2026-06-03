#include "NFCManager.h"
#include <string.h>
#include <stdio.h>
#include <Arduino.h>

NFCManager g_nfcManager;

NFCManager::NFCManager()
    : _nfc(255, 255, &Wire),
      _hasTag(false), _isValid(false) {
    _uid[0]                 = 0;
    _ndefPayload[0]         = 0;
    _validationError[0]     = 0;
    _extractedName[0]       = 0;
    _extractedIdentifier[0] = 0;
}

NFCManager::~NFCManager() {}

bool NFCManager::init() {
    Wire.begin(13, 15);
    _nfc.begin();

    uint32_t ver = _nfc.getFirmwareVersion();
    if (!ver) {
        Serial.println("[NFC] PN532 not found");
        return false;
    }

    Serial.printf("[NFC] Found PN5%02X fw %d.%d\n",
        (ver >> 24) & 0xFF, (ver >> 16) & 0xFF, (ver >> 8) & 0xFF);

    _nfc.SAMConfig();
    Serial.println("[NFC] Ready");
    return true;
}

void NFCManager::update() {
    if (_hasTag) return;

    uint8_t uid[7];
    uint8_t uidLen = 0;
    if (!_nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 100)) {
        return;
    }

    _uid[0] = 0;
    for (uint8_t i = 0; i < uidLen; i++) {
        char h[5];
        snprintf(h, sizeof(h), i ? " %02X" : "%02X", uid[i]);
        strncat(_uid, h, sizeof(_uid) - strlen(_uid) - 1);
    }
    Serial.printf("[NFC] Tag: %s\n", _uid);

    _ndefPayload[0]         = 0;
    _validationError[0]     = 0;
    _extractedName[0]       = 0;
    _extractedIdentifier[0] = 0;

    _isValid = readNDEF(uidLen) && parseAndValidate();
    _hasTag  = true;

    if (_isValid) {
        Serial.printf("[NFC] Valid — name=%s\n", _extractedName);
    } else {
        Serial.printf("[NFC] Invalid — %s\n", _validationError);
    }
}

bool NFCManager::readNDEF(uint8_t /*uidLength*/) {
    const uint8_t START_PAGE = 4;
    const uint8_t MAX_PAGES  = 40;
    uint8_t raw[MAX_PAGES * 4];
    uint8_t rawLen = 0;

    for (uint8_t p = START_PAGE; p < START_PAGE + MAX_PAGES; p++) {
        uint8_t buf[4];
        if (!_nfc.ntag2xx_ReadPage(p, buf)) {
            if (p == START_PAGE) {
                strncpy(_validationError, "Tag unreadable", sizeof(_validationError) - 1);
                return false;
            }
            break;
        }
        memcpy(raw + rawLen, buf, 4);
        rawLen += 4;
    }

    // Walk TLV blocks
    uint8_t* cur = raw;
    uint8_t* end = raw + rawLen;

    while (cur < end) {
        uint8_t tlvType = *cur++;
        if (cur >= end) break;
        if (tlvType == 0xFE) break;  // terminator

        uint16_t tlvLen;
        if (*cur == 0xFF) {
            if (cur + 2 >= end) break;
            tlvLen = ((uint16_t)cur[1] << 8) | cur[2];
            cur += 3;
        } else {
            tlvLen = *cur++;
        }

        if (tlvType != 0x03) {
            cur += tlvLen;
            continue;
        }

        // NDEF message — Short Record assumed (SR=1)
        if (cur + 4 > end || tlvLen < 4) {
            strncpy(_validationError, "NDEF too short", sizeof(_validationError) - 1);
            return false;
        }

        uint8_t typeLen    = cur[1];
        uint8_t payloadLen = cur[2];
        uint8_t recordType = cur[3];

        if (typeLen != 1 || recordType != 'T') {
            snprintf(_validationError, sizeof(_validationError),
                     "Not a text record (type=%c)", recordType);
            return false;
        }

        // payload: [status_byte][lang...][text...]
        uint8_t* payload    = cur + 4;
        uint8_t  statusByte = payload[0];
        uint8_t  langLen    = statusByte & 0x3F;

        if (payloadLen < (uint8_t)(1 + langLen)) {
            strncpy(_validationError, "Malformed text record", sizeof(_validationError) - 1);
            return false;
        }

        uint8_t  textLen = payloadLen - 1 - langLen;
        uint8_t* text    = payload + 1 + langLen;

        if (textLen == 0 || text + textLen > end) {
            strncpy(_validationError, "Empty NDEF payload", sizeof(_validationError) - 1);
            return false;
        }

        size_t copyLen = textLen < sizeof(_ndefPayload) - 1 ? textLen : sizeof(_ndefPayload) - 1;
        memcpy(_ndefPayload, text, copyLen);
        _ndefPayload[copyLen] = 0;
        return true;
    }

    strncpy(_validationError, "No NDEF record found", sizeof(_validationError) - 1);
    return false;
}

// Extract a JSON string value for the given key. Returns true and fills buf on success.
static bool extractJsonString(const char* json, const char* key,
                               char* buf, size_t bufSize) {
    const char* k = strstr(json, key);
    if (!k) return false;
    const char* colon = strchr(k + strlen(key), ':');
    if (!colon) return false;
    const char* valStart = strchr(colon + 1, '"');
    if (!valStart) return false;
    valStart++;
    const char* valEnd = strchr(valStart, '"');
    if (!valEnd) return false;
    size_t len = (size_t)(valEnd - valStart);
    if (len == 0 || len >= bufSize) return false;
    strncpy(buf, valStart, len);
    buf[len] = 0;
    return true;
}

bool NFCManager::parseAndValidate() {
    // "name" is required
    if (!extractJsonString(_ndefPayload, "\"name\"", _extractedName, sizeof(_extractedName))) {
        // Check why it failed (empty vs missing vs too long)
        const char* k = strstr(_ndefPayload, "\"name\"");
        if (!k) {
            strncpy(_validationError, "Missing name field", sizeof(_validationError) - 1);
        } else {
            strncpy(_validationError, "Invalid name value", sizeof(_validationError) - 1);
        }
        return false;
    }
    if (_extractedName[0] == 0) {
        strncpy(_validationError, "Name is empty", sizeof(_validationError) - 1);
        return false;
    }

    // "id" is optional — silently empty if absent or malformed
    _extractedIdentifier[0] = 0;
    extractJsonString(_ndefPayload, "\"id\"", _extractedIdentifier, sizeof(_extractedIdentifier));

    Serial.printf("[NFC] name='%s' id='%s'\n", _extractedName, _extractedIdentifier);
    return true;
}

bool NFCManager::writeNameTag(const char* name) {
    // Build JSON payload
    char json[32];
    snprintf(json, sizeof(json), "{\"name\":\"%s\"}", name);
    size_t jsonLen = strlen(json);

    // NDEF text record:
    //   flags   = 0xD1 (MB|ME|SR, TNF=Well-Known)
    //   typeLen = 0x01
    //   payloadLen = 1 (status) + 2 (lang "en") + jsonLen
    //   type    = 0x54 ('T')
    //   status  = 0x02 (UTF-8, lang len=2)
    //   lang    = "en"
    //   text    = json
    uint8_t payloadLen = (uint8_t)(1 + 2 + jsonLen);
    uint8_t ndefLen    = (uint8_t)(4 + payloadLen); // flags+typeLen+payloadLen+type + payload

    // Detect tag (up to 2s timeout so user can present badge after clicking)
    uint8_t uid[7], uidLen = 0;
    if (!_nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 2000)) {
        Serial.println("[NFC] writeNameTag: no tag detected");
        return false;
    }
    Serial.printf("[NFC] Writing name='%s' to tag\n", name);

    // Build flat byte array: TLV + NDEF record + terminator
    const uint8_t START_PAGE = 4;
    const uint8_t MAX_BYTES  = 48;
    uint8_t raw[MAX_BYTES];
    memset(raw, 0, sizeof(raw));

    uint8_t i = 0;
    raw[i++] = 0x03;        // TLV type: NDEF message
    raw[i++] = ndefLen;     // TLV length
    raw[i++] = 0xD1;        // NDEF flags
    raw[i++] = 0x01;        // type length
    raw[i++] = payloadLen;  // payload length
    raw[i++] = 0x54;        // 'T'
    raw[i++] = 0x02;        // status: UTF-8, lang len=2
    raw[i++] = 0x65;        // 'e'
    raw[i++] = 0x6E;        // 'n'
    memcpy(raw + i, json, jsonLen);
    i += jsonLen;
    raw[i++] = 0xFE;        // TLV terminator

    uint8_t totalPages = (uint8_t)((i + 3) / 4);
    for (uint8_t p = 0; p < totalPages; p++) {
        if (!_nfc.ntag2xx_WritePage(START_PAGE + p, raw + p * 4)) {
            Serial.printf("[NFC] Write failed at page %d\n", START_PAGE + p);
            return false;
        }
    }

    Serial.println("[NFC] Write success");
    return true;
}

bool NFCManager::hasNewTag() const        { return _hasTag; }
String NFCManager::getUID() const         { return String(_uid); }
String NFCManager::getNDEFPayload() const { return String(_ndefPayload); }
bool NFCManager::isValidTag() const       { return _isValid; }
String NFCManager::getValidationError() const { return String(_validationError); }
const char* NFCManager::getExtractedName()       const { return _extractedName; }
const char* NFCManager::getExtractedIdentifier() const { return _extractedIdentifier; }

void NFCManager::clearDetection() {
    _hasTag                 = false;
    _isValid                = false;
    _uid[0]                 = 0;
    _ndefPayload[0]         = 0;
    _validationError[0]     = 0;
    _extractedName[0]       = 0;
    _extractedIdentifier[0] = 0;
}
