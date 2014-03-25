int latchPin = 8; //green
int dataPin  = 11; //blue
int clockPin = 12;  //white
int potentioMeterPin = 5; //yellow
int buttonPin = 2; //black

int desiredMinutes = 0;
unsigned long endTime = 0;

typedef enum alarm_state_t {SETUP, RUNNING, ALARM} alarm_state_t;

alarm_state_t alarmState = SETUP;
int buttonState = LOW;

void writeNumber(int number){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, number);
  digitalWrite(latchPin, HIGH);
}

void serialPrintNumber(int number){
  char buf[10];
  Serial.println(itoa(number,buf,10));
}

void signalStartup(int speed){
  int number = 0;
  for(int i=7; i>=0; i--){
    number += (1<<i); //2^i
    writeNumber(number);
    delay(speed);
  }
 writeNumber(0);
 delay(speed);
 writeNumber(255);
 delay(speed*10);
 writeNumber(0);
}

void blink(){
 writeNumber(255);
 delay(100);
 writeNumber(0);
 delay(100);
}

void setup() {
  pinMode(dataPin , OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Setup Start");
  signalStartup(50);
  Serial.println("Setup End");
}

void loop() {
  int oldButtonState = buttonState;
  buttonState = digitalRead(buttonPin);
  bool pressed = (oldButtonState == LOW && buttonState == HIGH);
  if(pressed && alarmState == SETUP){
    endTime = millis() + 1000 * desiredMinutes;
    alarmState = RUNNING;
  } else if (pressed) {
    alarmState = SETUP;
  }
  if(alarmState == SETUP){
    desiredMinutes = analogRead(potentioMeterPin) / 4;
    writeNumber(desiredMinutes);
  }
  else if (alarmState == RUNNING) {
    unsigned long currentTime = millis();
    if(currentTime >= endTime){
      alarmState = ALARM;
    } else {
      unsigned long remainingTime = (endTime - currentTime) / 1000;
      writeNumber(remainingTime);
    }
  }
  else if(alarmState == ALARM){
    blink();
  }
}



