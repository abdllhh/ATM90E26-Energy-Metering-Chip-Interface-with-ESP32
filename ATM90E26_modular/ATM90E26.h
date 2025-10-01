#ifndef ATM90E26_H
#define ATM90E26_H

#include <Arduino.h>
#include <SPI.h>

// ------------------- Registers -------------------
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
#define Ngain     0x25
#define Nphi      0x26
#define PStartTh  0x27
#define PNolTh    0x28
#define QStartTh  0x29
#define QNolTh    0x2A
#define MMode     0x2B
#define CSOne     0x2C

#define AdjStart  0x30
#define Ugain     0x31
#define IgainL    0x32
#define IgainN    0x33
#define Uoffset   0x34
#define IoffsetL  0x35
#define IoffsetN  0x36
#define PoffsetL  0x37
#define QoffsetL  0x38
#define PoffsetN  0x39
#define QoffsetN  0x3A
#define CSTwo     0x3B

#define APenergy  0x40
#define ANenergy  0x41
#define ATenergy  0x42
#define RPenergy  0x43
#define RNenergy  0x44
#define RTenergy  0x45
#define EnStatus  0x46

#define Irms      0x48
#define Urms      0x49
#define Pmean     0x4A
#define Qmean     0x4B
#define Freq      0x4C
#define PowerF    0x4D
#define Pangle    0x4E
#define Smean     0x4F
#define Irms2     0x68



// ------------------- Class Wrapper -------------------
class ATM90E26 {
public:
ATM90E26(uint8_t csPin, uint8_t sck, uint8_t miso, uint8_t mosi);


void begin();
void init();

double getVoltage();
double getCurrent();
double getActivePower();
double getReactivePower();
double getApparentPower();
double getPF();
double getFreq();
double getPhaseAngle();

double getForwardActiveEnergy();
double getReverseActiveEnergy();
double getAbsoluteActiveEnergy();
double getForwardReactiveEnergy();
double getReverseReactiveEnergy();
double getAbsoluteReactiveEnergy();

uint16_t calculateCS1();
uint16_t calculateCS2();


private:
SPIClass* hspi;
uint8_t csPin;
SPISettings settings;


uint16_t read16(uint8_t addr7);
void write16(uint8_t addr7, uint16_t value);


};

#endif
