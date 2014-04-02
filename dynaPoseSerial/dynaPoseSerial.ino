#include <ax12.h>
#include <BioloidController.h>

//#include "poses.h"

#define SERVOCOUNT 8
#define LED_PIN 0
#define MAX_POSES 5

BioloidController bioloid = BioloidController(1000000);


int poseData[SERVOCOUNT][MAX_POSES] ;

int servoError = 0 ; // 0 = no errors, continue with the program. 1 = error on at least one servo (this usually means a servo cannot be found)
int isTorqueOn = 0;
int currentPose;
void setup()
{
  Serial.begin(9600); //start serial communications at 9600bps

  // initialize pose data to -1
  for(int j = 0; j<MAX_POSES;j++)
  {
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      poseData[i][j] = -1;
    }
  }

  
  delay(100);  
  
  
  checkServos();  
  
  relaxServos(); //start servos in relaxed state



  
  Serial.println("DYNAPose");
  Serial.println("-----OPTIONS-----");
  Serial.println("1: Relax Servos");
  Serial.println("2: Enable Torque and Report Servo Position");
  Serial.println("3: Save Current position");
  Serial.println("4: Display Sequence");
  Serial.println("5: Play Sequence Once");
  Serial.println("Coming Soon");
  Serial.println("6: Change Speed");
  Serial.println("7: Set Next Pose");
    
  
        
}

void loop() 
{
  
  int inByte = Serial.read();

  switch (inByte)
  {
    case '1':    
      relaxServos();
    break;    

    case '2':
      torqueServos(); 
    break;

    case '3':
      savePose();
    break;
    
    case '4':
      displayPoses();
    break;
    
    
    case '5':
      playPoses();
    break;  
  }   
}


void checkServos()
{
    
  Serial.print("Looking for Servos 1 - ");
  Serial.println(SERVOCOUNT);
  
  Serial.println("Servos Relaxed");
    
  for(int i = 0; i<SERVOCOUNT;i++)
  {
    int id = i+1;  //the ids for the servos start at 1 while the array indexes start at 0. Add '1' to the array index to get the two to match up
    //use ax12GetRegister to check the id on a servo - if the servo is not connected/ communicating this will not return the correct id 
    if(id != ax12GetRegister(id, 3, 1));
    {
      servoError = 1; //the servo did not respond / did not respond correctly, so set the servoError to '1'
      Serial.print("Servo #");
      Serial.print(id);
      Serial.println(" not located");
    }  
    delay(1); //short pause before the next loop / dynamixel call
  }
  
  if(servoError == 0)
  {
    Serial.print("All ");
    Serial.print(SERVOCOUNT);
    Serial.println(" servos located");
    
  }
  
  else
  {
    
    Serial.println("Servo(s) are missing and/or not IDed properly");
  }
  
}


void relaxServos()
{
  isTorqueOn = false;
  Serial.print("torque is:");
  Serial.println(isTorqueOn);
  
  Serial.println("Servos Relaxed");
    
  for(int i = 0; i<SERVOCOUNT;i++)
  {
    int id = i+1;
    Relax(id);
    delay(10);
  }
  
}
    

void torqueServos()
{
  isTorqueOn = true;
  Serial.print("torque is:");
  Serial.println(isTorqueOn);
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      int id = i+1;
      TorqueOn(id);

      delay(10);
    }
 
 
    Serial.println("Torque On!");
 Serial.print("PROGMEM prog_uint16_t Center[] = {");
 
    for(int i = 0; i<SERVOCOUNT;i++)
  {
      int id = i+1;
    Serial.print(GetPosition(id));
    if(i < SERVOCOUNT-1)
    {
      Serial.print(",");
    }
  }
    Serial.println("};");
  
}
    
    
    
void savePose()
{
  Serial.print("Saving Pose #");
  Serial.println(currentPose);
  
  for(int i = 0; i<SERVOCOUNT;i++)
  {
    int id = i+1;
    poseData[i][currentPose] = (GetPosition(id));
  }
  
  currentPose = (currentPose + 1)%MAX_POSES;
  

  Serial.print("The Next save #");
  Serial.println(currentPose);
  
}



void displayPoses()
{   
    
  
  for(int j = 0; j<MAX_POSES;j++)
  {
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      Serial.print(poseData[i][j]);
      Serial.print(" ");
    }
    Serial.println("");
  }
  
}
    
    
void playPoses()
{   
  
  bioloid.poseSize = SERVOCOUNT;//


   for(int j = 0; j<MAX_POSES;j++)
  {  
    Serial.print("Pose #");
    Serial.println(j);
    
    bioloid.readPose();//find where the servos are currently
 
 
    int errorFlag = 0;
    
        for(int i = 0; i<SERVOCOUNT;i++)
        {
          int id = i+1;
         // SetPosition(id, positions[i]);
          Serial.println(poseData[i][j]);
           bioloid.setNextPose(id,poseData[i][j]);
           if(poseData[i][j] == -1)
           {
              errorFlag = 1; 
           }
         }

    if(errorFlag == 0)
    {
        bioloid.interpolateSetup(2000); // setup for interpolation from current->next over 1/2 a second
         while(bioloid.interpolating > 0)
         {  // do this while we have not reached our new pose
           bioloid.interpolateStep();     // move servos, if necessary. 
           delay(3);
         }
      
    }
    
    else
    {
      Serial.println("Error, cannot complete pose");
    }
  
                
    
       
  }
  
  
  
  for(int j = 0; j<MAX_POSES;j++)
  {
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      Serial.print(poseData[i][j]);
      Serial.print(" ");
    }
    Serial.println("");
  }
  
}
    
    



