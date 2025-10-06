#include <LiquidCrystal.h>
#include <IRremote.hpp>
#include <PulseSensorPlayground.h>

//RS - any pin
//E - any pin
//D4 - any pin
//D5 - any pin
//D6 - any pin
//D7 - any pin
//RW - GND
//VSS - GND
//VDD - 5v
//VO - middle of potentiometer, other ends to 5v and GND

// LiquidCrystal(rs, e, d4, d5, d6, d7)
LiquidCrystal lcd(7, 8, 12, 13, 11, 6);

const int green = 10;
const int yellow = 9;
const int red = 4;

const int buzzer = 3;

const int RECV_PIN = 2;//ir pin
bool systemOn = false;//for IR

//pulsesensor
const int PULSE_INPUT = A0;
const int PULSE_BLINK = LED_BUILTIN;//flashes on heartbeat
const int PULSE_FADE = 5;//can remove
const int THRESHOLD = 300;//noise cutoff, 500 might be better?

PulseSensorPlayground pulseSensor;

void setup() {
  Serial.begin(115200);//increase for pulsesensor

  pinMode(buzzer, OUTPUT); //buzzer

  pinMode(red, OUTPUT);//leds
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);

  IrReceiver.begin(RECV_PIN, DISABLE_LED_FEEDBACK);
  Serial.println("IR Receiver ready");

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for IR...");

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);
  pulseSensor.setThreshold(THRESHOLD);

  if (pulseSensor.begin()) {
    Serial.println("PulseSensor initialized");
  } else {
    Serial.println("PulseSensor failed!");
  }
}

void loop() {
  //start w/master ir signal
  if (IrReceiver.decode()) {
    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Code received: 0x");
    Serial.println(code, HEX);

   //toggle state with each button press
    if (code == 0xBA45FF00) {
      systemOn = !systemOn;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(systemOn ? "System ON       " : "System OFF      ");//ternary op, can change to if else for readibility

      if (!systemOn){
        digitalWrite(buzzer, LOW);
        digitalWrite(green, LOW);
        digitalWrite(yellow, LOW);
        digitalWrite(red, LOW);

        lcd.setCursor(0, 1);
        lcd.print("                "); // Clear BPM line
      }
    }

    IrReceiver.resume();
  }


  if (systemOn) {
    if (pulseSensor.sawStartOfBeat()) {
      int bpm = pulseSensor.getBeatsPerMinute();

      Serial.print("BPM: ");
      Serial.println(bpm);

      lcd.setCursor(0, 1);
      lcd.print("BPM: ");
      lcd.print(bpm);
      if (bpm > 65 && bpm <90){
        digitalWrite(buzzer, LOW);
        digitalWrite(green, HIGH);
        digitalWrite(yellow, LOW);
        digitalWrite(red, LOW);
      }else if ((bpm >= 60 && bpm <=65) || (bpm >=90 && bpm <= 100)){
        digitalWrite(buzzer, LOW);
        digitalWrite(yellow, HIGH);

        digitalWrite(red, LOW);
        digitalWrite(green, LOW);
      }else if (bpm < 60 || bpm > 105){
        digitalWrite(buzzer, HIGH);
        digitalWrite(red, HIGH);
        digitalWrite(yellow, LOW);
        digitalWrite(green, LOW);
      }
      lcd.print("    ");//remove old digits
    }
  }
}
