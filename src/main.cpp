#include <Arduino.h>
#include <math.h>

//Add pin numbers based on PCB design on chip
unsigned const int BARO_SDA_IN;
unsigned const int BARO_SDA_OUT;
unsigned const int BARO_SCLK;
unsigned const int ALTI_OUT_X;
unsigned const int ALTI_OUT_Y;
unsigned const int ALTI_OUT_Z;
unsigned const int REEF_FIRE;

//Check values to prevent reefing misfire
double CHECK_ALTITUDE = 700;
double CHECK_SPEED = 200;
double CHECK_ACCEL = 50;

double REEF_ALTITUDE = 100;
double REEF_SPEED = 15;

double prevAltitude = 0;
double prevTime = 0;

//Ready to reef
int ready = 0;

void setup() {

  pinMode(BARO_SDA_IN, OUTPUT);
  pinMode(BARO_SDA_OUT, INPUT);
  pinMode(BARO_SCLK, OUTPUT);
  pinMode(ALTI_OUT_X, INPUT);
  pinMode(ALTI_OUT_Y, INPUT);
  pinMode(ALTI_OUT_Z, INPUT);
  pinMode(REEF_FIRE, OUTPUT);

}

void loop() {

  //Barometer Output
  double baroData = readBarometer();

  //Accelerometer Outputs
  double *accelerometerData = readAccelerometer();
  double xAccel = accelerometerData[0];
  double yAccel = accelerometerData[1];
  double zAccel = accelerometerData[2];

  //Current time
  double currentTime;

  //Calculate speed
  double speed = calculateSpeed(baroData, prevAltitude, prevTime, currentTime);

  //Calculate acceleration
  double netAccel = calculateAccel(xAccel, yAccel, zAccel);

  //Update prevAltitude and Time
  prevAltitude = baroData;
  prevTime = currentTime;

  if (readyToFire(baroData, netAccel, speed) == 1) {
    fireReefingCutters();
  }
}

double readBarometer() {
  return -1;
}

double * readAccelerometer() {

}

double calculateSpeed(double altitude, double prevAlt, double previousTime, double currentTime) {
  return (altitude - prevAlt)/(currentTime - previousTime);
}

double calculateAccel(double xAccel, double yAccel, double zAccel) {
  return sqrt(pow(xAccel,2) + pow(yAccel,2) + pow(zAccel,2));
}

int readyToFire(double altitude, double accel, double speed) {

  if (ready == 1) {
    if (speed <= REEF_SPEED) {
      return 1;
    }
  } else if (ready == 0) {
    if (altitude <= CHECK_ALTITUDE && speed <= CHECK_SPEED) {
        if (accel >= CHECK_ACCEL) {
          ready = 1;
          return 0;
        }
    }
  }
}

void fireReefingCutters() {

}

