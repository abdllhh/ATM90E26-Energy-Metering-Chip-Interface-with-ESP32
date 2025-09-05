#include <SPI.h>

#define CS1_PIN 15
#define MOSI_PIN 12
#define MISO_PIN 13
#define SCK_PIN  14

SPIClass *hspi = nullptr;

// --- ATM90E26 registers
#define SoftReset 0x00
#define SysStatus 0x01
#define FuncEn    0x02
#define SagTh     0x03
#define LastData  0x06
#define LSB       0x08

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

// Default constants from whatnick’s example (match the checksums)
static const uint16_t DEF_LGAIN = 0x1D39;
static const uint16_t DEF_UGAIN = 0x6810; //was 0xD464 originally
static const uint16_t DEF_IGAIN = 0x7644; // was 6E49 originally
static const uint16_t DEF_CS1   = 0x4A34;
static const uint16_t DEF_CS2   = 0xD294;

// Safer, spec-friendly SPI settings (MODE3 per DS)
static SPISettings atmSettings(100000, MSBFIRST, SPI_MODE3);

uint16_t atm_read16(uint8_t addr7) {
  uint8_t header = 0x80 | (addr7 & 0x7F);  // bit7=1 for READ
  uint16_t msb, lsb;

  hspi->beginTransaction(atmSettings);
  digitalWrite(CS1_PIN, LOW);
  delayMicroseconds(5); // tCSS

  hspi->transfer(header);

  // datasheet allows a small delay after address phase
  delayMicroseconds(4);

  msb = hspi->transfer(0x00);
  lsb = hspi->transfer(0x00);

  delayMicroseconds(5); // tCSH
  digitalWrite(CS1_PIN, HIGH);
  hspi->endTransaction();

  return (uint16_t)((msb << 8) | lsb); // b15..b0 shift to make full 16
}

void atm_write16(uint8_t addr7, uint16_t value) {
  uint8_t header = (addr7 & 0x7F);       // bit7=0 for WRITE
  uint8_t msb = (value >> 8) & 0xFF;
  uint8_t lsb = value & 0xFF;

  hspi->beginTransaction(atmSettings);
  digitalWrite(CS1_PIN, LOW);
  delayMicroseconds(5); // tCSS

  hspi->transfer(header);
  delayMicroseconds(4);
  hspi->transfer(msb);
  hspi->transfer(lsb);

  delayMicroseconds(5); // tCSH
  digitalWrite(CS1_PIN, HIGH);
  hspi->endTransaction();
}

void atm_init() {
  // Soft reset, then small pause
  atm_write16(SoftReset, 0x789A);
  delay(200);

  // Enable sag warn to match example (optional)
  atm_write16(FuncEn, 0x0030);   // SagEn + SagWo
  atm_write16(SagTh,  0x1D6A);   // example threshold used was 1F2F //The power-on value of SagTh is 1D6AH, which is calculated by 22000*sqrt(2)*0.78/(4*Ugain/32768

  // Metering calibration/config block
  atm_write16(CalStart, 0x5678);
  atm_write16(PLconstH, 0x00B9);
  atm_write16(PLconstL, 0xC1F3);
  atm_write16(Lgain,    DEF_LGAIN);
  atm_write16(Lphi,     0x0000);
  atm_write16(PStartTh, 0x08BD);
  atm_write16(PNolTh,   0x0000);
  atm_write16(QStartTh, 0x0AEC);
  atm_write16(QNolTh,   0x0000);
  atm_write16(MMode,    0x9422);
  atm_write16(CSOne,    DEF_CS1);

  // Measurement calibration block
  atm_write16(AdjStart, 0x5678);
  atm_write16(Ugain,    DEF_UGAIN);
  atm_write16(IgainL,   DEF_IGAIN);
  atm_write16(Uoffset,  0x0000);
  atm_write16(IoffsetL, 0x0000);
  atm_write16(PoffsetL, 0x0000);
  atm_write16(QoffsetL, 0x0000);
  atm_write16(CSTwo,    DEF_CS2);

  // Start normal operation
  atm_write16(CalStart, 0x8765);
  atm_write16(AdjStart, 0x8765);

  // Optional: read back status
  uint16_t sys = atm_read16(SysStatus);
  Serial.print("Init SysStatus=0x"); Serial.println(sys, HEX);
}

// ---------- scaling as the lib ----------
double getVoltage() {
  uint16_t v = atm_read16(Urms);
  //return ((double)v) / 100.0;
  return ((double)v * 0.4687873714 ) / 100.0; // volts //0.47 cuz 220/466=0.47 (used a voltage ref and then found ratio)
}

double getCurrent() {
  uint16_t i = atm_read16(Irms);
  return ((double)i) / 1000.0; // amps
}

double getActivePower() {
  int16_t p = (int16_t)atm_read16(Pmean);
  return (double)p; // watts (per original lib’s interpretation)
}

double getPF() {
  int16_t pf = (int16_t)atm_read16(PowerF);
  if (pf & 0x8000) pf = - (pf & 0x7FFF);
  return ((double)pf) / 1000.0;
}

double getFreq() {
  uint16_t f = atm_read16(Freq);
  return ((double)f) / 100.0;
}

void setup() {
  Serial.begin(115200);

  hspi = new SPIClass(SPI);
  hspi->begin(SCK_PIN, MISO_PIN, MOSI_PIN, -1); // manual CS

  pinMode(CS1_PIN, OUTPUT);
  digitalWrite(CS1_PIN, HIGH);

  hspi->beginTransaction(atmSettings);
  hspi->endTransaction();

  // initialize the metering chip 
  atm_init();

  // quick sanity: LastData is “whatever was last read/written”, not an ID
  uint16_t last = atm_read16(LastData);
  Serial.print("LastData=0x"); Serial.println(last, HEX);
}

void loop() {
  uint16_t sys = atm_read16(SysStatus);
  uint16_t met = atm_read16(EnStatus);

  Serial.println("==== Readings ====");
  Serial.print("Sys Status:");   Serial.println(sys, HEX);
  Serial.print("Meter Status:"); Serial.println(met, HEX);

  Serial.print("Voltage:");      Serial.println(getVoltage(), 2);
  Serial.print("Current:");      Serial.println(getCurrent(), 3);
  Serial.print("Active power:"); Serial.println(getActivePower(), 2);
  Serial.print("p.f.:");         Serial.println(getPF(), 3);
  Serial.print("Freq:");         Serial.println(getFreq(), 2);

  delay(1000);
}
