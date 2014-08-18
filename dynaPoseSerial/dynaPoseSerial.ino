#include <ax12.h>
#include <BioloidController.h>

#define SERVO_COUNT 1
#define POSE_COUNT 5
#define ERROR_BIT_CHECK 0

BioloidController bioloid = BioloidController(1000000);


int poseData[SERVO_COUNT][POSE_COUNT] ;

int servoError = 0 ; // 0 = no errors, continue with the program. 1 = error on at least one servo (this usually means a servo cannot be found)
int isTorqueOn = 0;
int nextSavePose;
int mode = 0; // mode for the serial input. 0 = normal menu mode. 2 = speed change mode
int bioloidDelay = 5000;
int torqueCount = 1; //a counter for when a user turns the torque on

void setup()
{
  delay(100);
  Serial.begin(38400); //start serial communications at 38400bps
  
  // initialize pose data to -1 - this will help to identify 'bad' servo data if a servo gets disconnected
  for(int j = 0; j<POSE_COUNT;j++)
  {
    for(int i = 0; i<SERVO_COUNT;i++)
    {
      poseData[i][j] = -1;
    }
  }




  
  Serial.println("  _______     ___   _          _____               ");
  Serial.println(" |  __ \\ \\   / / \\ | |   /\\   |  __ \\              ");
  Serial.println(" | |  | \\ \\_/ /|  \\| |  /  \\  | |__) |__  ___  ___  ");
  Serial.println(" | |  | |\\   / | . ` | / /\\ \\ |  ___/ _ \\/ __|/ _ \\ ");
  Serial.println(" | |__| | | |  | |\\  |/ ____ \\| |  | (_) \\__ \\  __/ ");
  Serial.println(" |_____/  |_|  |_| \\_/_/    \\_\\_|   \\___/|___/\\___| ");
  
  
  
  Serial.println("Current DYNAPose Settings:");
  Serial.print("  Number of Servos:");
  Serial.println(SERVO_COUNT);
  Serial.print("  Number of Poses:");
  Serial.println(POSE_COUNT);
  Serial.print("  Default Pose-to-Pose Delay:");
  Serial.println(bioloidDelay);
  Serial.print("  Error-Bit Checking:");
  if(ERROR_BIT_CHECK == 1)
  {
    Serial.println("On");
  }
  else
  {
    Serial.println("Off");
  }
  Serial.println("----------");
  
  

  
  delay(100);  //delay to wait for DYNAMIXEL initialization 
  
  if(servoError == 0)
  {
    checkServos();  
  }
  
  if(servoError == 0)
  {
    checkVoltage();
  }
  
  
  if(servoError == 0)
  {  
    relaxServos(); //start servos in relaxed state
    displayMenu();
      
    
  }



  
  
        
}

void loop() 
{
  while(servoError == 0 && Serial.available())
  {
    //For most executions, the program will be in mode '0', and execute the main menu options
    //Any option that needs additional keyboard input (like speed control) will have a mode
    //equivalent to its menu option.
    
  
    //mode == 0 is for the normal menu interface
    if(mode == 0)
    {
            
      int inByte = Serial.read();       
      switch (inByte)
      {
        case '0':    
          relaxServos();
        break;    
    
        case '1':
          torqueServos(); 
        break;
    
        case '2':
          savePose();
        break;
        
        case '3':
          displayPoses();
        break;
        
        
        case '4':
          playPosesOnce(0);
        break;  
        
        
        case '5':
          playPosesRepeat();
        break;  
        
        
        case '6':
          mode = 7;
          Serial.println("Enter a value 500-10000 for the delay between eash pose in ms");
        break;  
        
        
        case '7':
          mode = 8;
          Serial.print("Enter a number between 1 and ");
          Serial.println(POSE_COUNT);
        break;  
        
        
        case '8':
          centerServos();
        break;  
        
        case '9':
          checkServos();
        break;  
        
        
      }   
        
    }
    
    //mode 7 is for speed control - this is just used for testing during playback
    else if (mode == 7)
    {
      int tempBioloidDelay = Serial.parseInt();
      if (tempBioloidDelay > 499 && tempBioloidDelay < 10001)
      {
        bioloidDelay = tempBioloidDelay;
        Serial.print("New Speed:");
        Serial.println(bioloidDelay);
        mode = 0; //return to menu mode
        displayMenu();

      } 
      else
      {
        Serial.println("Plesse enter a delay value between 500 and 10000");

        
      }
      
    }
    
    //mode 8 is for setting the next pose to save to manually
    else if (mode == 8)
    {
      int tempnextSavePose = Serial.parseInt();
      if (tempnextSavePose > 1 && tempnextSavePose < POSE_COUNT+1)
      {
        nextSavePose = tempnextSavePose -1 ; //end user is entering the pose number, subtract one to shift it to 0-indexed
        Serial.print("Next Save Pose: #");
        Serial.println(nextSavePose+1);
        mode = 0; //return to menu mode
        displayMenu();

      } 
      else
      {
          Serial.print("Enter a number between 1 and ");
          Serial.println(POSE_COUNT);
      }
      
    }
    
    
    
  }
  
}


void checkServos()
{
  
  Serial.print("Looking for Servos 1 - ");
  Serial.println(SERVO_COUNT);
  
  //check for each servo in the chain    
  for(int i = 0; i<SERVO_COUNT;i++)
  {
    int id = i+1;  //the ids for the servos start at 1 while the array indexes start at 0. Add '1' to the array index to get the two to match up

    //use ax12GetRegister to check the id on a servo - if the servo is not connected/ communicating this will not return the correct id 
    if(id != ax12GetRegister(id, AX_ID, 1))
    {
      servoError = 1; //the servo did not respond / did not respond correctly, so set the servoError to '1'
      Serial.print("ERROR: Servo #");
      Serial.print(id);
      Serial.println(" not located");
    }  
    //if all servos are found, then 'servoError' remains '0' i.e. no error 
    
    delay(10); //short pause before the next loop / dynamixel call
    
    
    if(ERROR_BIT_CHECK == 1)
    {
      
      int errorBit = ax12GetLastError();
      Serial.print("    Servo # ");
        Serial.print(id);
  
  
      Serial.print("     Error Bit:");
      Serial.println(errorBit);
      
      
      if(ERR_NONE & errorBit == 0)
      {
        Serial.println("          No Errors Found");
  
      }
      
      if(ERR_VOLTAGE & errorBit)
      {
        Serial.println("          Voltage Error");
  
      }
      
      if(ERR_ANGLE_LIMIT & errorBit)
      {
        Serial.println("          Angle Limit Error");
  
      }
      
      if(ERR_OVERHEATING & errorBit)
      {
        Serial.println("          Overheating Error");
  
      }
      
      if(ERR_RANGE & errorBit)
      {
        Serial.println("          Range Error");
  
      }
      
      if(ERR_CHECKSUM & errorBit)
      {
        Serial.println("          Checksum Error");
  
      }
      
      if(ERR_OVERLOAD & errorBit)
      {
        Serial.println("          Overload Error");
  
      }
      
      
      if(ERR_INSTRUCTION & errorBit)
      {
        Serial.println("          Instruction Error");
  
      }
    }
    
  }
  
  //if all servos are found, display a success message
  if(servoError == 0)
  {
    Serial.print("All ");
    Serial.print(SERVO_COUNT);
    Serial.println(" servos located");
    
  }
  
  //if any of the servos are missing, display an error message
  else
  {
    
    Serial.println("Servo(s) are missing and/or not IDed properly");
  }
  
}


void relaxServos()
{
    
  for(int i = 0; i<SERVO_COUNT;i++)
  {
    int id = i+1;
    Relax(id);
    delay(10); //short pause before the next loop / dynamixel call
  }
  
  isTorqueOn = false; 
  Serial.println("----------");
  Serial.println("Servos Relaxed");
  Serial.println("----------");  
}
    

void torqueServos()
{
  isTorqueOn = true;
  
    for(int i = 0; i<SERVO_COUNT;i++)
    {
      int id = i+1;
      TorqueOn(id);

      delay(10); //short pause before the next loop / dynamixel call
    }
 
 
    Serial.println("----------");
    Serial.println("Torque On!");
    Serial.println("Code for current pose:");
    Serial.print("  PROGMEM prog_uint16_t poseTemp");
    Serial.print(torqueCount++);
    Serial.print("[] = {");
    Serial.print(SERVO_COUNT);
    Serial.print(",");
 
    for(int i = 0; i<SERVO_COUNT;i++)
    {
      int id = i+1;
      Serial.print(GetPosition(id));
      if(i < SERVO_COUNT-1)
      {
        Serial.print(",");
      }
      delay(10);
    }
    Serial.println("};");
    Serial.println("----------");
  
}
    
    
    
void savePose()
{
  int errorFlag = 0;
  
  Serial.print("Saving Pose #");
  Serial.println(nextSavePose + 1);
  
  
  Serial.print("  PROGMEM prog_uint16_t pose");
    Serial.print(nextSavePose);
    Serial.print("[] = {");
    Serial.print(SERVO_COUNT);
    Serial.print(",");
    
  for(int i = 0; i<SERVO_COUNT;i++)
  {
    int id = i+1;
    poseData[i][nextSavePose] = (GetPosition(id));
    if(poseData[i][nextSavePose] == -1)
    {
      errorFlag = 1;
      Serial.println("");
      Serial.print("ERROR! Servo # ");
      Serial.print(id);
      Serial.println(" not read! Please check servos and re-save pose");
    }
    

      Serial.print(poseData[i][nextSavePose]);
      if(i < SERVO_COUNT-1)
      {
        Serial.print(",");
      }
      
    delay(10); //short pause before the next loop / dynamixel call
  }
  Serial.println("};");

  //check if there is no error (errorFlag is 0).   
  if (errorFlag ==0)
  {
    nextSavePose = (nextSavePose + 1)%POSE_COUNT; //only increment the pose counter if there was no error
  }

  Serial.print("The Next save #");
  Serial.println(nextSavePose + 1 );
  Serial.println(" ");
  
  if(nextSavePose == 0)
  {
   displayMenu(); 
  }
  
}

    


void displayPoses()
{   
  int errorFlag = 0;
  
  for(int j = 0; j<POSE_COUNT;j++)
  {
    Serial.print("  PROGMEM prog_uint16_t pose");
    Serial.print(j);
    Serial.print("[] = {");
    Serial.print(SERVO_COUNT);
    Serial.print(",");
 


    for(int i = 0; i<SERVO_COUNT;i++)
    {
      
      int id = i+1;
      Serial.print(poseData[i][j]);
      if(i < SERVO_COUNT-1)
      {
        Serial.print(",");
      }
      
      if(poseData[i][j] == -1)      
      {
        errorFlag = 1; 
      }
    
     delay(10); //short pause before the next loop / dynamixel call
      
    }
    
    Serial.println("};");
  }
  
  if(errorFlag = 1)
  {
    Serial.println("At least one position has not been sent"); 
  }
  
}
    
    
int playPosesOnce(int cycle)
{   
  Serial.println("Cycle # 1");
  
  bioloid.poseSize = SERVO_COUNT;//


  for(int j = 0; j<POSE_COUNT;j++)
  {
  
   if(Serial.available())  
   {
     //clear incoming buffer
     while(Serial.available())
     {
       Serial.read();
     }
    Serial.println("Playback Stopped Manually");
    displayMenu();
    return(1); 
   }
    
    Serial.print("    Moving to Pose #");
    Serial.println(j);
    
    bioloid.readPose();//find where the servos are currently
 
 
    int errorFlag = 0;
    
        for(int i = 0; i<SERVO_COUNT;i++)
        {
          int id = i+1;
          //SetPosition(id, positions[i]);
          //Serial.println(poseData[i][j]);
           bioloid.setNextPose(id,poseData[i][j]);
           if(poseData[i][j] == -1)
           {
              errorFlag = 1; 
           }
         }

    if(errorFlag == 0)
    {
        bioloid.interpolateSetup(bioloidDelay); // setup for interpolation from current->next over 1/2 a second
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
  return(0);
}
    
    
void playPosesRepeat()
{
 int repeatPoses = 1;
 int i = 0;
 
 while(1)
 {
   if(playPosesOnce(i++) == 1)
   {
    return; 
   }
  
  


 } 
 
 
}



void checkVoltage(){  
   // wait, then check the voltage (LiPO safety)
  float voltage = (ax12GetRegister (1, AX_PRESENT_VOLTAGE, 1)) / 10.0;

  Serial.println("----------");
  Serial.print ("System Voltage: ");
  Serial.print (voltage);
  Serial.println (" volts.");
  if (voltage < 10.0)
  {

    Serial.println("Voltage levels below 10v, please charge battery.");
    Serial.println("----------");
    servoError == 1;
  }  
  if (voltage > 10.0)
  {

    Serial.println("Voltage levels nominal.");
    Serial.println("----------");
  }
   
  delay(10); //short pause before the next loop / dynamixel call
  

}


void displayMenu()
{
 
    Serial.println("----------");
    Serial.println("-----MENU OPTIONS-----");
    Serial.println("0: Relax Servos");
    Serial.println("1: Enable Torque and Report Servo Position");
    Serial.print("2: Save Current Position to next pose(");
    Serial.print(nextSavePose+1);
    Serial.println(")");
    Serial.println("3: Display All Poses in memory");
    Serial.println("4: Play Sequence Once");
    Serial.println("5: Play Sequence Repeat");
    Serial.println("6: Change Speed");
    Serial.println("7: Set Next Pose Number"); 
    Serial.println("8: Center All Servos"); 
    Serial.println("9: Scan All Servos"); 
    Serial.println("----------");
}



void centerServos()
{   
  
  bioloid.poseSize = SERVO_COUNT;//


    bioloid.readPose();//find where the servos are currently
    
  Serial.println("Beginning Servo Centering");
    
        for(int i = 0; i<SERVO_COUNT;i++)
        {
          int id = i+1;
          int model = ax12GetRegister (id, AX_MODEL_NUMBER_L, 2);
          int centerPosition = -1;
          
          if(model == 12 || model == 18 || model == 300)
          {
            centerPosition = 512;
          }
          else if(model == 360 || model == 29 || model == 310 || model == 320)
          {
            centerPosition = 2048;
          }
         
          else
          {
            Serial.println("Error, model not found");
          }
           Serial.println(centerPosition);

          //SetPosition(id, positions[i]);
          //Serial.println(poseData[i][j]);
           bioloid.setNextPose(id,centerPosition);
           
         }

  
        bioloid.interpolateSetup(bioloidDelay); // setup for interpolation from current->next over 1/2 a second
         while(bioloid.interpolating > 0)
         {  // do this while we have not reached our new pose
           bioloid.interpolateStep();     // move servos, if necessary. 
           delay(3);
         }
         
         
  Serial.println("Servos are Centered");
}
