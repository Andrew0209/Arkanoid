#include <iostream>
#include <math.h>
#include <vector>
#include "opencv2/features2d.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace std;
using namespace cv;

#define NONE_CENTRE_POINT Point(-1, -1)
#define CAMERA 1 

struct Threshold {
    int minH, maxH, minS, maxS, minV, maxV;
};
struct MassCentre {
    Point point;
    Moments moment;
};

const Threshold RobotTH {123,203,0,151,217,255};

Threshold setupMask(Mat inputImage = Mat());
Mat getMaskImage(Threshold TH, Mat rowImage);
int main() {
    Threshold BallThreshold;
    BallThreshold = { 0, 28, 156, 255, 0, 255 };
    //const Threshold BallThreshold = { 173,0,128,255,0,255 }; clen red
    Mat inputImage{ imread("test-images/test3.png", IMREAD_COLOR) };
    //BallThreshold = setupMask();

    Mat CameraImage;//Declaring a matrix to load the frames//
    namedWindow("Video Player");//Declaring the video to show the video//
    VideoCapture cap(CAMERA);//Declaring an object to capture stream of frames from default camera//
    if (!cap.isOpened()) { //This section prompt an error message if no video stream is found//
        cout << "No video stream detected" << endl;
        system("pause");
        return -1;
    }
    MassCentre pastBall;
    double velocity = 0;
    while (true) { //Taking an everlasting loop to show the video//
        cap >> CameraImage;
        if (CameraImage.empty()) {
            break;
        }
        flip(CameraImage, CameraImage, 1); // flip while use frontal camera
        imshow("Video Player", CameraImage); // Showing the original video
        Mat resultImage, drawing, robotTracker;
        cvtColor(getMaskImage(BallThreshold, CameraImage) > 0, resultImage, COLOR_RGB2GRAY);
        cvtColor(getMaskImage(RobotTH, CameraImage) > 0, robotTracker, COLOR_RGB2GRAY);

#pragma region BallDetection

        // Similar to blurring image
        int size = 3;
        Mat element = getStructuringElement(MORPH_RECT,
            Size(2 * size + 1, 2 * size + 1),
            Point(size, size));
        erode(resultImage, resultImage, element);

        //Multi blob ball
        Mat canny_output;
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        // detect edges using canny
        Canny(resultImage, canny_output, 0, 5, 3);
        // find contours
        findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

        // get the moments and contours of blobs > BlobTH
        const int BlobTH = 500;
        vector<Moments> blobMoments;
        vector<vector<Point> > blobContours;
        for (int i = 0; i < contours.size(); i++) {
            Moments moment = moments(contours[i], false);
            if (moment.m00 > BlobTH) {
                blobMoments.push_back(moment);
                blobContours.push_back(contours[i]);
            }
        }
        int blobCount = blobMoments.size();
        int maxBlobIndex = 0;
        MassCentre ball;
        if (blobCount < 2) {
            // Single blob detection
            ball.moment = moments(resultImage, false);
            if (ball.moment.m00 > BlobTH) {
                ball.point = Point(ball.moment.m10 / ball.moment.m00,
                    ball.moment.m01 / ball.moment.m00);
                blobCount = 1;
            }
            else ball.point = NONE_CENTRE_POINT;
        }
        else {
            // get the centroid of figures.
            vector<Point2f> blobCenters(blobCount, Point2f(0, 0));
            for (int i = 0; i < blobCount; i++) {
                if (blobMoments[i].m00 != 0)
                    blobCenters[i] = Point2f(blobMoments[i].m10 / blobMoments[i].m00,
                        blobMoments[i].m01 / blobMoments[i].m00);
            }
            // Find largest blob
            for (int i = 0; i < blobCount; i++) {
                if (blobMoments[maxBlobIndex].m00 < blobMoments[i].m00)maxBlobIndex = i;
            }

            // max blob parametars 
            ball.point = blobCenters[maxBlobIndex];
            ball.moment = blobMoments[maxBlobIndex];

            // draw contours
            // Draw centers and contours of all blobs larger BlobTH
            //for (int i = 0; i < blobCount; i++) {
            //    drawContours(drawing, blobContours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
            //    circle(drawing, blobCenters[i], 5, Scalar(0, 0, 255), 10); // Red circle
            //}
        }
#pragma endregion BallDetection
#pragma region RoborDetection
        MassCentre robot;
        const int RobotTH = 500;
        robot.moment = moments(robotTracker, false);
        if (robot.moment.m00 > RobotTH) {
            robot.point = Point(robot.moment.m10 / robot.moment.m00,
                robot.moment.m01 / robot.moment.m00);
        }
        else robot.point = NONE_CENTRE_POINT;
#pragma endregion RoborDetection

        // calculate velocity
        double vx = 0,vy = 0;
        if (ball.point != NONE_CENTRE_POINT) {
            vx = (ball.point.x - pastBall.point.x);
            vy = (ball.point.y - pastBall.point.y);
        }
        velocity = sqrt(vx * vx + vy * vy);
        cout << "velocity: " << velocity << '\n';
        // calculate future ball position
        Point ballTarget = robot.point;
        if (vy > 5)ballTarget.x = ball.point.x + (vx * (robot.point.y - ball.point.y) / vy);

        // Drawing 
        cvtColor(255 - resultImage, drawing, COLOR_GRAY2RGB);
        if (ball.point != NONE_CENTRE_POINT) {
            drawContours(drawing, blobContours, maxBlobIndex, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
            circle(drawing, ball.point, 7, Scalar(0, 255, 0), -1); // Green circle
            arrowedLine(drawing, ball.point, ball.point + (ball.point - pastBall.point) / 2, Scalar(0,0,255), 3);
        }
        if (robot.point != NONE_CENTRE_POINT) {
            rectangle(drawing, robot.point - Point(40, 20), robot.point + Point(40, 20), Scalar(0, 0, 255), -1); // Red robot rectangle
            circle(drawing, ballTarget, 5, Scalar(255, 0, 0), -1); // blue ball future
        }
        // cout << blobCount << " blobs\n";
        // cout << "max blob ball: " << ball.point << '\t' << "max area: " << ball.moment.m00 << '\n';
        
        // update ball and image
        if (ball.point != NONE_CENTRE_POINT)pastBall = ball; 
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

    VideoCapture cap(CAMERA);
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