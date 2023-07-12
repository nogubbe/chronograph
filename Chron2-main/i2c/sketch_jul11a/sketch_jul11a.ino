#include <GyverOLED.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

const int maxSize=4;
float speed = 172.0;
float energy = 0.00;
float speed02 = 0.00;
float bbMass = 0.25;
int shotNumber = 0;


String SpeedValuesArr[maxSize] = {
    "000",
    "001",
    "002",
  };
  


void UpdateSpeedValuesArr(float new_speed) {
  for (int i = maxSize-1; i > 0; i--){
    SpeedValuesArr[i] =  SpeedValuesArr[i-1];
  };
  SpeedValuesArr[0] =  new_speed;
}

void CalculateEnergy() {
		energy=speed*speed*bbMass*0.001/2;
}

void ConvertEnergyToDefaultBb() {
		 speed02=sqrt(2 * energy / 0.0002);
  
}

void SpeedRenderer(float newSpeed, float newEnergy) {
  UpdateSpeedValuesArr(newSpeed);

  oled.clear(0, 35, 128, 64);       // очистка прошлой скорости
  oled.clear(35, 20, 45, 23);       // очистка прошлой энергии

  // курсор на начало 2 строки
  oled.setCursor(0,2);
  oled.print(String(bbMass));
  oled.setCursor(28,2);
  oled.print(String(newEnergy));
  oled.setCursor(60,2);
  oled.print(String(shotNumber));

  oled.setCursor(90,2);
  oled.print(String(speed02));

  oled.setCursor(0,4);
  oled.setScale(3);   // масштаб текста (1..4)
  oled.print(String(newSpeed));
  



  oled.setCursor(0,7);
  oled.setScale(1);   // масштаб текста (1..4)


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


void setup() {
  oled.init();        // инициализация
  oled.clear();       // очистка
  oled.setContrast(255);
  oled.line(25, 0, 25, 25);
  oled.line(55, 0, 55, 25);
  oled.line(88, 0, 88, 25);


  oled.line(1, 25, 128, 25);
  
  

  oled.setScale(1);   // масштаб текста (1..4)
  oled.home();        // курсор в 0,0

  oled.print("bb");
  oled.setCursor(28,0);
  oled.print("J");
  oled.setCursor(90,0);
  oled.print("0.2");
  oled.setCursor(60,0);
  oled.print("shot");


}

void loop() {
  speed = random(43, 172);
  CalculateEnergy();
  ConvertEnergyToDefaultBb();
  SpeedRenderer(speed, energy);
  delay(1000);
}