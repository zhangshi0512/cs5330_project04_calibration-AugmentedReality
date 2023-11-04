# CS5330 Computer Vision Project: Real-Time Augmented Reality

## Project Report

### 1. Introduction

This project aims to develop a system capable of performing real-time augmented reality by detecting a chessboard pattern, calibrating the camera based on that pattern, and overlaying virtual objects onto the live video feed. The system follows a step-by-step approach, starting with corner detection and extraction, selecting calibration images, calibrating the camera, and finally augmenting virtual objects based on the current position of the camera.

---

### 2. Visual Demonstrations

- **Task 1: Chessboard Corner Detection**

  ![Chessboard Detection](res/Task1/Task1_Demo.PNG)

  *Image showing detected chessboard corners with their coordinates.*

- **Task 2: Calibration Images Selection**

  ![Calibration Image](res/Task2/Task2_Demo.PNG)

  *Image showing a selected calibration image with highlighted chessboard corners.*

- **Task 3: Camera Calibration and Error Estimation**

  ![Calibration Output](res/Task3/Task3_Demo.PNG)

  *Output showing the camera matrix, distortion coefficients, and re-projection error.*

- **Task 4: Real-Time Camera Position**

  ![Camera Position](res/Task4/Task4_Demo.PNG)

  *Display of the camera's rotation and translation data in real-time.*

- **Task 5: Projection of 3D Axes**

  ![3D Axes Projection](res/Task5/Task5_Demo.PNG)

  *Image showing the projection of 3D axes onto the chessboard.*

- **Task 6: Virtual Object Augmentation**

  ![Virtual Object](res/Task6/Task6_Demo.PNG)

  *Image displaying a virtual object augmented onto the live video feed.*

- **Task 7: Feature Detection Demo**

  ![Feature Detection](res/Task7/Task7_Demo.PNG)

  *Image demonstrating the detection of robust features on a pattern.*

---

### 3. Extensions

- **Creative Virtual Objects**
  
  Detailed descriptions and images/videos of any notably creative virtual objects and scenes created as part of the project.

- **Integration with Other Markers**
  
  Documentation of any extensions involving the integration of different markers or object types, such as ArUco markers.

- **Multi-Target Support**
  
  Information on the implementation and challenges of supporting multiple targets within a single scene.

- **Camera Comparisons**
  
  A comparison of calibrations and results across different camera models used in the project.

- **Non-Chessboard Targets**
  
  Description of how the system was adapted to work with non-chessboard targets, including example images and discussions on the challenges faced.

---

### 4. Reflection

A short section discussing what was learned during the project, any challenges that were overcome, and how the project goals were achieved.

---

### 5. Acknowledgements

Acknowledgement of any assistance or resources that were utilized during the project.

## Project Running Instructions

### Development Environment

- Operating System: [Your OS]
- Compiler/IDE: [Your Compiler/IDE]
- Libraries: OpenCV [version], [any other libraries]

### Execution Instructions

#### Step 1: Camera Calibration

- Run the `CameraCalibration` program to detect the chessboard corners and calibrate the camera.
- Press 's' to save the current frame's corner data for calibration.
- After collecting enough frames (at least 5), press 'c' to calibrate the camera.

#### Step 2: Augmented Reality Demonstration

- Run the `AugmentedReality` program to start the video loop.
- The system will detect the chessboard in each frame and overlay the 3D axes or virtual objects.

#### Step 3: Feature Detection

- Run the `FeatureDetection` program to observe robust feature detection in the video stream.
- The detected features and their locations will be displayed in real-time.

#### Additional Instructions

- [Any additional instructions for setup or execution]
- [Details on the command-line arguments or configuration files, if any]

### Troubleshooting

- [Common issues and their solutions]
- [Contact information for further support]

---

### Time Travel Days

I request to use [number] travel days for this assignment.
