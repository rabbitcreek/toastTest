/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */
#include "Adafruit_PM25AQI.h"

// If your PM2.5 is UART only, for UNO and others (without hardware serial) 
// we must use software serial...
// pin #2 is IN from sensor (TX pin on sensor), leave pin #3 disconnected
// comment these two lines if using hardware serial
//#include <SoftwareSerial.h>
//SoftwareSerial pmSerial(2, 3);

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
#include <SD.h>
#define cardSelect 4
File logfile;
float timer;
float integralTimer = 0;
double integralTotal = 0;
bool tell = 0;
char filename[15];
void setup() {
  // Wait for serial monitor to open
  //Serial.begin(115200);
  //while (!Serial) delay(10);
 if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
   
  }
  strcpy(filename, "/ANALOG00.TXT");
Serial.println("hey im here!");
  for (uint8_t i = 0; i < 100; i++) {
    filename[7] = '0' + i/10;
    filename[8] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  logfile = SD.open(filename, FILE_WRITE);  
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    
  }
  Serial.print("Writing to "); 
  Serial.println(filename);
  Serial.println("Adafruit PMSA003I Air Quality Sensor");

  // Wait one second for sensor to boot up!
  delay(1000);
pinMode(8, OUTPUT);
digitalWrite(8, LOW);
  // If using serial, initialize it and set baudrate before starting!
  // Uncomment one of the following
  Serial1.begin(9600);
  //pmSerial.begin(9600);

  // There are 3 options for connectivity!
  //if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
  if (! aqi.begin_UART(&Serial1)) { // connect to the sensor over hardware serial
  //if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial 
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");
  timer = millis();
}

void loop() {
  
  PM25_AQI_Data data;
  
  if (! aqi.read(&data)) {
    
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
  Serial.println("AQI reading success");

  Serial.println();
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Concentration Units (standard)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
  Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
  Serial.println(F("Concentration Units (environmental)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_env);
  Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
  Serial.println(F("---------------------------------------"));
  Serial.print(F("Particles > 0.3um / 0.1L air:")); Serial.println(data.particles_03um);
  Serial.print(F("Particles > 0.5um / 0.1L air:")); Serial.println(data.particles_05um);
  Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
  Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  Serial.print(F("Particles > 5.0um / 0.1L air:")); Serial.println(data.particles_50um);
  Serial.print(F("Particles > 10 um / 0.1L air:")); Serial.println(data.particles_100um);
  Serial.println(F("---------------------------------------"));
  if((data.pm25_standard >= 1000) && !tell) {
    digitalWrite(8, HIGH);
    tell = 1;
    integralTimer = millis();
  }
  if(tell){
    integralTotal = integralTotal + data.pm25_standard;
  }
  float timeNow = (millis() - integralTimer)/1000;
  if(!tell)timeNow = 0;
  logfile.print(timeNow);
  logfile.print("       ");
  logfile.print(data.pm25_standard, DEC);
  logfile.print("       ");
  logfile.println(integralTotal);
  logfile.flush();
  
  
  delay(1000);
}
