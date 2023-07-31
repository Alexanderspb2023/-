#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>


/*
 * Данные COM-Port
 */
#define rxPin 9   //Определяем вывод RX для ComPort
#define txPin 10  //Определяем вывод TX для ComPort
SoftwareSerial MySerial = SoftwareSerial(rxPin, txPin); //Сериализуем comPort
String RxData; //Строка, которую получаем с COM-Port


/*
 * Данные дисплея
 */
LiquidCrystal_I2C lcd(0x3F,20,4); //Объявляем  объект библиотеки c нашими параметрами
unsigned long TimeUpdateDisplay = 100; //Частота обновления дисплея в мкс
unsigned long TimerDisplay; //Переменная для сравнения с разницой во времени, для обновления дисплея 


/*
 * Данные компаса
 */
#define compassAddr 0x1E // I2C 7-битный адрес компаса
float headingDegrees = 0.0;//Отклонение от севера с компаса 
float anglAntenna = 0.0;//Позиция антены
///Переменные для компаса 
float anglAntennaUp = 0.0;//север относительно координат антены


/*
 * Данные первого шагового двигателя + концевики
 */
#define dirPin_1 4 //пин для подключения контакта DIR
#define stepPin_1 5 //пин для подключения контакта STEP
unsigned long TimeDelayStepMotor = 3000; //Задержка между шагами в мкс
unsigned long MinTimeDelayStepMotor = 1000; //Минимальная задержка между шагами в мкс
unsigned long MaxTimeDelayStepMotor = 3000; //Максимальная задержка между шагами в мкс
unsigned long TimerStepDelay; //Переменная для сравнения с разницой во времени, для вращения двигателя

#define firstBarrier 7
#define secondBarrier 8


/*
 * Данные второго шагового двигателя + концевики
 */
#define dirPin_2 0 //пин для подключения контакта DIR
#define stepPin_2 1 //пин для подключения контакта STEP
//unsigned long TimeDelayStepMotor = 1000; //Задержка между шагами в мкс
//unsigned long TimerStepDelay; //Переменная для сравнения с разницой во времени, для вращения двигателя

#define threeBarrier 12
#define fourBarrier 13



/*
 * Данные с Orbitron
 */
String az = "";  //Азимут отслеживаемого спутника из Orbitron
String el = "";  //Высота отслеживаемого спутника из Orbitron
String el_true = "";


/*
 * Данные расчитанные при инициализации
 */
float angleXZ = 0.0;        //угл на один шаг горизонтальной оси
float angleOY = 0.0;        //угл на один шаг вертикальной оси
int MaxStepsOneRotateXZ = 0;//кол-во шагов на один оборот от границы до границы X-Z
int MaxStepsOneRotateOY = 0;//кол-во шагов на один оборот от границы до границы X-Y






bool displayChange = true;
float page = 0;
float page_prev = page;



void setup() 
{
  /*
   * Инициализация дисплея
   */
  lcd.init();            
  lcd.backlight(); 
  lcd.setCursor(0,0);     
  lcd.print("Loading...");
  
  delay(100);


  
  printAnotherMessage(0,0,"Initialization...");
  printAnotherMessage(0,1,"COM-Port");
  /*
   * Инициализация COM-Port
   */
  pinMode(rxPin, INPUT);    //Устанавливаем определенный пин на RX
  pinMode(txPin, OUTPUT);   //Устанавливаем определенный пин на TX
  MySerial.begin(9600);



  printAnotherMessage(0,1,"I2C");
  /*
   * Инициализация I2C канала
   */
  Serial.begin(9600);       // инициализация последовательного порта 
  Wire.begin();             // инициализация I2C
  


  printAnotherMessage(0,1,"Main driver");
  /*
   * Установка различных значений на пины
   */
  //Двигатель 1(основание антенны)
  pinMode(stepPin_1, OUTPUT); //Устанавливаем определенный STEPпин на выход 
  pinMode(dirPin_1, OUTPUT);  //Устанавливаем определенный DIRпин на выход 
  digitalWrite(stepPin_1, 1); //Стартовые значения
  digitalWrite(dirPin_1, 0);  //Стартовые значения



  printAnotherMessage(0,1,"Top driver");
  //Двигатель 2(верх антенны)
  pinMode(stepPin_2, OUTPUT); //Устанавливаем определенный STEPпин на выход 
  pinMode(dirPin_2, OUTPUT);  //Устанавливаем определенный DIRпин на выход 
  digitalWrite(stepPin_2, 1); //Стартовые значения
  digitalWrite(dirPin_2, 0);  //Стартовые значения



  printAnotherMessage(0,1,"Limit switches");
  //Концевики
  pinMode(firstBarrier, INPUT_PULLUP);  //Устанавливаем барьер на 7ой пин
  pinMode(secondBarrier, INPUT_PULLUP); //Устанавливаем барьер на 8ой пин
  pinMode(threeBarrier, INPUT_PULLUP);  //Устанавливаем барьер на 11ой пин
  pinMode(fourBarrier, INPUT_PULLUP);   //Устанавливаем барьер на 12ой пин


  
  //SetAnglForBarrier();    //Запуск движения для расчета различных значений

  
  headingDegrees = 90;    //Временная постановка направления компаса
}




/*
 * Вывод различных сообщений на дисплей, ошибки и другое
 */
void printAnotherMessage(int x, int y, String msg)
{ 
  //Как то сделать отчистку СТРОКИ от предыдущего текста
  lcd.setCursor(x,y);     
  lcd.print(msg);
}





bool isCheck1 = false;
bool isCheck2 = false;
bool isCheck3 = false;
bool isCheck4 = false;
bool isCheck1and2 = false;
bool isCheck3and4 = false;

/*
 * Проверка на аварийную ситуацию(ПЕРЕКРУТ АНТЕННЫ)
 */
 //дописать
bool Check1AndCheck2_1 = true;
bool Check1AndCheck2_2 = true;
bool Check1AndCheck2_3 = true;
bool Check1AndCheck2_4 = true;
bool checkLimitSwitches()
{
  if(!digitalRead(firstBarrier)) isCheck1 = true;
  else isCheck1 = false;
  
  if(!digitalRead(secondBarrier)) isCheck2 = true;
  else isCheck2 = false;
  
  if(digitalRead(threeBarrier)) isCheck3 = true;
  else isCheck3 = false;
  
  if(digitalRead(fourBarrier)) isCheck4 = true;
  else isCheck4 = false;

  //1 - trigger
  //2 - trigger
  //1 - untrigger
  //2 - untrigger

  if(Check1AndCheck2_1 and isCheck2 and !isCheck1) Check1AndCheck2_2 = true;
  else Check1AndCheck2_2 = false;
  
  if(isCheck1 and isCheck2) Check1AndCheck2_1 = true;
  else Check1AndCheck2_1 = false;
  
  
  
  if(Check1AndCheck2_2 and !isCheck2) Serial.println("Error");

  
  if(isCheck3 and isCheck4) Serial.println("Error check3 and check4 trigger!");


  
  Serial.println("Check1: " + (String)isCheck1 + "Check2: " + (String)isCheck2);
  Serial.println("Check3: " + (String)isCheck3 + "Check4: " + (String)isCheck4);
}



void loop() 
{
  //Чтение из COM-порта значений,
  while(MySerial.available() > 0) { //Заменил if на while, если не будет работать, вернуть
      bool firstAngle = true;
      RxData = MySerial.readString();
      az = "";
      el = "";
      el_true = "";
      for(int i = 0; i < RxData.length(); i++){
        if(RxData[i] == ',')
        {
          firstAngle = false;
          continue;
        }
        if(RxData[i] != 'A' and RxData[i] != 'Z' and RxData[i] != ':' and RxData[i] != 'E' and RxData[i] != 'L'){
          if(firstAngle) az += RxData[i];
          else el += RxData[i];
        }
      }
      
      int k = 0;
      while (true)
      {
        el_true += el[k];
        if (el[k] == '.')
        {
          el_true += el[k+1];
          break;
        }
        k += 1;
      }

      //Исправление входных данных
      if(az.toFloat() > 360 or az.toFloat() < 0) az = abs(az.toFloat()) - 360 * round(az.toFloat()/360);
      if(el.toFloat() > 180 or el.toFloat() < -180) el = abs(el.toFloat()) - 360 * round(el.toFloat()/360);
      
      AntennaBaseMovement();
      TopMovement();
  }
  checkLimitSwitches();
  if(micros()- TimerDisplay > TimeUpdateDisplay){
    //Обновление дисплея
    DisplayUpdate(displayChange);
    displayChange = false;
    //if (page_prev != page) lcd.clear();
    TimerDisplay = micros();
  }
}




/*
 * Вывод на дисплей
 */
void DisplayUpdate(bool changePage)
{
  if(changePage)
  {
      lcd.setCursor(0,0);
      lcd.print("AZ:       ");
      lcd.setCursor(10,0);
      lcd.print("EL: ");
      
      lcd.setCursor(0,1);
      lcd.print("Comp az: ");

      lcd.setCursor(0,2);
      lcd.print("Ante az: ");
      
      lcd.setCursor(9,3);
      lcd.print("<1>");
      lcd.setCursor(18,3);
      lcd.print("<2");
  }
  lcd.setCursor(4,0);
  lcd.print(az);
  lcd.setCursor(14,0);
  lcd.print(el_true);
  lcd.setCursor(9,1);
  lcd.print((String)headingDegrees);
  lcd.setCursor(9,2);
  lcd.print((String)anglAntenna);
}



/*
 * Движение основания антенны до определенной позиции
 */
void AntennaBaseMovement()
{
    while(true){
      
      if (micros()-TimerStepDelay >= TimeDelayStepMotor) {
          if(az.toFloat() < headingDegrees){
            if(360-abs(headingDegrees-az.toFloat()) > anglAntenna){
              digitalWrite(dirPin_1, HIGH); //Вращение против часовой стрелки
              anglAntenna += angleXZ;
            }
            else if(360-abs(headingDegrees-az.toFloat()) < anglAntenna){
              digitalWrite(dirPin_1, LOW); //Вращение по часовой стрелке
              anglAntenna -= angleXZ;
            }
          }
          else if(az.toFloat() > headingDegrees){
            if(abs(headingDegrees-az.toFloat()) > anglAntenna){
              digitalWrite(dirPin_1, HIGH); //Вращение против часовой стрелки
              anglAntenna += angleXZ;
            }
            else if(abs(headingDegrees-az.toFloat()) < anglAntenna){
              digitalWrite(dirPin_1, LOW); //Вращение по часовой стрелке
              anglAntenna -= angleXZ;
            }
          }
          
          digitalWrite(stepPin_1, HIGH);
          TimerStepDelay = micros();
          digitalWrite(stepPin_1, LOW);
    
          if(abs(abs(headingDegrees-az.toFloat()) - anglAntenna) < angleXZ and az.toFloat() > headingDegrees)break;
          else if(abs(abs(360-abs(headingDegrees-az.toFloat()) - anglAntenna)) < angleXZ and az.toFloat() < headingDegrees)break;
        }
    }
}



/*
 * Движение верхней части антенны
 */
void TopMovement()
{
    while(true){
      //if(digitalRead(threeBarrier) or digitalRead(fourBarrier)) break;
      
      if(micros()-TimerStepDelay >= TimeDelayStepMotor){
        if(abs(el.toFloat()) < anglAntennaUp){
          digitalWrite(dirPin_2,HIGH);
          anglAntennaUp -= angleOY;
        }
        else if(abs(el.toFloat()) > anglAntennaUp){
          digitalWrite(dirPin_2, LOW);
          anglAntennaUp += angleOY;
        }
        digitalWrite(stepPin_2, HIGH);
        TimerStepDelay = micros();
        digitalWrite(stepPin_2, LOW);
        if(abs(abs(el.toFloat()) - anglAntennaUp) < angleOY) break;
      }
    }
}



/*
 * Движение основания до упора
 */
///<param>направление вращения true - ; false - </param>
void FoundationMovement(bool clockwise)
{
  
  digitalWrite(dirPin_1, (clockwise)? LOW : HIGH);
  TimerStepDelay = 0;
  TimeDelayStepMotor = MaxTimeDelayStepMotor;
  while(true)
  {
    if (micros() - TimerStepDelay > TimeDelayStepMotor){
      
      MaxStepsOneRotateXZ++;
      //Работа двигателя
      digitalWrite(stepPin_1, HIGH);
      TimerStepDelay = micros();
      digitalWrite(stepPin_1, LOW);
      
      
      //Плавный старт
      if(TimeDelayStepMotor > MinTimeDelayStepMotor) TimeDelayStepMotor -= 10;

      //получение сигнала, что достигнут предел
      if((!digitalRead(firstBarrier) and !clockwise) or (!digitalRead(secondBarrier) and clockwise)) break;
    }
  }
}




/*
 * Движение верха до упора
 */
///<param>направление вращения true - ; false - </param>
void Movement(bool clockwise)
{
  digitalWrite(dirPin_2, (clockwise)?LOW:HIGH);
  TimerStepDelay = 0;
  TimeDelayStepMotor = MaxTimeDelayStepMotor;
  while(true)
  {
    if (micros()-TimerStepDelay > TimeDelayStepMotor){
      
      MaxStepsOneRotateOY++;
      //Работа двигателя
      digitalWrite(stepPin_2, HIGH);
      TimerStepDelay = micros();
      digitalWrite(stepPin_2, LOW);

      if(TimeDelayStepMotor > MinTimeDelayStepMotor) TimeDelayStepMotor -= 10;
      
      //получение сигнала, что достигнут предел
      if((digitalRead(threeBarrier) and !clockwise) or (digitalRead(fourBarrier) and clockwise)) break;
      
    }
  }
}



/*
 * Расчет углов движения
 */
void SetAnglForBarrier()
{

  //printAnotherMessage(0,0,"1)");
  //printAnotherMessage(0,1,"2)");
  //printAnotherMessage(0,2,"3)");
  //printAnotherMessage(0,3,"4)");

  
  /*
   * Основание антенны
   */
  //Первая граница  
  FoundationMovement(false); 
  
  //Вторая граница
  MaxStepsOneRotateXZ = 0;
  FoundationMovement(true);
  
  

  angleXZ = (float)360/(float)MaxStepsOneRotateXZ;



  /*
   * Верх антенны
   */
  //Первая граница
  Movement(true); 
  
  //Вторая граница
  MaxStepsOneRotateOY = 0;
  Movement(false);

  angleOY = (float)180/(float)MaxStepsOneRotateOY;


  delay(100);
  lcd.clear();

  printAnotherMessage(0,0,"1)Count steps: " + (String)MaxStepsOneRotateXZ);
  printAnotherMessage(0,1,"Angle" + (String)angleXZ);
  printAnotherMessage(0,2,"2)Count steps: " + (String)MaxStepsOneRotateOY);
  printAnotherMessage(0,3,"Angle" + (String)angleXZ);

  delay(500);
  lcd.clear();
}
