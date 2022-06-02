// array to map gesture index to a name
const char* GESTURES[] = {
  "hand",
  "mute",
  "camera"
};

const float ACCELERATION_RMS_THRESHOLD = 2.0;  // Root Mean Square threshold of significant motion in G's
const int NUM_CAPTURED_SAMPLES_PER_GESTURE = 119;
const int NUM_FEATURES_PER_SAMPLE = 6;
const int TOTAL_SAMPLES = NUM_CAPTURED_SAMPLES_PER_GESTURE * NUM_FEATURES_PER_SAMPLE;
const int THRESHOLD_SAMPLE_INDEX =  ((NUM_CAPTURED_SAMPLES_PER_GESTURE / 3) * NUM_FEATURES_PER_SAMPLE); // one-third of data comes before threshold

