//Library
#include "DHT.h"
#include "Adafruit_seesaw.h"
#include<Wire.h>

// Constants
#define DHTPIN 2
#define DHTTYPE DHT11 
#define DELAY_TIME 5000
#define LED 8

// PINS
const int LDR = A0;

// Initiate DHT11 and Adafruit Soil Sensor
DHT dht(DHTPIN, DHTTYPE);
Adafruit_seesaw ss;

// Sensor variables
int chk;
float hum;
float temp;
uint16_t waterlevel;
int LDR_val = 0;

// Fake-it variables for testing purposes
int countDays = 2;
bool startMotor = false;

// Status of environmental conditions
// 0 = "critically low" to 4 = "critically high" with 2 = OK as default
int waterStatus = 2;


// Output flags
bool criticalState = false;


// Create a variable to save the last executed time
unsigned long lastExecutedMillis = 0;


void setup() {

  Serial.begin(115200);
  dht.begin();

  // Start the I2C Bus as Master
  Wire.begin();

  // Set LED as OUTPUT
  pinMode(LED, OUTPUT);

  // Print an error if Soil Sensor is not found
  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw not found");
    while(1) delay(1);
  // Print that the Soil Sensor started properly
  } else {
    Serial.print("seesaw started! version: ");
    Serial.println(ss.getVersion(), HEX);
  }

}

void loop() {

  // Copy the current millis to our variable
  unsigned long currentMillis = millis(); 

  // Read sensor values
  read_light();
  read_temperature();
  read_waterLevel();
  read_humidity();


  // Set LED to HIGH to test if program is still running
  digitalWrite(LED, HIGH);

  // Set flags if plant is in a critical state
  set_critical_flag();

  // Transmit to slave 
  Wire.beginTransmission(9); 

  // Rest of I2C communication is fount in output_loop
  
  // Check to see how much time has passed
  if (currentMillis - lastExecutedMillis >= DELAY_TIME) 
  {
    // Save the last executed time
    lastExecutedMillis = currentMillis; 

    // Print  values to serial monitor
    Serial.print("Light: ");
    Serial.println(LDR_val);
    
    Serial.print("Humidity: ");
    Serial.println(hum);
    
    Serial.print("Temp: ");
    Serial.println(temp);
    
    Serial.print("Water level: ");
    Serial.println(waterlevel);

    // Deliver output based on flags
    output_loop();
    
  }
  
}


///////////////////  SENSOR READINGS  /////////////////// 



// Function for reading light sensor
void read_light()
{
  // Read sensor and save data as LDR_val
  LDR_val = analogRead(LDR);
}

// Function for reading soil sensor
void read_waterLevel()
{   
  // Read soil sensor and save data as capread variable
  waterlevel = ss.touchRead(0);
}

// Function for reading DHT11 temperature
void read_temperature()
{
  // Read temperature levels and save data as temp variable
  temp = dht.readTemperature();
}

// Function for reading DHT11 humidity
void read_humidity()
{
  // Read humidity levels and save data as hum variable
  hum = dht.readHumidity();
}



///////////////////  CRITICAL STATES AND OUTPUT  /////////////////// 



// If the plant is in a critical state (0 or 4) in any environmental attribute 
// we set a flag for determining the output behavior
void set_critical_flag()
{
  // See set_waterlevel function, if waterStatus = 4 the waterlevels are too high 
  // waterStatus = 0 means that the soil is dry
  if ( (waterStatus == 0) || (countDays == 2) )
  {
    criticalState = true;
    startMotor = true; // TEST
  }
  else
  {
    criticalState = false;
    startMotor = false; // TEST
  }
}

// Main function for outputting feedback
void output_loop()
{
  if(criticalState)
  {
    Serial.println("Plant is in a critical stage....");
    Serial.println(startMotor); // PRINT MOTOR VALUES

    // I2C communication
    Wire.write(startMotor); // TEST
    Wire.endTransmission(); // TEST
  }
  else {
    Serial.println("Everything is fine..");
    Serial.println(startMotor); // PRINT MOTOR VALUES
  }
}


///////////////////  SETTING INTERVALS FOR SENSOR READINGS  /////////////////// 

// Function for setting water level status
void set_waterlevel(uint16_t waterlevel)
{
  if (waterlevel > 960)
  {
    waterStatus = 4;
  }
  else if (waterlevel > 850)
  {
    waterStatus = 3;
  }
  else if (waterlevel > 650)
  {
    waterStatus = 2;
  }
  else if (waterlevel > 500)
  {
    waterStatus = 1;
  }
  else
  {
    waterStatus = 0;
  }
}