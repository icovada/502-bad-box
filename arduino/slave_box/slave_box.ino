#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "NeoPixelBus.h"

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

#define LEDNUMBER 9

painlessMesh mesh;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(LEDNUMBER);
uint32_t master_id = 0;

class LED
{
  bool state = false;
  int brightness = 255;
  int r = 0;
  int g = 0;
  int b = 0;
  String effect = "Fixed";
  long flash = 2500;

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

  void SetEffect(String value)
  {
    effect = value;
  }

  void SetFlash(long value)
  {
    if (value != 0) //Dividing by 0 is never a good idea
    {
      flash = value;
    }
  }

  void SetState(String value)
  {
    if (value == "ON")
    {
      state = true;
    }
    else
    {
      state = false;
    }
  }

  RgbColor GetColour(uint32_t usec)
  {
    int curBrightness = 0;
    RgbColor rgb;
    if (state == false)
    {
      curBrightness = 0;
    }
    else if ((state == true) && (effect == "Fixed"))
    {
      curBrightness = brightness;
    }
    else if ((state == true) && (effect == "Blink"))
    {
      if (((usec / 1000) / flash) % 2 == 0)
      {
        curBrightness = 0;
      }
      else
      {
        curBrightness = brightness;
      };
    }
    else if ((state == true) && (effect == "Breathing"))
    {
      curBrightness = breathingLed();
    }

    int outr = map(r, 0, 255, 0, curBrightness);
    int outg = map(g, 0, 255, 0, curBrightness);
    int outb = map(b, 0, 255, 0, curBrightness);
    rgb = (outr, outg, outb);
    return rgb;
  }

private:
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
};

LED ledstrip[LEDNUMBER];

// --------------------------------------------------------- painlessMesh
void receivedCallback(uint32_t from, String &msg)
{
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(msg);
  /* Array for {"led":"1",
                "data": {"state": "ON",
                         "brightness": 146,
                         "color": {"r": 255, "g": 255, "b": 255},
                         "effect": "breathing",
                         "flash":5000}} */

  if (root.containsKey("root"))
  {
    master_id = from;
  }
  else if (root.containsKey("led"))
  {
    int led = root["led"].as<int>();
    JsonObject &data = root["data"];
    if (data.containsKey("color"))
    {
      JsonObject &colour = data["color"];

      ledstrip[led].SetColour(colour["r"],
                              colour["g"],
                              colour["b"]);
    }
    if (data.containsKey("effect"))
    {
      ledstrip[led].SetEffect(String(data["effect"].as<char *>()).c_str());
    }
    if (data.containsKey("state"))
    {
      ledstrip[led].SetState(data["state"]);
    }
    if (data.containsKey("flash"))
    {
      ledstrip[led].SetFlash(data["flash"]);
    }
    if (data.containsKey("brightness"))
    {
      ledstrip[led].SetBrightness(data["brightness"]);
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
// --------------------------------------------------------- painlessMesh

class InputPin
{
public:
  InputPin() {}

  InputPin(int pin)
  {
    _pin = pin;
    _debounce = millis();
    pinMode(_pin, INPUT);
    pinMode(_pin, INPUT_PULLUP);
    _oldpin = digitalRead(_pin);
  }

  InputPin(int pin, bool islatching)
  {
    _pin = pin;
    _latching = islatching;
    _debounce = millis();
    pinMode(_pin, INPUT);
    pinMode(_pin, INPUT_PULLUP);
    _oldpin = digitalRead(_pin);
  }

  void Check()
  {
    if (millis() > _debounce + 30)
    {
      bool pinStatus = digitalRead(_pin);
      if (_latching)
      {
        if (pinStatus != _oldpin)
        {
          _notifyChange("single");
          Serial.println("Single press");
          _oldpin = pinStatus;
        }
      }
      else
      { //PIN is 0 if closed
        if (!pinStatus && _oldpin)
        { // if pressed and was not pressed
          _oldpin = 0;
          _lock = true;
          _activationTimer = millis();
        }
        else if (((millis() - _activationTimer) > 400) && _lock)
        { // If still pressed after 400 ms
          _lock = false;
          Serial.println("Long press");
          _notifyChange("long");
        }
        else if (pinStatus && !_oldpin)
        { // if Let go
          if (_lock)
          { // if still in action
            _oldpin = 1;
            _lock = false;
            Serial.println("Single press");
            _notifyChange("single");
          }
          else
          {
            _oldpin = 1;
          }
        }
      }
      _debounce = millis();
    }
  }

protected:
  bool _lock;
  bool _oldpin;
  bool _latching;
  unsigned long _debounce;
  int _pin;
  uint32_t _activationTimer;

  void _notifyChange(String event)
  {
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["pin"] = String(_pin);

    JsonObject &data = root.createNestedObject("data");
    data["event"] = event;
    String message;
    root.printTo(message);
    mesh.sendSingle(master_id, message);
  }
};

class InputManager
{
protected:
  InputPin _inputs[9];

public:
  InputManager(int pinlist[], bool switchtype[])
  {
    for (int i = 0; i < LEDNUMBER; i++)
    {
      _inputs[i] = InputPin(pinlist[i], switchtype[i]);
    }
  }

  void Check()
  {
    for (int i = 0; i < 9; i++)
    {
      _inputs[i].Check();
    }
  }
};

int pins[] = {16, 5, 4, 0, 2, 14, 12, 13, 15};
bool latch[] = {0, 1, 0, 0, 0, 0, 0, 0, 0};
InputManager switches(pins, latch);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start sketch");

  /*   SPIFFS.begin();

  if (!SPIFFS.exists("/formatComplete.txt"))
  {
    Serial.println("Please wait 30 secs for SPIFFS to be formatted");
    SPIFFS.format();
    Serial.println("Spiffs formatted");

    File f = SPIFFS.open("/formatComplete.txt", "w");
    if (!f)
    {
      Serial.println("file open failed");
    }
    else
    {
      f.println("Format Complete");
    }
  }
  else
  {
    Serial.println("SPIFFS is formatted. Moving along...");
  } */

  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  Serial.println("Mesh initialised");
  strip.Begin();
  for (int i = 0; i < LEDNUMBER; i++)
  {
    ledstrip[i] = LED();
  }

  Serial.println("Finish setup");
}

void loop()
{
  switches.Check();
  for (int i = 0; i < LEDNUMBER; i++)
  {
    strip.SetPixelColor(i, ledstrip[i].GetColour(mesh.getNodeTime()));
  }
  strip.Show();
  mesh.update();
}
