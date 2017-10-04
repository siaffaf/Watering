
// $Header: http://furist.kiev.ua/svn/Arduino/Watering_Controller/Slava_watering/Slava_watering.ino 32 2017-08-18 05:48:58Z siaffa $

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <FlexiTimer2.h>
#include <EEPROM.h>

//Inputs
#define E1_High 0
#define E1_Low  1
#define E2_High 2
#define E2_Low  3
#define E1_PRESS_LOW 4
#define E1_PRESS_HIGH 5
#define E2_PRESS_LOW 6
#define E2_PRESS_HIGH 7

//Outputs
#define E1_Fill 0
#define E1_Pump 1
#define E2_Fill 2
#define E2_Pump 3

#define PRESS_WORK 2
#define PRESS_SLEEP 1
#define PRESS_OFF 0
#define PRESS_MAX_TRY 2

// ID of the settings block
#define CONFIG_VERSION "fv2"
// Tell it where to store your config data in EEPROM
#define CONFIG_START 32


static char Rev[ ] = "$Id: Slava_watering.ino 32 2017-08-18 05:48:58Z siaffa $";

//--------------------------------
//Production MAC
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x04 };
//Testbed MAC
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x05 };
//---------------------------------

byte i_buf = 0;
EthernetServer server(80); //server port
EthernetUDP UdpClient;

char http_header[100];
byte inPins[] = {26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48};
byte outPins[] = {53, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27, 25, 23, 22, 24};
byte AnalogPorts[] = {2, 3};
float AnalogPortsCorrection[] = {6.5, 5.5};
volatile byte outStatus[sizeof(outPins)];
volatile word sec_Timer = 0;
volatile word press1_timer = 0;
volatile byte press1_status = 0;
volatile word press2_timer = 0;
volatile byte press2_status = 0;
volatile byte press1_try = 0;
volatile byte press2_try = 0;
volatile byte temperature = 0;
char log_message[4];


// Config structure
struct StoreStruct {
  // This is for mere detection if they are your settings
  char version[4];
  // The variables of your settings
  char log_server_ip[16];
  unsigned int log_server_port;
  byte http_conn_timeout;
  byte press_timeout;
  byte press_control1;
  byte press_control2;
} storage = {
  CONFIG_VERSION,
  // The default values
  "10.10.24.3",
  5151,
  5,
  20,
  1,
  1};

void loadConfig() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
    for (unsigned int t=0; t<sizeof(storage); t++)
      *((char*)&storage + t) = EEPROM.read(CONFIG_START + t);
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(storage); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&storage + t));
}
void write_log(char* msg){
    memcpy(log_message,msg,3);
}
void send_log(){
    if (log_message[0] != 0){
        UdpClient.beginPacket(storage.log_server_ip, storage.log_server_port);
        UdpClient.write(log_message);
        UdpClient.endPacket();
        delay(1);
        log_message[0]=0; 
    }

}

void level_control()
{
  sec_Timer++;
  if (digitalRead(inPins[E1_High])&& outStatus[E1_Fill]) {
    //Riched high level -> Switch off the filling valve
    digitalWrite(outPins[E1_Fill], 0);
    outStatus[E1_Fill] = 0;
    write_log("111");
  }
  if (digitalRead(inPins[E1_Low]) && outStatus[E1_Pump]) {
    //Riched low level -> Switch off the pump
    digitalWrite(outPins[E1_Pump], 0);
    outStatus[E1_Pump] = 0;
    press1_status = PRESS_OFF;
    write_log("112");
  }
  if (digitalRead(inPins[E2_High])&& outStatus[E2_Fill]) {
    //Riched high level -> Switch off the filling valve
    digitalWrite(outPins[E2_Fill], 0);
    outStatus[E2_Fill] = 0;
    write_log("121");
  }
  if (digitalRead(inPins[E2_Low]) && outStatus[E2_Pump]) {
    //Riched low level -> Switch off the pump
    digitalWrite(outPins[E2_Pump], 0);
    outStatus[E2_Pump] = 0;
    press2_status = PRESS_OFF;
    write_log("122");

  }

  //E1 pressure control
  if (storage.press_control1 && outStatus[E1_Pump]){
    //Skip timeout
    if (press1_status == PRESS_SLEEP && press1_timer == sec_Timer){
        press1_status = PRESS_WORK;
    }
    if (press1_status == PRESS_WORK){
        //Checking the pressure
        if (digitalRead(inPins[E1_PRESS_LOW])==1 && digitalRead(inPins[E1_PRESS_HIGH])==1){
            //Normal working pressure - OFF,OFF
            if (press1_timer == sec_Timer){
                write_log("110");
            }
            press1_try=0;
        }else if (digitalRead(inPins[E1_PRESS_LOW]) == 0 && digitalRead(inPins[E1_PRESS_HIGH]) == 1){
            // Low pressure - ON,OFF
            // Switch off Pump1 if it repeats PRESS_MAX_TRY times
            write_log("411");
            if (press1_try >= PRESS_MAX_TRY){
              digitalWrite(outPins[E1_Pump], 0);
              outStatus[E1_Pump] = 0;
              press1_status = PRESS_OFF;
            }else{
              press1_try++;
            }
        }else if (digitalRead(inPins[E1_PRESS_LOW]) == 1 && digitalRead(inPins[E1_PRESS_HIGH]) == 0){
            // High pressure - OFF,ON
            // Switch off Pump1
            write_log("412");
            if (press1_try >= PRESS_MAX_TRY){
              digitalWrite(outPins[E1_Pump], 0);
              outStatus[E1_Pump] = 0;
              press1_status = PRESS_OFF;
            }else{
              press1_try++;
            }
        }if (digitalRead(inPins[E1_PRESS_LOW]) == 0 && digitalRead(inPins[E1_PRESS_HIGH]) == 0){
            // Short wire - ON,ON
            // Switch off Pump1
            digitalWrite(outPins[E1_Pump], 0);
            outStatus[E1_Pump] = 0;
            press1_status = PRESS_OFF;
            write_log("413");
        }
    }
        
  }
  
  // E2 Pressure control
    if (storage.press_control2 && outStatus[E2_Pump]){
    //Skip timeout
    if (press2_status == PRESS_SLEEP && press2_timer == sec_Timer){
        press2_status = PRESS_WORK;
    }
    if (press2_status == PRESS_WORK){
        //Checking the pressure
        if (digitalRead(inPins[E2_PRESS_LOW])==1 && digitalRead(inPins[E2_PRESS_HIGH])==1){
            //Normal working pressure in E1 - OFF,OFF
            if (press2_timer == sec_Timer){
                write_log("120");
            }
            press2_try=0;            
        }else if (digitalRead(inPins[E2_PRESS_LOW]) == 0 && digitalRead(inPins[E2_PRESS_HIGH]) == 1){
            // Low pressure - ON,OFF
            // Switch off Pump2
            write_log("421");
            if (press2_try >= PRESS_MAX_TRY){
              digitalWrite(outPins[E2_Pump], 0);
              outStatus[E2_Pump] = 0;
              press2_status = PRESS_OFF;
            }else{
              press2_try++;
            }

        }else if (digitalRead(inPins[E2_PRESS_LOW]) == 1 && digitalRead(inPins[E2_PRESS_HIGH]) == 0){
            // High pressure - OFF,ON
            // Switch off Pump2
            write_log("422");
            if (press2_try >= PRESS_MAX_TRY){
              digitalWrite(outPins[E2_Pump], 0);
              outStatus[E2_Pump] = 0;
              press2_status = PRESS_OFF;
            }else{
              press2_try++;
            }
        }if (digitalRead(inPins[E2_PRESS_LOW]) == 0 && digitalRead(inPins[E2_PRESS_HIGH]) == 0){
            // Short wire - ON,ON
            // Switch off Pump1
            digitalWrite(outPins[E2_Pump], 0);
            outStatus[E2_Pump] = 0;
            press2_status = PRESS_OFF;
            write_log("423");
        }
    }
        
  }
}


void process_http_request(EthernetClient* pClient, char* pHttp_header) {
char* pos1=0;
char* pos2=0;
char port_text[6];
char value_char;
byte len=0;
unsigned int port_num=65535;
byte value_num=255;

    memset(port_text, 0, sizeof(port_text));
    pClient->println("HTTP/1.1 200 OK");
    pClient->println("Content-Type: application/json");
    pClient->println();
    //Analyze request URI
    //Looking for /get_status/ method
    if (strstr(pHttp_header, "/get_status/")) {
    pClient->print("{\n\"inputs\": [");
    for (byte i = 0; i < sizeof(inPins); i++) {
      if (i > 0) pClient->print(",");
      if (digitalRead(inPins[i]) == 1)
        // inverting values because of PULL_DOWN inputs
        pClient->print("false");
      else
        pClient->print("true");
    }
    pClient->print("],\n\"outputs\": [");
    for (byte i = 0; i < sizeof(outStatus); i++) {
      if (i > 0) pClient->print(",");
      if (outStatus[i] == 1)
        pClient->print("true");
      else
        pClient->print("false");
    }
    pClient->print("],\n\"temperature\": [");
    for (byte i=0; i<sizeof(AnalogPorts); i++){
       if (i > 0) pClient->print(",");
       temperature = getTemperature(i);
       pClient->print(temperature);
    }
    pClient->println("]\n}");
    }else
    // Looking for /set_value method
    if (pos1 = strstr(pHttp_header, "set_value?digital_pin=")) {
        if(pos2=strstr(pos1,"&value=")){
            len=pos2-pos1-22;
            if( len==1 or len==2 ){
                memcpy(port_text, pos1+22, len);
            }else{
                pClient->println("Error: Port number can't be more than 2 chars!");
                return;
            }
            if (len == 1){
                port_num=port_text[0]-'0';
            }else{
                port_num=(port_text[0]-'0')*10+(port_text[1]-'0');
            }
            if (port_num > sizeof(outPins)){
                pClient->print("Error: digital_pin is out of range! Allowed from 0 to ");
                pClient->println(sizeof(outPins));
            return;              
            }
            //Calculate value
            value_char=*(pos2+7);
            if (value_char == 48) value_num=0;
            else if (value_char == 49) value_num=1;
            else{
                pClient->print("Error: Port value can be either 0 or 1!");
                return;
            }
            //Serial.print("port_num=");
            //Serial.println(port_num);
            //Serial.print("value=");
            //Serial.println(value_num);
            //Start timeout for pressure control
            digitalWrite(outPins[port_num-1],value_num);
            outStatus[port_num-1] = value_num;
            if (port_num-1 == E1_Pump && value_num == 1){
                press1_timer=sec_Timer+storage.press_timeout;
                press1_status = PRESS_SLEEP;
            }else if(port_num-1 == E1_Pump && value_num == 0){
                press1_status = PRESS_OFF;
            }else if (port_num-1 == E2_Pump && value_num == 1){
                press2_timer=sec_Timer+storage.press_timeout;
                press2_status = PRESS_SLEEP;
            }else if(port_num-1 == E2_Pump && value_num == 0){
                press2_status = PRESS_OFF;
            }
            log_message[0]='5';
            log_message[1]='0' + port_num;
            log_message[2]='0' + value_num;
        }else{
            pClient->print("Error: Value is not found!");
            return;
        }
    pClient->print("OK!");
    }else
    // Looking for /version method
    if (strstr(pHttp_header, "/version/")) {
        pClient->println(Rev);
    }else 
        // Print temperature value
    if (strstr(pHttp_header, "/get_temp/")) {
        pClient->print("{\"temperature\": [");
        for (byte i=0; i<sizeof(AnalogPorts); i++){
            if (i > 0) pClient->print(",");
            temperature = getTemperature(i);
            pClient->print(temperature);
        }
        pClient->print("]}");

    }else 
    // Print current config values
    if (strstr(pHttp_header, "/print_cfg/")) {
        pClient->print("Version: ");
        pClient->println(storage.version);
        pClient->print("LogSrvIP: ");
        pClient->println(storage.log_server_ip);
        pClient->print("LogSrvPort: ");
        pClient->println(storage.log_server_port);
        pClient->print("HTTPConnTimeout: ");
        pClient->println(storage.http_conn_timeout);
        pClient->print("PressureTimeout: ");
        pClient->println(storage.press_timeout);
        pClient->print("PressureControl E1: ");
        pClient->println(storage.press_control1);
        pClient->print("PressureControl E2: ");
        pClient->println(storage.press_control2);
    }else 
    if (strstr(pHttp_header, "/save_cfg/")) {
        saveConfig();
        pClient->println("Config saved");
    }else
    if (pos1 = strstr(pHttp_header, "/mod_cfg/LogSrvIP/")) {
        if(pos2=strstr(pos1," HTTP")){
            len=pos2-pos1-18;
            if (len < sizeof(storage.log_server_ip)){
                strncpy(storage.log_server_ip,pos1+18,len);
                storage.log_server_ip[len] = '\0';
                pClient->println(storage.log_server_ip);
                pClient->println("OK!");
            }else {
                pClient->println("Error: value too long!");
            }
        }else {
            pClient->println("Error: value not found");
        }
    }else 
    if (pos1 = strstr(pHttp_header, "/mod_cfg/LogSrvPort/")) {
        if(pos2=strstr(pos1," HTTP")){
            len=pos2-pos1-20;
            if (len <= 5){
                strncpy(port_text,pos1+20,len);
                port_text[len] = '\0';
                port_num=0;
                char* chPtr=port_text;
                char ch = *(chPtr++);
                do
                {
                    if (ch < '0' || ch > '9')
                    {
                       pClient->print("Error: Wrong value, not digit!");
                       return;
                    }
                    port_num = port_num * 10 + (ch - '0');
                    ch = *(chPtr++);
                }while (ch != '\0');
                storage.log_server_port=port_num;
                pClient->println("OK!");               
            }else {
                pClient->println("Error: value too long!");
            }
        }else {
            pClient->println("Error: value not found");
        } 
    }else 
    if (pos1 = strstr(pHttp_header, "/mod_cfg/press_control/")) {
        port_num=pos1[23]-'0';
        value_num=pos1[25]-'0';
        if (value_num !=0 && value_num !=1){
            pClient->println("Error: /press_control/$port/$val  $val must be 0 or 1");
            return;
        }
        if (port_num == 1) {
            storage.press_control1=value_num;
        }else if (port_num == 2){
            storage.press_control2=value_num;
        } else{
            pClient->println("Error: /press_control/$port/$val  $port must be 1 or 2");
            return;
        }
        pClient->println("OK!");
    }else {
            pClient->println("Error: Unknown command!");
    }
}

void setup()
{
  Serial.begin(9600);
  // disable SD card
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  //start Ethernet
  //Ethernet.begin(mac, ip, gateway, subnet);
  while(Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP. Trying again...");
    // initialize the Ethernet device not using DHCP:
    //Ethernet.begin(mac, ip, gateway, subnet);
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  IPAddress ip = Ethernet.localIP();
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();


  // Init Input ports
  for (int i = 0; i < sizeof(inPins); i++) {
    pinMode(inPins[i], INPUT_PULLUP);
    //Serial.print("\nInput");
    //Serial.print(inPins[i]);
  }

  // Init Output ports
  for (int i = 0; i < sizeof(outPins); i++) {
    pinMode(outPins[i], OUTPUT);
    digitalWrite(outPins[i], LOW);
    outStatus[i] = 0;
    //Serial.print("\nOutput");
    //Serial.print(outPins[i]);
  }
  
  loadConfig();
  
  FlexiTimer2::set (1000, level_control); // 1s period
  FlexiTimer2::start ();
  //Start listening for TCP connetions
  server.begin();
  UdpClient.begin(5151);
  // analogReference(INTERNAL2V56);
}

void loop()
{
  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    char c = 0;
    byte End_Time = sec_Timer + storage.http_conn_timeout;
    memset(http_header, 0, sizeof(http_header));
    i_buf = 0;
    while (client.connected()) {
      if (sec_Timer == End_Time) {
        Serial.println("HTTP Session timeout!\nDisconnecting the Client!");
        break;
      }
      if (client.available()) {
        c = client.read();
        if (i_buf < sizeof(http_header) - 1) {
          //store characters to buffer
          http_header[i_buf] = c;
          i_buf++;
        }
        //if HTTP request has ended
        if (c == '\n' && currentLineIsBlank) {
          //Analyze HTTP request and send Answer
          process_http_request(&client, http_header);
          break;
        } else if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  } else{
        // Check if new log message exists and send it;
        send_log();
        //delay(1000);
    }
}

// function to retrieve temperature
unsigned int getTemperature(byte index){
float temp = 0;
int i;
 for (i=0; i<5; i++){
    temp = temp + (analogRead(AnalogPorts[index]) * 0.004741);  // =Vcc/1023
    //Serial.print(i);
    //Serial.print(" - ");
    //Serial.println(analogRead(pin));
    delay(10);
  }
  temp = temp/i;
  temp = (temp-.5) * 100;               // correct for offset (-0,5V) and 10mv per degree
  //Serial.println(temp);
  return  temp + 0.5 + AnalogPortsCorrection[index];                   // round to int and apply error correction 
}

    

