// 
// 
// 

#include "CycleDrive.h"
#define nCyclesI 0
#define sHiTempInitI 1
#define sHiTempI 2
#define sLoTempI 3
#define blueReadI 4
#define redReadI 5

CycleDrive::CycleDrive(ServoDrive& ss, ServoDrive& hs, LedDrive& bl,LedDrive& rl,int pinAnalogDetector,int setCycle[])
	: shuttleServo(ss), heaterServo(hs), blueLed(bl), redLed(rl)
{
nCycles = setCycle[nCyclesI];
sHiTempInit = setCycle[sHiTempInitI];
sHiTemp = setCycle[sHiTempI];
sLoTemp = setCycle[sLoTempI];
blueRead = setCycle[blueReadI];
redRead = setCycle[redReadI];
}

void CycleDrive::ListCycleParameters()
{
	Serial.print("nCycles = "); Serial.println(nCycles);

	Serial.print("sHiTempInit = "); Serial.println(sHiTempInit);
	Serial.print("sHiTemp = "); Serial.println(sHiTemp);
	Serial.print("sHLoTemp = "); Serial.println(sLoTemp);
	Serial.print("blueRead = "); Serial.println(blueRead);
	Serial.print("redRead = "); Serial.println(redRead);

}

void CycleDrive::StartCycling()
{
	cycleState = 1;
	shuttleServo.GoToLoc(1);
	Serial.print("start cycling, go to hiTemp loc = "); Serial.println(1);
	return;
}

void CycleDrive::StopCycling()
{
	return;
}

void CycleDrive::StoreNcycles(int nc) //int in base addr
{
	//EEPROM.put(eBaseAddrCycle + nCyclesBP, nc); //put any data type, write only byte
	nCycles = nc;
	return;
}

void CycleDrive::StoreHeatTimes(int hi0, int hi, int lo) //base+1,+2,+3 ints
{
	//EEPROM.put(eBaseAddrCycle + sHiTempInitBP, hi0);
	//EEPROM.put(eBaseAddrCycle + sHiTempBP, hi);
	//EEPROM.put(eBaseAddrCycle + sLoTempBP, lo);
	sHiTempInit = hi0;
	sHiTemp = hi;
	sLoTemp = lo;
	Serial.print("sHiTempInit = "); Serial.println(sHiTempInit);
	Serial.print("sHiTemp = "); Serial.println(sHiTemp);
	Serial.print("sLoTemp = "); Serial.println(sLoTemp);
	return;
}

void CycleDrive::StoreReadChan(boolean bc, boolean rc)
{
	//EEPROM.put(eBaseAddrCycle + blueReadBP, bc);
	//EEPROM.put(eBaseAddrCycle + redReadBP, rc);
	blueRead = bc;
	redRead = rc;
	Serial.print("blueRead = "); Serial.println(blueRead);
	Serial.print("redRead = "); Serial.println(redRead);
	return;
}

void CycleDrive::CheckStateTime()
/*
this is the state machine that steps through the PCR cycles
all the parameters need to pre-defined and stored in EEPROM
Steps are: 95, N x (60, 95, Read) done.
Locations are:
0 = home, load/unload
1 = 95
2 = read
3 = 60

*/
{
	switch (cycleState)
	{
	case 0: //idle
		return;
		break;
	case 1:  //moving to 95 heater for initial heat, start timer when arrive
		if (shuttleServo.Idle()) //shuttle move ended
		{
			heaterServo.GoToLoc(1);
			cycleState = 10;
		}
		break;
	case 10:
		if (heaterServo.Idle())  //heater move ended
		{
			msStartTimer = millis();
			msHiTempInit = 1000 * sHiTempInit;
			cycleState = 2;
			Serial.print("at hi temp heater, inital ms time = "); Serial.println(msHiTempInit);
		}
		break;
	case 2:  //waiting for initial 95C time, start cycle count, go to 60
		if (millis() - msStartTimer > msHiTempInit)
		{
			Serial.print("done with initial hiTemp heat, start cycling move to loc = "); Serial.println(3);
			heaterServo.GoToLoc(0);
			cycleState = 11;
		}
		break;
	case 11:
		if (heaterServo.Idle())
		{
			cycleCount = 0;
			shuttleServo.GoToLoc(3);
			cycleState = 3;
		}
		break;
	case 3:  //moving to 60 heater, engage heater and start timer when arrive
		if (shuttleServo.Idle()) //move ended
		{
			heaterServo.GoToLoc(2);
			cycleState = 12;
		}
		break;
	case 12:
		if(heaterServo.Idle())
		{
			msStartTimer = millis();
			msLoTemp = 1000 * sLoTemp;
			cycleState = 4;
			Serial.print("at 60 heater loc = 3, start Lo timer = "); Serial.println(msLoTemp);
		}
		break;
	case 4:  //waiting for 60C time, then move to 95C, loc=1
		if (millis() - msStartTimer > msLoTemp)
		{
			heaterServo.GoToLoc(0);
			cycleState = 13;
		}
		break;
	case 13:
		if(heaterServo.Idle())
		{
			shuttleServo.GoToLoc(1);
			cycleState = 5;
			Serial.print("done with LoTemp heat, start move to loc = "); Serial.println(1);
		}
		break;
	case 5:  //moving to 95 heater, start timer when arrive
		if (shuttleServo.Idle()) //move ended
		{
			heaterServo.GoToLoc(1);
			cycleState = 14;
		}
		break;
	case 14:
		if(heaterServo.Idle())
		{
			msStartTimer = millis();
			msHiTemp = 1000 * sHiTemp;
			cycleState = 6;
			Serial.print("at 95 heater loc = 1, start 95C timer =");  Serial.println(msHiTemp);
		}
		break;
	case 6:  //waiting for 95C time, then move to Read, loc=2
		if (millis() - msStartTimer > msHiTemp)
		{
			cycleState = 15;
			heaterServo.GoToLoc(0);
		}
		break;
	case 15:
		if(heaterServo.Idle())
		{
			shuttleServo.GoToLoc(2);
			cycleState = 7;
			Serial.print("done with hiTemp heat, move to loc = "); Serial.println(2);
		}
		break;
	case 7:  //waiting for move to Read
		if (shuttleServo.Idle()) //move ended
		{
			if (blueRead)
			{
				blueLed.On();
				int readDetector = analogRead(pinAnalogDetector);
				Serial.print("blue read = "); Serial.println(readDetector);
			}
			if (redRead)
			{
				redLed.On();
				int readDetector = analogRead(pinAnalogDetector);
				Serial.print("red read = "); Serial.println(readDetector);
			}
			Serial.println("done with reads");
			if (cycleCount < nCycles)
			{
				cycleCount++;
				shuttleServo.GoToLoc(3);
				cycleState = 3;
				Serial.println("");
				Serial.print("begin cycle = "); Serial.println(cycleCount);
				Serial.print("move to loc = "); Serial.println(3);
			}
			else
			{
				cycleCount = 0;
				shuttleServo.GoToLoc(0);
				cycleState = 8;
				Serial.print("done with cycling, move to loc = "); Serial.println(0);
			}
		}
		break;
	case 8:  //moving to home after last cycle
		if (shuttleServo.Idle()) //move ended
		{
			cycleState = 0;
			Serial.println("Back home again");
		}
		break;
	}
	return;
}
