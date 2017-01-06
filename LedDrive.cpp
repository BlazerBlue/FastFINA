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
	digitalWrite(pinLed, LOW);
	stateLed = 0;
}

void LedDrive::SetPctAmp(int pa)
/*
change this routine to read detector once a second
pa = 1 now means turn on reading, pa = 0 turn off
for now, just don't run it
*/
{
	Serial.println("Set intensity with pot");
	return;
}

void LedDrive::On()
{
	stateLed = 1; //turn LED on
	digitalWrite(pinLed, HIGH);
}

void LedDrive::Off()
{
	stateLed = 0; //turn LED off
	digitalWrite(pinLed, LOW);
}

int LedDrive::State()
{
	return stateLed;
}

void LedDrive::ListLedParameters()
{
	Serial.print("% max amps = "); Serial.println(pctAmpRead);
}





