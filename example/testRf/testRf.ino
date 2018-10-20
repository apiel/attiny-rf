// ATTINY85 example

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
  rfReceiver.init([](char * result){
    Serial.print("callback: ");
    Serial.println(result);
  });
}

void loop() {
  int pinValue = digitalRead(pinReceive);
  if (pinValue != lastPinValue) {
    lastPinValue = pinValue;
    rfReceiver.onInterrupt();
  }
}