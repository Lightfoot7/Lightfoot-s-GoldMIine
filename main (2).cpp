#include "mbed.h"
#undef __ARM_FP


//Left Motor Pins
PwmOut leftMotor(PTD4);
DigitalOut leftForwardControl(PTA12);
DigitalOut leftBackwardControl(PTA4);
//Right Motor Pins
PwmOut rightMotor(PTA5);
DigitalOut rightForwardControl(PTC9);
DigitalOut rightBackwardControl(PTC8);
//Detection LED
PwmOut powerLED(LED_RED);
//IR Sensor Pins
DigitalIn leftIR(PTD0);
DigitalIn middleIR(PTD2);
DigitalIn rightIR(PTD3);
DigitalIn rightTurnIR(PTD1);
DigitalIn leftTurnIR(PTD5);
 
//Ultrasonic Sensor Pins
DigitalOut Trigger(PTE0);
DigitalIn Echo(PTE1);
 
int MEGA_MODE = 1;
void forward(float SASAGEYOO_duty) {
    leftMotor.write(SASAGEYOO_duty);
    rightMotor.write(SASAGEYOO_duty);

    leftBackwardControl = 0;
    rightBackwardControl = 0;
    leftForwardControl  = 1;
    rightForwardControl = 1;
}


void reverse(float duty) {
    leftMotor.write(duty);
    rightMotor.write(duty);

    leftForwardControl  = 0;
    rightForwardControl = 0;
    leftBackwardControl = 1;
    rightBackwardControl = 1;
}

void turnLeft(float duty) {
    leftMotor.write(duty);
    rightMotor.write(duty);

    leftForwardControl  = 0;
    rightForwardControl = 1;
    leftBackwardControl = 1;
    rightBackwardControl = 0;
}

void turnRight(float duty) {
    leftMotor.write(duty);
    rightMotor.write(duty);

    leftForwardControl  = 1;
    rightForwardControl = 0;
    leftBackwardControl = 0;
    rightBackwardControl = 1;
}

void stop() {
    leftMotor.write(0.0f);
    rightMotor.write(0.0f);

    leftForwardControl  = 0;
    leftBackwardControl = 0;
    rightForwardControl = 0;
    rightBackwardControl = 0;
}


void Force_stop() {
    leftMotor.write(0.0f);
    rightMotor.write(0.0f);

    leftForwardControl  = 1;
    leftBackwardControl = 1;
    rightForwardControl = 1;
    rightBackwardControl = 1;
}


void cornerLeft(float dutyTurnRight) {
 
    turnLeft(dutyTurnRight);
    wait_us(40000);

}

void cornerRight(float dutyTurnLeft) {
 
    turnRight(dutyTurnLeft);
    wait_us(40000);

}

Timer High_Time;
Timer Emergency_Timer;
double Echo_High_Time;
double Object_Distance;
int FSM = 0;

int main()
{
    stop();
    float period = 1.0/40000;
    float duty = 0.6;
    float SASAGEYOO_duty = 0.1;
    float dutyTurnRight = 0.8;
    float dutyTurnLeft = 0.8;



    while (true) {

        powerLED.write(0.25);
        int leftValue = leftIR.read();
        int rightValue = rightIR.read();
        int leftTurnValue = leftTurnIR.read();
        int rightTurnValue = rightTurnIR.read();
        int middleValue = middleIR.read();

if (FSM == 0 && Echo == 0) {
            Trigger = 0;
            wait_us(1);   // Just a clean transition start
            Trigger = 1;
            wait_us(10);  // Set Trigger to High for 10 us
            Trigger = 0;

            Emergency_Timer.reset();     // we neeed thhis in case trigger goes of but echo does not due to a loose connection or any reason
            Emergency_Timer.start();

            FSM = 1;   // Finite-State Machine is on mode 1
        }

        if (FSM == 1 && Echo == 1) {
            High_Time.reset();
            High_Time.start();

            Emergency_Timer.stop();     // end the timer since if we get to this point we cant be stuck anymore

            FSM = 2;   // Finite-State Machine is on mode 2
        }

        if (FSM == 2 && Echo == 0) {
            High_Time.stop();
            FSM = 3;   // Finite-State Machine is on mode 2
        }    

        if (FSM == 3 && Echo == 0) {
            // Calculate time in us
            Echo_High_Time = High_Time.elapsed_time().count();
            // CONVERTED TO cm/us form m/s from datasheet
            Object_Distance = (Echo_High_Time * (0.034/ 2.0));
            FSM = 0;   // Finite-State Machine is on mode 2
        }    

        float Emergency_Time = Emergency_Timer.elapsed_time().count();

        /*cm and if 100 cm then 5882.35 us time to bounce back*/
        if (Object_Distance <= 40.0 ){Force_stop(); MEGA_MODE = 2;}

        if ((FSM == 1 || FSM == 2) && Emergency_Time > 60000.0){
            Emergency_Timer.stop();     // we neeed this in case trigger goes of but echo does not due to a loose connection or any reason
            Emergency_Timer.reset();
            FSM = 0;
        } // Prevent being stuck if wire is loose and echo nevcer goes high


        if (MEGA_MODE == 2){
            Force_stop();
            wait_us(75);
            cornerLeft(dutyTurnLeft);
            forward(SASAGEYOO_duty);
            wait_us(20000);
            cornerRight(dutyTurnRight);
            forward(SASAGEYOO_duty);
            wait_us(20000);

            cornerRight(dutyTurnRight);
            forward(SASAGEYOO_duty);
            wait_us(20000);

            cornerLeft(dutyTurnLeft);
            forward(SASAGEYOO_duty);
            wait_us(20000);

            MEGA_MODE = 1;
            continue;
        }




      //  redDetected();
        if (MEGA_MODE == 3){
            Force_stop();
            wait_us(75);
            MEGA_MODE = 1;
            continue;
        }


     if (MEGA_MODE == 1){
        // If 90 degree right turn is needed
        if ((leftValue == 1 && rightValue == 1 && rightTurnValue == 1 && middleValue == 1 && leftTurnValue == 0) || (leftTurnValue == 0 && leftValue == 0 && middleValue == 1 && rightValue == 1 && rightTurnValue == 1 )) {
       
            stop();
            wait_us(5);

            Force_stop();
            wait_us(75);

            cornerRight(dutyTurnRight);
        }
        // If 90 degree left turn is needed
        if ((leftValue == 1 && rightValue == 1 && rightTurnValue == 0 && middleValue == 1 && leftTurnValue == 1) || (leftTurnValue == 1 && leftValue == 0 && middleValue == 1 && rightValue == 0 && rightTurnValue == 0 )) {
         
            stop();
            wait_us(5);

            Force_stop();
            wait_us(75);

            cornerLeft(dutyTurnLeft);
        }
        // If both sensors are on BLACK, move forward
        else if (leftValue == 1 && rightValue == 1 && middleValue == 1 && leftTurnValue == 0 && rightTurnValue == 0) {
            forward(SASAGEYOO_duty);

        }
        // Left sensor on black line, turn left
        else if ( (leftTurnValue == 0 && leftValue == 1 &&  middleValue == 1 && rightValue == 0  &&  rightTurnValue == 0 )|| (leftTurnValue == 1 && leftValue == 1)) {
            turnLeft(duty);
        }
        // Right sensor on black line, turn right
        else if ( (leftTurnValue == 0 && leftValue == 0 &&  middleValue == 1 && rightValue == 1  &&  rightTurnValue == 0 )|| (rightTurnValue == 1 && rightValue == 1)) {
            turnRight(duty);
        }
             // Both sensors on WHITE, stop
        else if (leftValue == 1 && rightValue == 1 && middleValue == 1 && rightTurnValue == 1 && leftTurnValue == 1) {
            stop();
        }        }}
        
        }