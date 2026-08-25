#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
bool positivePeakDetected = false;
bool negativePeakDetected = false;
const byte sensorPin = A0;
const byte potPin = A2;
const byte confirmButton = 12;
const byte optoPin = 2;
const unsigned long minAveragingTime = 1000;
const unsigned long maxAveragingTime = 5000;
unsigned long averagingTime = 500;
const unsigned long calibrationTime = 2000;
long sumBias = 0;
long sampleCount = 0;
int bias = 512;
const int deadBand = 5;
unsigned long averagingStartTime = 0;
long positiveSum = 0;
long negativeSum = 0;
long positiveCount = 0;
long negativeCount = 0;
volatile unsigned long pulseCount = 0;
volatile unsigned long firstPulseTime = 0;
volatile unsigned long lastPulseTime = 0;
volatile bool frequencyReady = false;
const byte pulseNumber = 41;
float pulseFrequency = 0.0;
float motionFrequency = 0.0;
const bool reciprocatingSystem = true;
unsigned long lastDisplayUpdate = 0;
bool showPositive = true;
int positivePeakCandidate = 0;
int negativePeakCandidate = 0;
byte positivePeakStableCount = 0;
byte negativePeakStableCount = 0;
const byte PEAK_STABLE_SAMPLES = 3;
const int PEAK_TOLERANCE = 10;
int positivePeak = 0;
int negativePeak = 0;
float positivePeakSum = 0;
float negativePeakSum = 0;
unsigned long positivePeakCount = 0;
unsigned long negativePeakCount = 0;
float positivePeakAverage = 0;
float negativePeakAverage = 0;
bool newResultReady = false;
void setup()
{
 // Serial.begin(115200);
  pinMode(confirmButton, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Accelerometer");
  delay(1000);
  selectAveragingTime();
  calibrateSensor();
  pinMode(optoPin, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(optoPin),
    countPulse,
    RISING
  );

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Averaging:");
  lcd.setCursor(0, 1);
  lcd.print(averagingTime);
  lcd.print(" ms");

  delay(1000);

  averagingStartTime = millis();
}

void selectAveragingTime()
{
  int lastPotValue = -1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Time:");
while (true)
  {

    int potValue = analogRead(potPin);

    if (abs(potValue - lastPotValue) >= 5)
    {
      lastPotValue = potValue;

      averagingTime = map(potValue, 0, 1023, minAveragingTime, maxAveragingTime);

      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(averagingTime);
      lcd.print(" ms");
    }

    if (digitalRead(confirmButton) == LOW)
    {
      delay(50);

      if (digitalRead(confirmButton) == LOW)
      {
        while (digitalRead(confirmButton) == LOW)
        {
          delay(10);
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Time Selected:");
        lcd.setCursor(0, 1);
        lcd.print(averagingTime);
        lcd.print(" ms");
       // Serial.print("Selected Time = ");
       // Serial.print(averagingTime);
       // Serial.println(" ms");

        delay(1000);

        return;
      }
    }
  }
}


void calibrateSensor()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration");
  lcd.setCursor(0, 1);
  lcd.print("Keep Sensor Still");
  sumBias = 0;
  sampleCount = 0;
  unsigned long startTime = millis();
  while (millis() - startTime < calibrationTime)
  {
    int adc = analogRead(sensorPin);

    sumBias += adc;

    sampleCount++;
  }
  bias = sumBias / sampleCount;

  //Serial.println();
  //Serial.print("Bias = ");
  //Serial.println(bias);
  //Serial.println("Calibration Completed");
  //Serial.println("--------------------------------");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bias = ");
  lcd.print(bias);
  lcd.setCursor(0, 1);
  lcd.print("Calibration OK");

  delay(1500);
}

void loop()
{

  int adc = analogRead(sensorPin);
  int value = adc - bias;

  if (value > deadBand)
  {
  positivePeakDetected = false;
  positiveSum += value;
  positiveCount++;

  if (!positivePeakDetected)
  {
    if (value > positivePeakCandidate)
    {
      positivePeakCandidate = value;
      positivePeakStableCount = 1;
    }
    else if (abs(value - positivePeakCandidate) <= PEAK_TOLERANCE)
    {
      positivePeakStableCount++;

      if (positivePeakStableCount >= PEAK_STABLE_SAMPLES)
      {
        positivePeakSum += positivePeakCandidate;
        positivePeakCount++;

        positivePeakDetected = true;
      }
    }
    else
    {
      positivePeakStableCount = 1;
    }
  }
}
else if (value < -deadBand)
{
  negativePeakDetected = false;
  negativeSum += value;
  negativeCount++;

  if (!negativePeakDetected)
  {
    if (value < negativePeakCandidate)
    {
      negativePeakCandidate = value;
      negativePeakStableCount = 1;
    }
    else if (abs(value - negativePeakCandidate) <= PEAK_TOLERANCE)
    {
      negativePeakStableCount++;

      if (negativePeakStableCount >= PEAK_STABLE_SAMPLES)
      {
        negativePeakSum += negativePeakCandidate;
        negativePeakCount++;

        negativePeakDetected = true;
      }
    }
    else
    {
      negativePeakStableCount = 1;
    }
  }
}


  updateFrequency();

  if (millis() - averagingStartTime >= averagingTime)
{

  if (positivePeakCount > 0)
  {
    positivePeakAverage =
      (positivePeakSum / positivePeakCount) / 20.48;
  }
  else
  {
    positivePeakAverage = 0;
  }


  if (negativePeakCount > 0)
  {
    negativePeakAverage =
      (negativePeakSum / negativePeakCount) / 20.48;
  }
  else
  {
    negativePeakAverage = 0;
  }


  newResultReady = true;



  positiveSum = 0;
  negativeSum = 0;

  positiveCount = 0;
  negativeCount = 0;

  positivePeakSum = 0;
  negativePeakSum = 0;

  positivePeakCount = 0;
  negativePeakCount = 0;

  positivePeakCandidate = 0;
  negativePeakCandidate = 0;

  positivePeakStableCount = 0;
  negativePeakStableCount = 0;

  averagingStartTime += averagingTime;
}

  updateLCD();
}

void countPulse()
{
  unsigned long now = micros();

  if (pulseCount == 0)
  {
    firstPulseTime = now;
  }
  pulseCount++;

  if (pulseCount >= pulseNumber)
  {
    lastPulseTime = now;

    frequencyReady = true;

    pulseCount = 0;
  }
}

void updateFrequency()
{
  if (frequencyReady)
  {
    unsigned long startTime;
    unsigned long endTime;

    noInterrupts();

    startTime = firstPulseTime;
    endTime = lastPulseTime;

    frequencyReady = false;

    interrupts();

    unsigned long elapsedTime = endTime - startTime;
    if (elapsedTime > 0)
    {
      float timeSeconds = elapsedTime / 1000000.0;

      pulseFrequency = 40.0 / timeSeconds;

      if (reciprocatingSystem)
      {
        motionFrequency = pulseFrequency / 2.0;
      }
      else
      {
        motionFrequency = pulseFrequency;
      }
    }
  }
}
void updateLCD()
{
  if (!newResultReady)
  {
    return;
  }

  newResultReady = false;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("F:");
  lcd.print(motionFrequency, 2);
  lcd.print(" Hz");

  lcd.setCursor(0, 1);

  if (showPositive)
  {
    lcd.print("P:");
    lcd.print(positivePeakAverage, 2);
    lcd.print("g");
  }
  else
  {
    lcd.print("N:");
    lcd.print(negativePeakAverage, 2);
    lcd.print("g");
  }

  showPositive = !showPositive;
}