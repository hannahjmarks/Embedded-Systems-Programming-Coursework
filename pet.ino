// include the library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//define colours to make my life easier
#define RED 0x1
#define GREEN 0x2
#define YELLOW 0x3
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

//volatile variables are used as timers and will be accessed/set by multiple functions
volatile int startDelayTime=0;
volatile int happyTimer=0;
volatile int fullTimer=0;
volatile int cleanTimer=0;

//boolean used to decide which display to run
bool menuOpen=false;

//the "pet" will be represented as an array of 5 integers: developement stage, happiness, fullness, cleanliness and age
int pet[5];
//the pausePet array will be used to reload the pet when the simulation is paused
int pausePet[5]={0,0,0,0,0};

//these are the time intervals in seconds after which the relevant stats will decrease
int happyDecrease=7;
int fullDecrease=11;
int cleanDecrease=20;

//this is used to calculate the age of the pet after the simulation has been paused
int resumeTime=0;





void setup() {
  //this code is only run once on startup
  // set up the LCD's number of columns and rows and the background colour to white
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);

  //this reads the data from the eeprom and sets the values in the pet array
  pet[0]=EEPROM.read(0);
  pet[1]=EEPROM.read(1);
  pet[2]=EEPROM.read(2);
  pet[3]=EEPROM.read(3);
  pet[4]=EEPROM.read(4);

  //pauses the simulation so that the pet does not age up or decrease any stats while loading
  menuOpen=true;

  //if there is no saved pet then the age item of the array will be 0
  if (pet[4]==0){
    //saved data can only be a new pet or the user saved pet so there is no need to change any of the values here
    lcd.setBacklight(VIOLET);
    lcd.setCursor(0,0);
    lcd.print("No Save Found");
    lcd.setCursor(0,1);
    lcd.print("Start New Pet");

    //unpauses simulation
    menuOpen=false;
    displayDelay();
  }else{
    //if there is a user saved pet then this while loop will run until the user chooses to load the pet or start a new one
    while (true){
      lcd.setCursor(0,0);
      lcd.print("Load Saved Pet?");
      lcd.setCursor(0,1);
      lcd.print("Yes");
      lcd.setCursor(14,1);
      lcd.print("No");
      //starts button reads
      uint8_t buttons = lcd.readButtons();
        //if left button is pressed the saved pet is used so there is no need to change any stats
        if (buttons & BUTTON_LEFT){
          buttonDelay();
          menuOpen=false;
          lcd.clear();
          break;
        }
        //if the right button is pressed then a new pet is started so the  stats are reset
        if (buttons & BUTTON_RIGHT) {
          buttonDelay();
          pet[0]=0;
          pet[1]=2;
          pet[2]=3;
          pet[3]=3;
          pet[4]=0;
          menuOpen=false;
          lcd.clear();
          break;
        }
    }
    //makes sure that the pet age is not effected by the amount of time spent on the load menu
    resumeTime=(millis()/1000)-pet[4];
  }
}

//this code is run continuously
void loop() {
  //starts button reads
  uint8_t buttons = lcd.readButtons();

  //this stops the pet age from increasing past 9:59
  if (pet[4]<599){
    pet[4]=(millis()/1000)-(resumeTime-pausePet[4]);
  }else{
    pet[4]=599; 
  }

  //increases the developement of the pet from 0(egg) to 1(young) when the pet is 5 secs old
  if (pet[0]==0 && pet[4]>=5 && menuOpen==false){
    lcd.clear();
    lcd.setBacklight(TEAL);
    lcd.setCursor(0,0);
    lcd.print("Age Up!");
    lcd.setCursor(0,1);
    lcd.print("Egg -> Young");
    pet[0]+=1;
    displayDelay();
    //makes it so that the stats cannot begin decreasing until the pet is at least developemnet stage 1
    happyTimer=millis()/1000;
    fullTimer=millis()/1000;
    cleanTimer=millis()/1000;
  }

  //if the menu is closed
  if (menuOpen==false){
    //run the statChange function and display the stats on the lcd screen
    statChange();
    printStats();
      //if the select button is pressed open the menu and pause the simulation
      if (buttons & BUTTON_SELECT) {
        pause();
        buttonDelay();
        lcd.clear();
        lcd.setBacklight(WHITE);
        menuOpen=true;
      }
      //if the up button is pressed run the growPet function
      if (buttons & BUTTON_UP) {
        buttonDelay();
        lcd.clear();
        growPet();
      }
     //if the down button is pressed run the feedPet function
     if (buttons & BUTTON_DOWN) {
       buttonDelay();
       lcd.clear();
       feedPet();
     }
     //if the left button is pressed run the playPet function 
     if (buttons & BUTTON_LEFT) {
      buttonDelay();
      lcd.clear();
      playPet();
     }
     //if the right button is pressed then run the cleanPet function
     if (buttons & BUTTON_RIGHT) {
      buttonDelay();
      lcd.clear();
      cleanPet();
     }
   //if the menu is open
  }else if (menuOpen==true){
    //display the menu
    printMenu();
      //if the select button is pressed the close the menu and resume the simulation
      if (buttons & BUTTON_SELECT) {
        buttonDelay();
        lcd.clear();
        lcd.setBacklight(WHITE);
        menuOpen=false;
        resumeSimulation();
        resumeTime=millis()/1000;
      }
      //if the up button is pressed then the current pet is scrapped and a new one is started
      if (buttons & BUTTON_UP) {
       lcd.clear();
       startNewPet();
      }
      //if the left button is pressed then the current pet is saved and the simulation is stopped (save and exit)
      if (buttons & BUTTON_LEFT) {
       lcd.clear();
       savePet();
      }
      //if the right button is pressed then the saved pet is deleted (the saved pet array is reset to a new pet)
      if (buttons & BUTTON_RIGHT) {
       lcd.clear();
       deletePet();
      }
  }
}





//function used to display the pet stats to the lcd screen
void printStats(){
  //print the developement stage to the upper left of the screen
  lcd.setCursor(0,0);
  if (pet[0]==0){
    lcd.print("Egg");
  }else if (pet[0]==1){
    lcd.print("Young");
  }else{
    lcd.print("Adult");
  }

  //print the pet age in minutes and seconds to the upper right of the screen
  lcd.setCursor(8,0);
  lcd.print("Age ");
  int mins=pet[4]/60;
  int secs=pet[4]%60;
  lcd.print(mins);
  lcd.print(":");
  if (secs<10){
    lcd.print("0");
  }
  lcd.print(secs);

  //print the remaining stats to the bottom row of the screen
  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.print(pet[1]);

  lcd.setCursor(6,1);
  lcd.print("F:");
  lcd.print(pet[2]);

  lcd.setCursor(12,1);
  lcd.print("C:");
  lcd.print(pet[3]);
}





//prints the menu to the screen
void printMenu(){
  lcd.setCursor(4,0);
  lcd.print("New Pet");
  
  lcd.setCursor(0,1);
  lcd.print("Save");

  lcd.setCursor(10,1);
  lcd.print("Delete");
}





//the feed pet function will be called from a button press outside of the menu and will print the effects of the function to the screen
//the timer is also reset everytime the function is run
void feedPet(){
  //if the fullness stat is less than 3 then it will be increased by 1
  if (pet[2]<3){
    lcd.setBacklight(GREEN);
    lcd.setCursor(0,0);
    lcd.print("Fullness +1");
    pet[2]+=1;
  }
  //if the fullness stat is 3 then the fullness is increased by 1 and the happiness is set to 0 
  else if (pet[2]==3){
    lcd.setBacklight(RED);
    lcd.setCursor(0,0);
    lcd.print("Fullness +1");
    lcd.setCursor(0,1);
    lcd.print("Happiness -");
    lcd.print(pet[1]);
    pet[2]+=1;
    pet[1]=0;
  }
  //if the fullness stat is 4 then the fullness stat is not cheanged but the happiness is set to 0
  else{
    lcd.setBacklight(RED);
    lcd.setCursor(0,0);
    lcd.print("Happiness -");
    lcd.print(pet[1]);
    pet[1]=0;
  }
  displayDelay();
  fullTimer=millis()/1000;
}





//the play pet function will be called from a button press outside of the menu and will print the effects of the function to the screen
//the timer is also reset everytime the function is run
void playPet(){
  //if the happiness is less than 2 and the fullness is greater than 1 then the happiness is increased by 1
  if (pet[1]<2 && pet[2]>=2){
    lcd.setBacklight(GREEN);
    lcd.setCursor(0,0);
    lcd.print("Happiness +1");
    pet[1]+=1;
  }
  //if the fullness is less than 2 then there will be no effect
  else{
    lcd.setBacklight(YELLOW);
    lcd.setCursor(0,0);
    lcd.print("No Effect");
  }
  displayDelay();
  happyTimer=millis()/1000;
}





//the clean pet function will be called from a button press outside of the menu and will print the effects of the function to the screen
//the timer is also reset everytime the function is run
void cleanPet(){
  //if cleanliness is less than 2 then it is set to 3 and happiness is increased by 1 (if it is below 2)
  if (pet[3]<2){
    lcd.setBacklight(GREEN);
    lcd.setCursor(0,0);
    lcd.print("Cleanliness +");
    lcd.print(3-pet[3]);
    pet[3]=3;
    if (pet[1]<2){
      lcd.setCursor(0,1);
      lcd.print("Happiness +1");
      pet[1]+=1;
    }
  }
  //if the cleanliness is 2 then it is set to 3 but happiness is descreased by 1 (if it is greater than 0)
  else if (pet[3]==2){
    lcd.setBacklight(YELLOW);
    lcd.setCursor(0,0);
    lcd.print("Cleanliness +1");
    if (pet[1]>0){
      lcd.setCursor(0,1);
      lcd.print("Happiness -1");
      pet[1]-=1;
    }
    pet[3]=3;
  }
  //if the cleanliness is 3 then the happiness is decreased by 1 (if it is greater than 0) otherwise no effect
  else{
    if (pet[1]>0){
      lcd.setBacklight(RED);
      lcd.setCursor(0,0);
      lcd.print("Happiness -1");
      pet[1]-=1;
    }else{
      lcd.setBacklight(YELLOW);
      lcd.setCursor(0,0);
      lcd.print("No Effect");
    }
  }
  displayDelay();
  cleanTimer=millis()/1000;
}





//the grow pet function will be called from a button press outside of the menu and will print the effects of the function to the screen
void growPet(){
  //if the development stage is 1 (young), the happiness is at least 1, the fullness is at least 3 and the age is at least 35 seconds
  if (pet[0]==1 && pet[1]>=1 && pet[2]>=3 && pet[4]>=35){
    lcd.setBacklight(TEAL);
    lcd.setCursor(0,0);
    lcd.print("Age Up!");
    lcd.setCursor(0,1);
    lcd.print("Young -> Adult");
    pet[0]+=1;
    displayDelay();
  }
  //if any of the above conditions are not met then there is no effect
  else{
    lcd.setBacklight(YELLOW);
    lcd.setCursor(0,0);
    lcd.print("No Effect");
    displayDelay();
  }
}





//the save pet function saves the current pet to the eeprom and then "exits" the sketch
void savePet(){
  //save each stat to eeprom
  EEPROM.write(0, pausePet[0]);
  EEPROM.write(1, pausePet[1]);
  EEPROM.write(2, pausePet[2]);
  EEPROM.write(3, pausePet[3]);
  EEPROM.write(4, pausePet[4]);
  lcd.setBacklight(VIOLET);
  lcd.setCursor(0,0);
  lcd.print("Pet Saved");
  lcd.setCursor(0,1);
  lcd.print("Exiting...");
  displayDelay();

  //disable the lcd display and all interrupts
  lcd.noDisplay();
  lcd.setBacklight(0x0);
  noInterrupts();
  //puts the system into an endless loop to give the appearence of exitting (the only true way to stop the arduino functioning is to remove the power supply)
  while(true){
  }
}





//the new pet function will reset all the stats of the current pet
void startNewPet(){
  pet[0]=0;
  pet[1]=2;
  pet[2]=3;
  pet[3]=3;
  pet[4]=0;
  //the pausePet array will also be rewritten to make absolutely sure that there will be no errors when pausing after the function is run
  pausePet[0]=0;
  pausePet[1]=0;
  pausePet[2]=0;
  pausePet[3]=0;
  pausePet[4]=0;
  
  lcd.setBacklight(VIOLET);
  lcd.setCursor(0,0);
  lcd.print("New Pet Created");
  displayDelay();

  //timers are reset
  happyTimer=millis()/1000;
  fullTimer=millis()/1000;
  cleanTimer=millis()/1000;
  resumeTime=millis()/1000;
  //simulation is un-paused
  menuOpen=false;
}





//the delete pet function rewrites the pet saved in the eeprom
void deletePet(){
  //stats rewritten to those of a new pet
  EEPROM.write(0, 0);
  EEPROM.write(1, 2);
  EEPROM.write(2, 3);
  EEPROM.write(3, 3);
  EEPROM.write(4, 0);
  lcd.setBacklight(VIOLET);
  lcd.setCursor(0,0);
  lcd.print("Saved Pet");
  lcd.setCursor(0,1);
  lcd.print("Deleted");
  displayDelay();

  //resets all timers
  happyTimer=millis()/1000;
  fullTimer=millis()/1000;
  cleanTimer=millis()/1000;
  resumeTime=millis()/1000;
  menuOpen=false;
}




//this function is a substitute for the delay function
//its purpose is to give the user time to release the button they are currently pressing so that it does not register as multiple button presses
void buttonDelay(){
  startDelayTime=millis()/100;
  while ((millis()/100)-startDelayTime<3){
  }
}




//this function is a substitute for the delay function
//it makes the current lcd display show for 2 seconds then clears it and resets the background
void displayDelay(){
  startDelayTime=millis()/1000;
  while ((millis()/1000)-startDelayTime<2){
  }
  lcd.clear();
  lcd.setBacklight(WHITE);
}




//the stat change function is responsible for all of the real-time stat changes
void statChange(){
  //if it has been 7 seconds since the happiness stat was lowered or the playPet function was run then happiness decreases by 1
  if((millis()/1000)-happyTimer>=happyDecrease){
    if (pet[1]>0){
      pet[1]-=1;
    }
    //timer is reset
    happyTimer=millis()/1000;
  }
  //if it has been 11 seconds since the fullness stat was lowered or the feedPet function was run then fullness decreases by 1
  if((millis()/1000)-fullTimer>=fullDecrease){
    if (pet[2]>0){
      pet[2]-=1;
    }
    //if fullness is 0 then happiness is also 0
    if (pet[2]==0){
      pet[1]=0;
    }
    //timer is reset
    fullTimer=millis()/1000;
  }
  //if it has been 20 seconds since the cleanliness stat was lowered or the cleanPet function was run then cleanliness decreases by 1
  if((millis()/1000)-cleanTimer>=cleanDecrease){
    if (pet[3]>0){
      pet[3]-=1;
    }
    //if cleanliness is 0 then happiness is also 0
    if (pet[3]==0){
      pet[1]=0;
    }
    //timer is reset
    cleanTimer=millis()/1000;
  }
}




//pause function changes the stats of the pausePet array to those of the pet array at the moment when the simulation is paused
void pause(){
  pausePet[0]=pet[0];
  pausePet[1]=pet[1];
  pausePet[2]=pet[2];
  pausePet[3]=pet[3];
  pausePet[4]=pet[4];
}




//the resume simulation function sets the stats of the pet array to those saved in the pausePet array and resets the various timers
void resumeSimulation(){
  pet[0]=pausePet[0];
  pet[1]=pausePet[1];
  pet[2]=pausePet[2];
  pet[3]=pausePet[3];
  pet[4]=pausePet[4];
  happyTimer=millis()/1000;
  fullTimer=millis()/1000;
  cleanTimer=millis()/1000;
}
