# Posture Detection Using TinyML 🧑‍💻🧘‍♂️

Academic Project for CE4172: Tiny Machine Learning (NTU)

## Overview 
This project detects sitting posture while working on a laptop to remind users to sit upright and avoid slouching. Using an **Arduino Nano 33 Sense BLE board** with IMU sensors (Accelerometer and Gyroscope), the system captures motion data from the user's right shoulder. The data is processed using sensor fusion techniques and fed into a machine learning model that classifies postures such as **Front Slouch**, **Back Slouch**, and **Correct Posture**.

## Implementation

- **Hardware**: Arduino Nano 33 Sense BLE, IMU sensors (Accelerometer and Gyroscope)
- **Posture Types**: Correct Posture, Front Slouch, Back Slouch, Right Slouch, Left Slouch
- **Model**: A two-layer neural network with **92.5%** test accuracy, optimized for edge deployment using TensorFlow Lite.

### Process:
1. Data from IMU sensors is processed to compute pitch and roll.
2. The data is classified by a machine learning model.
3. Notifications are sent to remind users to correct their posture.

## Data Collection

Data was collected for different postures, with **119 samples per second**. The sensor fusion method computes pitch and roll, which are more reliable for posture detection than raw sensor values.

## Model

- **Accuracy**: 92.5% on test data 🏅
- **Size**: The model was quantized to reduce size by **75%**, making it efficient for edge deployment. 💾
- **Runtime**: Inference time was optimized, ensuring quick feedback to users. ⏱️

## Future Work

- Use **two Arduino boards** for better posture detection.
- Improve sensor fusion techniques with a **Kalman filter**.
- Expand the model to work for more users and provide real-time posture analytics.
