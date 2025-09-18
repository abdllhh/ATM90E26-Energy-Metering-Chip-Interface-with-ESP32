#ifndef myATM90E26_H
#define myATM90E26_H

#include <Arduino.h>
#include <SPI.h>

// --- Pin defaults (may change in begin() if diff pins) ---
#define ATM_CS_DEFAULT   15
#define ATM_MOSI_DEFAULT 12
#define ATM_MISO_DEFAULT 13
#define ATM_SCK_DEFAULT  14

class ATM90E26 {
public:
    ATM90E26();
    bool begin(uint8_t csPin = ATM_CS_DEFAULT,
               uint8_t sckPin = ATM_SCK_DEFAULT,
               uint8_t misoPin = ATM_MISO_DEFAULT,
               uint8_t mosiPin = ATM_MOSI_DEFAULT);

    // Measurements
    double getVoltage();
    double getCurrent();
    double getActivePower();
    double getPowerFactor();
    double getFrequency();

    // Low-level access
    uint16_t read16(uint8_t addr7);
    void write16(uint8_t addr7, uint16_t value);

private:
    void initChip();
    SPIClass *hspi;
    uint8_t csPin;
};

#endif

