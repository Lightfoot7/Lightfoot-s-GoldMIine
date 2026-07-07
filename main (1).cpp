#undef __ARM_FP
#include "mbed.h"


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

int main()
{
    stop();
    float period = 1.0/40000;
    float duty = 0.6;
    float SASAGEYOO_duty = 0.1;
    float dutyTurnRight = 0.8;
    float dutyTurnLeft = 0.8;
    powerLED.period(2);
    leftMotor.period(period);
    rightMotor.period(period);

    while (true) {
        powerLED.write(0.25);
        int leftValue = leftIR.read();
        int rightValue = rightIR.read();
        int leftTurnValue = leftTurnIR.read();
        int rightTurnValue = rightTurnIR.read();
        int middleValue = middleIR.read();

        // If 90 degree right turn is needed
        if (leftValue == 1 && rightValue == 1 && rightTurnValue == 1 && middleIR == 1) {
            
            stop();
            wait_us(5);

            Force_stop();
            wait_us(75);

            cornerRight(dutyTurnRight);        }
        // If 90 degree left turn is needed
        else if (leftValue == 1  && rightValue == 1 && leftTurnValue == 1 && middleIR == 1) {
            cornerLeft(dutyTurnLeft);
        }
        // If both sensors are on BLACK, move forward
        else if (leftValue == 1 && rightValue == 1 && middleIR == 1) {
            forward(SASAGEYOO_duty);
        }
        // Left sensor on black line, turn left
        else if (leftValue == 1 && rightValue == 0 && middleIR == 1) {
            turnLeft(duty);
        }
        // Right sensor on black line, turn right
        else if (leftValue == 0 && rightValue == 1 && middleIR == 1) {
            turnRight(duty);
        }
        // Both sensors on WHITE, stop
        else if (leftValue == 1 && rightValue == 1 && middleIR == 1) {
            stop();
        }
    }
}