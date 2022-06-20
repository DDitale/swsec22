const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = LOW;
int counter = 0;
unsigned long last_counter = 0;
int pin_da_alzare ;
unsigned long current_time = 0;
int tempo = 0; 


void setup() {

  pinMode(ledPin, OUTPUT);
  for (int i = 7 ; i < 13; i++) {
    pinMode(i, OUTPUT);
  }
  pinMode(interruptPin, INPUT_PULLUP);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
}

void loop() {
  //digitalWrite(ledPin, state);
  if(tempo < 5){
  current_time = millis();
  current_time += 10;
  if ( (counter > 0) && ( (current_time - last_counter) > 250)) {
    Serial.println(millis() - last_counter);
    pin_da_alzare = counter + 6;
    counter = 0;
    Serial.println(pin_da_alzare - 6);
    digitalWrite(pin_da_alzare, HIGH);
    delay(25);
    digitalWrite(pin_da_alzare, LOW);

      tempo += 1;
   }
  }
}

void blink() {
    counter += 1;
    last_counter = millis();
}