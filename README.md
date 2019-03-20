# Test_ADS1X15
Arduino sketch to test an ADS1015 or ADS1115 breakout board

This repository contains an Arduino sketch that is used to test an Adafruit
ADS1015 or ADS1115 analog-to-digital converter (ADC) breakout board. 

This is a simple test to make sure the Arduino is communicating reliably
with the ADS1015 or ADS1115 over I2C and that it is measuring voltages
correctly. On each pass of the loop() function, it does the following:
 
  - Writes 64 incrementing values to the High Threshold register. After
    each write, it reads the same register and verifies that the read
    value is the same as the write value.
 
  - For each PGA value, reads each of the four inputs in single-ended
    mode and each of the pairs (0/1 and 2/3) in differential mode
 
It should be run with the Arduino IDE's Serial Monitor open, and with
its baud rate set to 57600.
 
Each of the register tests that passes prints a dot (".") and after the
64 tests, a count of the cumulative number of passed and failed register
tests is printed. Although it tries to keep going when there is an
error, some errors will cause it to hang.
 
The voltage values are reported in microvolts (uV). If the ADC value is
saturated at the maximum, the voltage is reported as \*\*MAX\*\*. If the ADC
value is saturated at the minimum (maximum negative value for
differential), the voltage is reported as \*\*MIN\*\*. The test has no idea
what voltage to expect, so no checking is done on the values. For the
results to be meaningful, the input pins should be tied to known
voltages.
 
Setup:
  - Install the Adafruit ADS1X15 library:
  
      From the Arduino IDE,
      
         Sketch->Include Library->Manage Libraries…
         
         Search for “Adafruit”
         
         Click on the one with the name “Adafruit ADS1X15”
         
         Click on the Install button.
         
  - If needed, make changes to the sketch:
  
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

 If you are testing an ADS1015, you must change the #define for
 ADS1015_MODE to "true".
 
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

     You can also change the value of SERIAL_BAUD to something other than
     57600 if desired.
     
     You must change the value of ADS1X15_ADDR to something other than its
     default of 0x48 if the ADDR pin is being used to override the default
     I2C address. Only one ADS1X15 is tested at a time even if there are 
     more than one on the I2C bus.
