#include "CameraCalibration.h"
#include "ChessboardDetection.h"
#include "AugmentedReality.h"
#include "FeatureDetection.h"
#include "ModelLoader.h"
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

// Global flag to control the persistent display of the virtual object
std::atomic<bool> displayVirtualObjectPersistent(false);

// Global flag to control the display of features
std::atomic<bool> displayFeatures(false);

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

// Function to define 3D points for the axes
std::vector<cv::Point3f> defineAxesPoints() {
    std::vector<cv::Point3f> axesPoints;
    axesPoints.push_back(cv::Point3f(0, 0, 0));    // Origin on the chessboard
    axesPoints.push_back(cv::Point3f(3, 0, 0));    // X-axis (3 units length)
    axesPoints.push_back(cv::Point3f(0, 3, 0));    // Y-axis (3 units length)
    axesPoints.push_back(cv::Point3f(0, 0, -3));   // Z-axis (3 units length out the board)
    return axesPoints;
}

// Function to check if a point is inside a rectangle defined by four points
bool isInsideRectangle(const cv::Point2f& p, const std::vector<cv::Point2f>& rect) {
    double area1 = cv::contourArea(rect);
    double area2 = cv::contourArea(std::vector<cv::Point2f>{rect[0], rect[1], p}) +
        cv::contourArea(std::vector<cv::Point2f>{rect[1], rect[2], p}) +
        cv::contourArea(std::vector<cv::Point2f>{rect[2], rect[3], p}) +
        cv::contourArea(std::vector<cv::Point2f>{rect[3], rect[0], p});
    return std::abs(area1 - area2) < 1e-3;
}

// Function to draw virtual object
void drawVirtualObject(cv::Mat& frame, const cv::Mat& cameraMatrix, const cv::Mat& distCoefficients, const cv::Mat& rvec, const cv::Mat& tvec, const std::vector<cv::Point2f>& chessboardCorners, const cv::Size& patternSize) {
    // Define the chessboard boundary
    std::vector<cv::Point2f> chessboardBoundary = {
        chessboardCorners.front(),
        chessboardCorners[patternSize.width - 1],
        chessboardCorners.back(),
        chessboardCorners[patternSize.width * (patternSize.height - 1)]
    };

    // Define 3D coordinates of the virtual object (pyramid with a square base)
    float baseSize = 0.5f; // Size of the base of the pyramid
    float height = 0.5f; // Height of the pyramid

    // Assuming the chessboard size is 9x6 and each square is 1 unit
    float boardCenterX = patternSize.width / 2.0f - 0.5f;
    float boardCenterY = -(patternSize.height / 2.0f - 0.5f);  // negative because Y-axis points downwards in image space

    // Center pyramid on the chessboard
    float offsetX = boardCenterX;
    float offsetY = boardCenterY;

    std::vector<cv::Point3f> objectPoints = {
        cv::Point3f(-0.5f + offsetX, -0.5f + offsetY, 0), // Bottom left
        cv::Point3f(0.5f + offsetX, -0.5f + offsetY, 0),  // Bottom right
        cv::Point3f(0.5f + offsetX, 0.5f + offsetY, 0),   // Top right
        cv::Point3f(-0.5f + offsetX, 0.5f + offsetY, 0),  // Top left
        cv::Point3f(offsetX, offsetY, height)             // Tip of the pyramid
    };

    // Project the 3D points to the image plane
    std::vector<cv::Point2f> imagePoints;
    cv::projectPoints(objectPoints, rvec, tvec, cameraMatrix, distCoefficients, imagePoints);

    // Check if any projected points are outside the chessboard boundary
    for (const auto& point : imagePoints) {
        if (!isInsideRectangle(point, chessboardBoundary)) {
            // Scale down the object if any point is outside
            baseSize *= 0.75f;
            height *= 0.75f;
            objectPoints = {
                cv::Point3f(-baseSize + offsetX, -baseSize + offsetY, 0), // Bottom left
                cv::Point3f(baseSize + offsetX, -baseSize + offsetY, 0), // Bottom right
                cv::Point3f(baseSize + offsetX, baseSize + offsetY, 0), // Top right
                cv::Point3f(-baseSize + offsetX, baseSize + offsetY, 0), // Top left
                cv::Point3f(offsetX, offsetY, height) // Tip of the pyramid
            };
            cv::projectPoints(objectPoints, rvec, tvec, cameraMatrix, distCoefficients, imagePoints);
            break;
        }
    }

    // Draw the virtual object (pyramid) on the frame using lines
    for (int i = 0; i < 4; ++i) {
        cv::line(frame, imagePoints[i], imagePoints[(i + 1) % 4], cv::Scalar(0, 255, 0), 5); // Side of pyramid in green
        cv::line(frame, imagePoints[i], imagePoints[4], cv::Scalar(0, 255, 0), 5); // Edge to tip in green
    }

    // Draw base of pyramid in red with increased thickness
    for (int i = 0; i < 4; ++i) {
        cv::line(frame, imagePoints[i], imagePoints[(i + 1) % 4], cv::Scalar(0, 0, 255), 10);
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
    cv::Mat frame, cameraMatrix = cv::Mat::eye(3, 3, CV_64F), distCoefficients = cv::Mat::zeros(8, 1, CV_64F);
    bool foundPreviously = false;
    int imageCounter = 0; // Counter for saved images

    std::string calibrationFilePath = "C:\\Users\\Shi Zhang\\source\\repos\\cs5330_project04_calibration&AugmentedReality\\res\\calibration_data.csv";
    if (!readCalibrationData(calibrationFilePath, cameraMatrix, distCoefficients)) {
        std::cerr << "Failed to read calibration data." << std::endl;
        return -1;
    }

    // Load the 3D model
    std::vector<Vertex> vertices;
    std::vector<TextureCoord> textures;
    std::vector<Normal> normals;
    std::vector<Face> faces;
    std::string modelPath = "C:\\Users\\Shi Zhang\\source\\repos\\cs5330_project04_calibration&AugmentedReality\\res\\Lowpoly_tree_sample2.obj";
    if (!loadOBJModel(modelPath, vertices, textures, normals, faces)) {
        std::cerr << "Failed to load the model." << std::endl;
        return -1;
    }
    
    // Flags to control the display of 3D axes and virtual object
    bool display3DAxes = false;
    bool displayVirtualObject = false;

    // print the camera matrix from calibration file
    std::cout << "Camera Matrix:" << std::endl << cameraMatrix << std::endl;
    std::cout << "Distortion Coefficients:" << std::endl << distCoefficients << std::endl;


    // instructions for user to navigate the program
    std::cout << "Press 's' to save a calibration image. Press 'c' to perform calibration. Press 'p' to print board's pose. Press 'd' to display the virtual object persistently on the chessboard. Press 'f' to display a robust feature on the chessboard. Press 'q' to exit." << std::endl;

    std::vector<cv::Point3f> axesPoints = defineAxesPoints();
    cv::Mat rvec, tvec;
    bool solvePnP_success = false;
    std::vector<cv::Vec3f> objectPoints;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Task 1: Detect and draw chessboard corners
        bool found = findChessboardCorners(frame, patternSize, corner_set);
        if (found) {
            objectPoints.clear();
            for (int i = 0; i < patternSize.height; ++i) {
                for (int j = 0; j < patternSize.width; ++j) {
                    objectPoints.push_back(cv::Vec3f(j, -i, 0.0f));
                }
            }

            solvePnP_success = cv::solvePnP(objectPoints, corner_set, cameraMatrix, distCoefficients, rvec, tvec);
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

        // Task 5: Project 3D Axes on the Chessboard
        if (found && display3DAxes && solvePnP_success) {
            std::vector<cv::Point2f> imagePoints;
            cv::projectPoints(axesPoints, rvec, tvec, cameraMatrix, distCoefficients, imagePoints);

            // Drawing the axes on the image
            cv::line(frame, imagePoints[0], imagePoints[1], cv::Scalar(0, 0, 255), 3); // X-axis in red
            cv::line(frame, imagePoints[0], imagePoints[2], cv::Scalar(0, 255, 0), 3); // Y-axis in green
            cv::line(frame, imagePoints[0], imagePoints[3], cv::Scalar(255, 0, 0), 3); // Z-axis in blue
        }

        // Task 6: Draw Virtual Object
        if (found && displayVirtualObject && solvePnP_success) {
            // Task 6: Draw Virtual Object

            // Project vertices of the model onto the image
            std::vector<cv::Point2f> modelImagePoints;
            for (const Vertex& vertex : vertices) {
                std::vector<cv::Point3f> singlePoint = { cv::Point3f(vertex.x, vertex.y, vertex.z) };
                std::vector<cv::Point2f> projectedPoint;
                cv::projectPoints(singlePoint, rvec, tvec, cameraMatrix, distCoefficients, projectedPoint);
                modelImagePoints.push_back(projectedPoint[0]);
            }

            // Draw the projected points onto the image
            for (const auto& point : modelImagePoints) {
                cv::circle(frame, point, 2, cv::Scalar(0, 255, 0), -1);  // Draw a small green circle at each projected vertex
            }

            // For visualization purposes, draw lines between the vertices of each face.
            for (const auto& face : faces) {
                for (int i = 0; i < face.vertexIndices.size(); i++) {
                    cv::Point2f p1 = modelImagePoints[face.vertexIndices[i] - 1]; // -1 because OBJ indices are 1-based
                    cv::Point2f p2 = modelImagePoints[face.vertexIndices[(i + 1) % face.vertexIndices.size()] - 1];
                    cv::line(frame, p1, p2, cv::Scalar(255, 0, 0), 1);  // Draw line in blue
                }
            }
        }

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

                    // Save calibration data
                    if (saveCalibrationData(calibrationFilePath, cameraMatrix, distCoefficients, reProjectionError)) {
                        std::cout << "Calibration data saved to " << calibrationFilePath << std::endl;
                    }
                    else {
                        std::cerr << "Failed to save calibration data." << std::endl;
                    }
                }
                else {
                    std::cerr << "Not enough calibration images. Need at least 5." << std::endl;
                }
            }

            // Task 4: Print Board's Pose
            // control the display of all virtual object
            if (key == 'd') {
                displayVirtualObjectPersistent.store(true);
            }

            // Toggle the display3DAxes flag when 'p' is pressed
            if (key == 'p') {
                display3DAxes = !display3DAxes;
            }

            // Toggle the displayVirtualObject flag when 'o' is pressed
            if (key == 'o') {
                displayVirtualObject = !displayVirtualObject;
            }

            // Toggle the displayFeatures flag when 'f' is pressed
            if (key == 'f') {
                displayFeatures = !displayFeatures;  
            }


            keyPressed.store(' ');  // Reset the key
        }

        if (displayVirtualObjectPersistent.load() && solvePnP_success) {
            if (found) {
                drawVirtualObject(frame, cameraMatrix, distCoefficients, rvec, tvec, corner_set, patternSize);
            }
        }

        if (displayFeatures.load()) {
            detectAndDrawFeatures(frame);
        }

        // Display the frame
        cv::imshow("Frame", frame);
        cv::waitKey(1);

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
