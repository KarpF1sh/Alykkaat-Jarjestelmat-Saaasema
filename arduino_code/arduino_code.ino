// include the library code:
#include <LiquidCrystal.h>
#include <TimerOne.h>

int keystate = 0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int padPins[4] = {14, 15, 16, 17};
enum states { wind, humidity, test1, test2 };
states States = wind; // Dafault state

volatile byte puls = 0;   // interrupt pulse count
volatile byte i_time = 0; 
volatile byte freq = 0;   // Calculated frequency

// Seconds 
const int interval = 5;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++){
     pinMode(padPins[i], INPUT_PULLUP);
  }

  //pinMode(2, INPUT_PUDOWN); 
  attachInterrupt(digitalPinToInterrupt(2), isrCount, FALLING);
  
  Timer1.initialize(500000); // 500 000 us = 0.5s
  Timer1.attachInterrupt(timerInt);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void loop() {

  if (digitalRead(17) == LOW) {
    // 1 Key
    States = wind;

  } else if (digitalRead(16) == LOW){
    // 2 Key
    States = humidity;

  } else if (digitalRead(15) == LOW){
    // 3 Key
    States = test1;

  } else if (digitalRead(14) == LOW){
    // A key
    States = test2;
  }

  switch (States) {
    case wind:
      lcd.setCursor(0, 0);
      lcd.print("Wind speed:     ");
      lcd.setCursor(0, 1);
      lcd.print(0.6344 * freq + 0.2493); // Calculate wind speed in m/s
      break;

    case humidity:
      lcd.setCursor(0, 0);
      lcd.print("Humidity:       ");
      break;

    case test1:
      lcd.setCursor(0, 0);
      lcd.print("Amogus          ");
      break;

    case test2:
      lcd.setCursor(0, 0);
      lcd.print("SUS             ");
      break;
  }
  
  //Serial.println(keyVal);
  /*
  if (key != '-') {
    //Serial.println(keyVal);
    //lcd.print(1);
    //Serial.println(key);
    lcd.setCursor(16, 0);
    lcd.print(key);
  }*/
  
}

// interrupt subroutine
void isrCount()
{
  puls++;
}

// timer subroutine
void timerInt(){
  i_time++; // increment every 0.5s

  if (i_time > 9){
    i_time = 0;
    freq = puls / interval; // calculate freq by puls / seconds

    puls = 0; // Reset 
  }
}