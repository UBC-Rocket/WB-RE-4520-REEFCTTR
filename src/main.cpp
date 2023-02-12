#include <Arduino.h>
#include <math.h>

#include <MS5xxx.h>
MS5xxx sensor(&Wire);

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

//Create file for data storing
FILE * fData;

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

long ematch1FireTime;

////////////////////////////////////////////////////////////////////


void setup() {
  
  //assign pins
  pinMode(BARO_SDA_IN, OUTPUT);
  pinMode(BARO_SDA_OUT, INPUT);
  pinMode(BARO_SCLK, OUTPUT);
  pinMode(ALTI_OUT_X, INPUT);
  pinMode(ALTI_OUT_Y, INPUT);
  pinMode(ALTI_OUT_Z, INPUT);
  pinMode(REEF_FIRE_1, OUTPUT);
  pinMode(REEF_FIRE_2, OUTPUT);

}


////////////////////////////////////////////////////////////////////


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
      if (baroData <= dataLogStartAltitude && speed < -5){
        state = state_t::START_DATA_COLLECTION;  
      }
      break; 
    case START_DATA_COLLECTION:
      storeData(baroData, netAccel, xAccel, yAccel, zAccel, speed, currentLogTime);
      if (mainsReleased == 1){
        state = state_t::MAINS_RELEASED;
      }
      break;
    case MAINS_RELEASED:
      if (readyToFire1 == 1){
        fireEmatch(1);
        state = state_t::FIRST_REEF_FIRED;
      }
      break;
    case FIRST_REEF_FIRED:
      if (currentTime >= prevTime + 500 ){                //store current time, check if time is current time +500millis seconds      // not finished 
        ematchOff(1);
        state = state_t::FIRST_EMATCH_OFF;
      }
      break;
    case FIRST_EMATCH_OFF:
      
// seven millisseconds 



        // Outdated if statements. to be replaced and integrated into state machine 
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


////////////////////////////////////////////////////////////////////


// return altitude
double readBarometer() {
  sensor.ReadProm();
  sensor.Readout();
  double Pressure = sensor.GetPres();
  //Pressure [Pa] to alt [m]
  //convert pressure to altitude. Formula from https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
  double alt = (1-pow((Pressure/100)/1013.25, 0.190284)) * 145366.45 * 0.3048;

  return alt;
}

// return data from accelerometer
double readAccelerometer() {
    //get outputs from each axis
  double xAccel = analogRead(ALTI_OUT_X);
  double yAccel = analogRead(ALTI_OUT_Y);
  double zAccel = analogRead(ALTI_OUT_Z);

  double rawX;
  double rawY;


  return sqrt( pow(xAccel, 2) + pow(yAccel, 2) + pow(zAccel, 2) );
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

int storeData(double netAccel, double altitude, double speed, long time) {
  
  fData = fopen()
  
}

int mainsReleased(){
  return -1
}

void fireEMatch(int ematchNum){
  digitalWrite(ematchNum, HIGH);
}

void ematchOff(int ematchNum){
  digitalWrite(ematchNum, LOW);
}

long getCurrentTime(){
  return millis();
}