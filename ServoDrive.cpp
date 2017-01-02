//#define debug
#include "ServoDrive.h"


ServoDrive::ServoDrive(int p, int dpw, int nl, int sl[])
{
	//copy args into object
	pinServo = p;
	pwDelta = dpw;
	nLoc = nl;
	for (int i = 0; i < nLoc; i++)
	{
		setLocs[i] = sl[i];
	}
}





void ServoDrive::Attach()
{
	cmdServo.attach(pinServo);
	Serial.print("Attach servo on pin = "); Serial.println(pinServo);
	/*
	since the current location of servo is unknown
	the timed move method dPW per msDelta can't be used
	So just set both targetPW and stepPW to the homePW and delay long enough for it to get there
	CheckStateTime will set state back to 0
	*/
	if (pwDelta < 2 || pwDelta>12)
	{
		pwDelta = 5;
		Serial.println("pwDelta not set");
	}
	targetPW = setLocs[0];
#ifdef debug
	Serial.print("setLocs[0] = "); Serial.println(targetPW);
#endif
	if (targetPW < 900 || targetPW>2100)
	{
		targetPW = 1500;
		Serial.println("Home not set");
	}
	stepPW = targetPW;
	//set pwDelta for cmd moves

	signPWdelta = pwDelta;
	#ifdef debug  
	Serial.print("msDelta="); Serial.println(msDelta);
	Serial.print("pwDelta="); Serial.println(pwDelta);
	Serial.print("targetPW="); Serial.println(targetPW);
	Serial.print("stepPW="); Serial.println(stepPW);
	#endif
	cmdServo.writeMicroseconds(stepPW);
	delay(coldStartHomeDelay); //exception to "no delay" since CheckStateTime can't might stop before move done
	servoState = 1;
	startMsec = millis();
}
void ServoDrive::SetDeltaPW(int dpw)
{
	if (dpw > 12)
	{
		Serial.println("Set at max speed, pwDelta = 12");
		pwDelta = 12;
	}
	else
	{
		pwDelta = dpw;
		setLocs[3] = dpw;
		Serial.print("pwDelta = "); Serial.println(pwDelta);
	}
	return;
}
void ServoDrive::SetLoc(int loc)
{
	setLocs[loc] = currentPW;
	Serial.print("Set Loc = "); Serial.print(loc); Serial.print("  PW = "); Serial.println(currentPW);
}
void ServoDrive::GoToLoc(int loc)
{
	targetPW = setLocs[loc];

	int movePW = targetPW - currentPW;
	if (movePW == 0) return;
	if (abs(movePW) < pwDelta)
	{
		Serial.print("Move < pwDelta = "); Serial.println(pwDelta);
		return;
	}
	if (abs(movePW) < 8)
	{
		Serial.println("Move < deadband = 8");
		return;
	}
	if (movePW < 0)
	{
		signPWdelta = -pwDelta;
	}
	else signPWdelta = pwDelta;
	stepPW = currentPW + signPWdelta;

	#ifdef debug
	Serial.print("Loc = "); Serial.print(loc); Serial.print("  targetPW = "); Serial.println(targetPW);
	#endif
	cmdServo.writeMicroseconds(stepPW);
	servoState = 1;
	startMsec = millis();
}
void ServoDrive::ListServoParameters()
{
	int dpw;
	Serial.print("digital PWM Pin = "); Serial.println(pinServo);
	Serial.print("pw delta per 4ms = "); Serial.println(pwDelta);

	for (int i = 0; i < nLoc; i++)
	{
		Serial.print("Loc = "); Serial.print(i); Serial.print("  PW = "); Serial.println(setLocs[i]);
	}
	return;
}
void ServoDrive::GoToPWM(int pwm)
{
	targetPW = pwm;
	int movePW = targetPW - currentPW;
	if (movePW == 0) return;
	if (abs(movePW) < pwDelta)
	{
		Serial.print("Move < pwDelta = "); Serial.println(pwDelta);
		return;
	}
	if (abs(movePW) < 8)
	{
		Serial.println("Move < deadband = 8");
		return;
	}
	if (movePW < 0)
	{
		signPWdelta = -pwDelta;
	}
	else signPWdelta = pwDelta;
	stepPW = currentPW + signPWdelta;
#ifdef debug
	Serial.println("GoToPWM");
	Serial.print("currentPW="); Serial.println(currentPW);
	Serial.print("signPWdelta="); Serial.println(signPWdelta);
	Serial.print("targetPW="); Serial.println(targetPW);
	Serial.print("stepPW="); Serial.println(stepPW);
	Serial.print("Start mSec="); Serial.println(startMsec);
#endif
	servoState = 1;
	startMsec = millis();
	cmdServo.writeMicroseconds(stepPW);
	return;
}
int ServoDrive::GetPWM()
{
	return currentPW;
}
int ServoDrive::CheckStateTime()
/*
pulse freq = 50 Hz, 20 msec between pulses to servo
I'll send update every 5 msec, but only the the last one received before pulse goes out will be acted on
4 updates per pulse will minimize aliasing, and give finer resolution on end of move
the main advantage of this "moving target" method is I know approximately when the target is reached
so I can change states,
and I know the speed so long as pwDelta/msDelta is < 2.9 (the max no load speed = 60 deg/230ms x 1000pw/90deg}

*/
{
	if (servoState == 0) return 0;
	else if (servoState == 1)
	{
		if (millis() - startMsec > msDelta)
		{
			stepPW += signPWdelta;
#ifdef debug
			Serial.println("CheckStateTime");
			Serial.print("startMsec = "); Serial.println(startMsec);
			Serial.print("millis = "); Serial.println(millis());
			Serial.print("targetPW = "); Serial.println(targetPW);
			Serial.print("stepPW = "); Serial.println(stepPW);
			Serial.print("stepPW-targetPW = "); Serial.print(abs(stepPW - targetPW)); Serial.println();
#endif
			if (abs(stepPW - targetPW) <= pwDelta)
			{
				stepPW = targetPW;
				currentPW = targetPW;
				servoState = 0;
#ifdef debug
				Serial.print("Stop mSec = "); Serial.println(millis());
				Serial.print("Servo at pw = "); Serial.println(currentPW); Serial.println();
#endif
			}
			startMsec = millis();
			cmdServo.writeMicroseconds(stepPW);
		}
		return servoState;
	}
}
bool ServoDrive::Idle()
{
	if (servoState == 0) return true;
	else return false;
}


