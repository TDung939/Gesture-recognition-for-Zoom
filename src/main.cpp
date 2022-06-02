#include <Arduino.h>

#include <Arduino_LSM9DS1.h>
#include <USBKeyboard.h>
#include <PluggableUSBHID.h>

#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "model.h"
#include "constants.h"

#define NUM_GESTURES 3

int capturedSamples = 0;

tflite::MicroErrorReporter tflErrorReporter;
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize];

USBKeyboard keyboard;

void setup() {
  Serial.begin(9600);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.println();

  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  tflInterpreter->AllocateTensors();

  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;
  while (1) {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      memmove(tflInputTensor->data.f, tflInputTensor->data.f + NUM_FEATURES_PER_SAMPLE, sizeof(float) * NUM_FEATURES_PER_SAMPLE * 39);
      
      //normalize the input data, between 0 to 1:
      //acceleration is between: -4 to +4
      //gyroscope is between: -2000 to +2000
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX+1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX+2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX+3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX+4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX+5] = (gZ + 2000.0) / 4000.0;

      float accelerationRMS =  sqrt(fabs(aX) + fabs(aY) + fabs(aZ));

      if (accelerationRMS > ACCELERATION_RMS_THRESHOLD) {
        break;
      }
    }
  }

  capturedSamples = THRESHOLD_SAMPLE_INDEX + NUM_FEATURES_PER_SAMPLE;

  while (capturedSamples < TOTAL_SAMPLES) {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      tflInputTensor->data.f[capturedSamples] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[capturedSamples+1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[capturedSamples+2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[capturedSamples+3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[capturedSamples+4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[capturedSamples+5] = (gZ + 2000.0) / 4000.0;

      capturedSamples += NUM_FEATURES_PER_SAMPLE;
    }
  }

  TfLiteStatus invokeStatus = tflInterpreter->Invoke();

  if (invokeStatus != kTfLiteOk) {
    Serial.println("Invoke failed!");
    while (1);
    return;
  }

  //Key combinations could be different on MacOS
  for (int i = 0; i < NUM_GESTURES; i++) {
    if (tflOutputTensor->data.f[i] > 0.75) {
      Serial.print(GESTURES[i]);
      switch(i) {
        case 0:
          keyboard.key_code('Y', KEY_ALT);
          break;
        case 1:
          keyboard.key_code('A', KEY_ALT);
          break;
        case 2:
          keyboard.key_code('V', KEY_ALT);
          break;
      }
    }
  }
  Serial.println();
}
