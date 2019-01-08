/* A bit of documentation

MQTT to mesh:
-------------

MQTT topics to be formatted like this:
/503-bad-box/to/nodeId/x/y

MQTT message MUST be a valid JSON

If topic does not contain x and y
/503-bad-box/to/nodeID
JSON is to be passed without any modification

If topic contains x and y, Json is to be
modified as follows:

{"x":"y",
 "data": MQTTmsg}

It is up to the receiving module to re-cast data


Mesh to MQTT:
-------------

Same applies the other way around:
If incoming JSON contains only 2 nodes,
one of which is called "data" and contains another
JSON, it is to be sent to
/503-bad-box/from/nodeId/x/y

Else, only from /503-bad-box/from/nodeId

*/

#include <PubSubClient.h>
#include <WiFiClient.h>
#include "painlessMesh.h"

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

#define STATION_SSID "mosquittotest"
#define STATION_PASSWORD "mosquittotest"

#define HOSTNAME "503badboxmaster"

//Without these it dies idk
void receivedCallback(const uint32_t &from, const String &msg);
void mqttCallback(char *topic, byte *payload, unsigned int length);

IPAddress getlocalIP();

IPAddress myIP(0, 0, 0, 0);
IPAddress mqttBroker(37, 187, 106, 16);

painlessMesh mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

uint32_t ready;

bool triggered;

void newConnectionCallback(uint32_t nodeId)
{
  String captain = "{'root':'me'}";
  mesh.sendBroadcast(captain);
  Serial.print("New Connection ");
}

void changedConnectionCallback()
{
  String captain = "{'root':'me'}";
  mesh.sendBroadcast(captain);
  Serial.println("Changed Connection");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  String captain = "I am the captain now";
  mesh.sendBroadcast(captain);
  Serial.println("Node time adjusted");
}

void receivedCallback(const uint32_t &from, const String &msg)
{
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(msg);
  
  String topic = "/503-bad-box/from/" + String(from) + "/";
  if (root.containsKey("data") && (root.size() == 2))
  {
    JsonObject &data = root["data"];
    String x;
    String message; 
    String y;
    for (JsonPair &p : root)
    {
      //p.key       // is a const char* pointing to the key
      //p.value     // is a JsonVariant
      if (strcmp(p.key, "data") != 0)
      {
        topic = topic + p.key + "/" + String(p.value.as<char*>()).c_str();;
      }
    }
    data.printTo(message);
    String sender = String(from);

    mqttClient.publish(topic.c_str(), message.c_str());
  } else {
  }
}

void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  StaticJsonBuffer<400> jsonBuffer;
  char *cleanPayload = (char *)malloc(length + 1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length + 1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  String Stopic = String(topic).substring(16); // /503-bad-box/to/758607613/1
  uint32_t target = strtoul(Stopic.substring(0, 9).c_str(), NULL, 10);

  if (Stopic == "gateway")
  {
    if (msg == "getNodes")
    {
      mqttClient.publish("/503-bad-box/from/gateway", mesh.subConnectionJson().c_str());
    }
  }
  else if (Stopic == "broadcast")
  {
    mesh.sendBroadcast(msg);
  }

  else //IT'S A JSON!
  {
    if (mesh.isConnected(target))
    {
      if (Stopic.endsWith("/"))
      {
        // Stopic ends with a /. It is NOT valid
        return;
      }

      if (Stopic.indexOf("/", 9) == -1)
      { //If this is a direct message to a node after a valid 9 digit nodeID
        mesh.sendSingle(target, msg);
      }
      else
      { //if this contains x and y
        int slash = Stopic.indexOf("/", 10);
        String x = Stopic.substring(10, slash).c_str();
        String y = Stopic.substring(slash + 1).c_str();

        JsonObject &object = jsonBuffer.createObject();
        object[x] = y;
        object["data"] = RawJson(msg);

        String fulljson;
        object.printTo(fulljson);

        mesh.sendSingle(target, fulljson);
      }
    }
    else
    {
      mqttClient.publish("/503-bad-box/from/gateway", "Client not connected!");
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  Serial.println("Finish setup");
  ready = millis();
}

void loop()
{
  mesh.update();
  mqttClient.loop();

  if (myIP != getlocalIP())
  {
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("503-bad-box-master"))
    {
      mqttClient.subscribe("/503-bad-box/to/#");
      mqttClient.publish("/503-bad-box/from/gateway", "Ready!");
      Serial.println("MQTT connected");
    }
  }
}

IPAddress getlocalIP()
{
  return IPAddress(mesh.getStationIP());
}