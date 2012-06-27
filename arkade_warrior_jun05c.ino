//teensy++ code for arcade warrior
//tomash ghz
//www.tomashg.com

#include <Bounce.h>

//debugging will print out in serial instead of midi
boolean debugging=false;
boolean traktorrr=true;

//channel number for midi messages
int midiChannel=3;

//pin numbers for elements

int leds[3]={20,21,22};

int buttons[16]={5,9,13,17, //first row
               4,8,12,16,
               1,7,11,15,
               0,6,10,14};
               
int knobs[4]={A2,A3,A0,A1};

int faders[3]={A7,A6,A5};

// pins for joystick buttons
int joystic[4]={24,26,25,23};

// variables to store values
Bounce *buttonState[16];
int knobVal[4];
int faderVal[3];
boolean joysticState[4];
int joysticVal[4]; // save the joystic cc values
boolean ledState[3];

Bounce jUp = Bounce( joystic[0], 10 );
Bounce jDown = Bounce( joystic[1], 10 );
Bounce jLeft = Bounce( joystic[2], 10 );
Bounce jRight = Bounce( joystic[3], 10 );

int temp;

//initialize values and set modes
void setup(){
  
  //DEBUG
  if(debugging){
    Serial.begin(9600);//open serail port
  }
  else{
    Serial.begin(31250);//open midi
    usbMIDI.setHandleNoteOn(myNoteOn);
    usbMIDI.setHandleNoteOff(myNoteOff);
  }
  
  //arcade buttons
  for(int i=0;i<16;i++){
    pinMode(buttons[i],INPUT_PULLUP);
    *buttonState[i]= Bounce(buttons[i],5);
  }
  
  pinMode(6,OUTPUT); //hope this doesnt fry the led 
  
  // LEDS
  for(int i=0;i<3;i++){
    pinMode(leds[i],OUTPUT);
    digitalWrite(leds[i],HIGH);
    delay(50);
    digitalWrite(leds[i],LOW);
  }
  
  //joystick buttons
  for(int i=0;i<4;i++){
    pinMode(joystic[i],INPUT_PULLUP);
    joysticVal[i]=0;
  }
    
}

void loop(){
  //read buttons
  for(int i=0;i<16;i++){
    
    if(buttonState[i]->update()){//state changed
      
      if(buttonState[i]->read()==LOW){//is pressed
         midiNoteOnOff(true,i+36);
      }
      else{
        midiNoteOnOff(false,i+36);
      }
      
    }
  }//end loop
  
  //read knobs and faders
  for(int i=0;i<4;i++){
    temp=map(analogRead(knobs[i]),0,1023,0,127);

    if(temp!=knobVal[i]){ //value changed
      midiCC(temp,i*2+16);
    }
    knobVal[i]=temp;
  }// end loop
  
  for(int i=0;i<3;i++){
    temp=map(analogRead(faders[i]),0,1023,0,127);

    if(temp!=faderVal[i]){ //value changed
      midiCC(temp,i*2+24);
    }
    faderVal[i]=temp;
  }// end loop
  
  // read joystic guestures
  readJoystic();
  
  // update leds
  for(int i=0;i<3;i++){
    digitalWrite(leds[i],ledState[i]);
  }
  
  //recieve MIDI messages
  if(!debugging){
    usbMIDI.read();
  }
  
}

// function to handle noteon outgoing messages
void midiNoteOnOff(boolean s, int n){
  
   if(s){
     if(debugging){//debbuging enabled
        Serial.print("Button ");
        Serial.print(n);
        Serial.println(" pressed.");
     }else{
       usbMIDI.sendNoteOn(n, 127, midiChannel);
     }
   }
   else {
     if(debugging){//debbuging enabled
        Serial.print("Button ");
        Serial.print(n);
        Serial.println(" released.");
     }else{
       usbMIDI.sendNoteOff(n, 0, midiChannel);
     }
   }
}

// function to handle cc outgoing messages
void midiCC(int v,int n){
  if(debugging){//debbuging enabled
    Serial.print("Potentiometer ");
    Serial.print(n);
    Serial.print(" changed value to ");
    Serial.println(v);
  }else{
    usbMIDI.sendControlChange(n, v, midiChannel);
    if(traktorrr){
      if(v==0)
        usbMIDI.sendNoteOn(n+84, 127, midiChannel);
      else if(v==127)
        usbMIDI.sendNoteOn(n+85, 127, midiChannel);
      if(v<=64)
        usbMIDI.sendControlChange(n+1, map(v,0,64,0,127), midiChannel);
    }
  }
  
}

void readJoystic(){
     //up joystick
   if ( jUp.update() ) {//state changed
     if ( jUp.read() != HIGH) {// it is pressed

       if ( joysticState[0] == LOW ) {//last state was low
         joysticState[0] = HIGH;
         midiNoteOnOff(true,4);
       }
     }
     else{// it was released
        if ( joysticState[0] == HIGH ) {//last state was low
         joysticState[0] = LOW;
         joysticVal[0]=0;
         
         if(debugging){
             Serial.println(joysticVal[0]);
           }else{
             usbMIDI.sendControlChange(30, joysticVal[0], midiChannel);
           }
           //midiNoteOnOff(true,4);
           midiNoteOnOff(false,4);
       }
     }
   }
   else{//state didnot change
     if ( jUp.read() != HIGH) {//is held on
       if(jUp.duration()>500){// was held longer than half sec
         if(jUp.duration()%100>90){//increment the value
           joysticVal[0]++;
           joysticVal[0]=constrain(joysticVal[0],0,127);
           if(debugging){
             Serial.println(joysticVal[0]);
           }else{
             usbMIDI.sendControlChange(30, joysticVal[0], midiChannel);
           }
         }
       }
     }
   }
   
   //down joystick
   if ( jDown.update() ) {//state changed
     if ( jDown.read() != HIGH) {// it is pressed

       if ( joysticState[1] == LOW ) {//last state was low
         joysticState[1] = HIGH;
         midiNoteOnOff(true,5);
       }
     }
     else{// it was released
        if ( joysticState[1] == HIGH ) {//last state was low
         joysticState[1] = LOW;
         joysticVal[1]=0;
         
         if(debugging){
             Serial.println(joysticVal[1]);
           }else{
             usbMIDI.sendControlChange(31, joysticVal[1], midiChannel);
       }
       //midiNoteOnOff(true,5);
       midiNoteOnOff(false,5);
     }
     }
   }
   else{//state didnot change
     if ( jDown.read() != HIGH) {//is held on
       if(jDown.duration()>500){// was held longer than half sec
         if(jDown.duration()%100>95){//increment the value
           joysticVal[1]++;
           joysticVal[1]=constrain(joysticVal[1],0,127);
           if(debugging){
             Serial.println(joysticVal[1]);
           }else{
             usbMIDI.sendControlChange(31, joysticVal[1], midiChannel);
           }
         }
       }
     }
   }
   
   //left joystick
   //act as rotary encoder
   if ( !jLeft.update() ){//state didnot change
     if ( jLeft.read() != HIGH) {//is held on
         if(jLeft.duration()%100>85){//increment the value
           if(debugging){
             Serial.println("<<");
           }else{
             usbMIDI.sendControlChange(32, 63, midiChannel);
           }
       }
     }
   }
   
   //right joystick
   //act as rotary encoder
   if ( !jRight.update() ){//state didnot change
     if ( jRight.read() != HIGH) {//is held on
         if(jRight.duration()%100>85){//increment the value
           if(debugging){
             Serial.println(">>");
           }else{
             usbMIDI.sendControlChange(32, 65, midiChannel);
           }
       }
     }
   }
   
}

//event handlers for recieved note ons
void myNoteOn(byte channel,byte  note,byte velocity){
  if(channel==midiChannel){
    if(note==(byte)0)
      ledState[0]=true;
    if(note==(byte)1)
      ledState[1]=true;
    if(note==(byte)2)
      ledState[2]=true;
  }
  
  if((channel==midiChannel)&&(velocity==0)){
    if(note==(byte)0)
      ledState[0]=false;
     if(note==(byte)1)
      ledState[1]=false;
     if(note==(byte)2)
      ledState[2]=false;
  }
  
}

void myNoteOff(byte channel,byte  note,byte velocity){
  if(channel==midiChannel){
    if(note==(byte)0)
      ledState[0]=false;
     if(note==(byte)1)
      ledState[1]=false;
     if(note==(byte)2)
      ledState[2]=false;
  }
}
