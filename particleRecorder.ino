// Arduino Due code for Honeywell HPMA115S0-XXX particle sensor
// https://electronza.com/arduino-measuring-pm25-pm10-honeywell-hpma115s0/

bool my_status;
#include <SD.h>
// IMPORTANT!!! We are working on an Arduino DUE, 
// so int is 32 bit (-2,147,483,648 to 2,147,483,647)
// For Arduino Uno int size is 8 bit, that is -32,768 to 32,767
// Use long or float if working with an Uno or simmilar 8-bit board
long PM25;
long PM10;
float timer = 0.0;
#define cardSelect 4
File logfile;
char filename[15];
void setup() {
  Serial.begin(9600);
   delay(100);
  Serial1.begin(9600);
  delay(100);
  while (!Serial);
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
  // Stop autosend
my_status = stop_autosend(); 
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
  Serial.print("Stop autosend status is ");
  Serial.println(my_status, BIN);
  Serial.println(" ");
  delay(500);
  
  // Start fan
my_status = start_measurement(); 
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
  Serial.print("Start measurement status is ");
  Serial.println(my_status, BIN);
  Serial.println(" ");
  delay(5000);
  timer = millis();
}


void loop() {
    logfile = SD.open(filename, FILE_WRITE);
  // Read the particle data every minute
  my_status = read_measurement(); 
  delay(100);
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
  Serial.print("Read measurement status is ");
  Serial.println(my_status, BIN);
  Serial.print("PM2.5 value is ");
  Serial.println(PM25, DEC);
  Serial.print("PM10 value is ");
  Serial.println(PM10, DEC);
  Serial.println(" ");
  // Wait one minute
  float timeNow = (millis() - timer)/1000;
  logfile.print(timeNow);
  logfile.print("       ");
  logfile.println(PM25, DEC);
  logfile.flush();
  logfile.close();
  

  delay(1000);
}

bool start_measurement(void)
{
  // First, we send the command
  byte start_measurement[] = {0x68, 0x01, 0x01, 0x96 };
  Serial1.write(start_measurement, sizeof(start_measurement));
  //Then we wait for the response
  while(Serial1.available() < 2);
  char read1 = Serial1.read();
  char read2 = Serial1.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}

bool stop_measurement(void)
{
  // First, we send the command
  byte stop_measurement[] = {0x68, 0x01, 0x02, 0x95 };
  Serial1.write(stop_measurement, sizeof(stop_measurement));
  //Then we wait for the response
  while(Serial1.available() < 2);
  char read1 = Serial1.read();
  char read2 = Serial1.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}

bool read_measurement (void)
{
  // Send the command 0x68 0x01 0x04 0x93
  byte read_particle[] = {0x68, 0x01, 0x04, 0x93 };
  Serial1.write(read_particle, sizeof(read_particle));
  // A measurement can return 0X9696 for NACK
  // Or can return eight bytes if successful
  // We wait for the first two bytes
  while(Serial1.available() < 1);
  byte HEAD = Serial1.read();
  while(Serial1.available() < 1);
  byte LEN = Serial1.read();
  // Test the response
  if ((HEAD == 0x96) && (LEN == 0x96)){
    // NACK
    Serial.println("NACK");
    return 0;
  }
  else if ((HEAD == 0x40) && (LEN == 0x05))
  {
    // The measuremet is valid, read the rest of the data 
    // wait for the next byte
    while(Serial1.available() < 1);
    byte COMD = Serial1.read();
    while(Serial1.available() < 1);
    byte DF1 = Serial1.read(); 
    while(Serial1.available() < 1);
    byte DF2 = Serial1.read();     
    while(Serial1.available() < 1);
    byte DF3 = Serial1.read();   
    while(Serial1.available() < 1);
    byte DF4 = Serial1.read();     
    while(Serial1.available() < 1);
    byte CS = Serial1.read();      
    // Now we shall verify the checksum
    if (((0x10000 - HEAD - LEN - COMD - DF1 - DF2 - DF3 - DF4) % 0XFF) != CS){
      Serial.println("Checksum fail");
      return 0;
    }
    else
    {
      // Checksum OK, we compute PM2.5 and PM10 values
      PM25 = DF1 * 256 + DF2;
      PM10 = DF3 * 256 + DF4;
      return 1;
    }
  }
}

bool stop_autosend(void)
{
 // Stop auto send
  byte stop_autosend[] = {0x68, 0x01, 0x20, 0x77 };
  Serial1.write(stop_autosend, sizeof(stop_autosend));
  //Then we wait for the response
  while(Serial1.available() < 2);
  char read1 = Serial1.read();
  char read2 = Serial1.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}

bool start_autosend(void)
{
 // Start auto send
  byte start_autosend[] = {0x68, 0x01, 0x40, 0x57 };
  Serial1.write(start_autosend, sizeof(start_autosend));
  //Then we wait for the response
  while(Serial1.available() < 2);
  char read1 = Serial1.read();
  char read2 = Serial1.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}
