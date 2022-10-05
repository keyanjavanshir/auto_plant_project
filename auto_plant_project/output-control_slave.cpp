// Include the required Wire library for I2C
#include<Wire.h>

// Constants
#define DELAY_TIME 1000 // Variable to read water flow data once per second


// From output_loop in Master code, runs when there is a critical flag
bool startMotor = false;


// PINS
const int pumpPin = 4;  // Water pump
int flowInterrupt = 0;  // Interrupt 0
int flowPin       = 2;  // Digital Pin 2


/*The hall-effect flow sensor outputs pulses per second per litre/minute of flow.*/
float calibrationFactor = 90; 


// Variables for flow sensor
unsigned int waterLimit = 300; // This variable controls how much water is allowed to flow through the sensor before stopping the water pump
volatile byte pulseCount = 0;  
float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;


// Create a variable to save the last executed time
unsigned long lastExecutedMillis = 0;
unsigned long oldTime = 0;


void setup() {

  Serial.begin(115200);

  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  
  // Attach a function to trigger when data is received
  Wire.onReceive(receiveEvent);

  // Set up motor and sensor pins
  pinMode(pumpPin , OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(flowPin, INPUT);
  digitalWrite(flowPin, HIGH);

  // Interrupt when flowPin goes from HIGH to LOW
  attachInterrupt(flowInterrupt, pulseCounter, FALLING);
}

void receiveEvent(int bytes) {

  startMotor = Wire.read();    // read one character from the I2C

}

void loop() {

  // Copy the current millis to our variable
  unsigned long currentMillis = millis(); 

  //If value received from set_critical_flag from Master = true, then start water pump
  if (startMotor == true) {
    digitalWrite(pumpPin, HIGH);
  }

  // Check to see how much time has passed
  if(currentMillis - lastExecutedMillis >= DELAY_TIME)    
  { 

    // Disable the interrupt while calculating flow rate and sending the value to the host
    detachInterrupt(flowInterrupt);

    // Because this loop may not complete in exactly 1 second intervals we calculate the number of milliseconds that have passed since the last execution and use that to scale the output. We also apply the calibrationFactor to scale the output based on the number of pulses per second per units of measure (litres/minute in this case) coming from the sensor
    flowRate = ((1000.0 / (currentMillis - lastExecutedMillis)) * pulseCount) / calibrationFactor;

    // Note the time this processing pass was executed. Because we've disabled interrupts the millis() function won't 
    // be incrementing right at this point, but it will still return the value it was set to just before interrupts went away
    lastExecutedMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(flowMilliLitres, DEC);  // Print the integer part of the variable
    Serial.print("mL/Second");
    Serial.print("\t");           

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres,DEC);
    Serial.println("mL"); 
    Serial.print("\t");     
    
    // If over totalMilliLitres is over 300mL (waterLimit), then stop the water pump and set critical-flag variable to false
    if (totalMilliLitres > waterLimit)
    {
      StopWaterPump(); // Stop water pump
      startMotor = false; // Set critical-flag value to false
    }
  }
}


// Interrupt Service Routine

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void StopWaterPump()
{
  digitalWrite(pumpPin, LOW);
}