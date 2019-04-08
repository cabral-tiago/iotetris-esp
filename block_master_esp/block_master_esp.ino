#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*List all the addresses for the blocks
These should be matching with the address
of each block
*/
int addresses[] {8, 9};

const int size = sizeof(addresses)/sizeof(addresses[0]);

int loopPeriod = 10000; // in ms

// Network login (to be replaced with Wifi Manager?)
const char* ssid = "beep";
const char* password = "boop";

// MQTT setup
#define mqtt_server  "192.168.1.73"
#define mqtt_name  "ESPmaster"

#define data_topic    "esp1/data"

// Sleep time between updates, in seconds
#define sleepTime 60

// Initializes the espClient.
// The espClient name should be unique
WiFiClient espClient;
PubSubClient client(espClient);

String result[size][2];

void setup() {
  Wire.begin(SDA,SCL);

  Serial.begin(9600);
  while(!Serial) { };
  Serial.println("IoTetris - Master Start");

  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("ESP IP:");
  Serial.println(WiFi.localIP());

  // Setting up MQTT
  client.setServer(mqtt_server, 1883);
  client.connect(mqtt_name);

  /*
  Publishing to the MQTT (example)
    client.publish(humidity_topic, humidityTemp, true);
    client.publish(temperature_topic, celsiusTemp, true);
    Serial.println("MQTT Message Sent");
  */
}

void loop()
{
  masterRequestLoop();
  sendPayload();
  //This delay is to be changed for a sleep later on
  delay(loopPeriod);
}

void masterRequestLoop()
{
  for(int x=0; x<size; x++){
      wakeUpSlave(addresses[x]);
      delay(1000);
      result[x][0] = addresses[x];
      result[x][1] = readSlave(addresses[x]);
  }
}

void sendPayload()
{
  String payload = "{";
  for(int x=0; x<size; x++){
    payload+=result[x][0]+":";
    if(result[x][1]!=""){
      payload += result[x][1];
    }else{
      payload += "null";
    }
    if(x<size-1){
      payload+= ",";
    }
  }
  payload += "}";
  Serial.println(payload);
  if(client.publish(data_topic, payload.c_str(), true)){
    Serial.println("MQTT Message Sent");
  }else{
    Serial.println("MQTT Error");
  }
  
}

void wakeUpSlave(int slave)
{
  Serial.println("Waking up slave" + String(slave));
  Wire.beginTransmission(slave);
  Wire.write("beep");
  Wire.endTransmission();
}

String readSlave(int slave)
{
  /* Might want to change this from void
  so we can gather the data and process it in the loop
  maybe? 
  Also add the number of bytes to the args
  if needed*/
  String a = "";
  Serial.println("Reading from slave" + String(slave));
  Wire.requestFrom(slave, 20);
  while(Wire.available()){
      char c = Wire.read();
      a = a+c;
      Serial.print(c);
  }
  Serial.println();
  return a;
}
