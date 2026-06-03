#pragma once

#include <stdint.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

class NFCManager {
public:
    NFCManager();
    ~NFCManager();

    bool init();
    void update();

    bool        hasNewTag()              const;
    String      getUID()                 const;
    String      getNDEFPayload()         const;
    bool        isValidTag()             const;
    String      getValidationError()     const;
    const char* getExtractedName()       const;
    const char* getExtractedIdentifier() const;
    void        clearDetection();

    bool writeNameTag(const char* name);

private:
    Adafruit_PN532 _nfc;
    bool _hasTag;
    bool _isValid;
    char _uid[24];
    char _ndefPayload[256];
    char _validationError[50];
    char _extractedName[16];
    char _extractedIdentifier[16];

    bool readNDEF(uint8_t uidLength);
    bool parseAndValidate();
};

extern NFCManager g_nfcManager;
