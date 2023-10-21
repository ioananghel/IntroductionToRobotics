const int blueAnalogInPin = A0;
const int greenAnalogInPin = A1;
const int redAnalogInPin = A2;

const int blueAnalogOutPin = 11;
const int greenAnalogOutPin = 10;
const int redAnalogOutPin = 9;

int blueInVal = 0, greenInVal = 0, redInVal = 0;
int blueOutVal = 0, greenOutVal = 0, redOutVal = 0;

void setup()
{
  pinMode(blueAnalogInPin, INPUT);
  pinMode(greenAnalogInPin, INPUT);
  pinMode(redAnalogInPin, INPUT);

  pinMode(blueAnalogOutPin, OUTPUT);
  pinMode(greenAnalogOutPin, OUTPUT);
  pinMode(redAnalogOutPin, OUTPUT);
}

void loop()
{
  blueInVal = analogRead(blueAnalogInPin);
  blueOutVal = map(blueInVal, 0, 1023, 0, 255);

  greenInVal = analogRead(greenAnalogInPin);
  greenOutVal = map(greenInVal, 0, 1023, 0, 255);

  redInVal = analogRead(redAnalogInPin);
  redOutVal = map(redInVal, 0, 1023, 0, 255);

  analogWrite(blueAnalogOutPin, blueOutVal);
  analogWrite(greenAnalogOutPin, greenOutVal);
  analogWrite(redAnalogOutPin, redOutVal);
}
