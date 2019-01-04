#include "painlessMesh.h"
#include "ArduinoJson.h"

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

painlessMesh mesh;

uint32_t master_id = 0;

void notifyChange(int pin)
{
  String message = String(pin);
  mesh.sendSingle(master_id, message);
}

void receivedCallback(uint32_t from, String &msg)
{
  Serial.println("received message");
  if (msg == "I am the captain now" && master_id == 0)
  {
    master_id = from;
    Serial.print("Master ID ");
    Serial.println(master_id);
  }
  int pinNumber = msg.substring(0, 1).toInt();
  String colour = msg.substring(1, 7);
  bool onoff;

  Serial.println(colour);
  if (msg == "I am the captain now")
  {
    master_id = from;
  }
  else
  {
    const size_t bufferSize = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(3) + 50;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    // Array for "{led":"6","colour":[255,2552,55],"blink":3}";

    JsonObject &root = jsonBuffer.parseObject(msg);

    int led = root["led"]; // "6"

    JsonArray &colour = root["colour"];

    int blink = root["blink"];     // 3
    int enabled = root["enabled"]; // 0

    //LedStrip[led].SetColour(colour[0], colour[1], colour[2]);
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.println("New Connection");
}

void changedConnectionCallback()
{
  Serial.println("Changed COnnection");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.println("Node time adjusted");
}

class Input
{
  int inPin;
  bool oldpin;
  unsigned long debounce;

public:
  Input(int pin)
  {
    inPin = pin;
    debounce = millis();
    pinMode(inPin, INPUT_PULLUP);
    oldpin = digitalRead(inPin);
    Serial.print("INIT pin ");
    Serial.println(inPin);
  }

  void Check()
  {
    if (millis() > debounce + 50)
    {
      if (!digitalRead(inPin) && oldpin)
      {
        Serial.print("PRESSED ");
        Serial.println(inPin);
        notifyChange(inPin);
        oldpin = 0;
        debounce = millis();
      }
      if (digitalRead(inPin) && !oldpin)
      {
        oldpin = 1;
        debounce = millis();
      }
    }
  }
};

class LED
{
  int r;
  int g;
  int b;
  bool state;
  int blinkMode;
  long blinkModulo;
  int brightness;

public:
  void SetColour(int *colours)
  {
    r = colours[0];
    g = colours[1];
    b = colours[2];
  }

  void SetBrightness(int brgh){
    brightness = brgh;
  }

  int *GetColour(uint32_t usec)
  {
    int curBrightness = 0;
    static int rgb[3];
    if (state == false)
    {
      curBrightness = 0;
    }
    else if ((state == true) && (blinkMode == 0))
    {
      curBrightness = brightness;
    }
    else if ((state == true) && (blinkMode == 1))
    {
      if (((usec / 1000) / blinkModulo) % 2 == 0)
      {
        curBrightness = 0;
      }
      else
      {
        curBrightness = brightness;
      };
    }
    else if ((state == true) && (blinkMode == 2))
    {
      curBrightness = breathingLed();
    }
    rgb[0] = map(r, 0, 255, 0, curBrightness);
    rgb[1] = map(g, 0, 255, 0, curBrightness);
    rgb[2] = map(b, 0, 255, 0, curBrightness);
    return rgb;
  }
};

Input PIN[9] = {Input(16), Input(5), Input(4), Input(0), Input(2), Input(14), Input(12), Input(13), Input(15)};

int breathingLed()
{
  uint32_t meshTime = mesh.getNodeTime();
  //Values taken from http://thecustomgeek.com/2011/06/17/breathing-sleep-led/
  if ((0 < meshTime) && (meshTime < 208))
  {
    return (0);
  }
  if ((208 < meshTime) && (meshTime < 654))
  {
    return (12);
  }
  if ((654 < meshTime) && (meshTime < 1000))
  {
    return (38);
  }
  if ((1000 < meshTime) && (meshTime < 1247))
  {
    return (65);
  }
  if ((1247 < meshTime) && (meshTime < 1420))
  {
    return (91);
  }
  if ((1420 < meshTime) && (meshTime < 1544))
  {
    return (118);
  }
  if ((1544 < meshTime) && (meshTime < 1960))
  {
    return (144);
  }
  if ((1960 < meshTime) && (meshTime < 2376))
  {
    return (255);
  }
  if ((2376 < meshTime) && (meshTime < 2508))
  {
    return (144);
  }
  if ((2508 < meshTime) && (meshTime < 2676))
  {
    return (117);
  }
  if ((2676 < meshTime) && (meshTime < 3122))
  {
    return (91);
  }
  if ((3122 < meshTime) && (meshTime < 3651))
  {
    return (64);
  }
  if ((3651 < meshTime) && (meshTime < 4277))
  {
    return (37);
  }
  if ((4277 < meshTime) && (meshTime < 4511))
  {
    return (11);
  }
  if ((4511 < meshTime) && (meshTime < 5000))
  {
    return (0);
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
  Serial.println("Finish setup");
  pinMode(2, OUTPUT);
}

void loop()
{

  for (int i = 0; i < 9; i++)
  {
    PIN[i].Check();
  }

  mesh.update();
}
