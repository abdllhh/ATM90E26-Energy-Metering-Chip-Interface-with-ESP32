#include "myATM90E26.h"

// === Register definitions ===
#define SoftReset 0x00
#define SysStatus 0x01
#define FuncEn    0x02
#define SagTh     0x03
#define LastData  0x06
#define CalStart  0x20
#define PLconstH  0x21
#define PLconstL  0x22
#define Lgain     0x23
#define Lphi      0x24
#define PStartTh  0x27
#define PNolTh    0x28
#define QStartTh  0x29
#define QNolTh    0x2A
#define MMode     0x2B
#define CSOne     0x2C
#define AdjStart  0x30
#define Ugain     0x31
#define IgainL    0x32
#define Uoffset   0x34
#define IoffsetL  0x35
#define PoffsetL  0x37
#define QoffsetL  0x38
#define CSTwo     0x3B
#define APenergy  0x40
#define ANenergy  0x41
#define EnStatus  0x46
#define Irms      0x48
#define Urms      0x49
#define Pmean     0x4A
#define Qmean     0x4B
#define Freq      0x4C
#define PowerF    0x4D

// Default constants
static const uint16_t DEF_LGAIN = 0x1D39;
static const uint16_t DEF_UGAIN = 0xD464;
static const uint16_t DEF_IGAIN = 0x6E49;
static const uint16_t DEF_CS1   = 0x4A34;
static const uint16_t DEF_CS2   = 0xD294;

// SPI settings
static SPISettings atmSettings(100000, MSBFIRST, SPI_MODE3);

// --- Constructor ---
ATM90E26::ATM90E26() {
    hspi = nullptr;
    csPin = ATM_CS_DEFAULT;
}

// --- Begin ---
bool ATM90E26::begin(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi) {
    csPin = cs;
    hspi = new SPIClass(SPI);
    hspi->begin(sck, miso, mosi, -1);

    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);

    hspi->beginTransaction(atmSettings);
    hspi->endTransaction();

    initChip();
    return true;
}

// --- Low-level read ---
uint16_t ATM90E26::read16(uint8_t addr7) {
    uint8_t header = 0x80 | (addr7 & 0x7F);
    uint16_t msb, lsb;

    hspi->beginTransaction(atmSettings);
    digitalWrite(csPin, LOW);
    delayMicroseconds(5);

    hspi->transfer(header);
    delayMicroseconds(4);
    msb = hspi->transfer(0x00);
    lsb = hspi->transfer(0x00);

    delayMicroseconds(5);
    digitalWrite(csPin, HIGH);
    hspi->endTransaction();

    return (uint16_t)((msb << 8) | lsb);
}

// --- Low-level write ---
void ATM90E26::write16(uint8_t addr7, uint16_t value) {
    uint8_t header = (addr7 & 0x7F);
    uint8_t msb = (value >> 8) & 0xFF;
    uint8_t lsb = value & 0xFF;

    hspi->beginTransaction(atmSettings);
    digitalWrite(csPin, LOW);
    delayMicroseconds(5);

    hspi->transfer(header);
    delayMicroseconds(4);
    hspi->transfer(msb);
    hspi->transfer(lsb);

    delayMicroseconds(5);
    digitalWrite(csPin, HIGH);
    hspi->endTransaction();
}

// --- Init chip sequence ---
void ATM90E26::initChip() {
    write16(SoftReset, 0x789A);
    delay(200);

    write16(FuncEn, 0x0030);
    write16(SagTh,  0x1D6A);

    write16(CalStart, 0x5678);
    write16(PLconstH, 0x00B9);
    write16(PLconstL, 0xC1F3);
    write16(Lgain,    DEF_LGAIN);
    write16(Lphi,     0x0000);
    write16(PStartTh, 0x08BD);
    write16(PNolTh,   0x0000);
    write16(QStartTh, 0x0AEC);
    write16(QNolTh,   0x0000);
    write16(MMode,    0x9422);
    write16(CSOne,    DEF_CS1);

    write16(AdjStart, 0x5678);
    write16(Ugain,    DEF_UGAIN);
    write16(IgainL,   DEF_IGAIN);
    write16(Uoffset,  0x0000);
    write16(IoffsetL, 0x0000);
    write16(PoffsetL, 0x0000);
    write16(QoffsetL, 0x0000);
    write16(CSTwo,    DEF_CS2);

    write16(CalStart, 0x8765);
    write16(AdjStart, 0x8765);

    uint16_t sys = read16(SysStatus);
    Serial.print("Init SysStatus=0x"); Serial.println(sys, HEX);
}

// --- High-level helpers ---
double ATM90E26::getVoltage() {
    uint16_t v = read16(Urms);
    return ((double)v * 0.4687873714) / 100.0;
}

double ATM90E26::getCurrent() {
    uint16_t i = read16(Irms);
    return ((double)i) / 1000.0;
}

double ATM90E26::getActivePower() {
    int16_t p = (int16_t)read16(Pmean);
    return (double)p;
}

double ATM90E26::getPowerFactor() {
    int16_t pf = (int16_t)read16(PowerF);
    if (pf & 0x8000) pf = -(pf & 0x7FFF);
    return ((double)pf) / 1000.0;
}

double ATM90E26::getFrequency() {
    uint16_t f = read16(Freq);
    return ((double)f) / 100.0;
}
