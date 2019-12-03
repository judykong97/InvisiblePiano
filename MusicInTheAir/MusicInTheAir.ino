/*========================================================*/
/*
  
  Final Project "Music in the Air"
  by Judy Kong, HCII CMU. Dec 3, 2019.


  This is the final project for 05-833 Applied Gadgets, Sensors and
  Activity Recognition in HCI. 
  
  Hardware
  This code assumes eight finger rings connected to analog pins 0-7, 
  and two thumb rings connected to groud. 
  The speaker is assumed to be connected to digital I/O pin 2, and 
  should have a current limiting resistor. 

  Operation
  The fingers from left to right (analog pins 0-7) each represent 
  a key to be played. To play the key, touch the end of wire outside
  the finger with the copper film at the top of the thumb ring. 
  
  
*/
/*========================================================*/

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

/*------- Hardware configuration -------*/
const int  threshold = 10;      // Voltage below this value is considered ground
const int  numKeys = 8;         // Number of keys
const int  potPins[] = {A0, A1, A2, A3, A4, A5, A6, A7}; // eight keys
const int  smoothingN = 5;      // Length of smoothing vector
int  smoothing[] = {-1, -1, -1, -1, -1}; // Smoothing vector
int  currIdx = -1;              // Currently played key after smoothing
// Regular sequence starting with C note
const int  notes_C[] = {NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7}; 
// Demo sequence starting with G note
const int  notes[] = {NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6, NOTE_G6};   
const int  buzzer = 2;          // Pin number for buzzer 
const byte debugPin = 13;       // Pin that we put debug output on (set to 255 to disable)
                                // (most Arduino's have a built in LED on pin 13...)

/*-------------------------------------------*/
/* Initializization code */
void setup() {
  Serial.begin(9600);
  Serial.println("Hello world!"); // For debugging
  pinMode(debugPin, OUTPUT);
}

/* Main routine */
void loop() {
  // Find the index of the key currently pressed
  currIdx = findCurr();
  // Debugging: current index of key pressed after smoothing
  Serial.print(currIdx);
  Serial.print(", ");
  Serial.println(analogRead(potPins[currIdx]));
  // If confident that a key has been pressed
  if (currIdx >= 0) {
    // Play the tone for 0.1s
    tone(buzzer, notes[currIdx], 100);
    // Wait for 0.05s before playing the next note to avoid collision
    delay(50);
    // Stop the waveform generation before the next note.
    noTone(buzzer);
  }
}

/* Finds the currently pressed key and do smoothing to prevent jumping around.  
 * Returns the index of the key identified as "pressed" after smoothing, and 
 * return 0 if no such key is found. */
int findCurr() {
  // If the current key keeps a low voltage, consider it as still pressed 
  if (currIdx >= 0 && analogRead(potPins[currIdx]) <= threshold) {
    return currIdx;
  }
  // Otherwise, add to the smoothing vector
  for (int i = 0; i < smoothingN - 1; i++) {
    smoothing[i] = smoothing[i + 1];
  }
  // Add the index to the current minimum voltageto the end of smoothing vector
  smoothing[smoothingN - 1] = ground();
  // Use a max-finding algorithm to find the majority in smoothing
  return findCurrInSmoothing();
}

/* Identifies if there's a switch of mode. Reserved for changing tunes. */
bool switchMode() {
  // Press all keys to switch mode
  for (int i = 0; i < numKeys; i++) {
    // Returns false if any of the key voltages are above the threshold
    if (analogRead(potPins[i]) > threshold) return false;
  }
  // Otherwise, return true
  return true;
}

/* Finds out the index to the minimum voltage without smoothing */
int ground() {
  // Set default to idx 0
  int idx = 0;
  int minRead = analogRead(potPins[0]);
  for (int i = 1; i < numKeys; i++) {
    // Find minimum
    if (analogRead(potPins[i]) < minRead) {
      idx = i;
      minRead = analogRead(potPins[i]);
    }
  }
  // Only consider it a valid key if it's less than the threshold, 
  // i.e., connected to ground
  if (minRead <= threshold) {
    return idx;
  }
  // Otherwise consider not found
  return -1;
}

/* Finds the majority in the smoothing vector,
 * Returns the index of the analog pin identified
 * as the key pressed. 
 */
int findCurrInSmoothing() {
  int count = 0;
  int idx;
  // Majority finding algorithm
  for (int i = 0; i < smoothingN; i++) {
    if (count == 0) {
      idx = i;
      count++;
    } else if (smoothing[i] == smoothing[idx]) {
      count++;
    } else {
      count--;
    }
  }
  // Only consider it a stable pressed key if the biggest 
  // group has larger than half the size of the smoothing vector  
  if (count >= smoothingN / 2) {
    return smoothing[idx];
  }
  // Otherwise consider it not found
  return -1;
}
