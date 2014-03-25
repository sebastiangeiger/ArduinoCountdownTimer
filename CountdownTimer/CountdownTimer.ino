// The pins
int latchPin         = 8; //green
int dataPin          = 11;//blue
int clockPin         = 12;//white
int potentioMeterPin = 5; //yellow
int buttonPin        = 2; //black

// The state of the clock
typedef enum clock_state_t {SETUP, RUNNING, ALARM} clock_state_t;
clock_state_t clockState = SETUP;

int buttonState = LOW;

int desiredMinutes = 0; //Represents the state of poti
int desiredMinutesForRun = 0; //Hold the state while countdown is running
unsigned long startTime = 0; //Is set when transitioning from SETUP to RUNNING

//Blinking without using delay() to keep the button reactive, see blink()
boolean blinkOn = false;
unsigned long blinkTime = 0;


//This function is the only function allowed to write to the LEDs
//It is doing so by writing to the shift register and the triggering the latch
void displayNumber(int number){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, number);
  digitalWrite(latchPin, HIGH);
}

//For debugging purposes
void serialPrintNumber(unsigned long number){
  char buf[30];
  Serial.println(ultoa(number,buf,10));
}

//A nice effect when starting the board
void signalStartup(int speed){
  int number = 0;
  //Blink every LED from left to right
  for(int i=7; i>=0; i--){
    number = (1<<i); //2^i
    displayNumber(number);
    delay(speed);
  }
  //"Fill up" from right to left
  for(int i=1; i<=8; i++){
    number = (1<<i)-1; //2^i-1
    displayNumber(number);
    delay(speed);
  }
  delay(10*speed); //Show the full bar for a little longer
}

//Blinks the LEDs without using delay()
void blink(){
  //Adjust the state if necessary
  if(blinkTime == 0 || blinkTime + 100 < millis()){
    blinkOn = !blinkOn;
    blinkTime = millis();
  }
  //Now turn all LEDs either on or off
  if(blinkOn){
    displayNumber(255);
  } else {
    displayNumber(0);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(dataPin , OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  signalStartup(50);
  Serial.println("Setup Done");
}

void loop() {
  //Read the button and determine if it was pressed
  int oldButtonState = buttonState;
  buttonState = digitalRead(buttonPin);
  bool pressed = (oldButtonState == LOW && buttonState == HIGH);
  //Adjust the clockState depending on if the button was pressed
  if(pressed && clockState == SETUP){
    // Transition from SETUP to RUNNING
    startTime = millis();
    // Stash away the current state of the poti
    desiredMinutesForRun = desiredMinutes;
    clockState = RUNNING;
    Serial.println("RUNNING");
  } else if (pressed) {
    // Transition from (RUNNING or ALARM) to SETUP
    clockState = SETUP;
    Serial.println("SETUP");
  }
  //Now act on the current clockState
  if(clockState == SETUP){
    desiredMinutes = analogRead(potentioMeterPin) / 4;
    displayNumber(desiredMinutes);
  }
  else if (clockState == RUNNING) {
    unsigned long elapsedSeconds = (millis() - startTime)/1000;
    unsigned long remainingSeconds= (desiredMinutesForRun * 60 - elapsedSeconds);
    if(remainingSeconds <= 0){
      //Transition from RUNNING to ALARM
      clockState = ALARM;
      Serial.println("ALARM");
    } else {
      //Remain in RUNNING state, update the minutes displayed
      serialPrintNumber(elapsedSeconds);
      serialPrintNumber(remainingSeconds);
      float remainingMinutes = ((float) remainingSeconds)/60.0;
      displayNumber(ceil(remainingMinutes));
    }
  }
  else if(clockState == ALARM){
    blink();
  }
}
