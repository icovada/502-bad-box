class Input{
  int inPin;
  bool oldpin;
  long debounce;

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
    if (millis() > debounce + 10){
      if (!digitalRead(inPin) && oldpin){
        Serial.print("PRESSED ");
        Serial.println(inPin);
        oldpin = digitalRead(inPin);
        debounce = millis();
      }
      if (digitalRead(inPin) && !oldpin){
        oldpin = !digitalRead(inPin);
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
}

void loop() {
  di1.Check();
  di2.Check();
}
