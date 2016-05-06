#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10
#define RADIO_LED_PIN 6
#define ESC_SIGNAL_PIN 3
#define ESC_VCC_PIN 5
#define ESC_LOW_PULSE 1000
#define ESC_HIGH_PULSE 2000
#define DISCONNECT_DELAY 1000

const uint64_t pipe = 0xE8E8F0F0E1LL;

RF24 radio(CE_PIN, CSN_PIN);

int joystick[2];
Servo esc;
boolean armed = false;
boolean escConnected = false;
boolean connectionLost = false;
unsigned long connectionLostTime;

void setup() {
  Serial.begin(9600);
  pinMode(ESC_VCC_PIN, OUTPUT);
  pinMode(RADIO_LED_PIN, OUTPUT);
  digitalWrite(ESC_VCC_PIN, LOW);
  digitalWrite(RADIO_LED_PIN, LOW);
  esc.attach(ESC_SIGNAL_PIN);
  delay(1000);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();
}

void loop() {
  if(radio.available()) {
    digitalWrite(RADIO_LED_PIN, HIGH);
    connectionLost = false;
    bool done = false;
    while (!done) {
      done = radio.read( joystick, sizeof(joystick) );
      if(!escConnected) {
        connectEsc();
      }
      if(!armed) {
        arm();
      } else {
        int newSpeed =  map(joystick[0], 512, 1023, ESC_LOW_PULSE, ESC_HIGH_PULSE);
        esc.writeMicroseconds(newSpeed);
        Serial.println(newSpeed);
        //Serial.println("Setting pulse: "+newSpeed);
      }
    }
  } else {
    if(escConnected && !connectionLost) {
      connectionLost = true;
      connectionLostTime = millis();
    }
    if(escConnected && connectionLost) {
      unsigned long currentTime = millis();
      if(currentTime - connectionLostTime > DISCONNECT_DELAY) {
        Serial.println("Connection to transmitter lost");
        digitalWrite(RADIO_LED_PIN, LOW);
        if(armed) {
          esc.writeMicroseconds(ESC_LOW_PULSE);
        }
        disconnectEsc();
      }
    }
  }
}

void arm() {
  Serial.println("Arming ESC");
  esc.writeMicroseconds(ESC_LOW_PULSE);
  delay(1000);
  armed = true;
}

void connectEsc() {
  digitalWrite(ESC_VCC_PIN, HIGH);
  escConnected = true;
  delay(1000);
}

void disconnectEsc() {
  digitalWrite(ESC_VCC_PIN, LOW);
  escConnected = false;
}
