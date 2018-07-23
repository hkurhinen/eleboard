#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10
#define RADIO_LED_PIN LED_BUILTIN
#define ESC_SIGNAL_PIN 3
#define ESC_VCC_PIN 5
#define ESC_LOW_PULSE 1000
#define ESC_MID_PULSE 1500
#define ESC_HIGH_PULSE 2000
#define ARM_PULL_HIGH_DELAY 900
#define ARM_HIGH_PULSE_WIDTH 100
#define DISCONNECT_DELAY 500

const uint64_t pipe = 0xE8E8F0F0E1LL;

RF24 radio(CE_PIN, CSN_PIN);

int currentSpeed = ESC_MID_PULSE;
int joystick[2];
Servo esc;
boolean connectionLost = false;
boolean armed = false;
boolean arming = false;
unsigned long connectionLostTime;
unsigned long armStartTime;

void setup() {
  pinMode(ESC_VCC_PIN, OUTPUT);
  pinMode(RADIO_LED_PIN, OUTPUT);
  digitalWrite(RADIO_LED_PIN, LOW);
  digitalWrite(ESC_VCC_PIN, HIGH);
  esc.attach(ESC_SIGNAL_PIN);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();
}

void loop() {
  if (!armed) {
    if (!arming) {
      arming = true;
      armStartTime = millis();
    } else {
      unsigned long currentTime = millis();
      if(currentTime - armStartTime > ARM_PULL_HIGH_DELAY) {
        currentSpeed = ESC_HIGH_PULSE;
        digitalWrite(RADIO_LED_PIN, HIGH);
      }
      
      if (currentTime - armStartTime > (ARM_PULL_HIGH_DELAY + ARM_HIGH_PULSE_WIDTH)) {
        currentSpeed = ESC_MID_PULSE;
        digitalWrite(RADIO_LED_PIN, LOW);
        armed = true;
      }
    }
  } else {
    if(radio.available()) {
      digitalWrite(RADIO_LED_PIN, HIGH);
      connectionLost = false;
      bool done = false;
      while (!done) {
        done = radio.read( joystick, sizeof(joystick) );
        currentSpeed = map(joystick[0], 0, 1023, ESC_LOW_PULSE, ESC_HIGH_PULSE);
      }
    } else {
      if(!connectionLost) {
        connectionLost = true;
        connectionLostTime = millis();
      } else {
        if(millis() - connectionLostTime > DISCONNECT_DELAY) {
          currentSpeed = ESC_MID_PULSE;
        }
      }
    }
  }
  esc.writeMicroseconds(currentSpeed);
  delay(5);
}