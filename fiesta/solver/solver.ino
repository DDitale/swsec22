#include <SoftwareSerial.h>

int pinAttack = 11;

unsigned int uiInterval = 0;

unsigned int uBrute = 1;
unsigned int uDelta = 1;
unsigned long timeDelta = 100;

long randomValue;

unsigned short int counter = 0;

unsigned long tPrevious = 0;

SoftwareSerial sTarget(2, 3);

String targetMessage = "";
char cRead;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  sTarget.begin(19200);

  pinMode(pinAttack, OUTPUT);
  digitalWrite(pinAttack, HIGH);

  randomSeed(analogRead(0));

}

void loop() {
  // put your main code here, to run repeatedly:
  while (sTarget.available() > 0) {
    cRead = sTarget.read();

    targetMessage += cRead;
  }
  if ((targetMessage.length()) >= 7) {
    Serial.println(targetMessage);
  }

  if (targetMessage.indexOf("Lock") >= 0) {
    targetMessage = "";
  }

  if (Serial.available() > 0) {
    uiInterval = Serial.parseInt();
  }


  if (uiInterval != 0) {
    digitalWrite(pinAttack, LOW);
    delayMicroseconds(uiInterval);
    digitalWrite(pinAttack, HIGH);
    delay(1);
    for(int i = 0; i < 100; i++){
    digitalWrite(pinAttack, LOW);
    delayMicroseconds(1);
    digitalWrite(pinAttack, HIGH);e
    delayMicroseconds(1);
    }
    uiInterval = 0;
    tPrevious = millis();
  }

  /*

  if ((tPrevious + timeDelta) < millis()) {
    tPrevious = millis();
    Serial.print("Provo con ");
    Serial.print(uBrute);
    Serial.print(" microsecondi ");
    Serial.print(counter + 1);
    Serial.print("/20");
    for (int i = 0; i < 10; i ++) {
      digitalWrite(pinAttack, LOW);
      delayMicroseconds(uBrute);
      digitalWrite(pinAttack, HIGH);
    }
    Serial.println(" -> OK");
    counter += 1;
    if (counter >= 20) {
      counter = 0;
      //uBrute += uDelta;
    }
    randomValue = int(random(100));
    delay(randomValue);

  }
  */


}
