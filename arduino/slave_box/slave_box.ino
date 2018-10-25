#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

painlessMesh  mesh;

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
}

void receivedCallback( uint32_t from, String &msg ) { 
  int pinNumber = msg.substring(0,1).toInt();
  String colour = msg.substring(1);
  bool onoff;

  Serial.println(pinNumber);
  Serial.println(colour);

  if (colour == "FFFFFF"){
    onoff = 1;
  } else {
    onoff = 0;
  }

  digitalWrite(pinNumber, onoff);

}

void newConnectionCallback(uint32_t nodeId) {
}

void changedConnectionCallback() {
}

void nodeTimeAdjustedCallback(int32_t offset) {
}


class Input{
  int inPin;
  bool oldpin;
  unsigned long debounce;

  public:
    Input(int pin){
      inPin = pin;
      debounce = millis();
      pinMode(inPin, INPUT_PULLUP);
      oldpin = digitalRead(inPin);
      Serial.print("INIT pin ");
      Serial.println(inPin);
    }

  void Check(){
    if (millis() > debounce + 50){
      if (!digitalRead(inPin) && oldpin){
        Serial.print("PRESSED ");
        Serial.println(inPin);
        oldpin = 0;
        debounce = millis();
      }
      if (digitalRead(inPin) && !oldpin){
        oldpin = 1;
        debounce = millis();
      }
    }
  }
};

Input di1(4);
Input di2(5);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  di1.Check();
  di2.Check();
  mesh.update();
}
