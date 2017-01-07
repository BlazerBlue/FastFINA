/*
FastCycler.ino

Revised for Zero Atmel SAMD21
ARM Cortex M0+, 32bit, 48 MHz, 3.3V
256kB flash, 32kB SRAM, 0 EEPROM
12 bit ADC
??SAMD_AnalogCorrection?? why needed??
Absent EEPROM will be replaced short term by constants in programs,
longer term they can be stored on hard drive of PC running GUI

Written for Uno, ATmega328P
5V, 16MHz
1kB EEPROM, 2kB SRAM, 32kB Flash

Because of memory limitations, command error checking should be done
in PC app that will be the interface that most people use
*/

//#define debugCmd


#include "CycleDrive.h"
#include "LedDrive.h"
#include "ServoDrive.h"

//Global variables for command input processes
int cmdSize;
int cmdGroup;
int cmdNparameters;
int cmdParameter[5];
int cmdState;


//1. Set up parameters for servos, defined when instance created
int pinShuttle = 5; //5&6 are PWM set by timer 0, default period = 256
int dpwShuttle = 8;
int nLocShuttle= 4;
int locShuttle[] = { 2000,1700, 1350,1000}; //shuttle location settings
#define sHome 0
#define sHiHeat 1
#define sRead 2
#define sLoHeat 3
ServoDrive shuttleServo(pinShuttle, dpwShuttle,nLocShuttle,locShuttle);

#define hHome 0
#define hHiPress 1
#define hLoPress 2
#define hDpw  3
#define hNumLocs 3

int pinHeaters = 6; 
int locHeaters[] = { 1500,1420,1550}; //heater settings
int nLocHeaters = 3;
int dpwHeaters = 4;
ServoDrive heaterServo(pinHeaters, dpwHeaters, nLocHeaters, locHeaters);

//2. Set up parameters for LEDs, defined when instance created
int pinBlueLed = 9; //9&10 PWM set by timer 1, phase correct, default period = 510
int eBaseAddrBlue = 200;  //reserve 200->249 for blue LED parameters
LedDrive blueLed(pinBlueLed);
int pinRedLed = 10; 
LedDrive redLed(pinRedLed);

//3. Detector set up, Arduino only reads once and sends result to app on PC
int pinAnalogDetector = 0;
int msDelayDetector = 1;
int nRepsRead = 10;

//0. Set up parameters for cycling, store here, pass to CycleDrive when created
int setCycle[6] = { 3,5,1,2,1,1 };
#define nCyclesI 0
#define sHiTempInitI 1
#define sHiTempI 2
#define sLoTempI 3
#define blueReadI 4
#define redReadI 5

CycleDrive cycler(shuttleServo, heaterServo, blueLed, redLed, pinAnalogDetector, setCycle);

void setup()
{
	Serial.begin(115200);
	//while (!Serial);

	Serial.println("Unit parameter values");
	Serial.println("Cycler:");
	cycler.ListCycleParameters();
	Serial.println("Shuttle servo:");
	shuttleServo.ListServoParameters();
	Serial.println("Heater servo:");
	heaterServo.ListServoParameters();
	Serial.println("Blue LED:");
	blueLed.ListLedParameters();
	Serial.println("Red LED:");
	redLed.ListLedParameters();
	Serial.println("Send Enter for list of commands");
}

// the loop function runs over and over again until power down or reset
void loop()
{
	int servoState = shuttleServo.CheckStateTime(); //real time inputs get first slots
	servoState = heaterServo.CheckStateTime(); //real time inputs get first slots
	cycler.CheckStateTime();

	//

	if (Serial.available()>0)
	{
		getCommand();
		if (cmdState == 1)
		{
			#ifdef debugCmd
			Serial.print("cmdSize="); Serial.println(cmdSize);
			Serial.print("cmdGroup="); Serial.println(cmdGroup);
			Serial.print("cmdNparameters="); Serial.println(cmdNparameters);
			Serial.print("cmdParameter[0]="); Serial.println(cmdParameter[0]);
			#endif
			if (cmdSize == 0)
			{
				Serial.println("Startup: List commands by entering unit nunber: ");
				Serial.println("0 Cycle");
				Serial.println("1 Servos");
				Serial.println("2 LEDs");
				Serial.println("3 Detector");
				Serial.println("4 Heaters");
				Serial.println();

			}
			else
			{
				switch (cmdGroup)
				{
				case 0: //cycle
					cycleCmds();
					break;
				case 1: //Servos
					servoCmds();
					break;
				case 2: //LED commands
					ledCmds();
					break;
				case 3: //Detector commands
					detectCmds();
					break;
				case 4: //Heater commands
					heaterCmds();
					break;
				default:
					Serial.println("Command error");
					break;
				}
			}

		}
		cmdState = 0;
	}
}

void detectCmds()
{
	if (cmdSize == 1)
	{
		Serial.println("Detect commands");
		Serial.println("3.0 read once");
		Serial.println("3.1 read N times");
		Serial.println("3.2.N read times to N");
		return;
	}
	else
	{
		int DetectCmdNumber = cmdParameter[0];
		switch (DetectCmdNumber)
		{
		case 0: // read once
			int rd;
			rd = analogRead(pinAnalogDetector);
			delay(msDelayDetector);
			Serial.print("Detector read = "); Serial.println(rd);
			break;
		case 1:  // read N times
			float sumReads = 0;
			float sumReadsSq = 0;
			for (int i = 0; i < nRepsRead; i++)
			{
				int r = analogRead(pinAnalogDetector);
				Serial.println(r);
				sumReads += r ;
				sumReadsSq += r^2;
			}
			float avgRead = sumReads / nRepsRead;
			float varRead = sumReadsSq-nRepsRead * avgRead*avgRead;
			float sdRead = sqrt(varRead);
			Serial.print("N Reads = "); Serial.println(nRepsRead);
			Serial.print("Mean = "); Serial.println(avgRead);
			Serial.print("Var = "); Serial.println(varRead);
			break;
		}
	}
}
void cycleCmds()
{
	if (cmdSize == 1)
	{
		Serial.println("0.0 List cycle parameters");
		Serial.println("0.1 start cycling");
		Serial.println("0.2 stop cycling");
		Serial.println("0.3.N number of cycles");
		Serial.println("0.4.I95.C95.C60 Heat times in sec");
		Serial.println("0.5.B.R Blue and red channels on=1, off=0");
	}
	else
	{
		int CycleCmdNumber = cmdParameter[0];
		switch (CycleCmdNumber)
		{
		case 0: // 0.0 List cycle parameters N, t95i,...
			Serial.println("cycle parameters");
			cycler.ListCycleParameters();
			break;
		case 1:  //0.1 start cycling
			cycler.StartCycling();
			Serial.println("Start, cycling parameters");
			cycler.ListCycleParameters();
			Serial.println("shuttle servo parameters");
			shuttleServo.ListServoParameters();
			Serial.println("heater servo parameters");
			heaterServo.ListServoParameters();
			break;
		case 2: //0.2 stop cycling
			cycler.StopCycling();
			break;
		case 3: //0.3.n store nCycles
			cycler.StoreNcycles(cmdParameter[1]);
			break;
		case 4: //0.4.t95o,t95v,t70
			cycler.StoreHeatTimes(cmdParameter[1], cmdParameter[2], cmdParameter[3]);
			break;
		case 5: //0.5.b.r  blue and red LEDs on=1, off=0
			cycler.StoreReadChan((boolean)cmdParameter[1], (boolean)cmdParameter[2]);
			Serial.print("blueRead = "); Serial.println((boolean)cmdParameter[1]);
			Serial.print("redRead = "); Serial.println((boolean)cmdParameter[2]);
			break;
		}

	}

}
void servoCmds()
{
	if (cmdSize == 1)
	{
		Serial.println("Servo command format: 1.SN.Cmd.Parameter");
		Serial.println("SN 0 = shuttle = pin 5");
		Serial.println("SN 1 = heaters = pin 6");
		Serial.println("1.SN.0 = home/attach/initialize");
		Serial.println("1.SN.1.LN = go to location number LN");
		Serial.println("1.SN.2.PW = go to PW(900-2100)");
		Serial.println("1.SN.3.LN = set current PW to LN");
		Serial.println("1.SN.4.DPW = set dPW");
		Serial.println("1.SN.5 = Print stored parameters");
	}
	else
	{
		int ServoNumber = cmdParameter[0];
		int ServoCmdNumber = cmdParameter[1];
		switch (ServoCmdNumber)
		{
		case 0: //1.N.0 attach servo
			if (ServoNumber == 0) shuttleServo.Attach();
			else heaterServo.Attach();
			break;
		case 1:  //1.N.1.LN Go to location number LN
			if (ServoNumber == 0) shuttleServo.GoToLoc(cmdParameter[2]);
			else heaterServo.GoToLoc(cmdParameter[2]);
			break;
		case 2: //1.N.2.PW go to PW
				//Serial.print("cmdParameter[2]"); Serial.println(cmdParameter[2]);
			if (ServoNumber == 0) shuttleServo.GoToPWM(cmdParameter[2]);
			else heaterServo.GoToPWM(cmdParameter[2]);
			break;
		case 3: //1.N.3.LN set current PW to LN
			if (ServoNumber == 0) shuttleServo.SetLoc(cmdParameter[2]);
			else heaterServo.SetLoc(cmdParameter[2]);
			break;
		case 4: //1.N.4.DPW set dPW 
			if (ServoNumber == 0) shuttleServo.SetDeltaPW(cmdParameter[2]);
			else heaterServo.SetDeltaPW(cmdParameter[2]);
			break;
		case 5: //1.N.5  Print parameters in EEPROM
			if (ServoNumber == 0) shuttleServo.ListServoParameters();
			else heaterServo.ListServoParameters();
			break;
		}

	}

}


void ledCmds()
{
	if (cmdSize == 1)
	{
		Serial.println("LED commands 1.Led#.Cmd#.Parameter#");
		Serial.println("Led# 0 = pin 9, 1 = pin 10");
		Serial.println("2.N.0 = initialize");
		Serial.println("2.N.1 = off");
		Serial.println("2.N.2 = on");
	}
	else
	{
		int LedNumber = cmdParameter[0];
		int LedCmdNumber = cmdParameter[1]; 
		switch (LedCmdNumber)
		{
		case 0: //2.N.0 initialize
			if (LedNumber == 0) blueLed.Init();
			else redLed.Init();
			break;
		case 1:  //2.N.1 = turn N off
			if (LedNumber == 0) blueLed.Off();
			else redLed.Off();
			break;
		case 2: //2.N.2 = turn N on
			if (LedNumber == 0) blueLed.On();
			else redLed.On();
			break;
		}
	}

}

void heaterCmds()
{
	return;
}

//if I could pass Serial as parameter, then I could make this a class
//I hang here until no characters left in serial in buffer
void getCommand()
{
	String inStr = "";
	char inChr;
	cmdSize = 0;
	cmdNparameters = 0;
	cmdGroup = 0;

	while (Serial.available() > 0)
	{
		inChr = Serial.read();
		#ifdef debugCmd
		Serial.print("digit = "); Serial.println(inChr);
		#endif
		if (isDigit(inChr))
		{
			inStr += inChr;
			#ifdef debugCmd
			Serial.print("string of digits: "); Serial.println(inStr);
			#endif
		}
		else if (inStr.length()==0 && (cmdSize == 0 && inChr == '\n'))
		{
			#ifdef debugCmd
			Serial.println("NL only character sent");
			#endif
			cmdState = 1;
			return;
		}
		else if (cmdSize == 0 && (inChr == '.'|| inChr == '\n'))
		{
			#ifdef debugCmd
			Serial.println("cmdSize==0 && (inChr==. || NL)");
			#endif
			cmdGroup = inStr.toInt();
			inStr = "";
			cmdSize++;
			cmdState = 1;
		}
		else if (cmdState == 1 && cmdSize < 6)
		{
			cmdParameter[cmdSize - 1] = inStr.toInt();
			cmdNparameters = cmdSize;
			#ifdef debugCmd
			Serial.print("cmdParameter = "); Serial.println(cmdParameter[cmdSize - 1]);
			#endif
			inStr = "";
			cmdSize++;
			cmdState = 1;
		}
		else
		{
			cmdState = 0;
			Serial.println("Invalid command");
		}
		delayMicroseconds(100); //ok for 115200, need longeer for 9600
	}
#ifdef debugCmd
	Serial.println("no more characters in serial buffer");
#endif
	return;
}




