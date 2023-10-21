const int blueAnalogInPin = A0;
const int greenAnalogInPin = A1;
const int redAnalogInPin = A2;

const int blueAnalogOutPin = 11;
const int greenAnalogOutPin = 10;
const int redAnalogOutPin = 9;

int blueInVal = 0, greenInVal = 0, redInVal = 0;
int blueOutVal = 0, greenOutVal = 0, redOutVal = 0;

const int minInValue = 0, maxInValue = 1023;
const int minOutValue = 0, maxOutValue = 255;

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
  blueOutVal = map(blueInVal, minInValue, maxInValue, minOutValue, maxOutValue);

  greenInVal = analogRead(greenAnalogInPin);
  greenOutVal = map(greenInVal, minInValue, maxInValue, minOutValue, maxOutValue);

  redInVal = analogRead(redAnalogInPin);
  redOutVal = map(redInVal, minInValue, maxInValue, minOutValue, maxOutValue);

  analogWrite(blueAnalogOutPin, blueOutVal);
  analogWrite(greenAnalogOutPin, greenOutVal);
  analogWrite(redAnalogOutPin, redOutVal);
}
