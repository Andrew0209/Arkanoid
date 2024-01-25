#include <iostream>
#include <math.h>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;	
struct Threshold {
    int minH, maxH, minS, maxS, minV, maxV;
};

Threshold setupMask(Mat inputImage);

int main() {

    Mat inputImage{ imread("C:/Users/37602/source/graphic/Arkanoid v0.1/test-images/test3.png", IMREAD_COLOR) };
    const Threshold BallThreshold = setupMask(inputImage);
    //const Threshold BallThreshold = { 0, 62, 43, 255, 137, 255 };

    Mat CameraImage;//Declaring a matrix to load the frames//
    namedWindow("Video Player");//Declaring the video to show the video//
    VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//
    if (!cap.isOpened()) { //This section prompt an error message if no video stream is found//
        cout << "No video stream detected" << endl;
        system("pause");
        return-1;
    }
    while (true) { //Taking an everlasting loop to show the video//
        cap >> CameraImage;
        if (CameraImage.empty()) { //Breaking the loop if no video frame is detected//
            break;
        }
        imshow("Video Player", CameraImage);//Showing the video//
        if (waitKey(30) == 27) { //If 'Esc' is entered break the loop//
            break;
        }
        Mat mask;
        // params: input array, lower boundary array, upper boundary array, output array
        inRange(
            CameraImage,
            Scalar(BallThreshold.minH, BallThreshold.minS, BallThreshold.minV),
            Scalar(BallThreshold.maxH, BallThreshold.maxS, BallThreshold.maxV),
            mask
        );
        Mat resultImage;
        // params: src1	array, src2 array, output array, mask
        bitwise_and(CameraImage, CameraImage, resultImage, mask);
        imshow("Result (Masked) Image", resultImage);
    }
    cap.release();//Releasing the buffer memory//
    return 0;


}
Threshold setupMask(Mat inputImage) {
    auto const MASK_WINDOW = "Mask Settings";
    namedWindow(MASK_WINDOW);

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

    while (true) {
        //// 2. Read and convert image to HSV color space
        Mat inputImageHSV;
        cvtColor(inputImage, inputImageHSV, COLOR_BGR2HSV);

        //// 3. Create mask and result (masked) image
        Mat mask;
        // params: input array, lower boundary array, upper boundary array, output array
        inRange(
            inputImageHSV,
            Scalar(minHue, minSat, minVal),
            Scalar(maxHue, maxSat, maxVal),
            mask
        );
        Mat resultImage;
        // params: src1	array, src2 array, output array, mask
        bitwise_and(inputImage, inputImage, resultImage, mask);

        //// 4. Show images        
       // imshow("Input Image", inputImage);
        imshow("Result (Masked) Image", resultImage);
        // imshow("Mask", mask);
        //// Wait for 'esc' (27) key press for 30ms. If pressed, end program.
        if (waitKey(30) == 27) {
            cout << '{' << minHue << ',' << maxHue << ',' << minSat << ',' << maxSat << ',' << minVal << ',' << maxVal << '}' << '\n';
            //destroyWindow("Result (Masked) Image");
            //destroyWindow("Input Image");
            //destroyWindow("Mask Settings");
            destroyAllWindows();
            return { minHue, maxHue, minSat, maxSat, minVal, maxVal };
        }
    }
}