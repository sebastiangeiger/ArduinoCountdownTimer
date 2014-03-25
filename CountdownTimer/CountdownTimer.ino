// The pins
int latchPin         = 8; //green
int dataPin          = 11; //blue
int clockPin         = 12;  //white
int potentioMeterPin = 5; //yellow
int buttonPin        = 2; //black

// The state of the clock
typedef enum clock_state_t {SETUP, RUNNING, ALARM} clock_state_t;

int desiredMinutes = 0;
int desiredMinutesForRun = 0;
unsigned long startTime = 0;

boolean blinkOn = false;
unsigned long blinkTime = 0;

clock_state_t clockState = SETUP;
int buttonState = LOW;

void writeNumber(int number){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, number);
  digitalWrite(latchPin, HIGH);
}

void serialPrintNumber(unsigned long number){
  char buf[30];
  Serial.println(ultoa(number,buf,10));
}

void signalStartup(int speed){
  int number = 0;
  for(int i=7; i>=0; i--){
    number = (1<<i); //2^i
    writeNumber(number);
    delay(speed);
  }
  for(int i=1; i<=8; i++){
    number = (1<<i)-1; //2^i-1
    writeNumber(number);
    delay(speed);
  }
  delay(10*speed);
}

void blink(){
  if(blinkTime == 0 || blinkTime + 100 < millis()){
    blinkOn = !blinkOn;
    blinkTime = millis();
  }
  if(blinkOn){
    writeNumber(255);
  } else {
    writeNumber(0);
  }
}

void setup() {
  pinMode(dataPin , OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Serial.begin(9600);
  signalStartup(50);
  Serial.println("Setup End");
}

void loop() {
  int oldButtonState = buttonState;
  buttonState = digitalRead(buttonPin);
  bool pressed = (oldButtonState == LOW && buttonState == HIGH);
  if(pressed && clockState == SETUP){
    startTime = millis();
    desiredMinutesForRun = desiredMinutes;
    clockState = RUNNING;
    Serial.println("RUNNING");
  } else if (pressed) {
    clockState = SETUP;
    Serial.println("SETUP");
  }
  if(clockState == SETUP){
    desiredMinutes = analogRead(potentioMeterPin) / 4;
    writeNumber(desiredMinutes);
  }
  else if (clockState == RUNNING) {
    unsigned long elapsedSeconds = (millis() - startTime)/1000;
    unsigned long remainingSeconds= (desiredMinutesForRun * 60 - elapsedSeconds);
    if(remainingSeconds <= 0){
      clockState = ALARM;
      Serial.println("ALARM");
    } else {
      serialPrintNumber(elapsedSeconds);
      serialPrintNumber(remainingSeconds);
      float remainingMinutes = ((float) remainingSeconds)/60.0;
      writeNumber(ceil(remainingMinutes));
    }
  }
  else if(clockState == ALARM){
    blink();
  }
}

