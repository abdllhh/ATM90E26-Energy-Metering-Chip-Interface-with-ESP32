#include "ATM90E26.h"

ATM90E26::ATM90E26(uint8_t csPin, uint8_t sck, uint8_t miso, uint8_t mosi)
: csPin(csPin), settings(100000, MSBFIRST, SPI_MODE3) {
hspi = new SPIClass(SPI);
hspi->begin(sck, miso, mosi, -1);
pinMode(csPin, OUTPUT);
digitalWrite(csPin, HIGH);
}

void ATM90E26::begin() {
hspi->beginTransaction(settings);
hspi->endTransaction();
init();
}

uint16_t ATM90E26::read16(uint8_t addr7) {
uint8_t header = 0x80 | (addr7 & 0x7F);
uint16_t msb, lsb;

hspi->beginTransaction(settings);
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

void ATM90E26::write16(uint8_t addr7, uint16_t value) {
uint8_t header = (addr7 & 0x7F);
uint8_t msb = (value >> 8) & 0xFF;
uint8_t lsb = value & 0xFF;


hspi->beginTransaction(settings);
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

void ATM90E26::init() {
write16(SoftReset, 0x789A);
delay(200);

write16(FuncEn, 0x0030);
write16(SagTh, 0x2630);

write16(CalStart, 0x5678);
write16(PLconstH, 0x0001);
write16(PLconstL, 0xD166);
write16(Lgain, 0x0000);
write16(Lphi, 0x0000);
write16(Ngain, 0x0000);
write16(Nphi, 0x0000);
write16(PStartTh, 0x0100);
write16(PNolTh, 0x0000);
write16(QStartTh, 0x0100);
write16(QNolTh, 0x0000);
write16(MMode, 0x9022);

delay(10);
uint16_t cs1 = calculateCS1();
write16(CSOne, cs1);

write16(AdjStart, 0x5678);
write16(Ugain, 0x6414);
write16(IgainL, 0xF1F2);
write16(IgainN, 0x7530);
write16(Uoffset, 0x0000);
write16(IoffsetL, 0x0000);
write16(IoffsetN, 0x0000);
write16(PoffsetL, 0x0000);
write16(QoffsetL, 0x0000);
write16(PoffsetN, 0x0000);
write16(QoffsetN, 0x0000);

delay(10);
uint16_t cs2 = calculateCS2();
write16(CSTwo, cs2);

write16(CalStart, 0x8765);
write16(AdjStart, 0x8765);


}

// ------------------- Getters -------------------
double ATM90E26::getVoltage() { return ((double)read16(Urms)) / 100.0; }
double ATM90E26::getCurrent() { return ((double)read16(Irms)) / 1000.0; }
double ATM90E26::getActivePower() { return ((int16_t)read16(Pmean)) / 1000.0; }
double ATM90E26::getReactivePower() { return (int16_t)read16(Qmean); }
double ATM90E26::getApparentPower() { return (int16_t)read16(Smean); }
double ATM90E26::getPF() { int16_t pf=(int16_t)read16(PowerF); if(pf&0x8000) pf=-(pf&0x7FFF); return ((double)pf)/1000.0; }
double ATM90E26::getFreq() { return ((double)read16(Freq))/100.0; }
double ATM90E26::getPhaseAngle() { return (int16_t)read16(Pangle); }

double ATM90E26::getForwardActiveEnergy() { return read16(APenergy); }
double ATM90E26::getReverseActiveEnergy() { return read16(ANenergy); }
double ATM90E26::getAbsoluteActiveEnergy() { return read16(ATenergy); }
double ATM90E26::getForwardReactiveEnergy() { return read16(RPenergy) * 0.1; }
double ATM90E26::getReverseReactiveEnergy() { return read16(RNenergy) * 0.1; }
double ATM90E26::getAbsoluteReactiveEnergy() { return read16(RTenergy) * 0.1; }

// ------------------- Checksums -------------------
uint16_t ATM90E26::calculateCS1() {
uint16_t regs[11] = {
read16(0x21), read16(0x22), read16(0x23), read16(0x24), read16(0x25),
read16(0x26), read16(0x27), read16(0x28), read16(0x29), read16(0x2A), read16(0x2B)};
uint8_t sum = 0, xorv = 0;
for (int i=0;i<11;i++){ uint8_t h=(regs[i]>>8)&0xFF,l=regs[i]&0xFF; sum+=h+l; xorv^=h^l; }
return (uint16_t)((xorv<<8)|sum);
}

uint16_t ATM90E26::calculateCS2() {
uint16_t regs[10] = {
read16(0x31), read16(0x32), read16(0x33), read16(0x34), read16(0x35),
read16(0x36), read16(0x37), read16(0x38), read16(0x39), read16(0x3A)};
uint8_t sum = 0, xorv = 0;
for (int i=0;i<10;i++){ uint8_t h=(regs[i]>>8)&0xFF,l=regs[i]&0xFF; sum+=h+l; xorv^=h^l; }
return (uint16_t)((xorv<<8)|sum);
}
