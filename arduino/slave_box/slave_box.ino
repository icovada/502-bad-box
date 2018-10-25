#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

painlessMesh  mesh;

uint32_t master_id = 0;

void notifyChange(int pin){
  String message = String(pin);
  mesh.sendSingle(master_id, message);
}

void receivedCallback( uint32_t from, String &msg ) { 
  Serial.println("received message");
  if (msg=="I am the captain now" && master_id==0){
    master_id=from;
    Serial.print("Master ID ");
    Serial.println(master_id);
  }  
  int pinNumber = msg.substring(0,1).toInt();
  String colour = msg.substring(1,7);
  bool onoff;

  Serial.println(colour);

  if (colour == "FFFFFF"){
    onoff = 1;
  } else {
    onoff = 0;
  }

  digitalWrite(pinNumber, onoff);

}

void newConnectionCallback(uint32_t nodeId) {
  Serial.println("New Connection");
}

void changedConnectionCallback() {
  Serial.println("Changed COnnection");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.println("Node time adjusted");
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
        notifyChange(inPin);
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
  Serial.println("Finish setup");
  pinMode(2, OUTPUT);
}

void loop() {
  di1.Check();
  di2.Check();
  mesh.update();
}
