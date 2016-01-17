/*
*	Car.cpp
*/
#include "CaroloCup.h"
/* --- CAR --- */

const unsigned short Car::DEFAULT_SERVO_PIN = 8;
const unsigned short Car::DEFAULT_ESC_PIN = 9;
const unsigned short Car::DEFAULT_PID_LOOP_INTERVAL = 40;
const float Car::DEFAULT_KP = 5.0;
const float Car::DEFAULT_KI = 0.0;
const float Car::DEFAULT_KD = 10.0;

const int IDLE_RAW_SPEED = 1500;
const int MAX_FRONT_RAW_SPEED = 1800; //can go to 1800
const int MAX_BACK_RAW_SPEED = 1200; //can go to 1200
const float IDLE_SPEED = 0.0; //the speed where the car is idle
const int BRAKE_FRONT_RAW_SPEED = 1300;
const int BRAKE_BACK_RAW_SPEED = 1550;
const float MAX_BACK_SPEED = -2.0; //the minimum user-set frequency we are allowed to go
const float MAX_FRONT_SPEED = 2.0; //the max user-set frequency we are allowed to go
const float MAX_BACK_CRUISE_SPEED = -2.0;
const float MAX_FRONT_CRUISE_SPEED = 2.0;
const int STRAIGHT_WHEELS = 90;
const int MAX_RIGHT_DEGREES = 120;
const int MAX_LEFT_DEGREES = 60;

Car::Car(unsigned short steeringWheelPin, unsigned short escPin){
	setSteeringWheelPin(steeringWheelPin);
	setESCPin(escPin);
	cruiseControl = false;
}

void Car::begin(){
	motor.attach(_escPin);
	steeringWheel.attach(_steeringWheelPin);
	setSpeed(IDLE_SPEED);
	setAngle(IDLE_SPEED);
}

void Car::setSteeringWheelPin(unsigned short steeringWheelPin){
	_steeringWheelPin = steeringWheelPin;
}

void Car::setESCPin(unsigned short escPin){
	_escPin = escPin;
}

void Car::setSpeed(float newSpeed){
	if (cruiseControl){
		if ((_speed != IDLE_SPEED) && (newSpeed * _speed) <= 0) stop(); //if the speeds are signed differently, stop the car and then set the new speed. Ignore this if the speed is already 0 and if speed is at the idle raw speed i.e. leftovers from non-cruise control mode (if IDLE_RAW_SPEED is not 0, it makes sense)
		_speed = constrain(newSpeed, MAX_BACK_CRUISE_SPEED, MAX_FRONT_CRUISE_SPEED);
	}else{
		if ( _speed != IDLE_SPEED && (newSpeed < 0.001 && newSpeed > -0.001)) stop(); //if we are not already stopped (_speed = idle raw speed) and the new speed is 0 then stop
		_speed = constrain(newSpeed, MAX_BACK_SPEED, MAX_FRONT_SPEED);
		setRawSpeed(speedToFreq(_speed));
	}
}

void Car::updateMotors(){
	if (cruiseControl && (millis() > _lastMotorUpdate + _pidLoopInterval)){
		if (_speed){ //if _speed is 0, we have already made sure the car is stopped. don't try to adjust if car is just drifting
			float measuredSpeed = _encoder.getSpeed(); //speed in m/s
			if (_speed < 0) measuredSpeed *= -1; //if we are going reverse, illustrate that in the value of measuredSpeed
			int controlledSpeed = motorPIDcontrol(_previousControlledSpeed, _speed, measuredSpeed);
			setRawSpeed(controlledSpeed);
			_previousControlledSpeed = controlledSpeed;
			_lastMeasuredSpeed = getGroundSpeed() * (abs(_speed)/_speed); //log down the (signed) measured speed by the controller
		}
		_lastMotorUpdate = millis();
	}
}

int Car::motorPIDcontrol(const int previousSpeed, const float targetSpeed, const float actualSpeed){
	float correction = 0;
	float error = targetSpeed - actualSpeed;
	_integratedError += error * _pidLoopInterval;
	correction = (_Kp * error) + (_Ki * _integratedError) + (_Kd * (error - _previousError)/_pidLoopInterval);                            
	_previousError = error;
	return constrain(previousSpeed + int(correction), MAX_BACK_RAW_SPEED, MAX_FRONT_RAW_SPEED);
}

 float Car::getGroundSpeed(){
	unsigned long currentDistance = _encoder.getDistance();
	unsigned long dX = currentDistance - _previousDistance;
	_previousDistance = currentDistance;
	unsigned long dT = millis() - _lastMotorUpdate;
	float velocity = float(dX)/ float(dT);
	return velocity;
}

void Car::setRawSpeed(int rawSpeed){ //platform specific method
	unsigned int speed = constrain(rawSpeed, MAX_BACK_RAW_SPEED, MAX_FRONT_RAW_SPEED);
	motor.write(speed);
}

void Car::setAngle(int degrees){ //platform specific method
	_angle = constrain(STRAIGHT_WHEELS + degrees, MAX_LEFT_DEGREES, MAX_RIGHT_DEGREES);
	steeringWheel.write(_angle);
}

void Car::stop(){ //platform specific method
	if (_speed > 0.001 || _speed < -0.001){	
		_lastMotorUpdate = millis();
		float velocity = 0.0;
		
		velocity = getGroundSpeed(); //just call it a couple of times so we get an idea of the current speed
		
		int attempts = 1;
		while ((attempts > 0) && (velocity > 0.2)){ //while we haven't ran out of attempts AND we detect some velocity, go the opposite way
			if (_speed>0){
				setRawSpeed(BRAKE_FRONT_RAW_SPEED);
			}else{
				setRawSpeed(BRAKE_BACK_RAW_SPEED);
			}
			velocity = getGroundSpeed();
			attempts--;
			delay(DEFAULT_PID_LOOP_INTERVAL);
		}
		setRawSpeed(IDLE_RAW_SPEED);
	}
	if (cruiseControl){
		setRawSpeed(IDLE_RAW_SPEED); //shut the engines down, we should be stopped by now
		enableCruiseControl(_encoder); //re-initialize the cruise control, se we get rid of previous error and pid output
		_speed = IDLE_SPEED;
	}else{
		setSpeed(IDLE_SPEED);
	}
}

float Car::getSpeed(){
	return _speed; //return speed in meters per second
	
}

int Car::getAngle(){
	return _angle - STRAIGHT_WHEELS;
}

void Car::enableCruiseControl(Odometer encoder, float Kp, float Ki, float Kd, unsigned short pidLoopInterval){
	_encoder = encoder;
	cruiseControl = true;
	_pidLoopInterval = pidLoopInterval;
	_lastMotorUpdate = 0;
	_previousControlledSpeed = IDLE_RAW_SPEED;
	_previousDistance = _encoder.getDistance();
	_integratedError = 0;
	_Kp = Kp;
	_Ki = Ki;
	_Kd = Kd;
}

void Car::disableCruiseControl(){
	cruiseControl = false;
	_speed = _previousControlledSpeed; //update the speed with the PWM equivalent
}

float Car::getMeasuredSpeed(){
	return _lastMeasuredSpeed;
}
int Car::speedToFreq(float MpS){
	if (MpS >0){
		return MpS*60 +IDLE_RAW_SPEED;
	}
	else{
		return MpS*200 +IDLE_RAW_SPEED;
	}
   
  }
