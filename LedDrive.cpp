// 
// 
// 

#include "LedDrive.h"

LedDrive::LedDrive(int pl)
{
	pinLed = pl;
	digitalWrite(pinLed, LOW); //turnoff PWM on power up
}

void LedDrive::Init()
{
	pinMode(pinLed, OUTPUT);
	analogWrite(pinLed, 255);
	stateLed = 0;
}

void LedDrive::SetPctAmp(int pa)
{
	if (pa > 100)
	{
		Serial.println("Cmd error: Max % intensity = 100");
		return;
	}
	pctAmpRead = pa;
	pwmAmp = (float)2.55 * (float)(100 - pctAmpRead);
	/*
	Serial.print("% max intensity = "); Serial.println(pctAmp);
	Serial.print("pwmAmp = "); Serial.println(pwmAmp);
	*/
	stateLed = 1; //turn LED on
	analogWrite(pinLed, pwmAmp);
	return;
}

void LedDrive::On()
{
	stateLed = 1; //turn LED on
	analogWrite(pinLed, pwmAmp);
}

void LedDrive::Off()
{
	stateLed = 0; //turn LED off
	analogWrite(pinLed, 255);
}

int LedDrive::State()
{
	return stateLed;
}

void LedDrive::ListLedParameters()
{
	Serial.print("% max amps = "); Serial.println(pctAmpRead);
}





