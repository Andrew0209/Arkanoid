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
    while (true) { //Taking an everlasting loop to show the video//
        cap >> CameraImage;
        if (CameraImage.empty()) {
            break;
        }
        imshow("Video Player", CameraImage);//Showing the video//
        Mat resultImage, drawing;
        cvtColor(getMaskImage(BallThreshold, CameraImage) > 0, resultImage, COLOR_RGB2GRAY);
        // Similar to blurring image
        int size = 5;
        Mat element = getStructuringElement(MORPH_RECT,
            Size(2 * size + 1, 2 * size + 1),
            Point(size, size));

        erode(resultImage, resultImage, element);

        // Single blob centre
        //Moments m = moments(resultImage, false);
        //Point p(m.m10 / m.m00, m.m01 / m.m00);
        //cvtColor(255 - resultImage, drawing, COLOR_GRAY2RGB);
        //circle(drawing, p, 10, Scalar(0, 0, 255), -1);
        //cout << p << '\n';

        //Multi blob centre
        Mat canny_output;
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        // detect edges using canny
        Canny(resultImage, canny_output, 0, 5, 3);
        // find contours
        findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
        //if (contours.empty()) {
        //    cout << "no blobs detected\n";
        //    continue;
        //}

        // get the moments and contours of blobs > BallTH
        const int BlobTH = 200;
        int blobCount = 0;
        vector<Moments> blobMoments;
        vector<vector<Point> > blobContours;
        for (int i = 0; i < contours.size(); i++) {
            Moments moment = moments(contours[i], false);
            if (moment.m00 > BlobTH) {
                blobMoments.push_back(moment);
                blobContours.push_back(contours[i]);
            }
        }
        blobCount = blobMoments.size();
        // get the centroid of figures.
        vector<Point2f> blobCenters(blobCount, Point2f(0, 0));
        for (int i = 0; i < blobCount; i++) {
            if (blobMoments[i].m00 != 0)blobCenters[i] = Point2f(blobMoments[i].m10 / blobMoments[i].m00, blobMoments[i].m01 / blobMoments[i].m00);
        }
        // Find largest blob

        int maxBlobIndex = 0;
        for (int i = 0; i < blobCount; i++) {
            if (blobMoments[maxBlobIndex].m00 < blobMoments[i].m00)maxBlobIndex = i;
            if (blobMoments[maxBlobIndex].m00);
        }
        // draw contours
        cvtColor(255 - resultImage, drawing, COLOR_GRAY2RGB);

        // Draw centers and contours of all blobs larger BallTH
        for (int i = 0; i < blobCount; i++){
            drawContours(drawing, blobContours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
            circle(drawing, blobCenters[i], 5, Scalar(0, 0, 255), 10); // Red circle
        }
        // Draw centers and contours of max area blob
        if (blobCount != 0) {
            drawContours(drawing, blobContours, maxBlobIndex, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
            circle(drawing, blobCenters[maxBlobIndex], 5, Scalar(0, 255, 0), 10); // Green circle
        }
        cout << blobCount << " blobs\n";
        if(blobCount != 0)
            cout << "max blob center: " << blobCenters[maxBlobIndex] << '\t' << "max area: " << blobMoments[maxBlobIndex].m00 << '\n';
        for (int i = 0; i < blobCount; i++)cout << "area: " << i << ' ' << blobMoments[i].m00 << '\n';
        imshow("Result Image", drawing);

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