/*
 *-----------------------------------------------------------------------------
 *
 * Test_ADS1X15.ino: Simple test for ADS1015 or ADS1115
 *
 * Copyright (C) 2019  Chris Satterlee
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *-----------------------------------------------------------------------------
 * This is a simple test to make sure the Arduino is communicating
 * reliably with an ADS1015 or ADS1115 over I2C and that it is measuring
 * voltages correctly. On each pass of the loop() function, it does the
 * following:
 *
 *   - Writes 64 incrementing values to the High Threshold
 *     register. After each write, it reads the same register and
 *     verifies that the read value is the same as the write value.
 *
 *   - For each PGA value, reads each of the four inputs in single-ended
 *     mode and each of the pairs (0/1 and 2/3) in differential mode
 *
 * It should be run with the Arduino IDE's Serial Monitor open, and with
 * its baud rate set to 57600.
 *
 * Each of the register tests that passes prints a dot (".") and after
 * the 64 tests, a count of the cumulative number of passed and failed
 * register tests is printed. Although it tries to keep going when there
 * is an error, some errors will cause it to hang.
 *
 * The voltage values are reported in microvolts (uV). If the ADC value
 * is saturated at the maximum, the voltage is reported as **MAX**. If
 * the ADC value is saturated at the minimum (maximum negative value for
 * differential), the voltage is reported as **MIN**.The test has no
 * idea what voltage to expect, so no checking is done on the values.
 * For the results to be meaningful, the input pins should be tied to
 * known voltages.
 *
 ******************************************************************************
 *  If you are testing an ADS1015, you must change the #define for
 *  ADS1015_MODE to "true" below.
 ******************************************************************************
 *
 */
#include <stdio.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define ADS1015_MODE false     // Default is ADS1115; set to true for ADS1015
#define SERIAL_BAUD 57600      // Serial port baud rate
#define ADS1X15_ADDR 0x48      // I2C address (0x48 is the default)

#define ADS1X15_UNITY_GAIN_MAX_MICROVOLTS 4096000
#define MAX_ADC ((long)1 << 15) - 1

int16_t ads1X15_write_val = 0;
unsigned long pass_count = 0, fail_count = 0;
float lsb_nanovolts[6];
#if ADS1015_MODE
  Adafruit_ADS1015 ads1x15(ADS1X15_ADDR);
  int multiplier = 16;
  int pos_sat_val = (1 << 11) - 1; //  2047
  int neg_sat_val = -(1 << 11);    // -2048
#else
  Adafruit_ADS1115 ads1x15(ADS1X15_ADDR);
  int multiplier = 1;
  int16_t pos_sat_val = (1 << 15) - 1; //  32767
  int16_t neg_sat_val = -(1 << 15);    // -32768
#endif

void setup()
{
  long max_microvolts;
  Serial.begin(SERIAL_BAUD);
  ads1x15.begin();
  Serial.println(F("This sketch tests the communication and voltage"));
  Serial.println(F("measuring ability of an ADS1X15. If it is working,"));
  Serial.println(F("you should first see a line of dots with a count of"));
  Serial.println(F("passed and failed tests. Following that, you should"));
  Serial.println(F("see six lines of voltage measurements for all"));
  Serial.println(F("combinations of PGA value and channel or channel"));
  Serial.println(F("pair. This repeats indefinitely."));
  
  Serial.println(F(""));
#if ADS1015_MODE
  Serial.println(F("*** Testing ADS1015 ***"));
#else
  Serial.println(F("*** Testing ADS1115 ***"));
#endif
  Serial.println(F(""));
  Serial.print(F("If you don't see anything after this line, the "));
  Serial.println(F("communication is totally broken."));
  Serial.flush();
  for (int pga = 0; pga < 6; pga++) {
    if (pga == 0) {  // 2/3 gain
      max_microvolts = (ADS1X15_UNITY_GAIN_MAX_MICROVOLTS * 3) / 2;
    } else {
      max_microvolts =
        ADS1X15_UNITY_GAIN_MAX_MICROVOLTS / (1 << (pga - 1));
    }
    lsb_nanovolts[pga] =
      ((float)max_microvolts / (float)(MAX_ADC + 1)) * 1000.0;
  }
}

void loop()
{
  int16_t ads1X15_read_val;
  adsGain_t ads1x15_gains[] = {GAIN_TWOTHIRDS,   // PGA = 0: +/- 6.144 V
                               GAIN_ONE,         // PGA = 1: +/- 4.096 V
                               GAIN_TWO,         // PGA = 2: +/- 2.048 V
                               GAIN_FOUR,        // PGA = 3: +/- 1.024 V
                               GAIN_EIGHT,       // PGA = 4: +/- 0.512 V
                               GAIN_SIXTEEN};    // PGA = 5: +/- 0.256 V
  char str[100];

  // Register write/read test
  for (int ii = 0; ii < 64; ii++) {
    write_register(ADS1X15_ADDR,
                   ADS1015_REG_POINTER_HITHRESH,
                   ads1X15_write_val);
    ads1X15_read_val = read_register(ADS1X15_ADDR,
                                     ADS1015_REG_POINTER_HITHRESH);
    if (ads1X15_read_val == ads1X15_write_val) {
      Serial.print(F("."));
      pass_count++;
    } else {
      sprintf(str, "\nERROR: wrote %u but read %u",
              ads1X15_write_val, ads1X15_read_val);
      Serial.println(str);
      fail_count++;
    }
    ads1X15_write_val++;
  }
  sprintf(str, "  %lu reg tests passed, %lu reg tests failed",
          pass_count, fail_count);
  Serial.println(str);
  Serial.flush();

  // Voltage measurement test
  for (int pga = 0; pga < 6; pga++) {
    ads1x15.setGain(ads1x15_gains[pga]);
    sprintf(str, "PGA=%d  ", pga);
    Serial.print(str);
    for (int ch = 0; ch < 4; ch++) {
      ads1X15_read_val = ads1x15.readADC_SingleEnded(ch);
      sprintf(str, "CH%d: ", ch);
      Serial.print(str);
      convert_to_microvolts(str, pga, ads1X15_read_val);
      Serial.print(str);
      Serial.print(F("  "));
    }
    ads1X15_read_val = ads1x15.readADC_Differential_0_1();
    Serial.print(F("CH0/1: "));
    convert_to_microvolts(str, pga, ads1X15_read_val);
    Serial.print(str);
    Serial.print(F("  "));
    ads1X15_read_val = ads1x15.readADC_Differential_2_3();
    Serial.print(F("CH2/3: "));
    convert_to_microvolts(str, pga, ads1X15_read_val);
    Serial.println(str);
  }
}

void write_register(uint8_t i2c_address, uint8_t reg, uint16_t value) {
  Wire.beginTransmission(i2c_address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value>>8));
  Wire.write((uint8_t)(value & 0xFF));
  Wire.endTransmission();
}

uint16_t read_register(uint8_t i2c_address, uint8_t reg) {
  Wire.beginTransmission(i2c_address);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(i2c_address, (uint8_t)2);
  return ((Wire.read() << 8) | Wire.read());
}

void convert_to_microvolts(char *microvolt_str,
                           int pga,
                           int ads1X15_read_val) {
  float nanovolts;
  long microvolts;
  nanovolts = (lsb_nanovolts[pga] * ads1X15_read_val) * multiplier;
  microvolts = nanovolts / 1000;
  if ((long)ads1X15_read_val == pos_sat_val) {
    sprintf(microvolt_str, "   **MAX**");
  } else if ((long)ads1X15_read_val == neg_sat_val) {
    sprintf(microvolt_str, "   **MIN**");
  } else {
    sprintf(microvolt_str, "%7ld uV", microvolts);
  }
}
