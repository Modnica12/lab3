#include <LedControl.h>


const int IRPin = A0;
const float voltsPerMeasurement = 5.0/1024.0;

// US датчик
const int echoPin = 8;
const int trigPin = 9;

const int displaysCount = 1;
 
const int dataPin = 12;
const int clkPin = 10;
const int csPin = 11;

int upper_bound = 60;
int bottom_bound = 20;
int new_upper_bound = 7;
int new_bottom_bound = 0;

LedControl lc = LedControl(dataPin, clkPin, csPin, displaysCount);
int prevRow = 0;
int prevCol = 0;

int distanceValues[8] = {0, 0, 0, 0, 0, 0, 0, 0};


void setup()
{
 
  Serial.begin(115200);
  lc.shutdown(0, false);
  lc.setIntensity(0, 16);
  lc.clearDisplay(0);

  pinMode(echoPin, INPUT);
  pinMode(IRPin, INPUT); 
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
}

void loop() 
{
  // Получаем значения с ИК дальномера
  float iRDistance = readIRDistance();
  
  // Получаем значения УЗ дальномера
  float uSDistance = readUSDistance();

  float middleValue = (iRDistance + uSDistance) / 2;
  
  // Конвертируем данные в диапазон [0, 7]
  int currentColumnHeight = translateToLedMatrixValues(middleValue);
  distanceValues[0] = currentColumnHeight;

  lc.clearDisplay(0);
  
  for (int j = 0; j <= 7; j++){
     for (int i = 0; i <= distanceValues[j] ; i++){
        lc.setLed(0, i, j, true);
    }
  }

  // сдвигаем значения, чтобы поместить новый столбик с новым значением
  for (int i = sizeof(distanceValues) - 1; i > 0; i--){
    distanceValues[i] = distanceValues[i-1];
  }
  
  delay(70);  
}

int translateToLedMatrixValues(float dist)
{
  // ограничение значений дальномеров
  float constrained = constrain(dist, bottom_bound, upper_bound);
  // перевод в нужный диапазон
  return map(constrained, bottom_bound, upper_bound, new_bottom_bound, new_upper_bound);
}


float readIRDistance()
{
  float volts = readIRAnalog() * voltsPerMeasurement;
  return 65 * pow(volts, -1.10); 
}

float readIRAnalog()
{
  int minV = 5000;
  int maxV = -5000;
  int sum = 0;
  int n = 15;
  
  for (int i = 0;  i < n; i++){
    int current = analogRead(IRPin);

    if (current < minV){
      minV = current;
    }
    sum += current;
    
    if (current > maxV){
      maxV = current;
    }
  }
  return (sum - maxV - minV) / (float)(n - 2);
}


float readUSDistance()
{
  const float speedOfSoundMPerSec = 340.0;
  const float speedOfSoundCmPerUs = speedOfSoundMPerSec / 10000.0;
  return readUSPulse() * speedOfSoundCmPerUs / 2.0;    
}

float readUSPulse()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration;
}
