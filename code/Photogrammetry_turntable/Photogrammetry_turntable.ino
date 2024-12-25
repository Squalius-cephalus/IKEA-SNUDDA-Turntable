#include <Stepper.h>
#include "pio_encoder.h"
// Uses https://github.com/gbr1/rp2040-encoder-library library for encoder

const int stepsPerRevolution = 2048;
const int shutterSpeed = 300;        // Steps per iteration
const int stabilisationDelay = 100;  // Steps per iteration
const int TotalStepsForRevolution = 23296;
const int imageCount = 32;
// Big gear has 182 teeth, little gear 16. 23296 steps for one revolution

// Pin GPIO8 to IN1 on the ULN2003 driver
// Pin GPIO9 to IN2 on the ULN2003 driver
// Pin GPIO10 to IN3 on the ULN2003 driver
// Pin GPIO11 to IN4 on the ULN2003 driver
Stepper StepperMotor = Stepper(stepsPerRevolution, 8, 10, 9, 11);
PioEncoder encoder(2);  // encoder is connected to GPIO2 and GPIO3

void setup() {
  encoder.begin();
  StepperMotor.setSpeed(9);  // Set the speed 1-9 rpm:
  pinMode(1, INPUT_PULLUP);  // Encoder switch pin
  pinMode(0, OUTPUT);        // Relay pin for camera control
  Serial.begin(9600);
}
int encoderLastState = encoder.getCount();
bool isButtonStillPressed = false;
void loop() {
  if (digitalRead(1) == HIGH) {
    isButtonStillPressed = false;
  }
  delay(10);
  if (encoderLastState < encoder.getCount()) {
    StepperMotor.step(-50);
  }
  if (encoderLastState > encoder.getCount()) {
    StepperMotor.step(50);
  }
  encoderLastState = encoder.getCount();

  if (digitalRead(1) == LOW && isButtonStillPressed == false) {
    isButtonStillPressed = true;
    delay(500);
    if (digitalRead(1) == LOW) {
      Serial.println("Video mode, hold button to stop");
      while (true) {
        if (digitalRead(1) == HIGH) {
          isButtonStillPressed = false;
        }
        StepperMotor.step(100);
        if (digitalRead(1) == LOW && isButtonStillPressed == false) {
          Serial.println("Turntable stopped!");
          isButtonStillPressed = true;
          delay(100);
          break;
        }
      }

    } else {

      Serial.println("Photo mode, hold button to stop");

      for (int i = 0; i < imageCount; ++i) {
        digitalWrite(0, HIGH);
        delay(50);  // Shutter activation
        digitalWrite(0, LOW);
        delay(shutterSpeed);
        StepperMotor.step(TotalStepsForRevolution / imageCount);
        delay(stabilisationDelay);

        if (digitalRead(1) == LOW) {
          Serial.println("Turntable stopped!");
          String PicturesTaken = "Pictures taken: ";
          String PicturesTakenPrint = PicturesTaken + (i + 1);
          Serial.println(PicturesTakenPrint);
          isButtonStillPressed = true;
          delay(100);
          break;
        }
      }
    }
  }
}
