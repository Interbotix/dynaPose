#include <ax12.h>
#include <BioloidController.h>

#include <EEPROM.h> //Needed to access the eeprom read write functions

//#include "poses.h"



#define SERVOCOUNT 8
#define LED_PIN 0
#define TORQUE_BUTTON_PIN  1
#define ENTER_BUTTON_PIN  2
#define MODE_BUTTON_PIN  3
#define PREV_BUTTON_PIN  4
#define NEXT_BUTTON_PIN  5


#define NUMBER_OF_MODES 4;

//#define MAX_POSES 5

#define lcd lcd

#define USE_LCD
//#define

#ifdef USE_LCD
  //include the I2C Wire library - needed for communication with the I2C chip attached to the LCD manual 
  #include <Wire.h> 
  // include the RobotGeekLCD library
  #include <RobotGeekLCD.h>
  
  // create a robotgeekLCD object named 'lcd'
  RobotGeekLCD lcd;
#else 
  //serial

#endif



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
  
int MAX_POSES  = 5;
  
//int poses[MAX_POSES][SERVOCOUNT+1]; //10 poses
void setup()
{
  
  
  #ifdef USE_LCD
    // initlaize the lcd object - this sets up all the variables and IIC setup for the LCD object to work
    lcd.init();
    lcd.print("DynaPose");
    delay(1000);
    lcd.clear();
  #else 
    Serial.begin(9600);

  #endif
  
  
  
  pinMode(TORQUE_BUTTON_PIN, INPUT);
  pinMode(ENTER_BUTTON_PIN, INPUT);
  pinMode(MODE_BUTTON_PIN, INPUT);
  pinMode(PREV_BUTTON_PIN, INPUT);
  pinMode(NEXT_BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  positions[0] = SERVOCOUNT;
  
  delay(100);  
  lcd.setCursor(0, 0);
  lcd.print("M:Set");
  
  lcd.setCursor(0, 1);
  RelaxServos();
  lcd.print("T:Off ");

  lcd.setCursor(8, 1);
  lcd.print("Pose ");
  lcd.print(currentPose);
        
}

void loop() 
{
  
    
  digitalWrite(LED_PIN, HIGH);
  //torque
  if(debounceDigitalRead(0) == true)
  {
        isTorqeOn = !isTorqeOn;
      
        // set the cursor to column 0, line 1
        // (note: line 1 is the second row, since counting begins with 0):
        lcd.setCursor(0, 1);
        if(isTorqeOn == 0)
        {
          RelaxServos();
        lcd.print("T:Off ");
        }
        
        else
        {
          TorqueServos();
        lcd.print("T:On ");
        }
  }

  //enter
  if(debounceDigitalRead(1) == true)
  {
    

    
    
    
        
    switch(mode)
    {
     case 0:
         //poses[currentPose][0] = SERVOCOUNT;
         EEPROM.write(0, MAX_POSES);
         
        for(int i = 0; i<SERVOCOUNT;i++)
        {
          int id = i+1;
          EEPROMWriteInt(currentPose*(20)+1+ 2*i, GetPosition(id));
          
          //poses[currentPose][id] = GetPosition(id);
        }
        lcd.setCursor(8, 0);
        lcd.print("Saved");
        lcd.print(currentPose);
        
        currentPose = (currentPose +1)%MAX_POSES;
        lcd.setCursor(8, 1);
        lcd.print("Pose ");
        lcd.print(currentPose);
        
        

       break;
     case 1:
        bioloid.poseSize = SERVOCOUNT;//2 servos, so the pose size will be 2
        bioloid.readPose();//find where the servos are currently
 
        for(int i = 0; i<SERVOCOUNT;i++)
        {
          int id = i+1;
         // SetPosition(id, positions[i]);
         bioloid.setNextPose(id,EEPROMReadInt(currentPose*(20)+1+ 2*i));
         }

        bioloid.interpolateSetup(1000); // setup for interpolation from current->next over 1/2 a second
         while(bioloid.interpolating > 0)
         {  // do this while we have not reached our new pose
           bioloid.interpolateStep();     // move servos, if necessary. 
           delay(3);
         }
       

       break;
     case 2:
         currentPose = 0;
         while(1) //loop forever
         {
          
          
        lcd.setCursor(8, 1);
        lcd.print("Pose ");
        lcd.print(currentPose);
        
          
          
          
            bioloid.poseSize = SERVOCOUNT;//2 servos, so the pose size will be 2
            bioloid.readPose();//find where the servos are currently
     
            for(int i = 0; i<SERVOCOUNT;i++)
            {
              int id = i+1;
             // SetPosition(id, positions[i]);
            // bioloid.setNextPose(id,poses[currentPose][id]);
            
         bioloid.setNextPose(id,EEPROMReadInt(currentPose*(20)+1+ 2*i));
             }
    
            bioloid.interpolateSetup(1000); // setup for interpolation from current->next over 1/2 a second
             while(bioloid.interpolating > 0)
             {  // do this while we have not reached our new pose
               bioloid.interpolateStep();     // move servos, if necessary. 
               delay(3);
             }
             currentPose = (currentPose + 1)% MAX_POSES;
            
            
            lcd.setCursor(0, 1);
            lcd.print("delay ");
            lcd.print(analogRead(0) * 1);
            
            delay(analogRead(0) * 1);
          
         }
     


       break;
     case 3:
 
 
       break;
      
    }
    
    
    
    

  }


//mode
  if(debounceDigitalRead(2) == true)
  {
    mode = (mode + 1) % NUMBER_OF_MODES;
    
    switch(mode)
    {
     case 0:
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("M:Save");
       break;
     case 1:
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("M:Set");
       break;
     case 2:
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("M:Play");
       break;
     case 3:
      
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("M:Output");
       break;
      
    }
    
        currentPose = 0;
        lcd.setCursor(9, 1);
        lcd.print("Pose:");
        lcd.print(currentPose);
       
  }
//prev
  if(debounceDigitalRead(3) == true)
  {
        
    if(mode ==5)
    {
      MAX_POSES = MAX_POSES - 1;
      if(currentPose < 1)
      {
        MAX_POSES =1;
      }
      
    }
    else
    {
      currentPose = (currentPose -1);
      if(currentPose < 0)
      {
        currentPose = MAX_POSES-1;
      }
      
      // set the cursor to column 0, line 1
      // (note: line 1 is the second row, since counting begins with 0):
      lcd.setCursor(9, 1);
      lcd.print("Pose:");
      lcd.print(currentPose);
    }
   
  }
//next
  if(debounceDigitalRead(4) == true)
  {    
    if(mode ==5)
    {
      MAX_POSES = MAX_POSES + 1;
      
    }
    else
    {
        currentPose = (currentPose +1)    % MAX_POSES;
      // set the cursor to column 0, line 1
      // (note: line 1 is the second row, since counting begins with 0):
      lcd.setCursor(9, 1);
        lcd.print("Pose:");
        lcd.print(currentPose);
    }
   
    
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


