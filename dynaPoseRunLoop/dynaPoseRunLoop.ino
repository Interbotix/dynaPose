#include <ax12.h>                //AX-12 libraries allow the ArbotiX to communicate with DYNAMIXEL servos, AX and MX
#include <BioloidController.h>   //The Bioloid controller library is used for servo interpolation
#include <avr/pgmspace.h>        //PROGMEM library allows the ArboitX to store poses in Flash memory, so that SRAM is not used

#define SERVO_COUNT 6            //the number of servos that will be attached - change this for your specific robot
#define POSE_COUNT 5

#define DEFAULT_BIOLID_DELAY 5000  //the time in ms it will take to move from one pose to the next
#define DEFAULT_POSE_DELAY 1000    //the time to hold a pose before moving to the next pose

BioloidController bioloid = BioloidController(1000000);  //initialize the bioloid controller to 1000000 baud

void setup()
{
  delay(100);  //delay to wait for DYNAMIXEL initialization         
}


void loop() 
{ 
  
   playPoseOnce(pose1, DEFAULT_BIOLID_DELAY );
   delay(DEFAULT_POSE_DELAY)
   
}

    
int playPoseOnce( const unsigned int * pose, int bioloidDelay)
{   
  bioloid.poseSize = SERVO_COUNT;  // define the poseSize for the bioloid controller

  for(int j = 0; j<POSE_COUNT;j++)
  {  

    bioloid.loadPose(poseTemp1);     // load the pose from FLASH, into the nextPose buffer
    bioloid.readPose();              // read in current servo positions to the curPose buffer 
    bioloid.interpolateSetup(bioloidDelay); // setup for interpolation from current->next over 1 second
      
    while(bioloid.interpolating > 0)
    {  // do this while we have not reached our new pose
      bioloid.interpolateStep();     // move servos, if necessary. 
      delay(3);
    }     
  }  
}
  





