#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "NeoPixelBus.h"

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

painlessMesh mesh;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(9);
uint32_t master_id = 0;

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

class LED
{
  int r = 0;
  int g = 0;
  int b = 0;
  bool state = false;
  int blinkMode = 0;
  long blinkModulo = 1000;
  int brightness = 255;

public:
  void SetColour(int ir, int ig, int ib)
  {
    r = ir;
    g = ig;
    b = ib;
  }

  void SetBrightness(int value)
  {
    brightness = value;
  }

  void SetBlink(int value)
  {
    blinkMode = value;
  }

  void SetBlinkModulo(long value)
  {
    if (value != 0) //Dividing by 0 is never a good idea
    {
      blinkModulo = value;
    }
  }

  void SetState(bool value)
  {
    state = value;
  }

  RgbColor GetColour(uint32_t usec)
  {
    int curBrightness = 0;
    RgbColor rgb;
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

    int outr = map(r, 0, 255, 0, curBrightness);
    int outg = map(g, 0, 255, 0, curBrightness);
    int outb = map(b, 0, 255, 0, curBrightness);
    Serial.print(r);
    Serial.print(g);
    Serial.println(b);
    rgb = (outr, outg, outb);
    return rgb;
  }
};

LED ledstrip[9];

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
  else
  {
    const size_t bufferSize = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(3) + 70;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    /* Array for {"led":6,
                  "colour":[255,2552,55],
                  "blink":3,
                  "blinkModulo":2500,
                  "enabled": true}; */

    JsonObject &root = jsonBuffer.parseObject(msg);

    int led = root["led"];

    if (root.containsKey("colour"))
    {
      JsonArray &colour = root["colour"];
      //ledstrip[led].SetColour(colour[0], colour[1], colour[2]);
    }
    if (root.containsKey("blink"))
    {
      //ledstrip[led].SetBlink(root["blink"]);
    }
    if (root.containsKey("blinkModulo"))
    {
      //ledstrip[led].SetBlinkModulo(root["blinkModulo"]);
    }
    if (root.containsKey("enabled"))
    {
      //ledstrip[led].SetState(root["enabled"]);
    }
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.println("New Connection");
}

void changedConnectionCallback()
{
  Serial.println("Changed Connection");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.println("Node time adjusted");
}

/* class InputPin
{
  int inPin;
  bool oldpin;
  unsigned long debounce;

public:
  InputPin() {}

  InputPin(int inPin)
  {
    debounce = millis();
    pinMode(inPin, OUTPUT);
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
}; */

/* class InputManager
{
  InputPin inputs[9];

public:
  InputManager(int pinlist[9])
  {
    for (int i = 0; i < 9; i++)
    {
      inputs[i] = InputPin(8); //pinlist[i]);
    }
  }

  void Check()
  {
    for (int i = 0; i < 9; i++)
    {
      inputs[i].Check();
    }
  }
}; */

//int pins[9] = {16, 5, 4, 0, 2, 14, 12, 13, 15};
//InputManager switches(pins);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start sketch");
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  Serial.println("Mesh initialised");
  for (int i = 0; i < 9; i++)
  {
    ledstrip[i] = LED();
  }

  Serial.println("Finish setup");
}

void loop()
{
  for (int i = 0; i < 9; i++)
  {
    Serial.println(i);
    strip.SetPixelColor(i, ledstrip[i].GetColour(mesh.getNodeTime()));
  }
  strip.Show();
  mesh.update();
  Serial.println("Looped");
}
