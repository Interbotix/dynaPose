#include <ax12.h>
#include <BioloidController.h>
//#include "poses.h"

#define SERVOCOUNT 8
#define LED_PIN 0
#define TORQUE_BUTTON_PIN  1
#define ENTER_BUTTON_PIN  2
#define MODE_BUTTON_PIN  3
#define PREV_BUTTON_PIN  4
#define NEXT_BUTTON_PIN  5

#define MAX_POSES 5

BioloidController bioloid = BioloidController(1000000);

int positions[SERVOCOUNT+1] ;

int buttonState[5];             // the current reading from the input pin
int lastButtonState[5] = {LOW,LOW,LOW,LOW,LOW};   // the previous reading from the input pin
int isTorqeOn = 1;
int currentPose = 0;

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime[5] = {0,0,0,0,0};  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

int reading[5];
  
 int poses[MAX_POSES][SERVOCOUNT+1]; //10 poses
void setup()
{
  pinMode(TORQUE_BUTTON_PIN, INPUT);
  pinMode(ENTER_BUTTON_PIN, INPUT);
  pinMode(MODE_BUTTON_PIN, INPUT);
  pinMode(PREV_BUTTON_PIN, INPUT);
  pinMode(NEXT_BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
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
    //delay(5000);
    
    
}

void loop() 
{
  digitalWrite(LED_PIN, HIGH);
  
  if(debounceDigitalRead(0) == true)
  {
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

  if(debounceDigitalRead(1) == true)
  {
    //poses[currentPose] = positions;    
    //memcpy(poses[currentPose],positions,SERVOCOUNT+1);
    poses[currentPose][0] = SERVOCOUNT;
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      int id = i+1;
      //Relax(id);
      //delay(10);
      poses[currentPose][id] = GetPosition(id);
     // positions[id] =  ax12GetRegister(id, 36, 2);
        
    }
    
    
    
    Serial.println( poses[0][0]);
    Serial.println( poses[0][1]);
    Serial.println( poses[0][2]);
    Serial.println( poses[0][3]);
    Serial.println("  ");
    
    

  }

  if(debounceDigitalRead(2) == true)
  {
    
//        poses[0][1] = 512;
//        poses[0][2] = 512;
//        poses[0][3] = 512;
    
    
    bioloid.poseSize = SERVOCOUNT;//2 servos, so the pose size will be 2
    bioloid.readPose();//find where the servos are currently
 
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      int id = i+1;
    
      //TorqueOn(id);
      
     // SetPosition(id, positions[i]);
     bioloid.setNextPose(id,poses[currentPose][id]);
   
   
    
      
    }
    
    
    
        bioloid.interpolateSetup(1000); // setup for interpolation from current->next over 1/2 a second
    while(bioloid.interpolating > 0){  // do this while we have not reached our new pose
        bioloid.interpolateStep();     // move servos, if necessary. 
        delay(3);
       }
       
       
  }
//prev
  if(debounceDigitalRead(3) == true)
  {
        

    currentPose = (currentPose -1)    % MAX_POSES;
    Serial.print("Pose:");
    Serial.println(currentPose);
   
  }
//next
  if(debounceDigitalRead(4) == true)
  {
    currentPose = (currentPose +1)    % MAX_POSES;
    Serial.print("Pose:");
    Serial.println(currentPose);
   
    
  }






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
    


boolean debounceDigitalRead(int button)
{
  
  

  // read the state of the switch into a local variable:'
   reading[button] = digitalRead(button+1);
   

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading[button] != lastButtonState[button]) 
  {
    // reset the debouncing timer
    lastDebounceTime[button] = millis();
  } 
  
  if ((millis() - lastDebounceTime[button]) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading[button] != buttonState[button]) {
      buttonState[button] = reading[button];

      // only toggle the LED if the new button state is HIGH
      if (buttonState[button] == HIGH) 
      {
         lastButtonState[button] = reading[button];
         return(true);
 
      }
    }
  }
    // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState[button] = reading[button];
  
  
  return(false);
}


