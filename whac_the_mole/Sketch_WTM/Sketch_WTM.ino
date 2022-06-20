#include <SoftwareSerial.h>

const int NBUT = 7;
const int NPIN = 11;
int counter = 0;
unsigned long last_counter = 0;
unsigned long current_time = 0;
unsigned long last_hit = 0;

bool found[NBUT];

unsigned short int previous_button = 0;

int state = 0;

char temp = '>';
String msg;

SoftwareSerial mySerial(8, 9);


int button[NBUT];
int pin[NPIN];


void press_mole(int mole_pin) {
  //Simula la pressione della talpa mole_pin
  int _pin = pin[mole_pin];
  if(_pin == -1){
    Serial.println("HARAKIRI");
    Serial.print(mole_pin);
    Serial.print(" --> ");
    Serial.println(pin[mole_pin]);
    return;
  }
  digitalWrite(_pin, HIGH);
  delay(50);
  digitalWrite(_pin, LOW);
  last_hit = millis();
}


bool is_invalid(int mole_pin){
  //Verifica se il tasto è errato o già usato
  if(pin[mole_pin] == -1){
    return true;
  }
  for(int i = 1; i < NBUT; i++){
    if(found[i] && mole_pin == button[i]){
      return true;
    } 
  }
  return false;
}


void test_pin(int mole_pos) {
  //Se la posizione non è nota, cerca la prossima
  //Premi il pin alla posizione scelta
  if (found[mole_pos] == false) {
    Serial.print(button[mole_pos]);
    button[mole_pos] += 1;
    while(is_invalid(button[mole_pos])){
      Serial.println(" INVALID");
      button[mole_pos] += 1;
      Serial.print(button[mole_pos]);
    }
    Serial.println(" valid");
    button[mole_pos] = button[mole_pos] % NPIN;
  }
  Serial.print("Provo il pin ");
  Serial.print(button[mole_pos]);
  Serial.print(" per la talpa ");
  Serial.println(mole_pos);
  press_mole(button[mole_pos]);
}


void setup() {
  for (int i = 0; i < NBUT ; i++ ) {
    button[i] = -1;
    found[i] = false;
  }
  for (int i = 0 ; i < 6 ; i++) {
    pin[i] = i + 2;
  }

 for(int i = 6; i < NPIN; i++){
    pin[i] = i+8;
  }

  for (int i = 0; i < NPIN; i++) {
    pinMode(pin[i], OUTPUT);
    digitalWrite(pin[i], LOW);
  }

  pinMode(13, INPUT);
  Serial.begin(115200);

  mySerial.begin(19200);

  while (mySerial.available() && temp != '>') {
    temp = mySerial.read();
  }

  Serial.println("Pronto per l'uso");

}

void loop() {
  while (mySerial.available()) {
    temp = mySerial.read();
    msg += temp;
  }

  if (msg.indexOf("You missed it") >= 0 || msg.indexOf(" please step into the yard by pressing") >= 0 ) {
    mySerial.write("\r");

    unsigned long delta = millis() - last_hit;
    Serial.print("Fallito dopo ");
    Serial.print(delta);
    Serial.println(" ms");
    if(delta > 5000){
      pin[button[previous_button]] = -1;
      //Escludiamo il pin dai possibili
    }
    msg = "";
  } else if (msg.indexOf("You whacked it") >= 0) {
    if (!found[previous_button]) {
      found[previous_button] = found;
      Serial.print(">\tTrovata la talpa ");
      Serial.print(previous_button)
      Serial.print(" in posizione ");
      Serial.println(button[previous_button]);  
    }
    msg = "";
  }



  //Leggo lo stato del led
  int value = digitalRead(13);
  if (value != state) {
    state = value;
    // Conta solo i fronti di salita
    if (state == HIGH) {
      last_counter = millis();
      counter++;
    }
  }

  if (last_counter && last_counter + 103 < millis() && state == LOW) {
    previous_button = counter;
    test_pin(counter);
    last_counter = 0;
    counter = 0;
  }

}