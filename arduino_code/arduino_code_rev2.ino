#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <PubSubClient.h>


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

///////////////////////////////////////////////////////////////////

EthernetClient  ethClient;

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

  for (int i = 0; i < 4; i++){
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

 //fetch ip from dhcp, requires more work, prints connected IP(?) currently
  //fetch_IP(); 
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
      lcd.print("AmogSus          ");
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

  // sending the message :) commented out due to keyboard stopping working while active. works for sending data though :)
 // int inx=10;
  
 // while(true){
 // send_MQTT_message(inx);
 // inx++;
 // delay(1500);
 // }
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

void fetch_IP(void)
{
  byte rev=1;

  lcd.setCursor(0,1);

  //         01234567890123456789  
  lcd.print("     Waiting IP     ");

  rev=Ethernet.begin( mymac);                  // get IP number
     
  Serial.print( F("\nW5500 Revision ") );
    
  if ( rev == 0){
                   
                      Serial.println( F( "Failed to access Ethernet controller" ) );
                   
                                                // 0123456789012345689 
                    lcd.setCursor(0,0); lcd.print(" Ethernet failed   ");
                 }    
                 
              
     Serial.println( F( "Setting up DHCP" ));
     Serial.print("Connected with IP: "); 
     Serial.println(Ethernet.localIP()); 


  lcd.setCursor(0,1);
  //         012345678901234567890
  lcd.print("                     ");
  
  lcd.setCursor(0,1);
  lcd.print("myIP=");
  lcd.print(Ethernet.localIP());
  delay(1500);


}

//MQTT Routines

void send_MQTT_message(int num){
  char bufa[50];
  if (client.connected()){
    sprintf(bufa,"Amogus: value =%d", num);
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

 void callback(char* topic, byte* payload, unsigned int length){ 
  char* receiv_string;                               // copy the payload content into a char* 
  receiv_string = (char*) malloc(length + 1); 
  memcpy(receiv_string, payload, length);           // copy received message to receiv_string 
  receiv_string[length] = '\0';           
  Serial.println( receiv_string );
  free(receiv_string); 
} 
