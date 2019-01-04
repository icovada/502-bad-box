#include <PubSubClient.h>
#include <WiFiClient.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "mosquittotest"
#define   STATION_PASSWORD "mosquittotest"

#define HOSTNAME "503badboxmaster"

//Without these it dies idk
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(37, 187, 106, 16);


painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);
uint32_t ready;

bool triggered;


void newConnectionCallback(uint32_t nodeId) {
  String captain = "I am the captain now";
  mesh.sendBroadcast(captain);
  Serial.println("New Connection");
}

void changedConnectionCallback() {
  String captain = "I am the captain now";
  mesh.sendBroadcast(captain);
  Serial.println("Changed COnnection");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  String captain = "I am the captain now";
  mesh.sendBroadcast(captain);
  Serial.println("Node time adjusted");
}


void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "painlessMesh/from/" + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
}


void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length+1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length+1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if(targetStr == "gateway")
  {
    if(msg == "getNodes")
    {
      mqttClient.publish("painlessMesh/from/gateway", mesh.subConnectionJson().c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  Serial.println("Finish setup");
  ready = millis();
}

void loop() {
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("painlessMeshClient")) {
      mqttClient.publish("painlessMesh/from/gateway","Ready!");
      mqttClient.subscribe("painlessMesh/to/#");
    } 
  }

  if (millis() > 10000 && !triggered){
    triggered = 1;
    String message = "2FFFFFF";
    mesh.sendBroadcast(message);   
    Serial.println("Message sent. I had a good life.");
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}