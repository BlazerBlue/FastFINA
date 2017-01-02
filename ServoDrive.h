// ServoDrive.h

#ifndef _ServoDrive_h
#define _ServoDrive_h
#include "arduino.h"


#include <Servo.h>
#include <math.h>




//heater parameters
#define hHome 0
#define hHiPress 1
#define hLoPress 2
#define hDpw 3

#define coldStartHomeDelay 4000

class ServoDrive
{
	Servo cmdServo;  //create instance of Servo
	unsigned long int delayMsec;
	unsigned long int startMsec;
	int servoState;
	int currentLoc;
	int currentPW;
	int targetPW;
	/*
	pwDelta and msDelta control both the speed and state change at the end
	the pulse width (PW) tells the servo where it should be, and it goes there if it's not where it's at

	if you know the start and stop pw's, then you can change state once pw = target pw.
	I define a "step" as the interval between sending new PWs to the servo controller
	the servo is sending out a new PW every 20 msec (50Hz), so if I send a new one every 5 msec
	then I could have 4 updates for every one sent out
	longer than 20 ms would miss some, many more than 3 or 4 would just waste time

	the speed is delta pw / delta msec = pwDelta/msDelta
	we know from the specs that change in the pw of 1000 usec, moves the servo 90 deg.
	90 deg/ 1000 pw x pwDelta / msDelta = deg/ms, for pwDelta = 10 us and msDelta= 5 ms
	speed = 180 deg/sec, which may seem slow, but may reduce "buzzing" or "humming"
	to go faster, increase pwDelta (leave msDelta = 4 which sends updates per servo frame)

	at each step, I send the servo a value of stepPW, and each step it's incremented by signPWdelta
	stepPW could be going up or down
	targetPW is the destination, where I stop incrementing
	when abs(stepPW > targetPW), we're done and the state changes

	Table of times to move 18deg and number of frames sent to servo for range of pwDeltas:
	pwDelta	ms/18dg	frames
	5		200		10
	10		100		5
	20		50		2.5

	*/
	int nLoc;
	int setLocs[5];
	int pwDelta = 5;
	int signPWdelta;
	int msDelta = 4;
	int stepPW;

	int pinServo;
public:
	//pinShuttle, dpwShuttle,nLocShuttle,locShuttle
	ServoDrive(int p, int dpw, int nl, int l[]);
	void Attach();
	void SetDeltaPW(int dpw);
	void GoToPWM(int pwm);
	void SetLoc(int loc);
	void ListServoParameters();
	void GoToLoc(int loc);
	int GetPWM();
	int CheckStateTime();
	bool Idle();
};

#endif

