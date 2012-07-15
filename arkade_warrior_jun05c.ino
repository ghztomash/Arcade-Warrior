//teensy++ code for arcade warrior
//tomash ghz
//www.tomashg.com

#include <Bounce.h>

//debugging will print out in serial instead of midi
boolean debugging=false;
boolean traktorrr=false;

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

//bank
int bank=0;

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

int comboState=0; //state of the current combo
int temp;

//initialize values and set modes
void setup(){
  
  //DEBUG
  if (debugging){
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
    buttonState[i]= new Bounce(buttons[i],5);
  }
  
  pinMode(6,OUTPUT); //hope this doesnt fry the led 
  
  //joystick buttons
  for(int i=0;i<4;i++){
    pinMode(joystic[i],INPUT_PULLUP);
    joysticVal[i]=0;
  }
  
  // LEDS
  for(int i=0;i<3;i++){
    pinMode(leds[i],OUTPUT);
    digitalWrite(leds[i],HIGH);
    delay(50);
    digitalWrite(leds[i],LOW);
  }
  
  //switch to ableton mode
  // to switch hold joystick up and the first button while the device is booting
  for(int i=0;i<10;i++){
    digitalWrite(leds[0],HIGH);
    delay(100);
    digitalWrite(leds[0],LOW);
    delay(100);
    
  if ((buttonState[0]->read()==LOW)&&(buttonState[3]->read()==LOW)){
    
    traktorrr=true;
    //flash leds to indicate the mode change
    for(int i=0;i<10;i++){
      digitalWrite(leds[0],HIGH);
      digitalWrite(leds[1],HIGH);
      digitalWrite(leds[2],HIGH);
      delay(100);
      digitalWrite(leds[0],LOW);
      digitalWrite(leds[1],LOW);
      digitalWrite(leds[2],LOW);
      delay(100);
    }
        break;
  }
  }
    
}

void loop(){
  //read buttons
  for(int i=0;i<16;i++){
    
    if (buttonState[i]->update()){//state changed
      
      if (buttonState[i]->read()==LOW){//is pressed
         midiNoteOnOff(true,i+36+bank*16);
      }
      else{
        midiNoteOnOff(false,i+36+bank*16);
      }
      
    }
  }//end loop
  
  //read knobs and faders
  for(int i=0;i<4;i++){
    temp=map(analogRead(knobs[i]),0,1023,0,127);

    if (temp!=knobVal[i]){ //value changed
      midiCC(temp,i*2+16+bank*17);
    }
    knobVal[i]=temp;
  }// end loop
  
  for(int i=0;i<3;i++){
    temp=map(analogRead(faders[i]),0,1023,0,127);

    if (temp!=faderVal[i]){ //value changed
      midiCC(temp,i*2+24+bank*17);
    }
    faderVal[i]=temp;
  }// end loop
  
  // read joystic guestures
  readJoystic();
    
  // update leds depending on bank
  switch(bank){
    case 0:
        ledState[0]=false;
        ledState[1]=false;
        ledState[2]=false;
        break;
    case 1:
        ledState[0]=true;
        ledState[1]=false;
        ledState[2]=false;
        break;
    case 2:
        ledState[0]=true;
        ledState[1]=true;
        ledState[2]=false;
        break;
    case 3:
        ledState[0]=true;
        ledState[1]=true;
        ledState[2]=true;
        break;
  }
  
  // update leds
  for(int i=0;i<3;i++){
    digitalWrite(leds[i],ledState[i]);
  }
  
  //recieve MIDI messages
  if (!debugging){
    usbMIDI.read();
  }
  
}

// function to handle noteon outgoing messages
void midiNoteOnOff(boolean s, int n){
  
  //check if the key pressed is part of any secret combo
  if (traktorrr){
    checkCombo(s);
  }
  
   if (s){
     if (debugging){//debbuging enabled
        Serial.print("Button ");
        Serial.print(n);
        Serial.println(" pressed.");
     }else{
       usbMIDI.sendNoteOn(n, 127, midiChannel);
     }
   }
   else {
     if (debugging){//debbuging enabled
        Serial.print("Button ");
        Serial.print(n);
        Serial.println(" released.");
     }else{
       usbMIDI.sendNoteOff(n, 0, midiChannel);
     }
   }
}

//function to check for secret combos
void checkCombo(boolean s){
  
  // from MIDI Fighter Pro documentation:
  
  // COMBOS
  //
  //   A          B           C           D           E
  // +--------+ +--------+  +--------+  +--------+  +--------+
  // |        | |        |  |        |  |        |  |        |
  // |        | |        |  |  3 4   |  |        |  |u       |
  // |        | |n n n n |  |  1 2   |  |a b c d |  |l r A B |
  // |1 2 3 4 | |        |  |        |  |        |  |d       |
  // +--------+ +--------+  +--------+  +--------+  +--------+
  //                                     a-b-c-c-d   uuddlrlrBA
  //
  // Combos retain a NoteOn while the final key is depressed and emit a NoteUp
  // when it is released.
  // Combo A               G#-2
  // Combo B               A-2
  // Combo C               A#-2
  // Combo D	B-2
  // Combo E	C-1
  
  
  // lets simplify things a bit :)
  //   A          B           C           D           E
  // +--------+ +--------+  +--------+  +--------+  +--------+
  // |        | |        |  |        |  |        |  |        |
  // |        | |        |  |  x x   |  |        |  |x       |
  // |        | |x x x x |  |  x x   |  |  x x x |  |    x x |
  // |x x x x | |        |  |        |  |        |  |x       |
  // +--------+ +--------+  +--------+  +--------+  +--------+

  
  //no combo active and a button is pressed
  if ((comboState==0)&&(s)){
    // combo A
    if ((buttonState[0]->read()==LOW)
        &&(buttonState[1]->read()==LOW)
        &&(buttonState[2]->read()==LOW)
        &&(buttonState[3]->read()==LOW)){
      comboState=1;
      usbMIDI.sendNoteOn(8, 127, midiChannel);
    }else
    //combo B
    if ((buttonState[4]->read()==LOW)
        &&(buttonState[5]->read()==LOW)
        &&(buttonState[6]->read()==LOW)
        &&(buttonState[7]->read()==LOW)){
      comboState=2;
      usbMIDI.sendNoteOn(9, 127, midiChannel);
    }else
    //combo C
    if ((buttonState[5]->read()==LOW)
        &&(buttonState[6]->read()==LOW)
        &&(buttonState[9]->read()==LOW)
        &&(buttonState[10]->read()==LOW)){
      comboState=3;
      usbMIDI.sendNoteOn(10, 127, midiChannel);
    }else
    //combo D
    if ((buttonState[5]->read()==LOW)
        &&(buttonState[6]->read()==LOW)
        &&(buttonState[7]->read()==LOW)){
      comboState=4;
      usbMIDI.sendNoteOn(11, 127, midiChannel);
    }else
    //combo E
    if ((buttonState[0]->read()==LOW)
        &&(buttonState[8]->read()==LOW)
        &&(buttonState[6]->read()==LOW)
        &&(buttonState[7]->read()==LOW)){
      comboState=5;
      usbMIDI.sendNoteOn(12, 127, midiChannel);
    }
    
  }else if ((comboState!=0)&&(!s)){//combo was active and released
    switch(comboState){
      case 1:
        usbMIDI.sendNoteOff(8, 0, midiChannel);
        break;
      case 2:
        usbMIDI.sendNoteOff(9, 0, midiChannel);
        break;  
      case 3:
        usbMIDI.sendNoteOff(10, 0, midiChannel);
        break;
      case 4:
        usbMIDI.sendNoteOff(11, 0, midiChannel);
        break;  
      case 5:
        usbMIDI.sendNoteOff(12, 0, midiChannel);
        break;
    }
    comboState=0;
  }
  
}

// function to handle cc outgoing messages
void midiCC(int v,int n){
  if (debugging){//debbuging enabled
    Serial.print("Potentiometer ");
    Serial.print(n);
    Serial.print(" changed value to ");
    Serial.println(v);
  }else{
    usbMIDI.sendControlChange(n, v, midiChannel);
    if (traktorrr){
      if (v==0)
        usbMIDI.sendNoteOn(n+84, 127, midiChannel);
      else if (v==127)
        usbMIDI.sendNoteOn(n+85, 127, midiChannel);
      if (v<=64)
        usbMIDI.sendControlChange(n+1, map(v,0,64,0,105), midiChannel);
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
         
         if (debugging){
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
       if (jUp.duration()>500){// was held longer than half sec
         if (jUp.duration()%100>90){//increment the value
           joysticVal[0]++;
           joysticVal[0]=constrain(joysticVal[0],0,127);
           if (debugging){
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
         
         if (debugging){
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
       if (jDown.duration()>500){// was held longer than half sec
         if (jDown.duration()%100>95){//increment the value
           joysticVal[1]++;
           joysticVal[1]=constrain(joysticVal[1],0,127);
           if (debugging){
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
       if ( joysticState[2] == LOW ) {//last state was low
         joysticState[2] = HIGH;
         if (bank==0)
           bank=3;
         else
           bank=bank-1;
       }
     }else{// it was released
        if ( joysticState[2] == HIGH ) {//last state was low
         joysticState[2] = LOW;
     }
     }
   }
   
   //right joystick
   //act as rotary encoder
   if ( !jRight.update() ){//state didnot change
     if ( jRight.read() != HIGH) {//is held on
         if ( joysticState[3] == LOW ) {//last state was low
         joysticState[3] = HIGH;
           bank=(bank+1)%4;
       }
     }
     else{// it was released
        if ( joysticState[3] == HIGH ) {//last state was low
         joysticState[3] = LOW;
     }
   }
   }
}

//event handlers for recieved note ons
void myNoteOn(byte channel,byte  note,byte velocity){
  if (channel==midiChannel){
    if (note==(byte)0)
      ledState[0]=true;
    if (note==(byte)1)
      ledState[1]=true;
    if (note==(byte)2)
      ledState[2]=true;
  }
  
  if ((channel==midiChannel)&&(velocity==0)){
    if (note==(byte)0)
      ledState[0]=false;
     if (note==(byte)1)
      ledState[1]=false;
     if (note==(byte)2)
      ledState[2]=false;
  }
  
}

void myNoteOff(byte channel,byte  note,byte velocity){
  if (channel==midiChannel){
    if (note==(byte)0)
      ledState[0]=false;
     if (note==(byte)1)
      ledState[1]=false;
     if (note==(byte)2)
      ledState[2]=false;
  }
}