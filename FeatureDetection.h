#ifndef FEATURE_DETECTION_H
#define FEATURE_DETECTION_H

#include <opencv2/opencv.hpp>
#include <vector>

void detectHarrisCorners(const cv::Mat& frame);

// Function to detect and draw features on the frame
void detectAndDrawFeatures(cv::Mat& frame);

#endif // FEATURE_DETECTION_H