#include "CameraCalibration.h"
#include "ChessboardDetection.h"
#include "AugmentedReality.h"
#include "FeatureDetection.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>

// Global atomic variable to store the key pressed
std::atomic<char> keyPressed(' ');

// Function to capture key input from the console
void captureKeyInput() {
    char key;
    while (true) {
        std::cin >> key;
        keyPressed.store(key);
        if (key == 'q') {  // 'q' to exit key input loop
            break;
        }
    }
}

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    // Start a separate thread for capturing key input from the console
    std::thread keyInputThread(captureKeyInput);

    cv::Size patternSize(9, 6); // Size of the chessboard pattern
    std::vector<cv::Point2f> corner_set; // To store the detected corners
    std::vector<std::vector<cv::Point2f>> corner_list; // To store corners for multiple frames
    std::vector<std::vector<cv::Vec3f>> point_list; // To store 3D world points for calibration
    cv::Mat frame;
    bool foundPreviously = false;
    int imageCounter = 0; // Counter for saved images

    std::cout << "Press 's' to save a calibration image. Press 'c' to perform calibration. Press 'q' to exit." << std::endl;

    // Task 3: Callibrate the Camera
    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cameraMatrix.at<double>(0, 0) = 1; // Assuming fx = 1
    cameraMatrix.at<double>(1, 1) = 1; // Assuming fy = 1
    cameraMatrix.at<double>(0, 2) = frame.cols / 2; // Assuming cx = frame width / 2
    cameraMatrix.at<double>(1, 2) = frame.rows / 2; // Assuming cy = frame height / 2

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Task 1: Detect and draw chessboard corners
        bool found = findChessboardCorners(frame, patternSize, corner_set);
        if (found) {
            if (!foundPreviously) {
                // Print corner info only when chessboard is first detected
                std::cout << "Number of corners found: " << corner_set.size() << std::endl;
                if (!corner_set.empty()) {
                    std::cout << "First corner coordinates: " << corner_set[0].x << ", " << corner_set[0].y << std::endl;
                }
            }
            foundPreviously = true;
        }
        else {
            foundPreviously = false;
        }

        // Display the frame
        cv::imshow("Frame", frame);
        cv::waitKey(1);

        // Check if any key is pressed in the console
        char key = keyPressed.load();
        if (key != ' ') {

            // Task 2: Select Calibration Images
            if (key == 's' && found) {
                corner_list.push_back(corner_set);
                std::vector<cv::Vec3f> point_set;
                for (int i = 0; i < patternSize.height; ++i) {
                    for (int j = 0; j < patternSize.width; ++j) {
                        point_set.push_back(cv::Vec3f(j, -i, 0.0f)); // Assuming each square is 1 unit
                    }
                }
                point_list.push_back(point_set);
                std::cout << "Saved calibration image with " << corner_set.size() << " corners." << std::endl;
            }

            else if (key == 'c') {
                // Perform calibration when 'c' is pressed
                if (corner_list.size() >= 5) {
                    cv::Mat distCoefficients = cv::Mat::zeros(8, 1, CV_64F);
                    std::vector<cv::Mat> rvecs, tvecs;

                    // Print camera matrix before calibration
                    std::cout << "Initial Camera Matrix:" << std::endl << cameraMatrix << std::endl;

                    // Calibrate camera
                    double reProjectionError = cv::calibrateCamera(point_list, corner_list, frame.size(), cameraMatrix, distCoefficients, rvecs, tvecs, cv::CALIB_FIX_ASPECT_RATIO);

                    // Print results
                    std::cout << "Calibration done with re-projection error: " << reProjectionError << std::endl;
                    std::cout << "Camera Matrix:" << std::endl << cameraMatrix << std::endl;
                    std::cout << "Distortion Coefficients:" << std::endl << distCoefficients << std::endl;

                    // Saving calibration data to a file
                    std::string filename = "C:\\Users\\Shi Zhang\\source\\repos\\cs5330_project04_calibration&AugmentedReality\\res\\calibration_data.csv";
                    bool success = saveCalibrationData(filename, cameraMatrix, distCoefficients, reProjectionError);

                    if (success) {
                        std::cout << "Calibration data saved to " << filename << std::endl;
                    }
                    else {
                        std::cerr << "Failed to save calibration data." << std::endl;
                    }
                }
                else {
                    std::cerr << "Not enough calibration images. Need at least 5." << std::endl;
                }
            }

            else if (key == 'i') {
                // Save the current frame as an image
                std::stringstream ss;
                ss << "C:\\Users\\Shi Zhang\\source\\repos\\cs5330_project04_calibration&AugmentedReality\\res\\frame_" << imageCounter++ << ".jpg";
                if (cv::imwrite(ss.str(), frame)) {
                    std::cout << "Saved frame as " << ss.str() << std::endl;
                }
                else {
                    std::cerr << "Failed to save frame." << std::endl;
                }
            }

            keyPressed.store(' ');  // Reset the key

        }

        if (key == 'q') {
            break; // Exit on 'q'
        }    
    }

    // Wait for the key input thread to finish
    if (keyInputThread.joinable()) {
        keyInputThread.join();
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
