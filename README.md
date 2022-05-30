<div id="top"></div>


<!-- PROJECT LOGO -->
<br />
<div align="center">

  <h3 align="center">Gesture Recognition for Zoom</h3>

  <p align="center">
    A wearable gadget that will allow users to focus on their Zoom meeting rather than messing with the mouse to operate tiny on-screen buttons
    <br />
    <a href="#"><strong>View Demo »</strong></a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <!-- <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li> -->
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

<!-- [![Product Name Screen Shot][product-screenshot]](https://example.com) -->

Covid-19 has forced many institutions to relocate educational activities to platforms like Zoom. However, many reports suggest that the lack of interactive features significantly lowers the effectiveness as well as learning experience. The main purpose of this project is to ease this
situation by providing a more interactive environment for learning.

Another use case is for device this device is for meetings where users often keep the camera at a certain distance to have space to prepare necessary documents or simply to get a better viewing angle. In such cases, reaching for keyboard and mouse is disruptive and inconvenience.

This device would also be appreciated by users who are yet to familiar with platforms like Zoom. Although Zoom did provide a user-friendly interface, remembering all of its function might be daunting, especially for ones who have little-to-none experience in using computer like primary schoolers, seniors, etc. All things considered, gesture provide a much more natural and convenience way to control
compared to traditional use of keyboard and mouse.

Our device take advantage of gesture recognition to provide a more immersive learning experience. User can utilize this device to perform basic operations on Zoom by using hand gesture instead of keyboard and mouse. In this project, we aim to deliver following functions:
* Mute/Unmute microphone.
* Turn camera on/off.
* Raise/Lower hand.


<p align="right">(<a href="#top">back to top</a>)</p>



### Built With

Due to its integrated 3D digital linear acceleration sensor LSM9DS1, the **Arduino Nano 33 BLE Sense** will serve as our primary development board. **Tensorflow lite** will also be utilized in the development of our machine learning model for gesture recognition. We will choose C++ as our programming language because of its quick computation and memory-efficiency, with the help among several Arduino community libraries. 

* [Arduino Nano 33 BLE Sense](https://docs.arduino.cc/hardware/nano-33-ble-sense)
* [Arduino_LSM9DS1](https://www.arduino.cc/reference/en/libraries/arduino_lsm9ds1/)
* [Arduino_TensorflowLite](https://www.arduino.cc/reference/en/libraries/arduino_tensorflowlite/)
* [Google Colab](https://drive.google.com/file/d/1iYOik6-nWXY7YByqHToWi4ziPltngjGt/view?usp=sharing)


<p align="right">(<a href="#top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

These are the instructions on setting up the project locally. To get a local copy up and running follow these simple steps.

### Prerequisites

We use VScode as our project IDE with PlatformIO IDE extension
* [Visual Studio Code](https://code.visualstudio.com/)
* [Platform IO](https://platformio.org/)

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/TDung939/Gesture-recognition-for-Zoom.git
   ```
2. Open VSCode Extension Manager

3. Search for official PlatformIO IDE extension

4. Install PlatformIO IDE.


<p align="right">(<a href="#top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

1. Connect the Arduino board to the computer

2. Check the hardware connection with the *blink.cpp* sketch. The built-in LED should be blinking.
    ```ssh
    pio run -e hardware -t upload
    ```
3. Next, we need to capture the data from the LSM9DS1. Upload the *captureData.cpp* sketch to our board.
    ```ssh
    pio run -e capture -t upload
    ```
4. Let'start with the **raise/lower hand** function as an example. Attach the board to your hand and open the serial monitor. Raise your hand and you should see the captured data shown on the monitor. Repeat the motion 15-20 times.

5. Copy all the captured data in the serial monitor and paste into the **hand.csv** in the *data* folder.

6. Repeat steps 4 and 5 for the two other functions with different motions.

7. Now we need to train our ML model with the captured data. Open & Make a copy of this [Google Colab](https://drive.google.com/file/d/1iYOik6-nWXY7YByqHToWi4ziPltngjGt/view?usp=sharing) file. Upload the *data* folder from this project to the Google Colab. *Runtime>Run All* to train our model.

8. You will see the generated **model.h** file. Download and put the file inside the *include* folder.

9. Now you can verify the model by uploading the **verifyModel.cpp** sketch.
    ```ssh
    pio run -e verify -t upload
    ```
10. When all is well, you can upload the **main.cpp** sketch. Join a new meeting on Zoom and go crazy.
    ```ssh
    pio run -e nano33ble -t upload
    ```

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments
* [Tensorflow Lite for Micro - Magic wand example](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/magic_wand)
* [Harvard TinyML Edx](https://www.edx.org/professional-certificate/harvardx-tiny-machine-learning)
* [TinyML book](https://www.oreilly.com/library/view/tinyml/9781492052036/)
* [Getting started with Microcontrollers | Tensorflow Lite](https://www.tensorflow.org/lite/microcontrollers/get_started_low_level)
* [Best-README-Template](https://github.com/othneildrew/Best-README-Template)

<p align="right">(<a href="#top">back to top</a>)</p>
