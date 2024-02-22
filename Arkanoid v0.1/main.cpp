#include <iostream>
#include <math.h>
#include <vector>
#include "opencv2/features2d.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace std;
using namespace cv;	

struct Threshold {
    int minH, maxH, minS, maxS, minV, maxV;
};

//struct HSV_Color {
//    int Hue, Sat, Val;
//    HSV_Color(int H, int S, int V) :Hue(H), Sat(S), Val(V) {}
//};

Threshold setupMask(Mat inputImage = Mat());
Mat getMaskImage(Threshold TH, Mat rowImage);
int main() {
    //const Threshold BallThreshold = { 88, 151, 33, 116, 0, 232 };// blue color pen
    //const Threshold BallThreshold = { 173,0,128,255,0,255 };
    Mat inputImage{ imread("test-images/test3.png", IMREAD_COLOR) };
    const Threshold BallThreshold = setupMask();

    Mat CameraImage;//Declaring a matrix to load the frames//
    namedWindow("Video Player");//Declaring the video to show the video//
    VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//
    if (!cap.isOpened()) { //This section prompt an error message if no video stream is found//
        cout << "No video stream detected" << endl;
        system("pause");
        return -1;
    }
    SimpleBlobDetector::Params params;
    // Blob dedectoe setup
    // Change thresholds
    params.minThreshold = 0;
    params.maxThreshold = 255;
    // Filter by Area.
    params.filterByArea = true;
    params.minArea = 1'000;
    params.maxArea = 10'000'000;
    // Filter by Circularity        
    params.filterByCircularity = false;
    params.minCircularity = 0.01;
    // Filter by Convexity
    params.filterByConvexity = false;
    params.minConvexity = 0.87;
    // Filter by Inertia
    params.filterByInertia = false;
    params.minInertiaRatio = 0.01;
    // Storage for blobs
    vector<KeyPoint> keypoints;
    // Set up detector with params
    Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
    // end blob detector setup 
    // 
    //test 
    //Mat im_with_keypoints;
    //Mat im = imread("blob.jpg", IMREAD_GRAYSCALE);
    //detector->detect(im, keypoints);
    //drawKeypoints(im, keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    //// Show blobs
    //imshow("keypoints", im_with_keypoints);
    //waitKey(0);
    //
    while (true) { //Taking an everlasting loop to show the video//
        cap >> CameraImage;
        if (CameraImage.empty()) { //Breaking the loop if no video frame is detected//
            break;
        }
        imshow("Video Player", CameraImage);//Showing the video//
        //Mat maskedImage = getMaskImage(BallThreshold, CameraImage) > 0;
        Mat resultImage;
        cvtColor(getMaskImage(BallThreshold, CameraImage) > 0, resultImage, COLOR_RGB2GRAY);
        resultImage = 255 - resultImage;

        // Blurring image
        int dilateSize = 5;
            Mat element = getStructuringElement(MORPH_RECT,
                Size(2 * dilateSize + 1, 2 * dilateSize + 1),
                Point(dilateSize, dilateSize));
            dilate(resultImage, resultImage, element);
        // 

        detector -> detect(resultImage, keypoints);
        drawKeypoints(resultImage, keypoints, resultImage, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        imshow("Result Image", resultImage);

        if (waitKey(30) == 27) {
            break;
        }
    }
    cap.release();//Releasing the buffer memory//
    return 0;
}

Threshold setupMask(Mat inputImage) {
    auto const MASK_WINDOW = "Mask Settings";
    namedWindow(MASK_WINDOW);
    bool useCamera = inputImage.empty();
    // HSV range to detect blue color
    int minHue = 0, maxHue = 255;
    int minSat = 0, maxSat = 255;
    int minVal = 0, maxVal = 255;

    // Create trackbars of mask settings window
    createTrackbar("Min Hue", MASK_WINDOW, &minHue, 255);
    createTrackbar("Max Hue", MASK_WINDOW, &maxHue, 255);
    createTrackbar("Min Sat", MASK_WINDOW, &minSat, 255);
    createTrackbar("Max Sat", MASK_WINDOW, &maxSat, 255);
    createTrackbar("Min Val", MASK_WINDOW, &minVal, 255);
    createTrackbar("Max Val", MASK_WINDOW, &maxVal, 255);

    VideoCapture cap(0);
    Mat CameraImage, maskedImage;
    while (true) {
        if (useCamera) {
            cap >> CameraImage;
            if (CameraImage.empty()) { //Breaking the loop if no video frame is detected//
                break;
            }
        }
        Threshold th = { minHue, maxHue, minSat, maxSat,  minVal, maxVal };

        if (useCamera)maskedImage = getMaskImage(th, CameraImage);
        else maskedImage = getMaskImage(th, inputImage);

        imshow("Result (Masked) Image", maskedImage);
        if (waitKey(30) == 27) {
            cout << '{' << 
                minHue << ',' << maxHue << ',' << 
                minSat << ',' << maxSat << ',' << 
                minVal << ',' << maxVal << '}' << '\n';
            destroyAllWindows();
            return { minHue, maxHue, minSat, maxSat, minVal, maxVal };
        }
    }
}

Mat getMaskImage(Threshold TH, Mat rowImage) {
    Mat inputImageHSV, maskedImage, mask;
    cvtColor(rowImage, inputImageHSV, COLOR_BGR2HSV);
    if (TH.minH <= TH.maxH) {
        inRange(
            inputImageHSV,
            Scalar(TH.minH, TH.minS, TH.minV),
            Scalar(TH.maxH, TH.maxS, TH.maxV),
            mask);
    }
    else {
        Mat mask1, mask2;
        inRange(
            inputImageHSV,
            Scalar(TH.minH, TH.minS, TH.minV),
            Scalar(255, TH.maxS, TH.maxV),
            mask1);
        inRange(
            inputImageHSV,
            Scalar(0, TH.minS, TH.minV),
            Scalar(TH.maxH, TH.maxS, TH.maxV),
            mask2);
        mask = mask1 + mask2;
    }
    bitwise_and(rowImage, rowImage, maskedImage, mask);
    return maskedImage;
}