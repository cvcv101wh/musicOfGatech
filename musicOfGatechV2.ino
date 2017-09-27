#include <Tone.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
int minForSensorLux;
int maxForSensorLux;
int minForPhotoResistor;
int maxForPhotoResistor;

Tone tone1;
Tone tone2;
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

//from tsl2561 light sensor Initialization example 
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */  
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");
}


void setup() {

    tone1.begin(8);
    tone1.play(NOTE_A4);
    tone2.begin(9);
    tone2.play(NOTE_C4);
  // initialize serial communications (for debugging only):
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 configureSensor();

// from the SD card example
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(10)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
   //wirte the data to the sd card
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    //to notify where the data starts for easier use of the data when processing later
    dataFile.println("/////////////////////new start!//////////////////////////");
    dataFile.close();
    // print to the serial port too:
    //Serial.println(sensorReading);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  
//read the value of photoresistor
int sensorReading = analogRead(A0);
minForPhotoResistor=sensorReading;
maxForPhotoResistor=sensorReading+1;
//read the value of lux sensor, from the TSL2561 example
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  /* Display the results (light is measured in lux) */
  if (event.light)
  {
   minForSensorLux = event.light;
   maxForSensorLux =  minForSensorLux+1;
  }

  
}

//make the device have a higher tone when lighter
int mapPitch(int sensorReading, int minValue, int maxValue){
int thisPitch ;
    if(sensorReading<(minValue))
thisPitch = 0;
  else if(sensorReading<(minValue+(maxValue-minValue)/7*1))
  thisPitch = NOTE_C4 ;
  else if(sensorReading<(minValue+(maxValue-minValue)/7*2))
  thisPitch = NOTE_D4 ;
 else if(sensorReading<(minValue+(maxValue-minValue)/7*3))
  thisPitch = NOTE_E4;
 else if(sensorReading<(minValue+(maxValue-minValue)/7*4))
 thisPitch =NOTE_F4;
 else if(sensorReading<(minValue+(maxValue-minValue)/7*5))
 thisPitch=NOTE_G4;
 else if(sensorReading<(minValue+(maxValue-minValue)/7*6))
 thisPitch =NOTE_A4;
 else if(sensorReading<maxValue)
thisPitch = NOTE_B4;
else 
thisPitch =NOTE_C5;

return thisPitch;
}

//make the device have a faster tempo when lighter
int mapTempo(int sensorReading, int minValue, int maxValue) 
{
  int delayTime ;
    if(sensorReading<minValue)
delayTime = 960;
  else if(sensorReading<(minValue+(maxValue-minValue)/4))
  delayTime = 480 ;
  else if(sensorReading<(minValue+(maxValue-minValue)/4*2))
  delayTime = 240 ;
 else if(sensorReading<(minValue+(maxValue-minValue)/4*3))
  delayTime = 120;
  else if(sensorReading<maxValue)
  delayTime = 60;
  return delayTime;
}
void loop()
{


//read the value of photoresistor
int sensorReading = analogRead(A0);

//read the value of lux sensor, from the TSL2561 example
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  /* Display the results (light is measured in lux) */
  if (event.light)
  {
    if(event.light<minForSensorLux)
    {
      minForSensorLux=event.light;
    }
    if(event.light>maxForSensorLux)
    {
      maxForSensorLux=event.light;
    }
    if(sensorReading<minForPhotoResistor)
    {
      minForPhotoResistor = sensorReading;
    }
    if(sensorReading>maxForPhotoResistor)
    {
      maxForPhotoResistor = sensorReading;
    }
    Serial.print(event.light); 
    Serial.print(" lux    ");
     Serial.print(sensorReading);
     Serial.println(" resistor    ");
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
  }
// use delay to make tempo

  delay(mapTempo(sensorReading,minForPhotoResistor,maxForPhotoResistor));
  //one continuous track
  if(tone1.isPlaying())
  {
  tone1.play(mapPitch(event.light,minForSensorLux,maxForSensorLux ));
  }
  
  //one discrete track
  if(tone2.isPlaying())
  {
    tone2.stop();
  }
  else
  {
    tone2.play(mapPitch(sensorReading,minForPhotoResistor,maxForPhotoResistor));
  }
}
