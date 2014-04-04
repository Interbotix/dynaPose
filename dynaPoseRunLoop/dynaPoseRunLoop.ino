#include <ax12.h>                //AX-12 libraries allow the ArbotiX to communicate with DYNAMIXEL servos, AX and MX
#include <BioloidController.h>   //The Bioloid controller library is used for servo interpolation
#include <avr/pgmspace.h>        //PROGMEM library allows the ArboitX to store poses in Flash memory, so that SRAM is not used

#define SERVO_COUNT 6            //the number of servos that will be attached - change this for your specific robot

BioloidController bioloid = BioloidController(1000000);  //initialize the bioloid controller to 1000000 baud

//PROGMEM prog_uint16_t poseTemp1[] = {};  // place-holder for your specific pose. Replace this line with one received from the ArbotiX running DYNAposeSerial 

//EXAMPLES
//PhantomX RobotTurret Example
//PROGMEM prog_uint16_t poseTemp1[] = {2,512,512};

//WidowX/ScorpionX RobotTurret Example
//PROGMEM prog_uint16_t poseTemp1[] = {2,2048,2048};

//PhantomX Pincher Arm Example

//PhantomX Reactor  Arm Example

//WidowX Arm Example
PROGMEM prog_uint16_t poseTemp1[] = {6,2130,1302,1413,1815,513,510};


//PhantomX Quadruped Example

//PhantomX Hexapod Example



void setup()
{
  delay(100);  //delay to wait for DYNAMIXEL initialization    
  
  bioloid.poseSize = SERVO_COUNT;  // define the poseSize for the bioloid controller
  bioloid.loadPose(poseTemp1);     // load the pose from FLASH, into the nextPose buffer
  bioloid.readPose();              // read in current servo positions to the curPose buffer 
  bioloid.interpolateSetup(1000); // setup for interpolation from current->next over 1 second
  
  while(bioloid.interpolating > 0)
  {  // do this while we have not reached our new pose
     bioloid.interpolateStep();     // move servos, if necessary. 
     delay(3);
  }
     
}


void loop() 
{ 
   //do nothing
}



    
    


