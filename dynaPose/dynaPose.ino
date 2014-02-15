#include <ax12.h>
#include <BioloidController.h>
//#include "poses.h"

#define SERVOCOUNT 3
#define TORQUE_BUTTON_PIN  1

BioloidController bioloid = BioloidController(1000000);

int positions[SERVOCOUNT+1] ;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
int isTorqeOn = 0;

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup()
{
  pinMode(TORQUE_BUTTON_PIN, INPUT);
   positions[0] = SERVOCOUNT;
   Serial.begin(9600);
   delay(1000);
      Serial.print(" ");
  
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      int id = i+1;
      Relax(id);
      delay(10);
      positions[id] = GetPosition(id);
      positions[id] =  ax12GetRegister(id, 36, 2);
      Serial.print(id);
      Serial.print("ss ");
      Serial.print(id);
      Serial.print(" ");
      Serial.println(positions[id]);
   
    }
    
  //  delay(5000);
   
    bioloid.poseSize = SERVOCOUNT;//2 servos, so the pose size will be 2
  bioloid.readPose();//find where the servos are currently
 
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      int id = i+1;
    
      //TorqueOn(id);
      
     // SetPosition(id, positions[i]);
     bioloid.setNextPose(id,positions[id]);
     
      
      Serial.print(id);
      Serial.print("f");
    }
      
      
    //bioloid.loadPose(positions);   // load the pose from FLASH, into the nextPose buffer
   // bioloid.readPose();            // read in current servo positions to the curPose buffer

    bioloid.interpolateSetup(1000); // setup for interpolation from current->next over 1/2 a second
    while(bioloid.interpolating > 0){  // do this while we have not reached our new pose
        bioloid.interpolateStep();     // move servos, if necessary. 
        delay(3);
      
      
      
    }
    delay(5000);
}

void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(TORQUE_BUTTON_PIN);

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        isTorqeOn = !isTorqeOn;
      Serial.print("t swap ");
        if(isTorqeOn == 0)
        {
          RelaxServos();
        }
        
        else
        {
          TorqueServos();
          
        }


      }
    }
  }
    // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;






}


void RelaxServos()
{
  
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      int id = i+1;
      Relax(id);

      delay(50);
    }
  
  
}
    

void TorqueServos()
{
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      int id = i+1;
      TorqueOn(id);

      delay(50);
    }
  
}
    

