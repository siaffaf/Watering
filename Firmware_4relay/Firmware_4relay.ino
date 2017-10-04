#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x44 };
IPAddress ip = { 10, 10, 24, 245 }; // ip in lan
IPAddress gateway = { 10, 10, 24, 254 }; // internet access via router
IPAddress subnet = { 255, 255, 255, 0 }; //subnet mask
EthernetServer server(80); //server port

String readString = String();
String portnum = String();
String portvalue = String();

///////////////////////
int pos;
int pos2;
//////////////////////

byte outPins[] = {7,6,5,3};
byte outStatus[sizeof(outPins)];


void setup()
{
  Serial.begin(9600);

  // disable SD card
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  //start Ethernet
  //Ethernet.begin(mac, ip, gateway, subnet);
   if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // initialize the Ethernet device not using DHCP:
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  ip = Ethernet.localIP();
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
  server.begin();

  // Init Output ports
  for (int i = 0; i < sizeof(outPins); i++) {
    pinMode(outPins[i], OUTPUT);
    digitalWrite(outPins[i], HIGH);
    outStatus[i] = 0;
  }

}

void loop()
{
  // Create a client connection
  EthernetClient client = server.available();
  unsigned int val;
  unsigned int prt;
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString.concat(c);
          //Serial.print(c);
        }

        //if HTTP request has ended
        if (c == '\n') {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();
          Serial.println(readString);
          if (pos = StringContains(readString, "set_value?digital_pin=")) {
            pos2 = 0;
            pos2 = readString.indexOf('&', pos + 22);
            portnum = readString.substring(pos + 22, pos2);
            portvalue = readString.substring(pos2 + 7, pos2 + 8);
            val = portvalue.toInt();
            prt = portnum.toInt();
            if (val == 0 or val == 1) {
              val = !val;
              digitalWrite(outPins[prt - 1], val);
              client.println("OK!\n");
            } else {
              client.println("Error: Wrong value! Allowed 0 or 1 only!\n");
            }
          } else {
            client.println("Error: Unknown command!\n");

          }
          ///////////////////
          delay(1);
          //stopping client
          client.stop();

          /////////////////////
          //clearing string for next read
          readString = "";
          portnum = "";
          portvalue = "";

        }
      }
    }
  }
}

int StringContains(String s, String search) {
  int max = s.length() - search.length();
  int lgsearch = search.length();

  for (int i = 0; i <= max; i++) {
    if (s.substring(i, i + lgsearch) == search) return i;
  }

  return 0;
}


