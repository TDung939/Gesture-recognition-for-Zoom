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

const float ACCELERATION_RMS_THRESHOLD = 2.0;  // RMS (root mean square) threshold of significant motion in G's
const int NUM_CAPTURED_SAMPLES_PER_GESTURE = 119;
const int NUM_FEATURES_PER_SAMPLE = 6;
const int TOTAL_SAMPLES = NUM_CAPTURED_SAMPLES_PER_GESTURE * NUM_FEATURES_PER_SAMPLE;
const int THRESHOLD_SAMPLE_INDEX =  ((NUM_CAPTURED_SAMPLES_PER_GESTURE / 3) * NUM_FEATURES_PER_SAMPLE); // one-third of data comes before threshold

int capturedSamples = 0;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize];

// array to map gesture index to a name
const char* GESTURES[] = {
  "hand",
  "mute",
  "camera"
};

#define NUM_GESTURES 3

USBKeyboard keyboard;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // print out the samples rates of the IMUs
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.println();

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;

  // wait for threshold trigger, but keep N samples before threshold occurs
  while (1) {
    // wait for both acceleration and gyroscope data to be available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // shift values over one position (TODO: replace memmove with for loop?)
      memmove(tflInputTensor->data.f, tflInputTensor->data.f + NUM_FEATURES_PER_SAMPLE, sizeof(float) * NUM_FEATURES_PER_SAMPLE * 39);

      // insert the new data at the threshold index
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX + 0] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX + 1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX + 2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX + 3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX + 4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[THRESHOLD_SAMPLE_INDEX + 5] = (gZ + 2000.0) / 4000.0;

      // calculate the RMS of the acceleration
      float accelerationRMS =  sqrt(fabs(aX) + fabs(aY) + fabs(aZ));

      if (accelerationRMS > ACCELERATION_RMS_THRESHOLD) {
        // threshold reached, break the loop
        break;
      }
    }
  }

  // use the threshold index as the starting point for the remainder of the data
  capturedSamples = THRESHOLD_SAMPLE_INDEX + NUM_FEATURES_PER_SAMPLE;

  // collect the remaining samples
  while (capturedSamples < TOTAL_SAMPLES) {
    // wait for both acceleration and gyroscope data to be available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // insert the new data
      tflInputTensor->data.f[capturedSamples + 0] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[capturedSamples + 1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[capturedSamples + 2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[capturedSamples + 3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[capturedSamples + 4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[capturedSamples + 5] = (gZ + 2000.0) / 4000.0;

      capturedSamples += NUM_FEATURES_PER_SAMPLE;
    }
  }

  // Run inferencing
  TfLiteStatus invokeStatus = tflInterpreter->Invoke();

  if (invokeStatus != kTfLiteOk) {
    Serial.println("Invoke failed!");
    while (1);
    return;
  }

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