#include <LiquidCrystal_I2C.h>

int pinNivelMuyBajo = 2;
int pinNivelBajo = 3;
int pinNivelMedio = 4;
int pinNivelAlto = 5;
int Buzzer = 6;
int Alcohol_SENSOR = A0;

LiquidCrystal_I2C lcd_1(0x27, 16, 2);

// Variables para calibración mejorada
int sensorBaseline = 0;
bool sensorReady = false;
unsigned long startTime = 0;
const unsigned long WARM_UP_TIME = 30000; // Aumentado a 30 segundos
const int NUM_READINGS = 20; // Más lecturas para mejor promedio
const int NOISE_THRESHOLD = 15; // Umbral de ruido para filtrar fluctuaciones
int lastStableReading = 0;

// Variables para filtrado y recuperación moderada constante
float filteredValue = 0;
const float FILTER_ALPHA = 0.4; // Factor moderado para recuperación constante

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
  lcd_1.print("Espere 10s");
  
  Serial.println("Iniciando calibración del sensor MQ-3...");
  Serial.println("IMPORTANTE: No soplar durante la calibración");
}

void loop() {
  // Simulación de calentamiento del sensor
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
        
        Serial.print("Calentando... ");
        Serial.print(remainingSeconds);
        Serial.println("s restantes");
      }
      
      delay(100);
      return;
    } else {
      // Calibración mejorada con más muestras
      long sum = 0;
      Serial.println("Iniciando calibración final...");
      
      for (int i = 0; i < NUM_READINGS; i++) {
        int reading = analogRead(Alcohol_SENSOR);
        sum += reading;
        Serial.print("Muestra ");
        Serial.print(i+1);
        Serial.print(": ");
        Serial.println(reading);
        delay(200); // Más tiempo entre lecturas
      }
      
      sensorBaseline = sum / NUM_READINGS;
      filteredValue = sensorBaseline; // Inicializar filtro
      lastStableReading = sensorBaseline;
      sensorReady = true;
      
      lcd_1.clear();
      lcd_1.setCursor(0, 0);
      lcd_1.print("CALIBRADO!");
      lcd_1.setCursor(0, 1);
      lcd_1.print("Base: ");
      lcd_1.print(sensorBaseline);
      
      Serial.println("=== CALIBRACIÓN COMPLETADA ===");
      Serial.print("Valor base promedio: ");
      Serial.println(sensorBaseline);
      Serial.println("Sistema listo para usar");
      
      delay(3000);
    }
  }// Fin de simulación del sensor
  
  // Operación normal con recuperación moderada constante
  lcd_1.clear();
  
  int rawReading = analogRead(Alcohol_SENSOR);
  
  // Aplicar filtrado moderado que ayuda a la recuperación gradual
  filteredValue = (FILTER_ALPHA * rawReading) + ((1 - FILTER_ALPHA) * filteredValue);
  
  // Agregar ligera tendencia hacia la baseline para recuperación natural
  float recoveryBias = (sensorBaseline - filteredValue) * 0.05; // 5% de tendencia hacia la base
  filteredValue += recoveryBias;
  
  // Calcular diferencia con valor filtrado
  float sensorDiff = filteredValue - sensorBaseline;
  
  // Aplicar umbral de ruido - si la diferencia es muy pequeña, considerarla como cero
  if (abs(sensorDiff) < NOISE_THRESHOLD) {
    sensorDiff = 0;
  }
  
  // Conversión mejorada a mg/l (ajustada para MQ-3)
  float mg_l = max(0, sensorDiff / 150.0); // Ajustado el factor de conversión
  
  Serial.print("Raw: ");
  Serial.print(rawReading);
  Serial.print(" Filtrado: ");
  Serial.print(filteredValue, 1);
  Serial.print(" Base: ");
  Serial.print(sensorBaseline);
  Serial.print(" Diff: ");
  Serial.print(sensorDiff, 1);
  Serial.print(" mg/l: ");
  Serial.println(mg_l, 3);
  
  lcd_1.setCursor(0, 0);
  lcd_1.print("mg/l: ");
  lcd_1.print(mg_l, 2);
  
  // Apagar todos los LEDs y buzzer
  digitalWrite(pinNivelAlto, LOW);
  digitalWrite(pinNivelMedio, LOW);
  digitalWrite(pinNivelBajo, LOW);
  digitalWrite(pinNivelMuyBajo, LOW);
  digitalWrite(Buzzer, LOW);
  
  // Lógica de niveles corregida para detectar aliento normal
  if (sensorDiff > 150) { // Nivel muy alto - solo alcohol real
    digitalWrite(pinNivelAlto, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("!!BORRACHO!! :O");
    digitalWrite(Buzzer, HIGH);
    delay(200);
    digitalWrite(Buzzer, LOW);
    delay(200);
  } else if (sensorDiff > 100) { // Nivel alto
    digitalWrite(pinNivelMedio, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("CASI AL LIMITE :/");
  } else if (sensorDiff > 60) { // Nivel moderado - aumentado umbral
    digitalWrite(pinNivelBajo, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("POSIBLE ALCOHOL -_-");
  } else if (sensorDiff > 15) { // Umbral reducido - cualquier cambio por soplar
    digitalWrite(pinNivelMuyBajo, HIGH);
    lcd_1.setCursor(0, 1);
    lcd_1.print("ALIENTO NORMAL:D");
  } else { // Solo aire estático sin cambios
    lcd_1.setCursor(0, 1);
    lcd_1.print("SIN DETECCION :)");
  }
  
  delay(500); // Velocidad moderada para recuperación constante
}
