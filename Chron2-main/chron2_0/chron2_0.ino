#include <GyverOLED.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

#define btnPlusPin 4
uint8_t btnPlusPrev;
#define btnMinusPin 5
uint8_t btnMinusPrev;
float dist = 0.199;       //расстояние между датчиками в метрах  
const int velocitiesArrMaxSize = 4;  // количесво хранимых значений прошлых выстрелов

int set;  // режим

int shotNum = 0;            //номер выстрела, начиная с 1
float velocity, energy;    //переменная для хранения скорости
float BbMass = 0.20;       //масса снаряда в килограммах
volatile unsigned long gap1, gap2;    //отметки времени прохождения пулей датчиков
float speed02 = 0.00;


String SpeedValuesArr[velocitiesArrMaxSize] = {
    "000",
    "001",
    "002",
  };


void CalculateEnergy() {
		energy = velocity * velocity * BbMass * 0.001 / 2;  // вычисление энергии выстрела
}


void ConvertEnergyToDefaultBb() {
		speed02 = sqrt(2 * energy / 0.0002);   // вычисление скорости выстрела шаром весом 0.2 гр на основе текущего выстрела используя его энергию и скорость
}


void RenderPreviousVelocities() {
  String previusSpeed = "";
  for (int i = 0; i < velocitiesArrMaxSize-1; i++){
    if (previusSpeed.length() == 0) {
      previusSpeed = previusSpeed + SpeedValuesArr[i];
    }
    else {
      previusSpeed = previusSpeed + " " + SpeedValuesArr[i];
    }
  };
  oled.print(previusSpeed);
}


void UpdateSpeedValuesArr() {
  for (int i = velocitiesArrMaxSize-1; i > 0; i--){
    SpeedValuesArr[i] =  SpeedValuesArr[i-1];
  };
  SpeedValuesArr[0] =  velocity;
}


void VelocityMeasureRenderSetup() {
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


void massRenrererUpdater() {
  oled.clear(0, 20, 24, 23);       // очистка значения энергии прошлого выстрела
  oled.setCursor(0,2);
  oled.print(String(BbMass));
}

void SpeedRender() {
  UpdateSpeedValuesArr();

  oled.clear(0, 35, 128, 64);       // очистка прошлой скорости
  oled.clear(35, 20, 45, 23);       // очистка прошлой энергии
  oled.clear(90, 20, 129, 23);       // очистка прошлой скорости 0.2 шаром


  oled.setCursor(0,2);
  oled.print(String(BbMass));
  oled.setCursor(28,2);
  oled.print(String(energy));
  oled.setCursor(60,2);
  oled.print(String(shotNum));

  oled.setCursor(90,2);
  oled.print(String(speed02));

  oled.setCursor(0,4);
  oled.setScale(3);
  oled.print(String(velocity));
  
  oled.setCursor(0,7);
  oled.setScale(1);
  RenderPreviousVelocities();
}


void setup() {
  pinMode(btnPlusPin, INPUT_PULLUP);
  btnPlusPrev = digitalRead(btnPlusPin);
  pinMode(btnMinusPin, INPUT_PULLUP);
  btnMinusPrev = digitalRead(btnMinusPin);

  VelocityMeasureRenderSetup();

	Serial.begin(9600);    //открываем COM порт
	attachInterrupt(1, start, RISING);     //аппаратное прерывание при прохождении первого датчика
	attachInterrupt(0, finish, RISING);      //аппаратное прерывание при прохождении второго датчика
}


void CalibrateSensors() {  // режим калибровки датчиков
    Serial.print("sensor 1: ");
		Serial.println(analogRead(2));  // значение на первом датчике
		Serial.print("sensor 2: "); 
		Serial.print(analogRead(3));   // значение на втором датчике
		Serial.println();

    oled.clear();
    oled.setContrast(255);
    oled.home();
    oled.setScale(1);
    oled.print("sensor 1:");
    oled.setCursor(0,2);
    oled.setScale(2);
    oled.print(String(analogRead(2)));
    oled.setScale(1);
    oled.setCursor(0,4);
    oled.print("sensor 2:");
    oled.setScale(2);
    oled.setCursor(0,6);
    oled.print(String(analogRead(3)));
}

void OperatingModeSwitch() {
  uint8_t btnPlus = digitalRead(btnPlusPin);
  if (btnPlus == LOW && btnPlusPrev == HIGH && BbMass < 1)
  {
    BbMass = BbMass + 0.01;
    massRenrererUpdater();
  }
  btnPlusPrev = digitalRead(btnPlusPin);

  int8_t btnMinus = digitalRead(btnMinusPin);
  if (btnMinus == LOW && btnMinusPrev == HIGH && BbMass > 0.1)
  {
    BbMass = BbMass - 0.01;
    massRenrererUpdater();
  }
  btnMinusPrev = digitalRead(btnMinusPin);

  if (btnPlus == LOW && btnMinus == LOW && set == 0) {
    set = 1;
  }
  else if (btnPlus == LOW && btnMinus == LOW && set == 1) {
    set = 0;
    VelocityMeasureRenderSetup();
  }
}


void VelocityMeasure() {
  if (gap1!=0 && gap2!=0 && gap2>gap1 && set==0) {        //если пуля прошла оба датчика в 0 режиме
		velocity=(1000000 * (dist) / (gap2-gap1));         //вычисление скорости как расстояние/время
		energy=velocity*velocity*BbMass*0.001/2;              //вычисление энергии
		Serial.print("Shot #");                        
		Serial.println(shotNum);                                 //вывод номера выстрела
		Serial.print("Speed: ");    
		Serial.println(velocity);                          //вывод скорости в COM
		Serial.print("Energy: ");    
		Serial.println(energy);                          //вывод энергии в COM
		Serial.println(" "); 

    CalculateEnergy();
    ConvertEnergyToDefaultBb();
    SpeedRender();

		gap1=0;                                   //сброс значений
		gap2=0;
		shotNum++;                                      //номер выстрела +1
	}
}


void VelocityMeasureFail() {
	if (micros() - gap1 > 200000 && gap1 != 0 && set != 1) { // (если пуля прошла первый датчик) И (прошла уже 0.2 секунды, а второй датчик не тронут)
		Serial.println("FAIL"); //выдаёт FAIL через 1 секунду, если пуля прошла через первый датчик, а через второй нет
    velocity = 999.99;
    shotNum++;
    SpeedRender();
		gap1 = 0;
		gap2 = 0;
	}
}


void start() 
{
	if (gap1 == 0) {   //если измерение еще не проводилось
		gap1 = micros(); //получаем время работы ардуино с момента включения до момента пролетания первой пули
	}
}


void finish() 
{
	if (gap2==0) {  //если измерение еще не проводилось
		gap2=micros();  //получаем время работы ардуино с момента включения до момента пролетания второй пули
	}
}


void loop() {
  OperatingModeSwitch();
	if (set == 1) {      
		CalibrateSensors();
	}
  VelocityMeasure();
	VelocityMeasureFail();
	delay(100);    //задержка для стабильности
}
