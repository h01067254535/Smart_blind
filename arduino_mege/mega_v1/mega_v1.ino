#include <Servo.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h> // Neopixel을 사용하기 위해서 라이브러리를 불러옵니다.
      
Servo myservo;

String incommingByte = "";
// Which pin on the Arduino is connected to the NeoPixels?
int LEDPIN = 12;
// How many NeoPixels are attached to the Arduino?
int NUMPIXELS = 60;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRBW + NEO_KHZ800);
int delayval = 500; // delay for half a second

//======================
char check = 0; 
String applicationInput{""};
String gestureInput{""};
bool applicationCompleted = false;
bool gestureCompleted = false;
int val = 0;
bool isGestureOn = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello!");
  Serial1.begin(9600); //PC
  Serial1.setTimeout(10);
  Serial2.begin(9600); //application
  //=====
  myservo.attach(9);
  pixels.begin(); // This initializes the NeoPixel library.
}
 
void loop() {
 applicationInput = "";
 gestureInput = "";
  // put your main code here, to run repeatedly:
  
  while(Serial2.available()) {
    char data = (char)Serial2.read();
    applicationInput += data;

    if(data == '\n'){
      applicationCompleted = true;
    }
  }

  if(applicationCompleted){
    applicationCompleted = false;
    int val = applicationInput.toInt();

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
    while(Serial1.available()) {
        delay(85);
        String data = Serial1.readString();
        Serial.println(data);
      
    
      gestureCompleted = false;
      int val = data.toInt();
          
          switch(val) {
          case 0: //STOP
          myservo.write(90);
          break;
          
          case -6:
          myservo.write(0);
          break;
               
          case -7:
          myservo.write(180); 
          break;
          
          case -1:
          pixels.setBrightness(10);
          for (int i=0; i<60; i++) {
            pixels.setPixelColor(i, pixels.Color(150,0,0));
            pixels.show();
            }
          break;
          
          case -2:
          pixels.setBrightness(10);
          for (int i=0; i<60; i++) {
            pixels.setPixelColor(i, pixels.Color(0,150,0));
            pixels.show();
            }
          break;

          case -3:
          pixels.setBrightness(10);
          for (int i=0; i<60; i++) {
            pixels.setPixelColor(i, pixels.Color(0,0,150));
            pixels.show();
            }
          break;

          case -4:
          pixels.setBrightness(10);
          for (int i=0; i<60; i++) {
            pixels.setPixelColor(i, pixels.Color(150,150,150));
            pixels.show();
            }
          break;
          
          case -5:
          pixels.setBrightness(10);
          for (int i=0; i<60; i++) {
            pixels.setPixelColor(i, pixels.Color(150,0,150));
            pixels.show();
            }
          break;
          }
          
          pixels.setBrightness(val-1);
          pixels.show();
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
