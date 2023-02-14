#include <Arduino.h>
#include <math.h>

#include <queue>

#include <MS5xxx.h>
MS5xxx sensor(&Wire);

//Add pin numbers based on PCB design on chip
unsigned const int BARO_SDA;
unsigned const int BARO_SCLK;
unsigned const int ALTI_OUT_X;
unsigned const int ALTI_OUT_Y;
unsigned const int ALTI_OUT_Z;
unsigned const int REEF_FIRE;

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

//Create file for data storing
FILE * fData;

typedef enum {
  BEFORE_DATA_COLLECTION,
  START_DATA_COLLECTION,
  MAINS_RELEASED,
  EMATCH_ON,
  EMATCH_OFF,
  END_DATA_COLLECTION
} state_t;

state_t state;

long detectionTime;
long fireTime = 6000;
long durationOfFire = 500;
long dataEndTimeFromEMatchOff = 45000;
double averageAlt = -1;
queue<double> currentAlts;

////////////////////////////////////////////////////////////////////


void setup() {
  
  //assign pins
  pinMode(BARO_SDA, INPUT);
  pinMode(BARO_SCLK, OUTPUT);
  pinMode(ALTI_OUT_X, INPUT);
  pinMode(ALTI_OUT_Y, INPUT);
  pinMode(ALTI_OUT_Z, INPUT);
  pinMode(REEF_FIRE, OUTPUT);

  state = state_t::BEFORE_DATA_COLLECTION;
}


////////////////////////////////////////////////////////////////////


void loop() {

  _delay_ms(50);
  
  //Barometer Output
  double baroData = readBarometer();

  //Current time
  double currentTime = millis();

  //Calculate speed
  double speed = calculateSpeed(baroData, prevAltitude, prevTime, currentTime);

  //Calculate acceleration
  double netAccel = readAccelerometer();

  //Update prevAltitude and Time
  prevAltitude = baroData;
  prevTime = currentTime;

  switch(state) {
    case BEFORE_DATA_COLLECTION:
      if (baroData <= dataLogStartAltitude && speed < -5) {
        state = state_t::START_DATA_COLLECTION;  
      }
      break; 
    case START_DATA_COLLECTION:
      storeData(baroData, netAccel, speed, currentTime);
      if (readyToFire(baroData, speed) == 1) {
          detectionTime = millis();
      }
      break;
    case MAINS_RELEASED:
      if (currentTime >= detectionTime + fireTime) {
        eMatchOn();
        state = state_t::EMATCH_ON;
      }
      break;
    case EMATCH_ON:
      if (currentTime >= detectionTime + fireTime + durationOfFire) {                //store current time, check if time is current time +500millis seconds      // not finished 
        eMatchOff();
        state = state_t::EMATCH_OFF;
      }
      break;
    case EMATCH_OFF:
      if (currentTime >= detectionTime + dataEndTimeFromEMatchOff && speed < 2 && netAccel < 4) {
        state = state_t::END_DATA_COLLECTION;
      }
      break;
    case END_DATA_COLLECTION:
      
      break;
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

  // if altitude reading is unreasonable, ignore it
  if (alt < averageAlt-5000 || alt > averageAlt+5000) {
    return averageAlt;
  }

  //update average
  if (currentAlts.size() >= 5) {
    double prevAlt = currentAlts.front();
    currentAlts.pop();
    currentAlts.push(alt);
    averageAlt -= prevAlt/5;
    averageAlt += alt/5;
  }
  else {
    currentAlts.push(alt);
    averageAlt += alt/5;
  }

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

int readyToFire(double altitude, double speed) {
  if (speed < 0) {
    if (averageAlt > altitude) { //two if statements to confirm it is going downwards
      if (altitude < REEF_ALTITUDE_1) {
          return 1;
      }
    }
  }
  return -1;
}

void storeData(double netAccel, double altitude, double speed, long time) {
  
  //fData = fopen();
}

void eMatchOn(){
  digitalWrite(REEF_FIRE, HIGH);
}

void eMatchOff() {
  digitalWrite(REEF_FIRE, LOW);
}