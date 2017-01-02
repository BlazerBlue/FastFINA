// CycleDrive.h

#ifndef _CYCLEDRIVE_h
#define _CYCLEDRIVE_h

#include "arduino.h"
#include "ServoDrive.h"
#include "LedDrive.h"

/*
#include <EEPROM.h>
#define nCyclesBP 0
#define sHiTempInitBP 2
#define sHiTempBP 4
#define sLoTempBP  6
#define blueReadBP 8
#define redReadBP 9
int eBaseAddrCycle;
*/

class CycleDrive
{
	int setCycle[6];
	//no EEPROM so store values here
	int nCycles=10;
	int sHiTempInit = 120;
	int sHiTemp = 5;
	int sLoTemp = 10;
	boolean blueRead = false;
	boolean redRead = false;

	unsigned long int msHiTempInit = 1000* sHiTempInit;
	unsigned long int msHiTemp = 1000* sHiTemp;
	unsigned long int msLoTemp = 1000* sLoTemp;


	int cycleCount = 0;
	int cycleState;
	unsigned long int msStartTimer;
	unsigned long int msWaitTime;

	ServoDrive& shuttleServo;
	ServoDrive& heaterServo;
	LedDrive& blueLed;
	LedDrive& redLed;
	int pinAnalogDetector;

public:
	CycleDrive(ServoDrive& ss, ServoDrive& hs,LedDrive& bl, LedDrive& rl,int pd,int sc[]);
	void ListCycleParameters();
	void StartCycling();
	void StopCycling();
	void StoreNcycles(int nc);
	void StoreHeatTimes(int hi0, int hi, int lo);
	void StoreReadChan(boolean bc, boolean rc);
	void CheckStateTime();
};
#endif

