#include "CameraCalibration.h"
#include "ChessboardDetection.h"
#include "AugmentedReality.h"
#include "FeatureDetection.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <atomic>
#include <regex>

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

// Function to read Camera Matrix from a local csv file
bool readCalibrationData(const std::string& filename, cv::Mat& cameraMatrix, cv::Mat& distCoefficients) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    std::string line;
    std::regex number_regex("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"); // Regular expression for floating point numbers
    std::smatch match;

    while (std::getline(file, line)) {
        if (line.find("Camera Matrix:") != std::string::npos) {
            // Read camera matrix
            for (int i = 0; i < 3; ++i) {
                std::getline(file, line);
                int j = 0;
                while (std::regex_search(line, match, number_regex) && j < 3) {
                    cameraMatrix.at<double>(i, j) = std::stod(match.str(0));
                    line = match.suffix().str();
                    j++;
                }
            }
        }
        else if (line.find("Distortion Coefficients:") != std::string::npos) {
            // Read distortion coefficients
            for (int i = 0; i < 5; ++i) {
                std::getline(file, line);
                if (std::regex_search(line, match, number_regex)) {
                    distCoefficients.at<double>(i, 0) = std::stod(match.str(0));
                }
            }
        }
    }

    file.close();
    return true;
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
    cv::Mat frame, cameraMatrix = cv::Mat::eye(3, 3, CV_64F), distCoefficients = cv::Mat::zeros(8, 1, CV_64F);
    bool foundPreviously = false;
    int imageCounter = 0; // Counter for saved images

    std::string calibrationFilePath = "C:\\Users\\Shi Zhang\\source\\repos\\cs5330_project04_calibration&AugmentedReality\\res\\calibration_data.csv";
    if (!readCalibrationData(calibrationFilePath, cameraMatrix, distCoefficients)) {
        std::cerr << "Failed to read calibration data." << std::endl;
        return -1;
    }

    std::cout << "Camera Matrix:" << std::endl << cameraMatrix << std::endl;
    std::cout << "Distortion Coefficients:" << std::endl << distCoefficients << std::endl;

    std::cout << "Press 's' to save a calibration image. Press 'c' to perform calibration. Press 'p' to print board's pose. Press 'q' to exit." << std::endl;

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

            // Task 3: Calibrate the Camera
            else if (key == 'c') {
                if (corner_list.size() >= 5) {
                    std::vector<cv::Mat> rvecs, tvecs;
                    double reProjectionError = cv::calibrateCamera(point_list, corner_list, frame.size(), cameraMatrix, distCoefficients, rvecs, tvecs, cv::CALIB_FIX_ASPECT_RATIO);
                    std::cout << "Calibration done with re-projection error: " << reProjectionError << std::endl;
                    std::cout << "Camera Matrix:" << std::endl << cameraMatrix << std::endl;
                    std::cout << "Distortion Coefficients:" << std::endl << distCoefficients << std::endl;
                }
                else {
                    std::cerr << "Not enough calibration images. Need at least 5." << std::endl;
                }
            }

            // Task 4: Print Board's Pose
            else if (key == 'p' && found) {
                std::vector<cv::Vec3f> objectPoints;
                for (int i = 0; i < patternSize.height; ++i) {
                    for (int j = 0; j < patternSize.width; ++j) {
                        objectPoints.push_back(cv::Vec3f(j, -i, 0.0f));
                    }
                }

                cv::Mat rvec, tvec;

                std::cout << "Object Points size: " << objectPoints.size() << std::endl;
                std::cout << "Corner set size: " << corner_set.size() << std::endl;

                bool solvePnP_success = cv::solvePnP(objectPoints, corner_set, cameraMatrix, distCoefficients, rvec, tvec);
                if (!solvePnP_success) {
                    std::cerr << "solvePnP failed to find a solution." << std::endl;
                    continue;
                }

                // Print rotation and translation data
                std::cout << "Rotation Vector: " << rvec.t() << std::endl;
                std::cout << "Translation Vector: " << tvec.t() << std::endl;
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
