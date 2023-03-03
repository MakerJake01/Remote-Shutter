#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//defines the shutter pins
int shutterPin = D8;
int focusPin = D7;

//defines the joystick pins
#define Xjoy A1
#define Yjoy A0

//curernt position of the joystick
int xVal = 0;
int yVal = 0;

//button setup
boolean buttonState = false;
#define buttonPin D2

//set length: shutter open for a set time, blub  on camera
//mutli exposure: shutter open for a set time at a set interval, blub on camera
//timer: shutter opens aftern a countdown, shutter set on camera, possible focus
//remote shutter: when you hit the button the shutter triggers, shutter set on camera, possible option for focus 
String menuOptions[] = {"set length", "mutli exposures", "timer", "remote shutter"};
int selected = 0;

String menus[] = {"set length", "mutli exposures", "timer", "remote shutter","main menu"};
int currentMenu = 4;

int numIndex = 0;
int expoLengthM = 0;
int expoLengthS = 0;
int expoLengthMs = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(shutterPin,OUTPUT);
  pinMode(focusPin,OUTPUT);

  pinMode(Xjoy, OUTPUT);
  pinMode(Yjoy, OUTPUT);

  pinMode(buttonPin, INPUT);

  longOff();
  
  //oled setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.cp437(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  oledHeader();
  xVal = analogRead(Xjoy);
  yVal = analogRead(Yjoy);

  buttonState = digitalRead(buttonPin);
  if(buttonState == 0){
    delay(400);
    if(currentMenu == 4){
      currentMenu = selected;
    }else{
      if(numIndex != -1){
        currentMenu = 4;
      }
    }
  }
  
  //display the options
  switch(currentMenu){
    case 4:
      for(int i = 0; i < 4; i++){
        display.setCursor(2,19+(i*12));
        if(i == selected){
          display.setTextColor(SSD1306_INVERSE);
          display.fillRect(0,17+i*12,128,12,SSD1306_WHITE);
        }else{
          display.setTextColor(SSD1306_WHITE);
        }
        display.print(menuOptions[i]);
      }
      if(yVal>950){
        selected --;
        if(selected < 0){
          selected = 3;
        }
        delay(100);
      }
      if(yVal<80){
        selected ++;
        if(selected > 3){
          selected = 0;
        }
        delay(100);
      }
      break;
    case 3:
      
      break;
    case 2:
      
      break;
    case 1:
      
      break;
    case 0:
      setLength();
      break; 
  }

  //display.drawPixel(map(xVal,0,1023,0,128), 63 - map(yVal,0,1023,0,64), SSD1306_WHITE);
  //display.println(yVal);
  //display.print(xVal);
  display.display();
}

//activates the shutter
//can try and focus but there is no way to check if it worked
//automatically releases the shutter unless {longer} is set to true
void shutter(bool doFocus, bool longer){
  if(doFocus==true){
    focus();
  }
  delay(10);
  digitalWrite(shutterPin,HIGH);
  delay(10);
  if(longer == false){
    delay(240);  
    digitalWrite(shutterPin,LOW);
  }
}
//pulls the focus pin high and then low
void focus(){
  digitalWrite(focusPin,HIGH);
  delay(250);
  digitalWrite(focusPin,LOW);
}
//relases the shutter if its a longer exposure
void longOff(){
  digitalWrite(shutterPin,LOW);
}

void oledHeader(){
  display.setCursor(3,3);
  display.print(menus[currentMenu]);
}

void setLength(){
  if(xVal < 80){
      numIndex --;
      if(numIndex<-1){
        numIndex = 2;
      }
      delay(80);
    }
    if(xVal > 950){
      numIndex++;
      if(numIndex > 2){
        numIndex = -1;
      }
      delay(80);
    }
    if(yVal < 80){
      switch (numIndex){
        case 0:
          if(expoLengthM > 0){
            expoLengthM--;
          }
          break;
        case 1:
          if(expoLengthS > 0){
            expoLengthS--;
          }
          break;
        case 2:
          if(expoLengthMs > 0){
            expoLengthMs--;
          }
          break;
      }
    }
    if(yVal > 950){
      switch (numIndex){
        case 0:
          expoLengthM++;
          break;
        case 1:
          expoLengthS++;
          break;
        case 2:
          expoLengthMs++;
          break;
      }
    }
    display.setCursor(3,20);
    display.print(expoLengthM);
    display.print(":");
    display.print(expoLengthS);
    display.print(":");
    display.println(expoLengthMs);
    switch (numIndex){
      case 0:
        display.print("Mins");
        break;
      case 1:
        display.print("Secs");
        break;
      case 2:
        display.print("Millis");
        break;
      case -1:
        display.print("Okay?");
        break;
    }
    if(numIndex == -1 && buttonState == 0){
      display.print(" shooting");
      Serial.print("starting");
      delay(2000);
      shutter(false,true);
      Serial.print(timeToMilli());
      delay(timeToMilli());
      Serial.print("done");
      longOff();
    }
}

void miltiExp(){
  
}

void picTiemr(){
  
}

void remoteShutter(){
  
}
int timeToMilli(){
  int total = 0;
  total = expoLengthM * 60 * 1000;
  total = total + expoLengthS * 1000;
  total = total + expoLengthMs;
  return total;
}
