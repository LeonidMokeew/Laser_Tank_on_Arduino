
#include <Servo.h>
#include <SoftwareSerial.h>
#include <AFMotor.h>



#define pinLaserGun A2
#define pinRedLED A3
#define pinYellowLED A4
#define SPEAKER A5
#define pinHorServo 9
#define pinVertServo 10

SoftwareSerial BTSerial(A1,A0); //rx, tx

AF_DCMotor left_motor(3); //initialise left and right motors
AF_DCMotor right_motor(1);

Servo vert_servo;//initialise vertical and horisontal servos
Servo hor_servo;

byte commands[4] = {0x00,0x00,0x00,0x00};
byte prevCommands[4] = {0x01,0x01,0x01,0x01};

//Variables will be used to determine the frequency at which the sensor readings are sent 
//to the phone, and when the last command was received.  
unsigned long timer0 = 2000;  //Stores the time (in millis since execution started) 
//unsigned long timer1 = 0;  //Stores the time when the last sensor reading was sent to the phone
unsigned long timer2 = 0;  //Stores the time when the last command was received from the phone

int i = 0;
int left_m_speed; //we devide overall speed to able to steer the tank with tracks
int right_m_speed;
//Opened position for tilt-pan
byte hor_pos_opened = 90;
byte vert_pos_opened = 90;
//Closed position for tilt-pan
byte hor_pos_closed = 180;
byte vert_pos_closed = 180;

void setup()
{

    BTSerial.begin(9600);
    //Serial.begin(9600);  //change the speed on HC-05
    pinMode(pinRedLED, OUTPUT);
    pinMode(pinYellowLED, OUTPUT);
    pinMode(pinLaserGun, OUTPUT);
    pinMode(SPEAKER, OUTPUT);
        //OpeningMelody();
    digitalWrite(pinYellowLED, HIGH);
    digitalWrite(pinRedLED, HIGH);
    digitalWrite(SPEAKER, HIGH);
    delay(500);
    digitalWrite(pinYellowLED, LOW);
    digitalWrite(pinRedLED, LOW);
    digitalWrite(SPEAKER, LOW);
    delay(500);
    digitalWrite(pinYellowLED, HIGH);
    digitalWrite(pinRedLED, HIGH);
    digitalWrite(SPEAKER, HIGH);
    delay(500);
    digitalWrite(pinYellowLED, LOW);
    digitalWrite(pinRedLED, LOW);
    digitalWrite(SPEAKER, LOW);

  //motors setup
  
  left_motor.setSpeed(255);
  left_motor.run(RELEASE);
  right_motor.setSpeed(255);
  right_motor.run(RELEASE);
  
  vert_servo.attach(pinVertServo);
  hor_servo.attach(pinHorServo);
}

void loop(){

  if(BTSerial.available() == 4){//исправить
    timer2 = millis();  //Store the time when last command was received
    memcpy(prevCommands,commands,4);  //Storing the received commands   
    commands[0] = BTSerial.read();  //Direction
    commands[1] = BTSerial.read();  //Speed
    commands[2] = BTSerial.read();  //Angle
    commands[3] = BTSerial.read();  //Lights and buttons states
    /*
    Serial.print('0');              // commands for debugging via Serial-port
    Serial.print(commands[0], HEX);
    Serial.print(' ');
    Serial.print('1');         
    Serial.print(commands[1], HEX);
    Serial.print(' ');
    Serial.print('2');         
    Serial.print(commands[2], HEX);
    Serial.print(' ');
    Serial.print('3');         
    Serial.print(commands[3], HEX);
    Serial.print(' ');
    Serial.print('L');         
    Serial.print(left_m_speed);
    Serial.print(' ');
    Serial.print('R');         
    Serial.print(right_m_speed);
    Serial.print('\n');
    */
    
    /*
     Since the last byte yields the servo's angle (between 0-180), it can never be 255. At times, the two
     previous commands pick up incorrect values for the speed and angle. Meaning that they get the direction 
     correct 100% of the time but sometimes get 255 for the speed and 255 for the angle.
     */
    if((commands[2]<=0xb4)&&((commands[0]<=0xf5)&&(commands[0]>=0xf1))) {
        //Make sure that the command received involves controlling the car's motors (0xf1,0xf2,0xf3)
        if (commands[0] <= 0xf3) {
            //Remove laser gun by turning tilt-pan to "closed" position and switch off LED
            
            if (prevCommands[0] == 0xf4) {
                hor_servo.write(hor_pos_closed);
                vert_servo.write(vert_pos_closed);
                digitalWrite(pinYellowLED, LOW);
            }
            
            if (commands[0] == 0xf3) {  //Check if the stop command was received
                if (prevCommands[0] != 0xf3) {  //Change pin state to stop only if previous state was not stop
                    left_motor.run(RELEASE);
                    right_motor.run(RELEASE);
                    //redCar.stopped_1W();
                    //Serial.println("Updated direction STP");
                }
            } else {
                if (commands[0] == 0xf1) {  //Check if the move forward command was received
                    /*if (prevCommands[0] == 0xf2) {  //Check that there is no change in direction
                        left_motor.run(RELEASE);
                        right_motor.run(RELEASE);
                    } else */
                    if (prevCommands[0] != 0xf1) {
                                 //Change pin state to move forward only if previous state was not move forward!!!
                        left_motor.run(FORWARD);
                        right_motor.run(FORWARD);

                        //Serial.println("Updated direction FWD");
                    }

                } else {  //Check if the move back command was received
                    /*if (prevCommands[0] == 0xf1) {  //Check that there is no change in direction
                        left_motor.run(RELEASE);
                        right_motor.run(RELEASE);
                    } else*/ 
                    if (prevCommands[0] != 0xf2) {
                                 //Change pin state to move back only if previous state was not move back
                        left_motor.run(BACKWARD);
                        right_motor.run(BACKWARD);

                        //Serial.println("Updated direction BACK");
                    }
                }
            }

            //Change speed only if new speed is not equal to the previous speed
            if ((prevCommands[1] != commands[1]) || (prevCommands[2] != commands[2])) {
                if (commands[2] == 90){ //going straight
                  left_m_speed = commands[1];
                  right_m_speed = commands[1];
                }
                else if(commands[2] > 90){//turning right
                  left_m_speed = commands[1];
                  right_m_speed = (commands[1]*(180-commands[2]))/90;
                }
                else{//turning left
                  left_m_speed = (commands[1]*commands[2])/90;
                  right_m_speed = commands[1];
                }
                //setting motors with calculated speed
                left_motor.setSpeed(left_m_speed);
                right_motor.setSpeed(right_m_speed);
            }

        } else if (commands[0] == 0xf5) {
            if (prevCommands[0] != 0xf5) {
                //Stop everything
                left_motor.run(RELEASE);
                right_motor.run(RELEASE);
            }
        } else {
            //Turn tilp-pan in "fire" position (open laser gun if it was closed) and switch on LED
            
            if (prevCommands != 0xf4) {
                //hor_servo.write(hor_pos_opened);
                //vert_servo.write(vert_pos_opened);
                digitalWrite(pinYellowLED, HIGH);
            }
            
            //Turn tilt-pan left-right if necessary
            if (commands[1] != prevCommands[1]) {
                hor_servo.write(commands[1]);
            }
            //Turn tilt-pan up-down if necessary
            if (commands[2] != prevCommands[2]) {
                vert_servo.write(commands[2]);
            }
            
            //Start a shot
            if ((bitRead(prevCommands[3], 3)) != (bitRead(commands[3], 3))){
              //Start a shot
              if ((bitRead(commands[3], 3)) == 1){
                //analogWrite(pinLaserGun, 255);
                  digitalWrite(pinYellowLED,LOW);
                  digitalWrite(pinRedLED,HIGH);
                  digitalWrite(pinLaserGun,HIGH);
                  digitalWrite(SPEAKER,HIGH);
             }
              //Stop a shot
              else{
                digitalWrite(pinYellowLED,HIGH);
                  digitalWrite(pinRedLED,LOW);
                  digitalWrite(pinLaserGun,LOW);
                  digitalWrite(SPEAKER,LOW);
              }
              }
        }
        //Check toggles
        if (commands[3] != prevCommands[3]) {
            //Serial.println(commands[3],BIN);
            //Change the light/button states
            //               _______________________________________________
            //command[3] =  |  0  |  0  |  0  |  0  |  0  |  0  |  0  |  0  |  binary
            //              |_____|_____|_____|_____|_____|_____|_____|_____|
            //Buttons ---->  Front  Back  Horn   A     B     C     D     E
            //Numeration       7     6     5     4     3     2     1     0   

            //Turn left on the spot              
            if ((bitRead(prevCommands[3], 4)) != (bitRead(commands[3], 4))){
              if((bitRead(commands[3], 4)) == 1){
              left_motor.setSpeed(255);
              left_motor.run(BACKWARD);
              right_motor.setSpeed(255);
              right_motor.run(FORWARD);
              }
              else{
              left_motor.run(RELEASE);
              right_motor.run(RELEASE);
              }
            }

            //Turn right on the spot              
            if ((bitRead(prevCommands[3], 2)) != (bitRead(commands[3], 2))){
              if((bitRead(commands[3], 2)) == 1){
              left_motor.setSpeed(255);
              left_motor.run(FORWARD);
              right_motor.setSpeed(255);
              right_motor.run(BACKWARD);
              }
              else{
              left_motor.run(RELEASE);
              right_motor.run(RELEASE);
              }
            }

            //Horn
            
        }

    }
    else{
      //Resetting the Serial port (clearing the buffer) in case the bytes are not being read in correct order.
       BTSerial.end();
       BTSerial.begin(9600);
    }

  }
  
  else{//comment this whole else if tank doesn't work with batteries
    timer0 = millis();  //Get the current time (millis since execution started)
    if((timer0 - timer2)>400){  //Check if it has been 400ms since we received last command
      //More tan 400ms have passed since last command received, car is out of range. Therefore
      //Stop the car and turn lights off
      left_motor.run(RELEASE);
      right_motor.run(RELEASE);
    }
  }
  
}
/*
void Fire(){
  digitalWrite(pinYellowLED,LOW);
  digitalWrite(pinRedLED,HIGH);
  digitalWrite(pinLaserGun,HIGH);
  digitalWrite(SPEAKER,HIGH);
  delay(3000);
  digitalWrite(pinRedLED,LOW);
  digitalWrite(pinLaserGun,LOW);
  digitalWrite(SPEAKER,LOW);
  digitalWrite(pinYellowLED,HIGH);
  delay(250);
  digitalWrite(pinYellowLED,LOW);
  delay(250);
  digitalWrite(pinYellowLED,HIGH);
  delay(250);
  digitalWrite(pinYellowLED,LOW);
  delay(250);
  digitalWrite(pinYellowLED,HIGH);
}
*/

/*

// STAR WARS THEME
int notes[] = {
  392, 392, 392, 311, 466, 392, 311, 466, 392,
  587, 587, 587, 622, 466, 369, 311, 466, 392,
  784, 392, 392, 784, 739, 698, 659, 622, 659,
  415, 554, 523, 493, 466, 440, 466,
  311, 369, 311, 466, 392
};
int times[] = {
  350, 350, 350, 250, 100, 350, 250, 100, 700,
  350, 350, 350, 250, 100, 350, 250, 100, 700,
  350, 250, 100, 350, 250, 100, 100, 100, 450,
  150, 350, 250, 100, 100, 100, 450,
  150, 350, 250, 100, 750
};
void OpeningMelody(){
 for (int i = 0; i < 25; i++){
  tone(SPEAKER, notes[i], times[i]*2);
  delay(times[i]*2);
  noTone(SPEAKER);
 }
}
*/
/*

//SUPER MARIO BROS THEME
int notes[] = {
 1318, 1318, 1318, 1046, 1318, 1568, 784,
 1046, 784, 659, 880, 987, 932, 880, 784,
 1318, 1568, 1750, 1396, 1568, 1318, 1046, 1174, 987,
 1046, 784, 659, 880, 987, 932, 880,
 784, 1318, 1568, 1750, 1396, 1568, 1318, 1046, 1174, 987,
 1568, 1480, 1396, 1244, 1318, 830, 880, 1046, 880, 1046, 1174,
 0, 1568, 1480, 1396, 1244, 1318, 2093, 2093, 2093,
 1568, 1480, 1396, 1244, 1318, 830, 880, 1046, 880, 1046, 1174, 1244, 1174, 1046, 
};
int times[] = {
 150, 300, 150, 150, 300, 600, 600,
 450, 150, 300, 300, 150, 150, 300, 210,
 210, 150, 300, 150, 150, 300, 150, 150, 450,
 450, 150, 300, 300, 150, 150, 300,
 210, 210, 150, 300, 150, 150, 300, 150, 150, 450,
 150, 150, 150, 300, 150, 150, 150, 150, 150, 150, 150,
 0, 150, 150, 150, 300, 150, 300, 150, 600,
 150, 150, 150, 300, 150, 150, 150, 150, 150, 150, 150, 300, 450, 600,
};
int delays[] = {
 150, 300, 300, 150, 300, 600, 600,
 450, 450, 450, 300, 300, 150, 300, 210,
 210, 150, 300, 150, 300, 300, 150, 150, 450,
 450, 450, 450, 300, 300, 150, 300,
 210, 210, 150, 300, 150, 300, 300, 150, 150, 600,
 150, 150, 150, 300, 300, 150, 150, 300, 150, 150, 150,
 300, 150, 150, 150, 300, 300, 300, 150, 600,
 150, 150, 150, 300, 300, 150, 150, 300, 150, 150, 450, 450, 450, 1200,
 
};

void OpeningMelody(){
 for (int i = 0; i < 23; i++){
  tone(SPEAKER, notes[i], times[i]);
  delay(delays[i]);
 }
 noTone(SPEAKER);
 
}
*/
