#include <ArduinoSTL.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h> // Neopixel을 사용하기 위해서 라이브러리를 불러옵니다.
#include <swRTC.h>
#include <time.h> 
#define total_time 52

using namespace std;

swRTC rtc;
String inputStr;
bool accordTime{false}, accordDay{false};
int percentage;
int start_time;
int end_time;
int working_time;
bool check = true;
bool isUp;
Servo myservo;

String incommingByte = "";
// Which pin on the Arduino is connected to the NeoPixels?
int LEDPIN = 12;
// How many NeoPixels are attached to the Arduino?
int NUMPIXELS = 60;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRBW + NEO_KHZ800);

double inc_t = 100.0/(double)total_time;
int blind_h = 0;
double time_delay = 0; 
int current_h = 0;

//======================
String applicationInput{""};
String gestureInput{""};
bool applicationCompleted = false;
bool gestureCompleted = false;
int val = 0;
bool isGestureOn = false;

vector<int> percentages;
vector<String> times;
vector<String> days;

void OperateBlind(int percentage)
{
  time_delay = abs(percentage-current_h)/inc_t;
  Serial.print("time_delay : ");
  Serial.println(time_delay);
  if (current_h < percentage){
     myservo.write(180);
     //current_h += blind_h;
     //Serial.println(current_h);
  }
  else{
    myservo.write(0);
    //current_h -= blind_h;
    delay(750);
  }
  current_h = percentage;
  Serial.print("curr_h-app : ");
  Serial.println(current_h);
  delay(time_delay*1000);
  myservo.write(90);
}

int GetDayOfWeek(int y, int m, int d)
{
    int date;
    int c;

    if (m <= 2)
    {
        m += 12;
        --y;
    }

    c = y / 100;
    y = y % 100;

    date = (21 * c / 4 + 5 * y / 4 + 26 * (m + 1) / 10 + d - 1);
    date = date % 7;

    return date;
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("Hello!");
    Serial1.begin(9600); // PC
    Serial1.setTimeout(10);
    Serial2.begin(9600); // application
    //=====
    myservo.attach(9);
    myservo.write(90);
    pixels.begin(); // This initializes the NeoPixel library.

    rtc.stopRTC();
    rtc.setTime(20, 0, 45);
    rtc.setDate(22, 5, 2022);
    rtc.startRTC();

    Serial.println("Start");
}

void loop()
{
    applicationInput = "";
    gestureInput = "";
    // put your main code here, to run repeatedly:
    int currHour = rtc.getHours();
    int currMinute = rtc.getMinutes();
    int currSecond = rtc.getSeconds();
    int dow = GetDayOfWeek(rtc.getYear(), rtc.getMonth(), rtc.getDay());

    while (Serial2.available())
    {
        char data = (char)Serial2.read();
        if (data == '\n')
        {
            applicationCompleted = true;
            break;
        }
        applicationInput += data;

        delay(50);
    }

    if (applicationCompleted)
    {
        applicationCompleted = false;
        int val = applicationInput.toInt();

        if (val == 1000)
        {
            isGestureOn = true;
        }
        else if (val == 2000)
        {
            isGestureOn = false;
        }

        else if (applicationInput.startsWith("delete"))
        {
            int idx = ((String)applicationInput[applicationInput.length() - 1]).toInt();

            percentages.erase(percentages.begin() + idx);
            times.erase(times.begin() + idx);
            days.erase(days.begin() + idx);
        }

        else if (applicationInput.length() <= 3) //percentage of blind 
        {
            percentage = applicationInput.toInt();
            OperateBlind(percentage);
            
        }

        else if (applicationInput.length() > 5)
        {
            int firstIdx = applicationInput.indexOf(',');
            int secondIdx = applicationInput.indexOf(',', firstIdx + 1);
            int strLength = applicationInput.length();

            String str1 = applicationInput.substring(0, firstIdx);
            String str2 = applicationInput.substring(firstIdx + 1, secondIdx);
            String str3 = applicationInput.substring(secondIdx + 1, strLength);

            int val = str1.substring(0, str1.indexOf('%')).toInt();

            percentages.push_back(val);
            times.push_back(str2);
            days.push_back(str3);
        }
    }
    // Serial –> Data –> BT

    for (int i = 0; i < percentages.size(); i++)
    {
        accordTime = false;
        accordDay = false;

        int alarmHour = times[i].substring(0, times[i].indexOf(':')).toInt();
        int alarmMinute = times[i].substring(times[i].indexOf(':') + 1, times[i].length()).toInt();

        if (alarmHour == currHour and alarmMinute == currMinute and currSecond == 0)
        {
            accordTime = true;
        }
        if(accordTime){
          Serial.println("Accord Time");
          for(int j=0;j<days[i].length();j++){
            if(((String)(days[i][j])).toInt() == dow){
              Serial.println("Accord Day");
              accordDay = true;
            }
          }
        }
        if(accordTime and accordDay){
          OperateBlind(percentages[i]);
        }
    }
    if (isGestureOn)
    {
        while (Serial1.available())
        {
            
            String data = Serial1.readString();
            Serial.println(data);

            gestureCompleted = false;
            int val = data.toInt();

            switch (val)
            {
            case 0: // STOP
                if(!check) {
                  end_time = rtc.getSeconds();
                  myservo.write(90);
                  check = true;
                  if (end_time>start_time) {
                    working_time = end_time-start_time;
                  }
                  else {
                    working_time = end_time + 60 - start_time;  
                  }
                  if(isUp) {
                    current_h = current_h - working_time*2;
                    if(current_h < 0) current_h = 0;
                  }
                  else {
                    current_h = current_h + working_time*2;
                    if(current_h > 100) current_h = 100;
                  }
                  Serial.print("curr_h-ges " );
                  Serial.println(current_h);
                  Serial.print("working time-ges " );
                  Serial.println(working_time);
                  break;
                }
           

            case -6: //up 
                if(check) {
                  check = false;
                  start_time = rtc.getSeconds();
                  myservo.write(0);
                  isUp = true;
                  break;
                 }

            case -7:
                if(check) {
                  check = false;
                  start_time = rtc.getSeconds();
                  myservo.write(180);
                  break;
                  isUp = false;
                 }

            case -1:
                pixels.setBrightness(10);
                for (int i = 0; i < 60; i++)
                {
                    pixels.setPixelColor(i, pixels.Color(150, 0, 0));
                    pixels.show();
                }
                break;

            case -2:
                pixels.setBrightness(10);
                for (int i = 0; i < 60; i++)
                {
                    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
                    pixels.show();
                }
                break;

            case -3:
                pixels.setBrightness(10);
                for (int i = 0; i < 60; i++)
                {
                    pixels.setPixelColor(i, pixels.Color(0, 0, 150));
                    pixels.show();
                }
                break;

            case -4:
                pixels.setBrightness(10);
                for (int i = 0; i < 60; i++)
                {
                    pixels.setPixelColor(i, pixels.Color(150, 150, 150));
                    pixels.show();
                }
                break;

            case -5:
                pixels.setBrightness(10);
                for (int i = 0; i < 60; i++)
                {
                    pixels.setPixelColor(i, pixels.Color(150, 0, 150));
                    pixels.show();
                }
                break;
            
            default:
              pixels.setBrightness(val - 1);
              pixels.show();
        }
       }
    }
    if (!isGestureOn)
    {
        Serial1.read();
    }
    delay(50);
}
