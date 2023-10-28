#include "FeatureDetection.h"

void detectHarrisCorners(const cv::Mat& frame) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Mat dst, dst_norm, dst_norm_scaled;
    dst = cv::Mat::zeros(frame.size(), CV_32FC1);

    cv::cornerHarris(gray, dst, 2, 3, 0.04);
    cv::normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
    cv::convertScaleAbs(dst_norm, dst_norm_scaled);

    for (int j = 0; j < dst_norm.rows; j++) {
        for (int i = 0; i < dst_norm.cols; i++) {
            if ((int)dst_norm.at<float>(j, i) > 200) {
                cv::circle(frame, cv::Point(i, j), 5, cv::Scalar(0), 2, 8, 0);
            }
        }
    }
}
