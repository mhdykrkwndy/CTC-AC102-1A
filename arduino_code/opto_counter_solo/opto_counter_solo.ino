#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2
volatile unsigned long pulseCount = 0;
unsigned long startTime;
unsigned long endTime;
const byte OPT_PIN = 2;
const unsigned int PULSE_COUNT = 100;
volatile unsigned int pulseCounter = 0;
volatile unsigned long firstPulseTime = 0;
volatile unsigned long lastPulseTime = 0;
volatile bool measurementReady = false;
void setup()
{
  Serial.begin(500000);

  pinMode(2, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(2),
    countPulse,
    FALLING
  );

  // LCD
  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Waiting...");

  lcd.setCursor(0, 1);
  lcd.print("100 Pulses");
}


void loop()
{
  if (measurementReady)
  {
    unsigned long firstTime;
    unsigned long lastTime;
    unsigned int count;

    noInterrupts();

    firstTime = firstPulseTime;
    lastTime = lastPulseTime;
    count = pulseCounter;

    pulseCounter = 0;
    measurementReady = false;

    interrupts();


    unsigned long deltaTime = lastTime - firstTime;

    float timeSeconds = deltaTime / 1000000.0;



    float frequency =
      (count - 1) / (2.0 * timeSeconds);



    Serial.print("Pulses: ");
    Serial.print(count);

    Serial.print(" | Time: ");
    Serial.print(timeSeconds, 4);

    Serial.print(" s | Frequency: ");
    Serial.print(frequency, 3);

    Serial.println(" Hz");



    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("F:");
    lcd.print(frequency, 2);
    lcd.print(" Hz");

    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(timeSeconds, 3);
    lcd.print(" s");
  }
}


void countPulse()
{
  if (measurementReady)
    return;

  unsigned long now = micros();

  if (pulseCounter == 0)
  {
    firstPulseTime = now;
    pulseCounter = 1;
  }
  else
  {
    pulseCounter++;

    lastPulseTime = now;

    if (pulseCounter >= PULSE_COUNT)
    {
      measurementReady = true;
    }
  }
}