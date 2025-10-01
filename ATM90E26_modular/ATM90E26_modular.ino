#include "ATM90E26.h"

ATM90E26 meter(15, 14, 13, 12); // CS=15, SCK=14, MISO=13, MOSI=12

void setup() {
Serial.begin(115200);
meter.begin();
}

void loop() {
Serial.print("Voltage: "); Serial.println(meter.getVoltage());
Serial.println("============================================");
Serial.print("Current: "); Serial.println(meter.getCurrent());
Serial.println("============================================");
Serial.print("Freq: "); Serial.println(meter.getFreq());
Serial.print("Power Factor: "); Serial.println(meter.getPF());
Serial.println("============================================");
Serial.print("Active Power: "); Serial.println(meter.getActivePower());
Serial.print("Reactive Power: "); Serial.println(meter.getReactivePower());
Serial.print("Apparent Power: "); Serial.println(meter.getApparentPower());
Serial.println("============================================");
delay(2000);
}
