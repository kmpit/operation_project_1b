#include <Wire.h> //library for I2C

const int LEDPins[8] = {2,3,4,5,6,7,8,9}; //output pins
const int shortDelay = 25; //time for which lights stay on when flashing

int currentState = 0; //recieves from master
bool stateReceived = 0; //prevents loop from running until slave has been updated by master

void flash(); //flashes appropriate LEDs for state

void getState(); //function to receive the state variable from the other arduino

void setup() 
{
  //put all of the LED pins in output mode
  for (int i = 0; i < 8; i++){
    pinMode (LEDPins[i], OUTPUT);
    digitalWrite (LEDPins[i], 0);  
    }
    
  //set up the master-slave communication
  Wire.begin(8); //this connects the I2C bus
  Wire.onReceive(getState); //will go to this function when a transmission is recieved
  Serial.begin(9600);
  
}

void loop () //flashes specific LEDs depending on current state to show user what items they need to fetch 
{ 
  //recieve state from other arduino

  //flash appropriate lights
  if (stateReceived){
    flash();
  }

  stateReceived = 0; 
  
}

  void getState()
  {
    currentState=Wire.read();
    Serial.println("Received state!");
    stateReceived = 1; //enable main loop to run 
  }

  void flash (){
    switch (currentState){
    case 0: 
      //flash Obj0 G
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH); //may want to take out some of these conditions once all cases are written 
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[0], HIGH); //turn on green while keeping red and yellow off
     
      delay (shortDelay);  //keep on for short amount of time  
      digitalWrite (LEDPins[0], LOW); //turn off green  
      
      break;
      
    case 1: 
      //flash 0 Y
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], LOW);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[0], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[0], LOW); 

      break;
      
    case 2: 
      //flash 0 R
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);  
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[0], HIGH); 
     
      delay (shortDelay);
      digitalWrite (LEDPins[0], LOW); 
      
      break; 
      
    case 3: 
      //flash 1 G 
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[1], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[1], LOW); 

      break;
      
    case 4: 
      //flash 1 Y
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], LOW);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[1], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[1], LOW); 

      break;
      
    case 5: 
      //flash 1 R
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[1], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[1], LOW); 

      //flash 2 G
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[2], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[2], LOW);
      
      break;

    case 6: 
      //modification of case 5 where O1 retrieved but not O2
      //flash 2 G
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[2], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[2], LOW);

      break;
      
    case 7: 
      //modification of case 5 where O2 retrieved but not O1
      //flash 1 R
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[1], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[1], LOW);

      break;

    case 8:
      //flash 2 Y
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], LOW);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[2], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[2], LOW);
      
      //flash 3 G 
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[3], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[3], LOW);
      
      break;
      
    case 9: 
      //modification of case 8 where O2 retrieved but not O3
      //flash 3 G 
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[3], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[3], LOW);
      
      break;

    case 10: 
      //modification of case 8 where O3 retrieved but not O2
      //flash 2 Y
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], LOW);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[2], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[2], LOW);

      break;

    case 11: 
      //flash 2 R 
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[2], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[2], LOW);
      
      //flash 3 Y
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], LOW);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[3], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[3], LOW);
      
      break;
      
    case 12: 
      //modification of case 11 where O2 retrieved but not O3
      //flash 3 Y
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], LOW);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[3], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[3], LOW);
      
      break;

    case 13: 
      //modification of case 11 where O3 retrieved but not O2
      //flash 2 R 
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[2], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[2], LOW);

      break;

    case 14: 
      //flash 3 R
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[3], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[3], LOW);

      //flash 4 G 
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[4], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[4], LOW);
      
      break;
      
    case 15:
      //mod of case 14 where O3 retrieved but not O4
      //flash 4 G 
      digitalWrite (LEDPins[5], LOW);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[4], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[4], LOW);
      
      break;

    case 16: 
      //mod of case 14 where O4 retrieved but not O3
      //flash 3 R
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[3], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[3], LOW);

      break;

    case 17: 
      //flash 4 Y 
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], LOW);
      digitalWrite (LEDPins[7], HIGH);
      digitalWrite (LEDPins[4], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[4], LOW);
      
      break;  

    case 18: 
      //flash 4 R
      digitalWrite (LEDPins[5], HIGH);
      digitalWrite (LEDPins[6], HIGH);
      digitalWrite (LEDPins[7], LOW);
      digitalWrite (LEDPins[4], HIGH); 
     
      delay (shortDelay);  
      digitalWrite (LEDPins[4], LOW);

      break;

    default:
       break;
    }
  }

