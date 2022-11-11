// DEVELOPED BY IVAN SUAREZ Y ROMAN GAUDENCIO
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include "max6675.h"
#define Btn1_GPIO 27
#define Btn2_GPIO 35
#define Btn3_GPIO 34
Servo myservo;
LiquidCrystal_I2C lcd(0x27, 20, 4);
int menu = 1;
int servoPin = 18;
volatile int state = 0;
int thermoDO = 19;
int thermoCS = 23;
int thermoCLK = 5;
int alarma = 0;
volatile boolean paro = false;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);


// Boton arriba pin 14
// boton abajo pin 13
// boton aceptar pin 26
String messageStatic1 = "Aluminio";
String messageStatic2 = "Hierro";
String messageStatic3 = "Cobre";
String messageStatic4 = "Modo manual (temp)";

void clearLCDLine(int line) {
  lcd.setCursor(0, line);
  for (int n = 0; n < 20; n++) {
    lcd.print(" ");
  }
}


void secuenciaparo() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Paro activado");
  while (paro == true) {
  }
}





void IRAM_ATTR interrupt() {
  state = 1;
}

void IRAM_ATTR botonparo() {
  paro = true;
}

void IRAM_ATTR botonmarcha() {
  paro = false;
}

void botones(int i, int u, int x, int z) {
  pinMode(i, INPUT_PULLUP);
  pinMode(u, INPUT_PULLUP);
  pinMode(x, INPUT_PULLUP);
  pinMode(z, OUTPUT);
}

void switchh(String message, int c) {
  lcd.print(message);
  lcd.setCursor(0, c);
}
void fundir (String message, int temp ){
  int TimeForADreading = 20000 * 1000;  // 100mS
  uint64_t TimeADreading = esp_timer_get_time();
  alarma = 0;
  lcd.clear();
  lcd.print(message);
  lcd.setCursor(0, 2);
  lcd.print("Temperatura actual=");
  while (state == 0 && alarma == 0 && paro == false) {
    myservo.write(180);

    clearLCDLine(3);
    lcd.setCursor(0, 3);
    lcd.print(thermocouple.readCelsius());
    lcd.print(":C");

    if ((esp_timer_get_time() - TimeADreading) >= TimeForADreading) {
      if (thermocouple.readCelsius() < temp) {
        myservo.write(10);
        lcd.clear();
        lcd.print("Verifique tanque ");
        lcd.setCursor(0, 1);
        lcd.print("de gas");
        alarma = 1;
        for (int x = 0; x < 5; x++) {
          digitalWrite(2, HIGH);  // poner el Pin en HIGH
          delay(500);             // esperar 5 segundos
          digitalWrite(2, LOW);   // poner el Pin en LOW
          delay(500);
        }
      }

      TimeADreading = esp_timer_get_time();
    }
    delay(500);
  }

  myservo.write(10);
}
void menup() {
  switch (menu) {

    case 2:
      lcd.clear();
      switchh(messageStatic1, 1);
      switchh("=>" + messageStatic2, 2);
      switchh(messageStatic3, 3);
      switchh(messageStatic4, 0);
      break;

    case 3:
      lcd.clear();
      switchh(messageStatic1, 1);
      switchh(messageStatic2, 2);
      switchh("=>" + messageStatic3, 3);
      switchh(messageStatic4, 0);
      break;

    case 4:
      lcd.clear();
      switchh(messageStatic1, 1);
      switchh(messageStatic2, 2);
      switchh(messageStatic3, 3);
      switchh("=>" + messageStatic4, 3);
      break;

    case 5:
      menu = 1;
      lcd.clear();
      switchh("=>" + messageStatic1, 1);
      switchh(messageStatic2, 2);
      switchh(messageStatic3, 3);
      switchh(messageStatic4, 0);

      break;

    default:
      menu = 1;
      lcd.clear();
      switchh("=>" + messageStatic1, 1);
      switchh(messageStatic2, 2);
      switchh(messageStatic3, 3);
      switchh(messageStatic4, 0);
      break;
  }
}
void action() {
  switch (menu) {
    case 1:
      accion1();
      break;
    case 2:
      accion2();
      break;
    case 3:
      accion3();
      break;
    case 4:
      accion4();
      break;
  }
}

void accion1() {
  fundir ("Fundir Aluminio", 50);
}
void accion2() {
  fundir ("Fundir hierro", 50);
}
void accion3() {
  fundir ("Fundir cobre", 50);
}
void accion4() {
 fundir ("Modo manual", 50);
}
void setup() {
  myservo.attach(servoPin);
  paro = false;
  botones(14, 13, 26, 2);
  pinMode(Btn1_GPIO, INPUT_PULLUP);
  attachInterrupt(Btn1_GPIO, interrupt, FALLING);
  pinMode(Btn2_GPIO, INPUT_PULLUP);
  attachInterrupt(Btn2_GPIO, botonparo, FALLING);
  pinMode(Btn3_GPIO, INPUT_PULLUP);
  attachInterrupt(Btn3_GPIO, botonmarcha, FALLING);
  lcd.init();
  lcd.backlight();
  menup();
}

void loop() {
  if (paro == true) {
    secuenciaparo();
    paro = false;
    menup();
  }

  if (!digitalRead(13)) {
    menu++;
    menup();
    delay(100);
    while (!digitalRead(13))
      ;
  }
  if (!digitalRead(14)) {
    menu--;
    menup();
    delay(100);
    while (!digitalRead(14))
      ;
  }
  if (!digitalRead(26)) {
    state = 0;
    action();
    menup();
    delay(100);
    while (!digitalRead(26))
      ;
  }
}
