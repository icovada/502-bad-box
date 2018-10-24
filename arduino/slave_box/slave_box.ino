class Input{
  int inPin;
  bool oldpin;

  public: Input(int pin){
    inPin = pin;
    pinMode(inPin, INPUT);
    oldpin = digitalRead(inPin);
    Serial.print("INIT pin ");
    Serial.println(inPin);
  }

  void Check(){
   Serial.print("Pin ");
   Serial.print(inPin);
   Serial.print(" ");
   Serial.println(digitalRead(inPin));
    if (digitalRead(inPin) && !oldpin){
      Serial.print("CHANGED STATE ");
      Serial.println(inPin);
      oldpin = digitalRead(inPin);
    }
    if (!digitalRead(inPin) && oldpin){
      oldpin = digitalRead(inPin);
    }
  }
};

Input di1(4);
Input di2(5);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  di1.Check();
  di2.Check();
  Serial.println("Loop");
  delay(200);
}
