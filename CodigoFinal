#include <LiquidCrystal_I2C.h>

int pinNivelMuyBajo = 2;
int pinNivelBajo = 3;
int pinNivelMedio = 4;
int pinNivelAlto = 5;
int Buzzer = 6;
int Alcohol_SENSOR = A0;

LiquidCrystal_I2C lcd_1(0x27, 16, 2);

// Variables para calibración
int sensorBaseline = 0;
bool sensorReady = false;
unsigned long startTime = 0;
const unsigned long WARM_UP_TIME = 5000;

void setup() {
  pinMode(pinNivelMuyBajo, OUTPUT);
  pinMode(pinNivelBajo, OUTPUT);
  pinMode(pinNivelMedio, OUTPUT);
  pinMode(pinNivelAlto, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  
  lcd_1.init();
  lcd_1.backlight();
  lcd_1.clear();
  Serial.begin(9600);
  
  startTime = millis();
  
  lcd_1.setCursor(0, 0);
  lcd_1.print("CALENTANDO...");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Espere 5s");
  
  Serial.println("Iniciando calibración del sensor MQ-3...");
}

void loop() {
  if (!sensorReady) {
    unsigned long elapsedTime = millis() - startTime;
    
    if (elapsedTime < WARM_UP_TIME) {
      int remainingSeconds = (WARM_UP_TIME - elapsedTime) / 1000;
      
      static int lastSecond = -1;
      if (remainingSeconds != lastSecond) {
        lcd_1.setCursor(0, 1);
        lcd_1.print("Espere ");
        if (remainingSeconds < 10) {
          lcd_1.print(" ");
        }
        lcd_1.print(remainingSeconds);
        lcd_1.print("s    ");
        lastSecond = remainingSeconds;
      }
      
      delay(100);
      return;
    } else {
      // Calibrar sensor
      long sum = 0;
      for (int i = 0; i < 10; i++) {
        sum += analogRead(Alcohol_SENSOR);
        delay(100);
      }
      sensorBaseline = sum / 10;
      sensorReady = true;
      
      lcd_1.clear();
      lcd_1.setCursor(0, 0);
      lcd_1.print("CALIBRADO!");
      lcd_1.setCursor(0, 1);
      lcd_1.print("Base: ");
      lcd_1.print(sensorBaseline);
      
      Serial.println("Calibración completada!");
      Serial.print("Valor base: ");
      Serial.println(sensorBaseline);
      
      delay(2000);
    }
  }
  
  // Operación normal
  lcd_1.clear();
  
  float sensor_In = analogRead(Alcohol_SENSOR);
  float sensorDiff = sensor_In - sensorBaseline;
  float mg_l = max(0, sensorDiff / 200.0);
  
  Serial.print("Valor: ");
  Serial.print(sensor_In);
  Serial.print(" Base: ");
  Serial.print(sensorBaseline);
  Serial.print(" Diff: ");
  Serial.print(sensorDiff);
  Serial.print(" mg/l: ");
  Serial.println(mg_l);
  
  lcd_1.setCursor(0, 0);
  lcd_1.print("mg/l: ");
  lcd_1.print(mg_l, 2);
  
  // Apagar todos los LEDs
  digitalWrite(pinNivelAlto, LOW);
  digitalWrite(pinNivelMedio, LOW);
  digitalWrite(pinNivelBajo, LOW);
  digitalWrite(pinNivelMuyBajo, LOW);
  digitalWrite(Buzzer, LOW);
  
  // Lógica de niveles
  if (sensorDiff > 150) {
    digitalWrite(pinNivelAlto, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("!!BORRACHO :/!!");
    digitalWrite(Buzzer, HIGH);
  } else if (sensorDiff > 100) {
    digitalWrite(pinNivelMedio, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("CASI AL LIMITE :( )");
  } else if (sensorDiff > 50) {
    digitalWrite(pinNivelBajo, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("MODERADO :)");
  } else if (sensorDiff > 20) {
    digitalWrite(pinNivelMuyBajo, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("TODO BIEN :D");
  } else {
    lcd_1.setCursor(0, 1);
    lcd_1.print("SIN DETECCION");
  }
  
  delay(1000);
}
