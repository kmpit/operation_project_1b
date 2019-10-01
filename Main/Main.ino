#include <Wire.h> //this is library for I2C connection //all the information on communicating between Arduinos can be found here: https://www.arduino.cc/en/Tutorial/MasterWriter

#include <LiquidCrystal.h> //This is the library for the LCD screen 

//VARIABLE DECLARATIONS

//A. interrupts
const int errorPin = 2;        //interrupt pin for errors
const int buttonInterruptPin = 3;  //interrupt pin for success buttons --> each button is pressed when corresponding object is retrieved
const int buttonPins [5] = {4,5,6,7,8}; //digitalRead these pins to see which button was pressed

volatile bool errorMade = 0; //flag for if any button was pressed
volatile int errorCounter = 0; //count errors for time scaling purposes

volatile bool buttonInterruptFired = 0; //flag for if any button was pressed
volatile int justPressed = 5; //stores which object was just retrieved, as identified in interrupt handler; starts as 5 because this has no button

//B. game logic 
int state = 0; //stores the current state that the game is in, e.g. Object 1 is yellow, Object 2 is green. 
const int stateCounts = 15; //max number of counts for each state 

bool alreadyPressed[5]={0,0,0,0,0}; //tracks if a button has already been pressed

const double objectRewards [5] = {20.0, 40.0, 100.0, 110.0, 200.0}; //standard rewards for the 5 objects
double totalRewards = 0; //stores total money earned by user
const double yellowFactor = 0.5; //multiplier for reward if object retrieved in yellow stage
const double redFactor = 0.2; //multiplier for reward if object retrieved in red stage

//C. timing
//general
double factoredCountValue = 0; //stores count value 
double factor = 1000; //factors millis value into game counts (each state times out after 15 counts)         
long long int prevTime = 0; //stores millis value upon each iteration of loop so that changing count factors can be accounted for 

//D. Timer variables, for writing millis
long double counter_freq=(16000000 / 256);//stores the actual frequency of Timer1, based on prescaling       
volatile unsigned long long int count=0; //variable to store the number of times the timer has overflowed 
unsigned long long int totalCounts=0; //variable to keep track of the value calculated by timer1
unsigned long long int totalTime=0;   //variable holds the number of seconds since start of program run 
double calculateTime();
//end of the section that i added for new millis

//game run times 
double gameRunTime [50]; //stores run time of each round of game (up to 50 games)
double gameStartStamp=0; //stores start time of current game; used to calculate above
int gameCounter = 0; //stores number of completed games

//times for each object 
double stateStartStamp [20];  //will hold time stamp at which each state was entered
double stateStartCount [20]; //same as above, for factored count value
double objectEndStamp [5] = {0,0,0,0,0};  //time stamp when each object is picked up 
double objectEndCount [5] = {0,0,0,0,0}; //same as above, for factored count value
double objectTotalTime [5] = {0,0,0,0,0}; //time between when object first became available (depends on state start) to when it was retrieved
double objectTotalCount [5] = {0,0,0,0,0}; //counts (light flashes) between when object first became available to when it was retrieved


//SET UP LCD SCREEN
LiquidCrystal lcd(A0,13,12,11,10,9);

//INITIALIZE FUNCTIONS
void error_ISR();

void button_ISR();

double factoredCount (); //converts from millis to our units of counting/time (conversion factor will change as number of errors increases) 

void objectRetrieved (int object); //runs appropriate events for a particular object's button having been pushed

int calculateReward (int object); //returns reward value corresponding to object and stage

void stateLogic(); //after an object is retrieved, decides what state in the game we should move to

bool checkStateCountOut(); //called by stateLogic 

void endOfGame(); //calculates and outputs stats and offers new game

void calculateObjectTimes(); //called by endOfGame, fills array ObjectTimes

void showStats(); //called by endOfGame, shows stats

void reinitialize(); //called by endOfGame


//MAIN
void setup()
{
  //initialize timing arrays
  for (int i = 0; i<20; i++){
    stateStartStamp[i]=0;
    stateStartCount[i]=0;
  }
  for (int i = 0; i<50; i++){
    gameRunTime[i]=0;
  }
  
  //make sure timer 0 is set to prescaling 64
  TCCR0A = 0x0; 
  TCCR0B = 0x0;
  TCCR0B = 0x3;

  //make sure timer 1 is set to prescaling 256
  TCCR1A = 0x0; 
  TCCR1B = 0x0;
  TCCR1B = 0x4;
  
  //this section is for the Arduinos communicating
  Wire.begin();

  //set up the lcd display
  lcd.begin(16,2); 
  
  //put intterupt and button pins in input mode
  pinMode(errorPin, INPUT);  
  pinMode (buttonInterruptPin, INPUT);
  for (int i = 0; i < 5; i++){
    pinMode (buttonPins[i], INPUT);}

  //initialize serial monitor 
  Serial.begin(9600);

  //set up timers
  TCCR1A =0x0; //reset Timer1 control registers
  TCCR1B=0x0;  //set WGM to 0
  TCCR1B=0x4; //Set Tumer1 clock frequency to clk/1, CS is 001
  TCNT1=0; //set 0 as timer1 starting value
  TIMSK1=0x1; //enable overflow interrupt bits

  //attach and enable interrupts
  attachInterrupt(0, error_ISR, RISING); // interrupt 0 is mapped to pin 2 on the Uno. Every rising edge of the incoming signal will now trigger an interrrupt
  attachInterrupt(1, button_ISR, RISING);
  sei();
}


void loop()
{ 
   
  //delay between loops 
  delay (factor);

  //pass new state to other Ard which will flash corresponding LEDs once
  Wire.beginTransmission(8); //this will transmit to device #8, is 8 what we want? I don't know we need to try it
  Wire.write(state); //send that state variable over to the other arduino to flash the lights
  Wire.endTransmission();//stop sending data to the other arduino
  
  //check if an object was just retrieved and run appropriate routines
  if (buttonInterruptFired){
    objectRetrieved (justPressed);
    buttonInterruptFired=0;
  }
  
  //check if error made and run appropriate routines
  if (errorMade == 1) {
  
    //write to LCD
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Error Made");
    lcd.setCursor(0,1);
    lcd.print(errorCounter);
    lcd.print(" total errors!");

    errorMade = 0; //reset
    
    //increase frequency of count by decreasing factor 
    if (errorCounter < 10)
      factor *= 0.8;
    }

  //determine next state
  stateLogic();   

}

void error_ISR ()        //interrupt handler for pin 2 which tracks errors 
{
  cli(); 
  errorMade = 1; 
  errorCounter++;
  sei();
  return;
}

void button_ISR () //interrupt handler for pin 3 which tracks if any success buttons were pressed 
{
  cli();
 
  buttonInterruptFired = 1; //change flag variable
  
  for (int i = 0; i<5; i++){
    if (digitalRead (buttonPins[i]))
      justPressed = i;   //record which button was pressed
      
  }
  sei();
  return;
}

double factoredCount (){
  factoredCountValue += (( calculateTime()- prevTime) / factor) ;
  //factoredCountValue = calculateTime() / factor; 
  prevTime=calculateTime();
  return factoredCountValue; 
  
} 

void objectRetrieved (int object){ //runs appropriate routines once button has been pressed for a particular object 
  
  //this condition will check if the button has already been pressed, in which case the user wins nothing
  if(!alreadyPressed[object])
  {
    //record time and count stamps for timing purposes
    objectEndStamp[object]=calculateTime();
    objectEndCount[object]=factoredCount();
    
    //calculate money depending on current state and base price of that item 
    int currentReward = 0; 
    currentReward = calculateReward (object);
    
    //output currentReward to LCD
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Earned $");
    lcd.print(currentReward);

    //add to totalrewards
    totalRewards += currentReward; 
    
    //output totalRewards to LCD 
    lcd.setCursor(0,1);
    lcd.print("Total $");
    lcd.print(totalRewards);
        
  }
  
  alreadyPressed[object]=1; //set array index corrseponding to this button press as true, so we can track if a button has been pressed twice
  stateLogic(); 
  
  return;
  
}

int calculateReward (int object){
  //fix for new states
  int reward = 0;
  
  switch (state){
    case 0: 
      if (object == 0){
        reward = objectRewards[object];}
      else {reward = 0;}//user will win no reward if they grab an object without a light flashing for it
      break;
      
    case 1: 
      if (object == 0){
        reward = yellowFactor * objectRewards[object];}
      else {reward = 0;}
      break;
      
    case 2: 
      if (object == 0){
        reward = redFactor * objectRewards[object];}
      else {reward = 0;}
      break;
      
    case 3:
      if (object == 1){
        reward = objectRewards[object];}
      else {reward = 0;}
      break;
      
    case 4: 
      if (object < 1 ){
        reward = 0; }
      else if (object == 1){
        reward = yellowFactor * objectRewards[object];}
      else {reward = 0;}
      break;
      
    case 5: 
      if (object == 1){
        reward = redFactor * objectRewards[object];}
      else if (object == 2){
        reward = objectRewards[object];}
      else {reward = 0;}
      break;

    case 6: 
      if (object == 2){
        reward = objectRewards[object];}
      else {reward = 0;}
      break;

    case 7: 
      if (object == 1){
        reward = redFactor*objectRewards[object];}
      else {reward = 0;}
      break;
      
    case 8: 
      if (object == 2){
        reward = yellowFactor * objectRewards[object];}
      else if (object == 3){
        reward = objectRewards [object];}
      else {reward = 0;}
      break;

    case 9: 
      if (object == 3){
        reward = objectRewards [object];}
      else {reward = 0;}
      break;

    case 10: 
      if (object == 2){
        reward = yellowFactor*objectRewards [object];}
      else {reward = 0;}
      break;
      
    case 11: 
      if (object == 2){
        reward = redFactor * objectRewards[object];}
      else if (object == 3){
        reward = yellowFactor * objectRewards [object];}
      else {reward = 0;}
      break;

    case 12: 
      if (object == 3){
        reward = yellowFactor * objectRewards [object];}
      else {reward = 0;}
      break;

    case 13: 
      if (object == 2){
        reward = redFactor * objectRewards[object];}
      else {reward = 0;}
      break;
      
    case 14: 
      if (object == 3){
        reward = redFactor * objectRewards[object];}
      else if (object == 4){
        reward = objectRewards[object];}
      else {reward = 0;}
      break;

    case 15: 
      if (object == 4){
        reward = objectRewards[object];}
      else {reward = 0;}
      break;

    case 16: 
      if (object == 3){
        reward = redFactor * objectRewards[object];}
      else {reward = 0;}
      break;
      
    case 17: 
      if (object == 4){
        reward = yellowFactor * objectRewards [object];}
      else {reward = 0;}
      break; 

    case 18: 
      if (object == 4){
        reward = redFactor * objectRewards [object];}
      else {reward = 0;}
      break;
      
    default: 
      reward = 0; 
      break;
      }
  
  return reward; 
}

void stateLogic (){
  int prevState = state;
  
if (state==0 && checkStateCountOut())
  state=1;  
else if (state==1 && checkStateCountOut())
state=2;
else if (state==2 && checkStateCountOut())
state=3;
else if((state==0||state==1||state==2)&&alreadyPressed[0])
state=3;
else if(state==3 && checkStateCountOut())
state=4;
else if ((state==3 || state == 4) && alreadyPressed[1])
state = 6; 
else if (state==4 && checkStateCountOut())
state=5;
else if(state==5)
{
  if(checkStateCountOut())
  state=8;
  else if(alreadyPressed[1])
  state=6;
  else if(alreadyPressed[2])
  state=7;
}
else if (state==6)
{
  if (checkStateCountOut())
  state=8;
  else if(alreadyPressed[2])
  state=9;  
}
else if(state==7)
{
  if(checkStateCountOut()||alreadyPressed[1])
  state=9;
  
}
else if(state==8)
{
  if(checkStateCountOut())
  state=11;
  else if(alreadyPressed[2])
  state=9;
  else if(alreadyPressed[3])
  state=10;
}
else if (state==9)
{
  if(checkStateCountOut())
  state=12;
  else if(alreadyPressed[3])
  state=15;
}
else if (state==10)
{
  if(checkStateCountOut())
  state=13;
  else if(alreadyPressed[2])
  state=15;
}
else if(state==11)
{
  if(checkStateCountOut())
  state=14;
  else if(alreadyPressed[2])
  state=12;
  else if(alreadyPressed[3])
  state=13;
}
else if(state==12)
{
  if(checkStateCountOut())
  state=14;
  else if(alreadyPressed[3])
  state=15;
}
else if(state==13)
{
  if (checkStateCountOut()||alreadyPressed[2])
  state=15;
}
else if(state==14)
{
  if(checkStateCountOut())
  state=17;
  else if(alreadyPressed[3])
  state=15;
  else if(alreadyPressed[4])
  state=16;
}
else if(state==15)
{
  if(checkStateCountOut())
  state=17;
  else if(alreadyPressed[4])
  state=19;
}
else if (state==16)
{
  if(checkStateCountOut()||alreadyPressed[3])
  state=19;
}
else if(state==17)
{
  if(checkStateCountOut())
  state=18;
  else if(alreadyPressed[4])
  state=19;
  
}
else if(state==18)
{
  if(checkStateCountOut()||alreadyPressed[4])
  state=19;
}

else if (state == 19)
{
  endOfGame();
}

if (state != prevState){
  stateStartStamp[state]=calculateTime();
  stateStartCount[state]=factoredCount();
}
}

bool checkStateCountOut(){
  if ((factoredCount() - stateStartCount[state])>=15)
    return 1; 
  else 
    return 0; 
}


void endOfGame(){
  gameCounter++;
  gameRunTime[gameCounter-1]=calculateTime()-gameStartStamp;

  lcd.clear ();
  lcd.print("GAME OVER PAL");
  delay(2000); //so user can read
  lcd.clear();

  calculateObjectTimes(); //fill the array ObjectTotalTime
  showStats();
  
  //allow them to play again 
  Serial.println("Wanna play again?");
  
  lcd.print("Play again?");
  delay(2000);
  lcd.clear();
  
  Serial.println("Press any button to play again.");
  
  lcd.print("Press any button");
  lcd.setCursor(0,1);
  lcd.print("to play again");

  //delay until they start again
  while (!buttonInterruptFired){
    
  }

  lcd.clear();

  reinitialize();

  gameStartStamp=calculateTime();
}

void showStats(){
  Serial.println("Here are your stats!");
  
  Serial.print("You earned: $");
  Serial.println(totalRewards);
  
  Serial.print("This is game number ");
  Serial.print(gameCounter);
  Serial.print(" and you've been playing for ");
  Serial.print(calculateTime()/1000);
  Serial.println("s.");
  Serial.print("This game lasted ");
  Serial.print(gameRunTime[gameCounter-1]/1000);
  Serial.println("s");

  Serial.print("You made ");
  Serial.print(errorCounter);
  Serial.println(" errors.");

  Serial.println("You took this much time for each object:");
  for (int i=0; i<5; i++){
    Serial.print("Object ");
    Serial.print(i);
    Serial.print(": ");
    if (objectTotalTime[i]<0){
      Serial.println("You didn't pick it up while its lights were flashing!");
    }
    else {
      Serial.print(objectTotalTime[i]/1000);
      Serial.print("s, which was ");
      Serial.print((int)objectTotalCount[i]);
      Serial.println(" flashes of the LED.");
    }
  }
}

void calculateObjectTimes(){
  objectTotalTime[0]=objectEndStamp[0]-stateStartStamp[0]; //object 0 always starts at state 0
  objectTotalCount[0]=objectEndCount[0]-stateStartCount[0]; //same but for count
  
  objectTotalTime[1]=objectEndStamp[1]-stateStartStamp[3]; //object 1 always starts at state 3
  objectTotalCount[1]=objectEndCount[1]-stateStartCount[3]; 

  //object 2 could start on state 5 or state 6 
  //whichever state it starts on, the other state will not ever start so its stateStartStamp will be 0 
  if (stateStartStamp[5]>stateStartStamp[6]){
      objectTotalTime[2]=objectEndStamp[2]-stateStartStamp[5];
      objectTotalCount[2]=objectEndCount[2]-stateStartCount[5];

  }
  else{
    objectTotalTime[2]=objectEndStamp[2]-stateStartStamp[6];
    objectTotalCount[2]=objectEndCount[2]-stateStartCount[6];
    }
  
  //object 3 is similar but can start on state 8 or 9
  if (stateStartStamp[8]>stateStartStamp[9]){
      objectTotalTime[3]=objectEndStamp[3]-stateStartStamp[8];
      objectTotalCount[3]=objectEndCount[3]-stateStartCount[8];
  }
  else{
    objectTotalTime[3]=objectEndStamp[3]-stateStartStamp[9];
    objectTotalCount[3]=objectEndCount[3]-stateStartCount[9];}
  
  //object 4 is similar but can start on state 14 or 15
  if (stateStartStamp[14]>stateStartStamp[15]){
      objectTotalTime[4]=objectEndStamp[4]-stateStartStamp[14];
      objectTotalCount[4]=objectEndCount[4]-stateStartCount[14];
  }
  else{
    objectTotalTime[4]=objectEndStamp[4]-stateStartStamp[15];
    objectTotalCount[4]=objectEndCount[4]-stateStartCount[15];
  }  
}

void reinitialize(){
  buttonInterruptFired = 0; 
  errorCounter = 0;
  justPressed = 5;
  state = 0;
  totalRewards = 0;
  factoredCountValue = 0;
  factor = 1000;

  for (int i = 0; i<20; i++){
    stateStartStamp[i]=0;
    stateStartCount[i]=0;
  }
  for (int i=0; i<5; i++){
    alreadyPressed[i]=0;
    objectEndStamp[i]=0;
    objectEndCount[i]=0;
    objectTotalTime[i]=0;
    objectTotalCount[i]=0;
  }
}

ISR(TIMER1_OVF_vect)
{

  TCNT1=0;
  count++;
}

double calculateTime()
{
  totalCounts=TCNT1+(count*65536);
  totalTime=1000*totalCounts/counter_freq;
 
  return totalTime;
}


