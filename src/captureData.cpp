#include <Arduino.h>
#include <Arduino_LSM9DS1.h>

#include "constants.h"

float samples[TOTAL_SAMPLES];
int capturedSamples = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("aX,aY,aZ,gX,gY,gZ");
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;

  // Keep 39 samples before threshold occurs
  while (1) {
    // wait for both acceleration and gyroscope data to be available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // shift values one block (6 samples) to the left
      memmove(samples, samples + NUM_FEATURES_PER_SAMPLE, sizeof(float) * NUM_FEATURES_PER_SAMPLE * 39);

      // insert the new data from sample[39] to sample[39 + 5]
      samples[THRESHOLD_SAMPLE_INDEX] = aX;
      samples[THRESHOLD_SAMPLE_INDEX+1] = aY;
      samples[THRESHOLD_SAMPLE_INDEX+2] = aZ;
      samples[THRESHOLD_SAMPLE_INDEX+3] = gX;
      samples[THRESHOLD_SAMPLE_INDEX+4] = gY;
      samples[THRESHOLD_SAMPLE_INDEX+5] = gZ;

      // calculate the RMS of the acceleration
      float accelerationRMS =  sqrt(fabs(aX)+fabs(aY)+fabs(aZ));

      if (accelerationRMS > ACCELERATION_RMS_THRESHOLD) {
        // break the infinity loop when threshold is reached
        break;
      }
    }
  }

  // Start from [threshold index + 6] for the remainder of the data
  capturedSamples = THRESHOLD_SAMPLE_INDEX + NUM_FEATURES_PER_SAMPLE;

  // collect the remaining samples
  while (capturedSamples < TOTAL_SAMPLES) {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      samples[capturedSamples] = aX;
      samples[capturedSamples+1] = aY;
      samples[capturedSamples+2] = aZ;
      samples[capturedSamples+3] = gX;
      samples[capturedSamples+4] = gY;
      samples[capturedSamples+5] = gZ;

      capturedSamples += NUM_FEATURES_PER_SAMPLE;
    }
  }

  // print the samples in csv format
  for (int i = 0; i < TOTAL_SAMPLES; i += NUM_FEATURES_PER_SAMPLE) {
    Serial.print(samples[i+0], 3);
    Serial.print(',');
    Serial.print(samples[i+1], 3);
    Serial.print(',');
    Serial.print(samples[i+2], 3);
    Serial.print(',');
    Serial.print(samples[i+3], 3);
    Serial.print(',');
    Serial.print(samples[i+4], 3);
    Serial.print(',');
    Serial.print(samples[i+5], 3);
    Serial.println();

    // delay between each collected sample
    // the data rate of IMU is 119Hz
    // 119 samples per second => 119 per 1,000,000 microseconds => 8403 microseconds per sample 
    delayMicroseconds(8403); 
  }

  Serial.println();
}
