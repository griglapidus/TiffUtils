#include "mainwindow.h"
#include "opencv2/opencv.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Load the two images
    cv::Mat image1 = cv::imread("C:\\work\\Quad_1_4_5_6.tif");
    cv::Mat image2 = cv::imread("C:\\work\\Quad_2_4_5_6.tif");

    if (image1.empty() || image2.empty()) {
        std::cerr << "Could not open one of the images!" << std::endl;
        return;
    }

    // Convert images to grayscale (optional)
    cv::Mat gray1, gray2;
    cv::cvtColor(image1, gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(image2, gray2, cv::COLOR_BGR2GRAY);

    // Compute the absolute difference between the two images
    cv::Mat diff;
    cv::absdiff(gray1, gray2, diff);

    // Apply a binary threshold to highlight differences
    cv::Mat diff_thresh;
    cv::threshold(diff, diff_thresh, 30, 255, cv::THRESH_BINARY);

    // Find contours of the differences
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(diff_thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Draw contours on the original image to highlight differences
    cv::Mat output = image1.clone();
    cv::drawContours(output, contours, -1, cv::Scalar(0, 0, 255), 2); // Red color for highlighting

    // Show the images
    // cv::imshow("Original Image 1", image1);
    // cv::imshow("Original Image 2", image2);
    cv::imshow("Difference", diff);
    cv::imshow("Highlighted Differences", output);

    // Wait until user presses a key
    cv::waitKey(0);

}

MainWindow::~MainWindow() {}
