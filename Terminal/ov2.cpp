#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

int main() {
    cv::Mat image = cv::imread("example.jpg", cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cout << "Could not open or find the image!" << std::endl;
        return -1;
    }

    cv::imshow("Display window", image);
    cv::waitKey(0);

    return 0;
}

