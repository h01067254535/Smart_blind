String incommingByte = "";
#include <Servo.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h> // Neopixel을 사용하기 위해서 라이브러리를 불러옵니다.
      
Servo myservo;

// Which pin on the Arduino is connected to the NeoPixels?
int LEDPIN = 12;
// How many NeoPixels are attached to the Arduino?
int NUMPIXELS = 60;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRBW + NEO_KHZ800);
int delayval = 500; // delay for half a second

//======================
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
  Serial1.setTimeout(100);
  Serial2.begin(9600); //application
  Serial2.setTimeout(100);
  //=====
  myservo.attach(9);
  pixels.begin(); // This initializes the NeoPixel library.
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
    while(Serial1.available()) {
        int data = 0;
        incommingByte = Serial1.readString();
        data = incommingByte.toInt();
        if (data <= 0) {
          switch(data) {
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

        }
        else {
          pixels.setBrightness(data-1);
          pixels.show();
        }
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
