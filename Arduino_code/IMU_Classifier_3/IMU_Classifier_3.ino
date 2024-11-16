/*
  IMU Classifier
  This example uses the on-board IMU to start reading acceleration and gyroscope
  data from on-board IMU, once enough samples are read, it then uses a
  TensorFlow Lite (Micro) model to try to classify the movement as a known gesture.
  Note: The direct use of C/C++ pointers, namespaces, and dynamic memory is generally
        discouraged in Arduino examples, and in the future the TensorFlowLite library
        might change to make the sketch simpler.
  The circuit:
  - Arduino Nano 33 BLE or Arduino Nano 33 BLE Sense board.
  Created by Don Coleman, Sandeep Mistry
  Modified by Dominic Pajak, Sandeep Mistry
  This example code is in the public domain.
*/

#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>

#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <PDM.h>
#include <math.h>

#include "IMU_model.h"

const float accelerationThreshold = 1.8; // threshold of significant in G's
const int numSamples = 119;

int samplesRead = numSamples;
// Constants for complementary filter
const float alpha = 0.99;
const float dt = 1.0/119.0;

// Variables for complementary filter
float pitch = 0.0;
float roll = 0.0;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
// tflite::AllOpsResolver tflOpsResolver;

// // This pulls in the deep learning operations that  are needed for this program
// // static tflite::MicroMutableOpResolver<3> micro_resolver(error_reporter);  // choice 2 //CHANGED

// This pulls in the deep learning operations that  are needed for this program
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
static tflite::MicroMutableOpResolver<5> tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 2 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map gesture index to a name
const char* GESTURES[] = {
    "FRONT_SLOUCH",
    "BACK_SLOUCH",
    "RIGHT_SLOUCH",
    "LEFT_SLOUCH"
    // "STRAIGHT"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // print out the samples rates of the IMUs
  // Serial.print("Accelerometer sample rate = ");
  // Serial.print(IMU.accelerationSampleRate());
  // Serial.println(" Hz");
  // Serial.print("Gyroscope sample rate = ");
  // Serial.print(IMU.gyroscopeSampleRate());
  // Serial.println(" Hz");

  // Serial.println();
  
  if (tflOpsResolver.AddFullyConnected() != kTfLiteOk) {
    return;
  }
  if (tflOpsResolver.AddQuantize() != kTfLiteOk) {
    return;
  }
  if (tflOpsResolver.AddDequantize() != kTfLiteOk) {
    return;
  }
  if (tflOpsResolver.AddSoftmax() != kTfLiteOk) {
    return;
  }
  if (tflOpsResolver.AddRelu() != kTfLiteOk) {
    return;
  }

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(IMU_model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {

  float aX, aY, aZ, gX, gY, gZ;
  // sensors_event_t accel, gyro;

  // wait for significant motion
  while (samplesRead == numSamples) {
    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(aX, aY, aZ);

      // sum up the absolutes
      float aSum = fabs(aX) + fabs(aY) + fabs(aZ);

      // check if it's above the threshold
      if (aSum >= accelerationThreshold) {
        // reset the sample read count
        samplesRead = 0;
        break;
      }
    }
  }

  // check if the all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < numSamples) {
    // check if new acceleration AND gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data

      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      if (samplesRead < 70){
        float pitch = alpha * (pitch + gX * dt) + (1 - alpha) * atan2(-aX, sqrt(aY * aY + aZ * aZ)) * 180.0 / M_PI;
        float roll = alpha * (roll + gY * dt) + (1 - alpha) * atan2(aY, sqrt(aX * aX + aZ * aZ)) * 180.0 / M_PI;

        // Pass pitch and roll to the inference model
        tflInputTensor->data.f[70 * 2 + 0] = roll;
        tflInputTensor->data.f[70 * 2 + 1] = pitch;  

        samplesRead++;   

        // Serial.print(samplesRead);
      }
      else{
        samplesRead++;
      }
    
      if (samplesRead == numSamples) {
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }
        delay(1000);

        // Loop through the output tensor values from the model
        for (int i = 0; i < NUM_GESTURES; i++) {
          Serial.print(GESTURES[i]);
          Serial.print(": ");
          Serial.println(tflOutputTensor->data.f[i], 2);
        }
        Serial.println();
      }
    }
  }
}

