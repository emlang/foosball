/*
 

 */

//Pin connected to latch pin (ST_CP) of 74HC595
const int latchPin = 8;
//Pin connected to clock pin (SH_CP) of 74HC595
const int clockPin = 9;
////Pin connected to Data in (DS) of 74HC595
const int dataPin = 10;

// pins for buttons/switches/goal signals
const int leftGoalPin = 4;
const int rightGoalPin = 2;
const int undoPin = 7;

const int segmentA = 1;
const int segmentB = 128;
const int segmentC = 64;
const int segmentD = 32;
const int segmentE = 16;
const int segmentF = 8;
const int segmentG = 4;
const int segmentDP = 128;

const int NONE = 0;
const int LEFT = 1;
const int RIGHT = 2;

int leftScore = 0;
int rightScore = 0;

const long DEBOUNCE_DELAY = 50;
const long RESET_HOLD = 2000;

int lastGoal = NONE;

int numbers[] = { segmentA + segmentB + segmentC + segmentD + segmentE + segmentF,
                  segmentB + segmentC,
                  segmentA + segmentB + segmentD + segmentE + segmentG,
                  segmentA + segmentB + segmentC + segmentD + segmentG,
                  segmentB + segmentC + segmentG + segmentF,
                  segmentA + segmentC + segmentD + segmentF + segmentG,
                  segmentA + segmentC + segmentD + segmentE + segmentF + segmentG,
                  segmentA + segmentB + segmentC,
                  segmentA + segmentB + segmentC + segmentD + segmentE + segmentF + segmentG, 
                  segmentA + segmentB + segmentC + segmentD + segmentF + segmentG
                };

void setup() {
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  
  pinMode(leftGoalPin, INPUT);
  pinMode(rightGoalPin, INPUT);
  pinMode(undoPin, INPUT);
  Serial.begin(9600);
}

boolean leftGoalTriggered() {
  static unsigned long startTime = 0;
  static boolean state = LOW;
  static boolean triggeredBefore = false;
  
  return switchTriggered(leftGoalPin, state, triggeredBefore, startTime);
}

boolean rightGoalTriggered() {
  static unsigned long startTime = 0;
  static boolean state = LOW;
  static boolean triggeredBefore = false;

  return switchTriggered(rightGoalPin, state, triggeredBefore, startTime);
}

boolean undoTriggered() {
  static unsigned long startTime = 0;
  static boolean state = LOW;
  static boolean triggeredBefore;
  
  if (switchTime(undoPin, state, startTime) == 0 && triggeredBefore) {
    triggeredBefore = false;
    return true;
  }
  if (triggeredBefore) {
    return false;
  }
  if (switchTime(undoPin, state, startTime) > DEBOUNCE_DELAY && !triggeredBefore) {
    triggeredBefore = true;
    return false;
  }
}

long undoTime() {
  static unsigned long startTime = 0;
  static boolean state = LOW;
  
  return switchTime(undoPin, state, startTime);
}

boolean switchTriggered(int pin, boolean &state, boolean &triggeredBefore, unsigned long &startTime) {
  if (switchTime(pin, state, startTime) == 0) {
    triggeredBefore = false;
    return false;
  }
  if (triggeredBefore) {
    return false;
  }
  if (switchTime(pin, state, startTime) > DEBOUNCE_DELAY && !triggeredBefore) {
    triggeredBefore = true;
    return true;
  }
}

boolean resetTriggered() {
  static boolean resetInProgress = false;
  
  if (undoTime() == 0) {
    resetInProgress = false;
  }
  
  if (undoTime() > RESET_HOLD && !resetInProgress) {
    resetInProgress = true;
    leftScore = 0;
    rightScore = 0;
    lastGoal = NONE;
    Serial.println("Reset!");
    showScore();
   }
   return resetInProgress;
}

long switchTime(int pin, boolean &state, unsigned long &startTime) {
  if (digitalRead(pin) != state) {// check to see if the switch has changed state
    state = !state;       // yes, invert the state
    startTime = millis();  // store the time
  }
  if (state == HIGH) {
    return millis() - startTime;   // switch pushed, return time in milliseconds
  } else {
    return 0; // return 0 if the switch is not pushed
  }
}

void showScore() {
  Serial.print(leftScore, DEC);
  Serial.print(":");
  Serial.println(rightScore, DEC);
}

void loop() {
  if (leftGoalTriggered()) {
    leftScore = (leftScore + 1) % 10;
    lastGoal = LEFT;
    showScore();
  }
  
  if (rightGoalTriggered()) {
    rightScore = (rightScore + 1) % 10;
    lastGoal = RIGHT;
    showScore();
  }
  

  if (!resetTriggered() && undoTriggered()) {
    if (lastGoal == LEFT) {
      leftScore--;
      if (leftScore < 0) {
        leftScore += 10;
      }
      Serial.println("Undo!");
      showScore();
    } else if (lastGoal == RIGHT) {
      rightScore = rightScore--;
      if (rightScore < 0) {
        rightScore += 10;
      }
      Serial.println("Undo!");
      showScore();
    }
    lastGoal = NONE;
  }

  registerWrite(numbers[rightScore], numbers[leftScore]);
}

// This method sends 2x 8bits to the shift registers:
void registerWrite(byte bitsToSend1, byte bitsToSend2) {
  // turn off the output so the pins don't light up
  // while you're shifting bits:
  digitalWrite(latchPin, LOW);

  // shift the bits out:
  shiftOut(dataPin, clockPin, MSBFIRST, bitsToSend1);
  shiftOut(dataPin, clockPin, MSBFIRST, bitsToSend2);

    // turn on the output so the LEDs can light up:
  digitalWrite(latchPin, HIGH);
}
