char check = 0; 

String inputStr{""};
bool completed = false;
int val = 0;
bool isGestureOn = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello!");
  Serial1.begin(9600); //PC
  Serial2.begin(9600); //application
}
 
void loop() {
 inputStr = "";
  // put your main code here, to run repeatedly:
  
  while(Serial2.available()) {
    char data = (char)Serial2.read();
    inputStr += data;

    if(data == '\n'){
      completed = true;
    }
  }

  if(completed){
    completed = false;
    int val = inputStr.toInt();

    if (val == 1000){
      Serial.print("gesture on");
      isGestureOn = true;
    }
    if(val == 2000){
      Serial.print("gesture off");
      isGestureOn = false;
    }
    
  }
  // Serial –> Data –> BT
  
  if(isGestureOn){
    if (Serial1.available()) {
        Serial.write(Serial1.read());
      }
  }
  if(!isGestureOn){
    Serial1.read();
  }

  if (Serial.available()) 
  {
    
    Serial1.write(Serial.read());
  }
 delay(50);
  
}
