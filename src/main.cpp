#include <Arduino.h>
#include <math.h>

//Add pin numbers based on PCB design on chip
unsigned const int BARO_SDA_IN;
unsigned const int BARO_SDA_OUT;
unsigned const int BARO_SCLK;
unsigned const int ALTI_OUT_X;
unsigned const int ALTI_OUT_Y;
unsigned const int ALTI_OUT_Z;
unsigned const int REEF_FIRE_1;
unsigned const int REEF_FIRE_2;

//Check values to prevent reefing misfire
double CHECK_ALTITUDE_1 = 700; //[m]
double CHECK_SPEED_1 = 200;
double CHECK_ACCEL_1 = 50;

double CHECK_ALTITUDE_2 = 700;
double CHECK_SPEED_2 = 200;
double CHECK_ACCEL_2 = 50;

double REEF_ALTITUDE_1 = 150;
double REEF_SPEED_1 = 15;

double REEF_ALTITUDE_2 = 150;
double REEF_SPEED_2 = 15;

double prevAltitude = 0;
double prevTime = 0;

double dataLogStartAltitude = 2000;
long dataStartTime = 0;

//Ready to reef
int ready = -1;

typedef enum {
  BEFORE_DATA_COLLECTION,
  START_DATA_COLLECTION,
  MAINS_RELEASED,
  FIRST_REEF_FIRED,
  FIRST_EMATCH_OFF,
  SECOND_REEF_FIRED,
  SECOND_REEF_OFF,
  END_DATA_COLLECTION
} state_t;

state_t state;

long detectionTime;
long fireTime1 = 6000;
long fireTime2 = 12000;
long durationOfFire = 500;

void setup() {

  pinMode(BARO_SDA_IN, OUTPUT);
  pinMode(BARO_SDA_OUT, INPUT);
  pinMode(BARO_SCLK, OUTPUT);
  pinMode(ALTI_OUT_X, INPUT);
  pinMode(ALTI_OUT_Y, INPUT);
  pinMode(ALTI_OUT_Z, INPUT);
  pinMode(REEF_FIRE_1, OUTPUT);
  pinMode(REEF_FIRE_2, OUTPUT);

}

void loop() {

  delayMicroseconds(50);
  
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

  switch(state){
    case BEFORE_DATA_COLLECTION:
      foo();
      break;
    case 
  }

  if (state == state_t::BEFORE_DATA_COLLECTION && baroData <= dataLogStartAltitude && speed < -5 && zAccel < 4) {
    state = state_t::START_DATA_COLLECTION;
  }

  if (ready >= 0 && ready <= 6) {
    long currentLogTime = millis();
    storeData(baroData, xAccel, yAccel, zAccel, speed, currentLogTime);
  }

  if (ready != 2 && readyToFire1(baroData, netAccel, speed) == 1) {

    detectionTime = millis();

  } else if (ready == 2) {

    long currentTime = millis();
    if (currentTime >= detectionTime + fireTime1 || baroData <= 50) {
      digitalWrite(REEF_FIRE_1, HIGH);
      ready = 3;
    }

  } else if (ready == 3) {

    long currentTime = millis();
    if (currentTime >= detectionTime + fireTime1 + durationOfFire) {
      digitalWrite(REEF_FIRE_1, LOW);
      ready = 4;
    }

  } else if (ready == 4) {

    long currentTime = millis();
    if (currentTime >= detectionTime + fireTime2) {
      digitalWrite(REEF_FIRE_2, HIGH);
      ready = 5;
    }

  } else if (ready == 5) {

    long currentTime = millis();
    if (currentTime >= detectionTime + fireTime2 + durationOfFire) {
      digitalWrite(REEF_FIRE_2, LOW);
      ready = 6;
    }

  } else if (ready == 6) {

    long currentTime = millis();
    if (currentTime >= detectionTime + fireTime2 + durationOfFire + 15000) {
      ready = 7;
    }

  }
}

double readBarometer() {
  return -1;
}

double * readAccelerometer() {

}

double calculateSpeed(double altitude, double prevAlt, double previousTime, double currentTime) {
  //Returns the current descent speed of the rocket (positive- upward, negative- downward)
  return (altitude - prevAlt)/(currentTime - previousTime);
}

double calculateAccel(double xAccel, double yAccel, double zAccel) {
  //Returns the net acceleration of the rocket
  return sqrt(pow(xAccel,2) + pow(yAccel,2) + pow(zAccel,2));
}

int readyToFire1(double altitude, double accel, double speed) {

  if (altitude <= CHECK_ALTITUDE_1 && speed <= CHECK_SPEED_1) {
    if (ready == 0) {
      if (accel >= CHECK_ACCEL_1) {
        ready = 1;
        return 0;
      }
    } else if (ready == 1) {
      if (speed <= REEF_SPEED_1) {
        ready = 2;
        return 1;
      } else if (altitude < REEF_ALTITUDE_1) {
        ready = 2;
        return 1;
      }
    }
  }
}

int storeData(double altitude, double xAccel, double yAccel, double zAccel, double speed, long time) {
  return -1;
}
