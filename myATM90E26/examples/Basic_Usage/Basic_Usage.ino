#include <myATM90E26.h>

ATM90E26 meter;

void setup() {
  Serial.begin(115200);
  meter.begin();
}

void loop() {
  Serial.println("==== Readings ====");
  Serial.print("Voltage: "); Serial.println(meter.getVoltage(), 2);
  Serial.print("Current: "); Serial.println(meter.getCurrent(), 3);
  Serial.print("Power: ");   Serial.println(meter.getActivePower(), 2);
  Serial.print("PF: ");      Serial.println(meter.getPowerFactor(), 3);
  Serial.print("Freq: ");    Serial.println(meter.getFrequency(), 2);

  delay(1000);
}
