/*
Created 2016
by AlexGyver
AlexGyver Home Labs Inc.
*/

#include <GyverOLED.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

// int buttonPlusPin = 4;
char masschar[5];  //массив символов для перевода
int set,setmass,rapidtime;
boolean initial,flagmass, flagmassset, rapidflag;  //флажки
float dist=0.199;       //расстояние между датчиками в метрах  
const int maxSize=4;

int n=1;            //номер выстрела, начиная с 1
float velocity, energy;    //переменная для хранения скорости
float BbMass=0.20;       //масса снаряда в килограммах
volatile unsigned long gap1, gap2;    //отметки времени прохождения пулей датчиков
unsigned long lastshot;
float speed02 = 0.00;


String SpeedValuesArr[maxSize] = {
    "000",
    "001",
    "002",
  };


void CalculateEnergy() {
		energy=velocity*velocity*BbMass*0.001/2;
}

void ConvertEnergyToDefaultBb() {
		speed02=sqrt(2 * energy / 0.0002);
}

void PrintPreviousVelocitys() {
  String previusSpeed = "";
  for (int i = 0; i < maxSize-1; i++){
    if (previusSpeed.length() == 0) {
    previusSpeed = previusSpeed + SpeedValuesArr[i];
    }
    else {
    previusSpeed = previusSpeed + " " + SpeedValuesArr[i];

    }
  };
  oled.print(previusSpeed);
}

void UpdateSpeedValuesArr(float newVelocity) {
  for (int i = maxSize-1; i > 0; i--){
    SpeedValuesArr[i] =  SpeedValuesArr[i-1];
  };
  SpeedValuesArr[0] =  newVelocity;
}

void SpeedRendererSetup() {
  oled.init();
  oled.clear();
  oled.setContrast(255);
  oled.line(25, 0, 25, 25);
  oled.line(55, 0, 55, 25);
  oled.line(88, 0, 88, 25);

  oled.line(1, 25, 128, 25);
  
  oled.setScale(1);
  oled.home();

  oled.print("bb");
  oled.setCursor(28,0);
  oled.print("J");
  oled.setCursor(92,0);
  oled.print("V 0.2");
  oled.setCursor(60,0);
  oled.print("shot");

  oled.setCursor(0,2);
  oled.print(String(BbMass));
  oled.setCursor(28,2);
  oled.print(String(energy));
  oled.setCursor(60,2);
  oled.print("0");

  oled.setCursor(92,2);
  oled.print(String(speed02));

  oled.setCursor(0,4);
  oled.setScale(3);
  oled.print(String(velocity));

  oled.setCursor(0,7);
  oled.setScale(1);
}


void SpeedRenderer(float newVelocity, float newEnergy) {
  UpdateSpeedValuesArr(newVelocity);

  oled.clear(0, 35, 128, 64);       // очистка прошлой скорости
  oled.clear(35, 20, 45, 23);       // очистка прошлой энергии

  oled.setCursor(0,2);
  oled.print(String(BbMass));
  oled.setCursor(28,2);
  oled.print(String(newEnergy));
  oled.setCursor(60,2);
  oled.print(String(n));

  oled.setCursor(90,2);
  oled.print(String(speed02));

  oled.setCursor(0,4);
  oled.setScale(3);
  oled.print(String(newVelocity));
  
  oled.setCursor(0,7);
  oled.setScale(1);
  PrintPreviousVelocitys();
}


void setup() {
  SpeedRendererSetup();

	Serial.begin(9600);    //открываем COM порт
	attachInterrupt(1,start,RISING);     //аппаратное прерывание при прохождении первого датчика
	attachInterrupt(0,finish,RISING);      //аппаратное прерывание при прохождении второго датчика
}
void start() 
{
	if (gap1==0) {   //если измерение еще не проводилось
		gap1=micros(); //получаем время работы ардуино с момента включения до момента пролетания первой пули
	}
}
void finish() 
{
	if (gap2==0) {  //если измерение еще не проводилось
		gap2=micros();  //получаем время работы ардуино с момента включения до момента пролетания второй пули
	}
}
void loop() {
	if (initial==0) {                          //флажок первого запуска
		Serial.println("Press 1 to service mode");                      //режим отладки (резисторы)
		Serial.println("Press 0 speed measure mode (default)");        //выход из режимов
		Serial.println("System is ready, just pull the f*ckin trigger!");   //уведомление о том, что хрон готов к работе
		Serial.println(" ");
		initial=1;       //первый запуск, больше не показываем сообщения
	}

	if (Serial.available() > 0 && set!=2) {   //если есть какие буквы на вход с порта и не выбран 2 режим
		int val=Serial.read();                  //прочитать что было послано в порт
		switch(val) {                           //оператор выбора

		case 48: set=0; flagmass=0; rapidflag=0; initial=0; break;    //если приняли 0 то выбрать 0 режим
		case 49: set=1; break;                //если приняли 1 то запустить режим 1
		}
	}

	if (set==1) {                    //если 1 режим
		Serial.print("sensor 1: ");
		Serial.println(analogRead(2));  //показать значение на первом датчике
		Serial.print("sensor 2: "); 
		Serial.print(analogRead(3));   //показать значение на втором датчике
		Serial.println();
		Serial.println();              //ну типо два переноса строки
	}


	if (gap1!=0 && gap2!=0 && gap2>gap1 && set==0) {        //если пуля прошла оба датчика в 0 режиме
		velocity=(1000000*(dist)/(gap2-gap1));         //вычисление скорости как расстояние/время
		energy=velocity*velocity*BbMass*0.001/2;              //вычисление энергии
		Serial.print("Shot #");                        
		Serial.println(n);                                 //вывод номера выстрела
		Serial.print("Speed: ");    
		Serial.println(velocity);                          //вывод скорости в COM
		Serial.print("Energy: ");    
		Serial.println(energy);                          //вывод энергии в COM
		Serial.println(" "); 

    CalculateEnergy();
    ConvertEnergyToDefaultBb();
    SpeedRenderer(velocity, energy);

		gap1=0;                                   //сброс значений
		gap2=0;
		n++;                                      //номер выстрела +1
	}

	if (micros()-gap1>200000 && gap1!=0 && set!=1) { // (если пуля прошла первый датчик) И (прошла уже 0.2 секунды, а второй датчик не тронут)
		Serial.println("FAIL"); //выдаёт FAIL через 1 секунду, если пуля прошла через первый датчик, а через второй нет
    velocity = 999.99;
    SpeedRenderer(velocity, energy);
		gap1=0;
		gap2=0;
	}

	delay(200);    //задержка для стабильности
}
