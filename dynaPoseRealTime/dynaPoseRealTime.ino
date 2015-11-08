//Experimental self-contained DYNApose firmware - time based

#include <ax12.h>
#include <BioloidController.h>

#define BUZZER_PIN 2
#define RECORD_BUTTON_PIN 3
#define PLAY_BUTTON_PIN 4
#define ANALOG_PIN 0

#define SERVOCOUNT 5

#define READ_INTERVAL 75 // time between reads

#define PLAYBACK_INTERVAL 150 // time between reads

#define MAX_READ_TRIES 3
#define MAX_POSES 320

unsigned long previousMillis; //last time that the micro started reading servo data
unsigned long currentMillis;

unsigned long previousMillisGripper; //last time that the micro started reading servo data
unsigned long currentMillisGripper;

int poseData [SERVOCOUNT][MAX_POSES];
int currentReadPosition;
int readTries;
int pose;

int recordFlag = 0;

int playFlag = 0;

BioloidController bioloid = BioloidController(1000000);

int playIndexLimit;
int playIndex;


int recordButtonState;             // the current reading from the input pin
int lastRecordButtonState = LOW;   // the previous reading from the input pin


int playButtonState;             // the current reading from the input pin
int lastPlayButtonState = LOW;   // the previous reading from the input pin


long lastRecordDebounceTime = 0;  // the last time the output pin was toggled
long lastPlayDebounceTime = 0;  // the last time the output pin was toggled

long debounceDelay = 50;    // the debounce time; increase if the output flickers


int melodyRecord[] = {  666, 1444, 2000}; //holds notes to be played on tone
int melodyPlay[] = {  777, 666, 1000}; //holds notes to be played on tone
int noteDurations[] = {4, 8, 8 }; //holds duration of notes: 4 = quarter note, 8 = eighth note, etc.



void setup()
{
  pinMode(RECORD_BUTTON_PIN, INPUT);
  pinMode(PLAY_BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.begin(38400);
  Serial.println("Start");

  bioloid.poseSize = SERVOCOUNT;
  RelaxServos();
}


void loop()
{
  
  int readingPlay = digitalRead(PLAY_BUTTON_PIN);
  int readingRecord = digitalRead(RECORD_BUTTON_PIN);
  
    // If the switch changed, due to noise or pressing:
  if (readingPlay != lastPlayButtonState) {
    // reset the debouncing timer
    lastPlayDebounceTime = millis();
  } 
      // If the switch changed, due to noise or pressing:
  if (readingRecord != lastRecordButtonState) {
    // reset the debouncing timer
    lastRecordDebounceTime = millis();
  } 
  
  
  if ((millis() - lastPlayDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if (readingPlay !=playButtonState) 
    {
      playButtonState = readingPlay;
      // only toggle the LED if the new button state is HIGH
      if (playButtonState == HIGH) 
      {
        playFlag = !playFlag; 
        Serial.println("TOGG:E PLAY");
                 //Comment out to turn off Debug Mode Buzzer START
            for (int thisNote = 0; thisNote < 3; thisNote++  )
            {
              int noteDuration = 1000/noteDurations[thisNote];
              tone(5, melodyPlay[thisNote],noteDuration);
              int pauseBetweenNotes = noteDuration * 1.30;
              delay(pauseBetweenNotes);
              noTone(5);
            }


      }
    }
  }
  
  if ((millis() - lastRecordDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    
    
        // if the button state has changed:
    if (readingRecord != recordButtonState) 
    {
      recordButtonState = readingRecord;
      // only toggle the LED if the new button state is HIGH
      if (recordButtonState == HIGH) 
      {
        recordFlag = !recordFlag; 
        Serial.println("TOGG:E RECORD");
    

            if(recordFlag == true)
            {

              pose = 0;
              playIndexLimit = 0;
              for (int thisNote = 0; thisNote < 3; thisNote++ )
              {

                Serial.print("playing ");

                int noteDuration = 1000/noteDurations[thisNote];
                tone(BUZZER_PIN, melodyRecord[thisNote],noteDuration);
                int pauseBetweenNotes = noteDuration * 1.30;
                delay(pauseBetweenNotes);
                noTone(BUZZER_PIN);
              }
              RelaxServos();

            }

            else
            {


              for (int thisNote = 2; thisNote >= 0; thisNote-- )
              {

                Serial.print("playing ");

                int noteDuration = 1000/noteDurations[thisNote];
                tone(BUZZER_PIN, melodyRecord[thisNote],noteDuration);
                int pauseBetweenNotes = noteDuration * 1.30;
                delay(pauseBetweenNotes);
                noTone(BUZZER_PIN);
              }


            }

      }
    }
  }
  
  lastPlayButtonState = readingPlay;
  
  lastRecordButtonState = readingRecord;
  
  if(recordFlag == 0 && playFlag == 1)
  {  

   


        bioloid.readPose();
        //first pose 1 second
      for(int s = 1; s <= SERVOCOUNT; s++ )
      {
        bioloid.setNextPose(s, poseData[s-1][0]);
      }
      
        bioloid.interpolateSetup(1000);
        while(bioloid.interpolating > 0){
          bioloid.interpolateStep();
          delay(3);
        }


    for(int p = 0; p <= playIndexLimit; p++ )
    {
        Serial.print("pose:");
        Serial.println(p);
      for(int s = 1; s <= SERVOCOUNT; s++ )
      {
        bioloid.setNextPose(s, poseData[s-1][p]);
        
        Serial.print("servo:");
        Serial.print(s);
        Serial.print(" val:");
        Serial.println(poseData[s-1][p]);
      }
      
        bioloid.interpolateSetup(READ_INTERVAL);
        while(bioloid.interpolating > 0){
          bioloid.interpolateStep();
          delay(3);
        }


    //delay(1000);

  
    }
    playFlag = 0;

  }
  
  
  if(recordFlag == 1 && playFlag == 0)
  {  
 
    currentMillis = millis();
    currentMillisGripper = currentMillis;

    if( ((currentMillisGripper - previousMillisGripper) > 20)  )
    {

      previousMillisGripper = currentMillisGripper;
      int gripVal = analogRead(ANALOG_PIN);
      gripVal = gripVal/2; //map 0-1023 to 0-512
      SetPosition(5, gripVal);
    }


  //    Serial.print(currentMillis);
  //    Serial.print("  ");
  //    Serial.print(previousMillis);
  //    Serial.print("  ");
  //    Serial.print(currentMillis - previousMillis);
  //    Serial.print("  ");
  //    Serial.print(pose);
  //    Serial.print("  ");
  //    Serial.println(MAX_POSES);
    if( ((currentMillis - previousMillis) > READ_INTERVAL) && (pose < MAX_POSES) )
    {
      //RelaxServos();
      
      previousMillis = currentMillis;
     
     
      Serial.print("Reading Pose ");
      Serial.print(pose);
      Serial.print(" - ");
     
      
      for(int i = 1; i <= SERVOCOUNT; i++)
      {
        readTries = 0;
        currentReadPosition = -1;
  
  
        while(readTries < MAX_READ_TRIES && currentReadPosition == -1)
        {
          currentReadPosition = GetPosition(i);
          if(currentReadPosition > 1023 ||  currentReadPosition < 0)//why do these show up as 1793 occassionally?
          {
            Serial.println("");
            Serial.print("ERROR: ");
            Serial.print(currentReadPosition);
            Serial.println("");
            currentReadPosition = -1;
          }
  
  
          readTries = readTries + 1;
          delay(1);
        }
        poseData[i-1][pose] = currentReadPosition ;
  //      Serial.print(i);
  //      Serial.print(":");
  //      Serial.print(currentReadPosition);
  //      Serial.print(" ");
      }
      
      for(int k = 0; k < SERVOCOUNT; k++)
      {
        Serial.print(k+1);
        Serial.print(":");
        Serial.print(poseData[k][pose]);
        Serial.print(" "); 
        
      }
      
      
      playIndexLimit = pose;
      pose = pose + 1;
      Serial.println();
      if(pose >= MAX_POSES)
      {
        recordFlag = 0;
            for (int thisNote = 2; thisNote >= 0 ; thisNote-- )
            {

              Serial.print("playing ");

              int noteDuration = 1000/noteDurations[thisNote];
              tone(BUZZER_PIN, melodyRecord[thisNote],noteDuration);
              int pauseBetweenNotes = noteDuration * 1.30;
              delay(pauseBetweenNotes);
              noTone(BUZZER_PIN);
            }

      }
    }
  }
  
}



void RelaxServos(){
  int id = 1;
  Serial.println("###########################");
  Serial.println("Relaxing Servos.");
  Serial.println("###########################");    
  while(id <= SERVOCOUNT){
    Relax(id);
    id = (id++);
    delay(50);
  }
  
}


