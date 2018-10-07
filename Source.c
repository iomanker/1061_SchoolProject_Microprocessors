#include <Servo.h> 
int ANGRY;
const byte switchPin = 13;
const byte testOut = 1;

typedef struct{
  int highest;
  int lowest;
}Angle;

typedef struct{
  Servo SERVO;
  int nowDegree;
}MOTOR;

enum Action{openCap,closeCap};
Angle BarRotate = {0,120};
Angle CapRotate_left = {80,160};
Angle CapRotate_right = {80,0};
MOTOR Bar,CapLeft,CapRight;
unsigned int pushSwTime;

int rotateAxis(MOTOR &motor, int degree){
  // 伺服馬達 0.1秒/60度 當電壓為4.8V
  const int refms = 120; // for 60 degree
  int diff = abs(motor.nowDegree - degree);
  motor.nowDegree = degree;
  motor.SERVO.write(degree);
  return (refms / 60) * diff;
}

void handleCap(MOTOR &LEFT,MOTOR &RIGHT,Action ac){
  if(ac == closeCap){
    for (int i=1;i<=20;++i){
      rotateAxis(RIGHT,CapRotate_right.highest-(i*4));
      rotateAxis(LEFT,CapRotate_left.highest+(i*4));
      delay(50);
    }
    delay(300);
  }else{
    if (ac == openCap){
      rotateAxis(CapLeft,CapRotate_left.highest);
      rotateAxis(CapRight,CapRotate_right.highest);
      delay(350);
    }
  }
}

int reduceAngry(){
  int result = ANGRY;
  if (ANGRY >= 75){
    result = (int)((result * (-1.0) / 5.0) + 22.0);
  }else{
    if (ANGRY >= 55){
      result = (int)((result * (-3.0) / 20.0) + 16.25);
    }else{
      result = (int)((result * (-7.0) / 55.0) + 15.0);
    }
  }
  if (result > ANGRY){
      result = ANGRY;
  }
  return result;
}

void setup(){
  Serial.begin(9600);
  ANGRY = 0;
  pushSwTime = millis();
  Bar.SERVO.attach(3, 500, 2400);
  CapRight.SERVO.attach(5, 500, 2400);
  CapLeft.SERVO.attach(6, 500, 2400);

  rotateAxis(Bar,BarRotate.lowest);
  rotateAxis(CapLeft,CapRotate_left.lowest);
  rotateAxis(CapRight,CapRotate_right.lowest);
  pinMode(switchPin,INPUT);
  pinMode(testOut,OUTPUT);
  delay(3000);
}

void loop(){
  unsigned long current_time = millis();
  int btn = digitalRead(switchPin);
  Serial.print("btn value: ");
  Serial.println(btn);
  if(btn == 1){
    pushSwTime = current_time;
    ANGRY += random(9,15); // 1 ~ 14
    if (ANGRY > 100)
      ANGRY = 100; // ANGRY 防呆
    if (ANGRY < 55){
      // 正常狀態
      handleCap(CapLeft,CapRight,openCap);
      delay(rotateAxis(Bar,BarRotate.highest)+100);
      delay(rotateAxis(Bar,BarRotate.lowest)+100);
      handleCap(CapLeft,CapRight,closeCap);
    }
    else{
      if (ANGRY < 70){
        // 不耐煩狀態 挾持開關較久
        handleCap(CapLeft,CapRight,openCap);
        delay(rotateAxis(Bar,BarRotate.highest)+2000);
        delay(rotateAxis(Bar,BarRotate.lowest)+100);
        handleCap(CapLeft,CapRight,closeCap);
      }else{
        // 生氣狀態
        handleCap(CapLeft,CapRight,openCap);
        for(int i=0;i<10;++i){
          rotateAxis(CapRight,CapRotate_right.highest-30);
          rotateAxis(CapLeft,CapRotate_left.highest+30);
          delay(100);
          rotateAxis(CapLeft,CapRotate_left.highest);
          rotateAxis(CapRight,CapRotate_right.highest);
          delay(100);
        }
        delay(rotateAxis(Bar,BarRotate.highest)+600);
        rotateAxis(Bar,BarRotate.lowest);
        handleCap(CapLeft,CapRight,closeCap);
      }
    }
  }else{
    if (current_time - pushSwTime > 3200){
      pushSwTime = current_time;
      int reduceTemp = -reduceAngry();
      if (reduceTemp + ANGRY >= 0){
        ANGRY += reduceTemp;
      }
    }
  }
  Serial.print("Angry value: ");
  Serial.println(ANGRY);
}
