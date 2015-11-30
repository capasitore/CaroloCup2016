/*
*	AndroidCar.h - A simple library for controlling the smartcar
*	sensors. 
*	Version: 0.3
*	Author: Dimitris Platis
*	Sonar class is essentially a stripped-down version of the NewPing library by Tim Eckel, adjusted to Smartcar needs
* 	Get original library at: http://code.google.com/p/arduino-new-ping/
* 	License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
*/

#ifndef CaroloCup_h
#define CaroloCup_h
#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <WProgram.h>
	#include <pins_arduino.h>
#endif
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Servo.h>
#include <Wire.h>

class Odometer {
	public:
		Odometer();
		int attach(unsigned short odometerPin);
		void begin();
		unsigned long getDistance();
		void detach();
	private:
		unsigned short _odometerInterruptPin, _odometerID;
};

class Car {
	public:
		Car(unsigned short steeringWheelPin = DEFAULT_SERVO_PIN, unsigned short escPin = DEFAULT_ESC_PIN);
		void begin();
		void setSpeed(float speed);
		void setAngle(int degrees);
		float getSpeed();
		int getAngle();
		void stop();
		void enableCruiseControl(Odometer encoder, float Kp = DEFAULT_KP, float Ki = DEFAULT_KI, float Kd = DEFAULT_KD, unsigned short frequency = DEFAULT_PID_LOOP_INTERVAL);
		void updateMotors();
		void disableCruiseControl();
	private:
		void setSteeringWheelPin(unsigned short steeringWheelPin);
		void setESCPin(unsigned short escPin);
		int motorPIDcontrol(const int previousSpeed, const float targetSpeed, const float actualSpeed);
		void setRawSpeed(int controlledSpeed);
		float getGroundSpeed();
		unsigned short _steeringWheelPin, _escPin, _pidLoopInterval;
		Servo motor, steeringWheel;
		int _angle;
		float _speed;
		static const unsigned short DEFAULT_SERVO_PIN, DEFAULT_ESC_PIN, DEFAULT_PID_LOOP_INTERVAL;
		static const float DEFAULT_KP, DEFAULT_KI, DEFAULT_KD;
		float _Kp, _Ki, _Kd;
		Odometer _encoder;
		boolean cruiseControl;
		unsigned long _lastMotorUpdate, _previousDistance;
		int _previousControlledSpeed;
		int _previousError, _integratedError;
};

class Sonar {
	public:
		Sonar();
		void attach(unsigned short triggerPin, unsigned short echoPin);
		unsigned int getDistance();
		unsigned int getMedianDistance(short iterations = SONAR_DEFAULT_ITERATIONS);

	private:
		unsigned int ping();
		boolean ping_trigger();
		uint8_t _triggerBit;
		uint8_t _echoBit;
		volatile uint8_t *_triggerOutput;
		volatile uint8_t *_triggerMode;
		volatile uint8_t *_echoInput;
		unsigned int _maxEchoTime;
		unsigned long _max_time;
		static const unsigned short SONAR_DEFAULT_ITERATIONS;
};

class Sharp_IR {
	public:
		Sharp_IR();
		void attach(unsigned short IR_pin);
		unsigned int getDistance();
		unsigned int getMedianDistance(short iterations = IR_DEFAULT_ITERATIONS);
	private:
		unsigned short _IR_pin;
		static const unsigned short IR_DEFAULT_ITERATIONS;
};

class NewPing{
	public:
		static void timer_start(unsigned long frequency, void (*userFunc)(void));
		static void timer_stop();
	private:
		static void timer_setup();
		static void timer_start_cntdwn();
};

class Gyroscope {
	public:
		Gyroscope();
		void attach();
		void begin(unsigned short samplingRate = DEFAULT_GYRO_SAMPLING); //in milliseconds
		int getAngularDisplacement();
		void update();
		unsigned int calibrate(unsigned int measurements = 100);
	private:
		void initMeasurement();
		void initializeGyro();
		int setupL3G4200D(int scale);
		void writeRegister(int deviceAddress, byte address, byte val);
		int getGyroValues();
		int readRegister(int deviceAddress, byte address);
		unsigned short _samplingRate;
		static const unsigned short DEFAULT_GYRO_SAMPLING;
		int _angularDisplacement;
		unsigned long _prevSample;
};

class Razorboard{
	public:
		Razorboard();
		void attach(HardwareSerial *razorSerial);
		boolean available();
		String readLine();
		String readLastLine();
		int getHeading();
		int getLatestHeading();
	private:
		String readRawSerialLine();
		int getRawHeading();
		HardwareSerial * _serial;
};

class MouseSensor{
	public:
		MouseSensor();
		void attach(HardwareSerial *mouseSerial);
		boolean available();
		String readLine();
		String readLastLine();
		int getX(String mouseInput);
		int getY(String mouseInput);
		void clear();
	private:
		String readRawSerialLine();
		HardwareSerial * _serial;
};

class SRF08{
	public:
		SRF08();
		void attach(const uint8_t address = DEFAULT_SRF08_ADDRESS);
		void setGain(const uint8_t gainValue);
		void setRange(const uint8_t rangeValue);
		void setPingDelay(const uint8_t milliseconds = DEFAULT_PING_DELAY);
		int getDistance();
		void changeAddress(const uint8_t newAddress);
	private:
		int ping();
		uint8_t _address, _delay;
		static const uint8_t DEFAULT_PING_DELAY, DEFAULT_SRF08_ADDRESS;

};

#endif
