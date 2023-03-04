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

//set length: shutter open for a set time, blub  on camera, manual focus
//mutli exposure: shutter open for a set time at a set interval, blub on camera, manual focus
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

int delayLengthM = 0;
int delayLengthS = 0;
int delayLengthMs = 0;

bool willFocus = 0;

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
      mainMenu();
      break;
    case 0:
      setLength();
      break; 
    case 1:
      miltiExp();
      break;
    case 2:
      picTiemr();
      break;
    case 3:
      remoteShutter();
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
  delay(10);
}
//relases the shutter if its a longer exposure
void longOff(){
  digitalWrite(shutterPin,LOW);
}

//writes the current screen to the top of the oled screen
void oledHeader(){
  display.setCursor(3,3);
  display.print(menus[currentMenu]);
}

void mainMenu(){
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
}

//takes a photo with a two second delay and keeps the shutter open for as long as the user inputs
//somewhat tested
void setLength(){
  //when the joystick is moved right and left change numIndex
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
  //when the joystick is moved down decrease the value at numIndex with the lowest being 0
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
  //when the joystick is moved up incrase the value at numIndex
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
    Serial.print(expotimeToMilli());
    delay(expotimeToMilli());
    Serial.print("done");
    longOff();
  }
}

//after two seconds takes a set number of photos and keeps the shutter open for a set time with a set delay inbetween every photo
void miltiExp(){
  //boilerplate input changing
  //for loop to take photo
}

//takes a photo after user inputed time delay with option to focus
void picTiemr(){
  //when the joystick is moved right and left change numIndex
  if(xVal < 80){
    numIndex --;
    if(numIndex<-1){
      numIndex = 3;
    }
    delay(80);
  }
  if(xVal > 950){
    numIndex++;
    if(numIndex > 3){
      numIndex = -1;
    }
    delay(80);
  }
  //when the joystick is moved down decrease the value at numIndex with the lowest being 0
  if(yVal < 80){
    switch (numIndex){
      case 0:
        if(delayLengthM > 0){
          delayLengthM--;
        }
        break;
      case 1:
        if(delayLengthS > 0){
          delayLengthS--;
        }
        break;
      case 2:
        if(delayLengthMs > 0){
          delayLengthMs--;
        }
        break;
      case 3:
        willFocus = 0;
        break;
    }
  }
  //when the joystick is moved up incrase the value at numIndex
  if(yVal > 950){
    switch (numIndex){
      case 0:
        delayLengthM++;
        break;
      case 1:
        delayLengthS++;
        break;
      case 2:
        delayLengthMs++;
        break;
      case 3:
        willFocus = 1;
        break;
    }
  }
  //displays the current state of needed variables
  display.setCursor(3,20);
  display.print(delayLengthM);
  display.print(":");
  display.print(delayLengthS);
  display.print(":");
  display.println(delayLengthMs);
  display.print("   ");
  display.print(willFocus);
  //displays what is being changed at a certain numIndex
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
     case 3:
      display.print("Focus");
      break;
    case -1:
      display.print("Okay?");
      break;
  }
  if(numIndex == -1 && buttonState == 0){
    display.print(" shooting");
    Serial.print("starting");
    Serial.print(delayTimeToMilli());
    delay(delayTimeToMilli());
    shutter(willFocus, false);
    Serial.print("done");
  }
}

//takes a photo with option to focus
void remoteShutter(){
  //when the joystick is moved up try and focus
  if(yVal > 950){
    focus();
  }
  //if the joystick is moved to the right or the left change the value of the numIndex
  if(xVal < 90){
    numIndex --;
    if(numIndex<-1){
      numIndex = 0;
    }
    delay(80);
  }
  if(xVal > 950){
    numIndex++;
    if(numIndex > 0){
      numIndex = -1;
    }
    delay(80);
  }
  //if the index is -1 and the button is pressed take a photo
  if(numIndex == -1 && buttonState == 0){
    shutter(false,false);
  }
}
//converts minutes seconds and milli seconds into the sum of milli seconds for the delay
int expoTimeToMilli(){
  int total = 0;
  total = expoLengthM * 60 * 1000;
  total = total + expoLengthS * 1000;
  total = total + expoLengthMs;
  return total;
}

int delayTimeToMilli(){
  int total = 0;
  total = delayLengthM * 60 * 1000;
  total = total + delayLengthS * 1000;
  total = total + delayLengthMs;
  return total;
}
