#include <Servo.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h> // Neopixel을 사용하기 위해서 라이브러리를 불러옵니다.
#include <Vector.h>
#include <swRTC.h>
swRTC rtc;
String inputStr;
bool accordTime{false}, accordDay{false};
int percentage;

const int ELEMENT_COUNT_MAX = 50;
typedef Vector<int> Percentages;
typedef Vector<String> Times;
typedef Vector<String> Days;
Percentages percentages;
Times times;
Days days;

Servo myservo;

String incommingByte = "";
// Which pin on the Arduino is connected to the NeoPixels?
int LEDPIN = 12;
// How many NeoPixels are attached to the Arduino?
int NUMPIXELS = 60;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRBW + NEO_KHZ800);

//======================
char check = 0;
String applicationInput{""};
String gestureInput{""};
bool applicationCompleted = false;
bool gestureCompleted = false;
int val = 0;
bool isGestureOn = false;

void OperateBlind(int percentage)
{
    Serial.println("operate blind");
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
    pixels.begin(); // This initializes the NeoPixel library.

    int storage_int[ELEMENT_COUNT_MAX];
    String storage_times[ELEMENT_COUNT_MAX];
    String storage_days[ELEMENT_COUNT_MAX];
    percentages.setStorage(storage_int);
    times.setStorage(storage_times);
    days.setStorage(storage_days);
    percentages.clear();
    times.clear();
    days.clear();

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
    String dowStr = "";

    switch (dow)
    {
    case 0:
        dowStr = "일";
        break;
    case 1:
        dowStr = "월";
        break;
    case 2:
        dowStr = "화";
        break;
    case 3:
        dowStr = "수";
        break;
    case 4:
        dowStr = "목";
        break;
    case 5:
        dowStr = "금";
        break;
    case 6:
        dowStr = "토";
        break;
    }

    while (Serial2.available())
    {
        char data = (char)Serial2.read();
        applicationInput += data;

        if (data == '\n')
        {
            applicationCompleted = true;
        }
    }

    if (applicationCompleted)
    {
        Serial.println(applicationInput);
        applicationCompleted = false;
        int val = applicationInput.toInt();

        if (val == 1000)
        {
            Serial.print("gesture on");
            isGestureOn = true;
        }
        else if (val == 2000)
        {
            Serial.print("gesture off");
            isGestureOn = false;
        }

        else if (applicationInput.startsWith("delete"))
        {
            int idx = ((String)applicationInput[applicationInput.length() - 1]).toInt();
            Serial.print("idx : ");
            Serial.println(idx);

            percentages.remove(idx);
            times.remove(idx);
            days.remove(idx);
        }

        else if (applicationInput.length() <= 3)
        {
            percentage = applicationInput.toInt();
            Serial.print("percentage ");
            Serial.println(percentage);
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

            Serial.print("percentage : ");
            Serial.println(val);
            Serial.print("time : ");
            Serial.println(str2);
            Serial.print("day : ");
            Serial.println(str3);
        }
    }
    // Serial –> Data –> BT

    int vectorSize = percentages.size();
    for (int i = 0; i < vectorSize; i++)
    {
        accordTime = false;
        accordDay = false;

        int alarmHour = times[i].substring(0, times[i].indexOf(':')).toInt();
        int alarmMinute = times[i].substring(times[i].indexOf(':') + 1, times[i].length()).toInt();

        if (alarmHour == currHour and alarmMinute == currMinute and currSecond == 0)
        {
            accordTime = true;
        }
        //    if(accordTime){
        //      Serial.println("Accord Time");
        //      for(int j=0;j<days[i].length();j++){
        //        if((String)days[i][j] == dowStr){
        //          Serial.println("Accord Day");
        //          accordDay = true;
        //        }
        //      }
        //    }
        //    if(accordTime and accordDay){
        //      OperateBlind(percentages[i]);
        //    }
    }

    if (isGestureOn)
    {
        while (Serial1.available())
        {
            delay(85);
            String data = Serial1.readString();
            Serial.println(data);

            gestureCompleted = false;
            int val = data.toInt();

            switch (val)
            {
            case 0: // STOP
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
            }

            pixels.setBrightness(val - 1);
            pixels.show();
        }
    }
    if (!isGestureOn)
    {
        Serial1.read();
    }

    if (Serial.available())
    {

        Serial1.write(Serial.read());
    }
    delay(50);
    applicationInput = "";
}
