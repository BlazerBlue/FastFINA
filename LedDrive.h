// LedDrive.h

#ifndef _LEDDRIVE_h
#define _LEDDRIVE_h

#include "arduino.h"



class LedDrive
{
protected:
	int pinLed;
	//int eBaseAddr;
	//int addrPctAmp;

	// stored parameters when no EEPROM
	int pctAmpRead = 5;

	int pwmAmp;
	int stateLed;

public:
	LedDrive(int pp);
	void Init();
	//void StorePctAmp();
	void SetPctAmp(int pa);
	void On();
	void Off();
	int State();
	void ListLedParameters();
};


#endif

