#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <PubSubClient.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

// Pins for keypad
const int padPins[4] = {14, 15, 16, 17};

// States for menu
enum states { wind, direction, test1, test2 };
states States = wind; // Default state

// Interrupt pulse count
volatile byte puls = 0;   
volatile byte i_time = 0; 
// Calculated frequency
volatile byte freq = 0;   

// Seconds 
const int interval = 5;

// Integer for wind direction
int windDir = 0;
// Float for wind speed
float windSpeed = 0;

unsigned long initialTime = millis();
bool sendMessage = false;
int sendDelay = 5000;
///////////////////////////////////////////////////////////////////

EthernetClient ethClient;

void fetch_IP(void);
static uint8_t mymac[6] = { 0x44,0x76,0x58,0x10,0x00,0x62 };

///////////////////////////////////////////////////////////////////

// MQTT Settings //

unsigned int  Port = 1883;
byte server[] = { 10,6,0,21 };

char* deviceId  = "Amogus";
char* clientId  = "abcd9876";
char* deviceSecret = "tamk";

///////////////////////

// MQTT Server Settings

void callback(char* topic, byte* payload, unsigned int length);

PubSubClient client(server, Port, callback, ethClient);

///////////////////////

// MQTT topic names

#define inTopic   "ahoi"
#define outTopic  "ICT4_out_2020"

///////////////////////

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++)
  {
     pinMode(padPins[i], INPUT_PULLUP);
  }

  //pinMode(2, INPUT_PUDOWN); 
  attachInterrupt(digitalPinToInterrupt(2), isrCount, FALLING);
  
  Timer1.initialize(500000); // 500 000 us = 0.5s
  Timer1.attachInterrupt(timerInt);
  fetch_IP();
  Connect_MQTT_server();
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

}

void loop() {
  // Calculate wind speed in m/s
  windSpeed = 0.6344 * freq + 0.249;

  // Calculate voltage and get wind direction
  windDir = WindDirection(analogRead(A4) * (5.0 / 1023.0));

  if (digitalRead(17) == LOW) {
    // 1 Key
    States = wind;

  } else if (digitalRead(16) == LOW){
    // 2 Key
    States = direction;

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
      lcd.print("Wind speed:        ");
      lcd.setCursor(0, 1);
      lcd.print(windSpeed); 
      lcd.setCursor(4, 1);
      lcd.print(" m/s           ");
      break;

    case direction:
      lcd.setCursor(0, 0);
      lcd.print("Wind direction:           ");
      lcd.setCursor(0, 1);

      if (windDir == 0) {
        lcd.print("North               ");

      } else if (windDir == 45) {
        lcd.print("Northeast               ");

      } else if (windDir == 90) {
        lcd.print("East             ");

      } else if (windDir == 135){
        lcd.print("Southgeast               ");

      } else if (windDir == 180) {
        lcd.print("South                ");

      } else if (windDir == 225) {
        lcd.print("Southwest(eastwest)             ");

      } else if (windDir == 270) {
        lcd.print("West              ");

      } else {
        lcd.print("Northwest           ");
      }
      break;

    case test1:
      lcd.setCursor(0, 0);
      lcd.print("IP:             ");
      lcd.setCursor(0, 1);
      lcd.print(Ethernet.localIP());
      lcd.setCursor(7, 1);
      lcd.print("           ");
      break;

    case test2:
      // Invert boolean
      sendMessage = !sendMessage;
      lcd.setCursor(0, 0);

      // Print sending status      
      if (sendMessage) {
        lcd.print("Sending on");
      } else {
        lcd.print("Sending off");
      }
      
      lcd.setCursor(0, 1);
      lcd.print("                   ");

      // Wait
      delay(1000);

      // Return state to wind
      States = wind;
      break; 
  }

  // If counted millis difference is over 5000 send message
  if (sendMessage && (millis() - initialTime > 5000))
  {
    // Reset reference time
    initialTime = millis();
    send_MQTT_message(windSpeed, windDir);
  }

  // If millis counter overflows reset the reference
  if (initialTime > millis()){
    initialTime = millis();
  }
}

// interrupt subroutine
void isrCount()
{
  puls++;
}

// timer subroutine
void timerInt()
{
  // increment every 0.5s
  i_time++; 

  if (i_time > 9){
    i_time = 0;

    // calculate freq by puls / seconds
    freq = puls / interval; 

    // Reset 
    puls = 0; 
  }
}

void fetch_IP(void)
{
  byte rev=1;
  lcd.setCursor(0,1);
  lcd.print("     Waiting IP     ");

  // get IP number
  rev = Ethernet.begin(mymac);
  Serial.print( F("\nW5500 Revision ") );
  
  if (rev == 0){
    Serial.println( F( "Failed to access Ethernet controller" ) );
    lcd.setCursor(0,0); lcd.print(" Ethernet failed   ");
  }

  Serial.println( F( "Setting up DHCP" ));
  Serial.print("Connected with IP: "); 
  Serial.println(Ethernet.localIP()); 

  lcd.setCursor(0,1);
  lcd.print("                     ");
  
  lcd.setCursor(0,1);
  lcd.print("myIP=");
  lcd.print(Ethernet.localIP());
  delay(1500);
}

//MQTT Routines
void send_MQTT_message(float windSpd, int windDir){
  char bufa[100];
  char bufWindspd[5];
  if (client.connected()){
    dtostrf(windSpd, 4, 2, bufWindspd);
    sprintf(bufa,"Team: \"(:\" Speedo: value =%s m/s Direction: value =%d", bufWindspd, windDir);
    
    Serial.println( bufa );
    client.publish(outTopic, bufa);
    
  }
  else {
    delay(500);
    Serial.println("No, reconnecting");
    client.connect(clientId, deviceId, deviceSecret);
    delay(1000);
  }
}

//MQTT server connection
void Connect_MQTT_server(){
  Serial.println(" Connecting to MQTT" );
  Serial.print(server[0]); Serial.print(".");     // Print MQTT server IP number to Serial monitor
  Serial.print(server[1]); Serial.print(".");
  Serial.print(server[2]); Serial.print(".");
  Serial.println(server[3]); 
  delay(500);

  if (!client.connected()){
    if (client.connect(clientId, deviceId, deviceSecret)) {
      Serial.println(" Connected ");
      client.subscribe(inTopic);
    }
    else{
      Serial.println(client.state());
    }
  }
}
  //receive incoming MQTT message

void callback(char* topic, byte* payload, unsigned int length)
{ 
  // copy the payload content into a char* 
  char* receiv_string;                               
  receiv_string = (char*) malloc(length + 1);
  
  // copy received message to receiv_string 
  memcpy(receiv_string, payload, length);
  receiv_string[length] = '\0';           
  Serial.println( receiv_string );
  free(receiv_string); 
} 

int WindDirection(float voltage)
{
  // Determine wind direction based on voltage
  if (voltage >= 0 && voltage < 0.47){
    return 0;

  } else if (voltage >= 0.47 && voltage < 0.95){
    return 45;

  } else if (voltage >= 0.95 && voltage < 1.43){
    return 90;

  } else if (voltage >= 1.43 && voltage < 1.90){
    return 135;

  } else if  (voltage >= 1.90 && voltage < 2.38){
    return 180;

  } else if (voltage >= 2.38 && voltage < 2.85){
    return 225;

  } else if (voltage >= 2.85 && voltage < 3.33){
    return 270;

  } else { 
    return 315;
  }
}
