#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// Your exact working display constructor
U8G2_SSD1315_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 22, 21);


// --- Button & Toggle Variables ---
       // Remembers if the screen is currently toggled ON or OFF
int buttonState = HIGH;          // Current stable state of the button
int lastButtonState = HIGH;      // Previous reading to check for changes
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 50; // 50ms to ignore bouncy signals
int isOn=LOW;

//Ball Bs
int ballx=20;
int bally=32;
float velocity=0;
float gravity=0.55;
float flapstrenght=3.7;
unsigned long lastFrametime=0;
unsigned long frameInterval=30;
int started=LOW;

//Pipes
const int numpipes=3;
int pipeX[numpipes];
int gapY[numpipes];
int pipeWidth=10;
int pipeSpeed=1;
int pipeSpacing=60;
int gapHeight=28;


bool gameover=false;
int score=0;
bool pipescore[numpipes]={false, false, false};

void setup() {
  u8g2.begin();
  pinMode(17, INPUT_PULLUP);
  u8g2.setFont(u8g2_font_6x10_tf);
  randomSeed(analogRead(0));
  for(int i=0; i<numpipes;i++){
    pipeX[i]= 128+i*pipeSpacing;
    gapY[i]=random(20,50);
  }
}

bool checkcollision(int i){
  bool withinX= (ballx+3>pipeX[i]) && (ballx-3<pipeX[i]+pipeWidth);
  if(withinX){
    bool inGap= (bally-3> gapY[i]-gapHeight/2)&& (bally+3<gapY[i]+gapHeight/2);
    if(!inGap) return true;
  }
  return false;

}

void loop() {
  int reading=digitalRead(17);
  if(reading!=lastButtonState) lastDebounceTime=millis();
  if((millis()-lastDebounceTime)>debounceDelay){

    if(reading!=buttonState){
      
      buttonState=reading;
      if(buttonState==LOW){
        
        if(gameover){
          gameover=false;
          bally=32;
          velocity=0;
          score=0;
          started=LOW;

          for(int i=0; i<numpipes; i++){
            pipeX[i]=128 +i*pipeSpacing;
            gapY[i]=128 +i*pipeSpacing;
            gapY[i]=random(20,50);
            pipescore[i]=false;
          }

        }
        else{
          velocity=-flapstrenght;
          started=HIGH;
        }
      }

    }
    

  }
  lastButtonState=reading;
  if((millis()-lastFrametime)>frameInterval  &&started && !gameover){
    
    velocity+=gravity;
    bally+=velocity;
    if(bally<3){bally=3; velocity=0;}
    if(bally>60){bally=60; velocity=0;}
    for(int i=0; i<numpipes; i++){
      pipeX[i]-=pipeSpeed;
      if(!pipescore[i]&& (pipeX[i]+pipeWidth<ballx)){
        score++;
        pipescore[i]=true;
      }
      if (pipeX[i] < -pipeWidth) {
        // Find the furthest pipe currently on the screen
        int maxX = 0;
        for (int j = 0; j < numpipes; j++) {
          if (pipeX[j] > maxX) {
            maxX = pipeX[j];
          }
        }
        
        // Spawn the new pipe perfectly spaced behind the furthest one
        pipeX[i] = maxX + pipeSpacing;
        gapY[i] = random(gapHeight/2 + 10, 64 - gapHeight/2 - 10);
        pipescore[i] = false;   // <-- add this line
         // Reset for next cycle
      }
        if(checkcollision(i)){
          gameover=true;
        }
    } 

    

    
  }
  u8g2.clearBuffer();
  u8g2.drawDisc(ballx, bally, 3);
  for(int i=0; i<numpipes; i++){
    u8g2.drawBox(pipeX[i], 0, pipeWidth, gapY[i]-gapHeight/2);
    u8g2.drawBox(pipeX[i], gapY[i]+gapHeight/2,pipeWidth ,64-(gapY[i]+gapHeight/2));
  }
  u8g2.setCursor(0, 10);
  u8g2.print(score);
  if(gameover){
    u8g2.drawStr(35, 32, "GAME OVER");
  }
  u8g2.sendBuffer();
  
}