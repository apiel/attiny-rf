// ATTINY85 example
// digipark default 16.5mhz

#include <SoftSerial.h>
#include <TinyPinChange.h>

#include <RfReceiver.h>

#define RX    2
#define TX    3

SoftSerial mySerial(RX, TX);

int pinReceive = 0;
int lastPinValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Start receiving");

  pinMode(pinReceive, INPUT);
}

void loop() {
  char * result;
  int pinValue = digitalRead(pinReceive);
  if (pinValue != lastPinValue) {
    lastPinValue = pinValue;
    rfReceiver.onInterrupt();
  }
}
