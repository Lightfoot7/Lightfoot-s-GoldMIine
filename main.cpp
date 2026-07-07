#include "mbed.h"
#include "TextLCD.h"
#include "Timer.h"


AnalogIn analog_input(p17);             // Read the unfiltered analog data form the analog pin on the MCU
AnalogOut digital_output(p18);         // Read the filtered, averaged, and 8 leveled digital output ;)
TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);

 // Initialise the digital pin LED1 as an output
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

int input_index = 0;            
int OnePulseMan = 0;
float index[5]; 
float rolling_avg = 0.0;                                // Create an array od the values that will be averaged
float Yi_1 = 0.0;     // initalsie it to the first value
float alpha = 0.4;
float Period = 0.0;
float BLINKING_RATE = 0;
int Peak_1 = 0, Peak_2 = 0; 
float HighThreshold = 4.0; 
float LowThreshold  = 1.7; 
int BPM = 0;

int Final_Value = 0;
float Max_input = 0.515, Min_input = 0.485;         // Th input goes from 0.475 to 0.525 which we need to scale it to be larger to go from 0 to 1
Timer timer;           // Create a timer object

void Period_Measure()
{
    if(Final_Value > HighThreshold && Peak_1 == 0){
        Peak_1 = 1;       // capture start time 
        timer.start();       // capture start time 
        
        led1 = !led1;                                       // Turn on the mbed LED
        led2 = !led2;
        led3 = !led3;
        led4 = !led4;
       
        lcd.cls() ;                                         // CLEAR ALL TEXT ON LCD

        thread_sleep_for(Period*500);                       // Wait for half a period to allow time for the led to be on

        led1 = !led1;                                       // Turn off the mbed LED
        led2 = !led2;
        led3 = !led3;
        led4 = !led4;

        thread_sleep_for(Period*500);                        // Wait for half a period to allow time for the led to be off

    }

    if(Final_Value < LowThreshold  && Peak_1 != 0){Peak_1 = 2;}        // If the pulse passes Lowthreshold after passing  HighThreshold, then Set Peak_2 to 2 to allow the detection of the 2nd Highthreshold
    if(Final_Value > HighThreshold && Peak_1 == 2 && Peak_2 == 0){     // If the pulse passes the HighThreshold again after passing it once and passing LowThreshold, only then the period is complete              
        Peak_1 = 1;                   // Set Peak_1 back to 1 to prevent a pulse detection twice                                    
        Peak_2 = 1;                   // Set Peak_1 back to 1 to prevent a pulse detection twice     
        Period = timer.elapsed_time().count() / 1000000.0;       // capture end time 
        timer.stop();           // Stop timer to complete the period cycle
        timer.reset();       // Reset timer to get ready for the next period
    }
}



int main()
{
    while (true) {

        float unfiltered_input = analog_input.read(); //.read  gives a value berween 0.0 and 1.0 so multiplying by 4095 will give a value between 0 and 4095 cuz 12 bit ADC cuz 2 power 12 is 4096

        // Scale the input to a range of values from 0.0 to 1.0
        unfiltered_input = (unfiltered_input - Min_input)/(Max_input - Min_input);          

        // Apply the filter equation
        float filtered_output = (alpha * unfiltered_input) + (1 - alpha) * Yi_1;
        Yi_1 = filtered_output;                   // Assigning previous output to new output

        // Apply the rolling average
        index[input_index] = filtered_output;         // Assign the data to the array
        input_index = (input_index + 1) % 5;          // Go to the next value in the array and make sure it loops back to 0 so it doesn't give a bound error

        rolling_avg = 0.0;
        for(int i = 0; i < 5; i++) {                  // Add up the last 5 samples
            rolling_avg = rolling_avg + index[i];
        }
        float final_rolling_avg = rolling_avg / 5.0;      // Calculate the average

        Final_Value = round(final_rolling_avg * 7.0); // Assuming the input range is [0, 1]


        if(Final_Value > 7) {Final_Value = 7;}     //Ensure the digital pulse doesnt exceed the maximum value
        if(Final_Value < 0) {Final_Value = 0;}     //Ensure the digital pulse doesnt go below the minimum value
            
        Period_Measure();
  
        if(Period > 0.0){BPM = 60.0/Period;}                  // The Beats per minute is 60 divided by the difference in time between the peaks

        lcd.printf("BPM: %d\n", BPM);                        // Print the BPM on the LCD
        lcd.printf("Period: %d\n", (int)(Period * 1000)); 

        // Update the digital output level
        digital_output.write(Final_Value / 7.0);            //DIVIDE INTO 8 LEVELS 

        if(Peak_1 != 0 && Peak_2 != 0){Peak_1 = 0; Peak_2 = 0;}      // If both peaks are detected then reset them to get ready for the new cycle
         
        thread_sleep_for(10);               // Wait for 10 milliseconds to prevent errors
    }
    
}

