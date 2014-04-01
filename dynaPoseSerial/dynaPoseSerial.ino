#include <ax12.h>
#include <BioloidController.h>

#include <EEPROM.h> //Needed to access the eeprom read write functions

//#include "poses.h"



#define SERVOCOUNT 5
#define LED_PIN 0
#define TORQUE_BUTTON_PIN  1
#define ENTER_BUTTON_PIN  2
#define MODE_BUTTON_PIN  3
#define PREV_BUTTON_PIN  4
#define NEXT_BUTTON_PIN  5


#define NUMBER_OF_MODES 4;

#define MAX_POSES 5
//#define MAX_POSES 5






BioloidController bioloid = BioloidController(1000000);

int positions[SERVOCOUNT+1] ;

int buttonState[5];             // the current reading from the input pin
int lastButtonState[5] = {LOW,LOW,LOW,LOW,LOW};   // the previous reading from the input pin
int isTorqeOn = 1;
int currentPose = 0;

int mode = 0; //mode 0= save 1=set 2 = play 3 = position export

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime[5] = {0,0,0,0,0};  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

int reading[5];
  
//int MAX_POSES  = 5;

//int i = 0; //counter for renaming 
  
//int poses[MAX_POSES][SERVOCOUNT+1]; //10 poses


int poseData[SERVOCOUNT][MAX_POSES] ;



//int currentPose = 0;

void setup()
{

  for(int j = 0; j<MAX_POSES;j++)
  {
    for(int i = 0; i<SERVOCOUNT;i++)
    {
      poseData[i][j] = -1;
    }
  }
  


    Serial.begin(9600);

  
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
    
  
    Serial.println("Enter '1' to relax servos. Enter '2' to turn on torque and report position.");
  
  
  positions[0] = SERVOCOUNT;
  
  delay(100);  
  
        
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


void relaxServos()
{
  
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
    
    




//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void EEPROMWriteInt(int p_address, int p_value)
      {
      byte lowByte = ((p_value >> 0) & 0xFF);
      byte highByte = ((p_value >> 8) & 0xFF);

      EEPROM.write(p_address, lowByte);
      EEPROM.write(p_address + 1, highByte);
      }

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address)
      {
      byte lowByte = EEPROM.read(p_address);
      byte highByte = EEPROM.read(p_address + 1);

      return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
      }


