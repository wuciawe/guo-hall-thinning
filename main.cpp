#include <iostream>
#include "thinning.h"
#include "cv.h"

using namespace std;

int main() {
    cout << "hello world" << endl;

    cv::Mat img = cv::imread("../WuYuih.png", CV_8UC1);
    img = img < 128;

    TheSkeleton skel("../WuYuih.png");

    cv::Mat display(img.rows, img.cols * 2 + 10, CV_8UC1, cv::Scalar(0.0));
    cv::resize(img, display(cv::Rect(0, 0, img.cols, img.rows)), img.size());
    cv::resize(skel.getThinnedImage(), display(cv::Rect(img.cols + 10, 0, img.cols, img.rows)), img.size());
    imshow("example", display);
    cvWaitKey(0);
    cvDestroyAllWindows();
    return 0;
}