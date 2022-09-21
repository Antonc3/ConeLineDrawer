// ConeLineDrawer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

static int MIN_CONTOUR_AREA = 55;
static int CIRCLE_SIZE = 20;


Mat applyColorFilter(Mat img) {
    Mat hsv;
    cvtColor(img, hsv, COLOR_BGR2HSV);
    Mat thresh;
    inRange(hsv, Scalar(0, 160, 160), Scalar(255, 255, 255), thresh);
    return thresh;
}
//found this on stack overflow
bool intersectsCircle(int a, int b, int c, int x, int y, int r) {
    int dist = abs(a * x + b * y + c) / sqrt(a * a + b * b);
    if (r >= dist) return true;
    return false;
}

void extendLine(Mat *img, Point p1, Point p2) {
    int height = (*img).rows;
    int width = (*img).cols;
    double m = (double) (p1.y - p2.y) / (double) (p1.x - p2.x);
    double b = -p1.x * m + p1.y;
    //cout << "\n\n\n" << m << " " << b << endl;
    //cout << width << "WIDTHASDASSAD";
    Point np1 = Point(0,b);
    Point np2 = Point(width, m * width + b);
    line((*img), np1, np2, Scalar(0, 255, 0), 4);
}


int main()
{
    //Image path, change to wherever image is
    string image_path = "C://Users/Anton/Downloads/red.png";
    Mat img = imread(image_path,IMREAD_COLOR);
    
    if (img.empty())
    {
        cout << "Could not read the image: " << image_path << std::endl;
        return 1;
    }

    // blur image
    Mat blur;
    medianBlur(img, blur, 3);
    //apply color threshold to get brightest elements, in this case the cones.
    Mat thresh = applyColorFilter(blur);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    findContours(thresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    
    //filter out contours that are too small
    vector<vector<Point>> filtered_contours;
    for (int i = 0; i < contours.size(); i++) {
        if (contourArea(contours[i]) >= MIN_CONTOUR_AREA) {
            filtered_contours.push_back(contours[i]);
        }
    }
    cout << "contour size: " << filtered_contours.size() << endl;

    vector<Moments> mu(filtered_contours.size());
    
    vector<Point> centers(filtered_contours.size());
    //generate centers of all of the contours
    for (int i = 0; i < filtered_contours.size(); i++) {
        
        mu[i] = moments(filtered_contours[i]);
        
        centers[i] = Point(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
        //circle(thresh, centers[i], CIRCLE_SIZE, Scalar(255, 255, 255));

        //putText(img,to_string(i),centers[i],FONT_HERSHEY_DUPLEX,1.0,CV_RGB(118, 185, 0),2);
        //cout << contours[i].size() << endl;
    }

    pair<int, int> line1;
    int maxPassThrough;
    vector<int> passThrough;
    //Brute force searhing through line of best fit
    for (int i = 0; i < filtered_contours.size(); i++) {

        Point c1 = centers[i];
        for (int j = i+1; j < filtered_contours.size(); j++) {
            Point c2 = centers[j];
            int a = c1.y - c2.y;
            int b = c2.x - c1.x;
            int c = c1.x * c2.y - c2.x * c1.y;
            vector<int> passesThrough;
            for (int k = 0; k < filtered_contours.size(); k++) {

                //approximating cones to circles to make computations easier
                if (intersectsCircle(a, b, c, centers[k].x, centers[k].y, CIRCLE_SIZE)) {
                    passesThrough.push_back(k);
                }
            }
            if (passesThrough.size() > passThrough.size()) {
                line1.first = i;
                line1.second = j;
                passThrough = passesThrough;
                cout << "CURRENT I AND J " <<  i << " " << j << endl;
                cout << "size: " << passesThrough.size();
            }   
        }
    }
    pair<int, int> line2;
    vector<int> passThrough2;
    //find the second line of best fit, ignoring all of the contours that have already been used.
    //basically copy paste of the code above
    for (int i = 0; i < filtered_contours.size(); i++) {
        bool skip = false;
        for (int t = 0; t < passThrough.size(); t++) {
            if (i == passThrough[t]) {
                skip = true;
                break;
            }
        }
        if (skip) continue;
        Point c1 = centers[i];
        for (int j = i + 1; j < filtered_contours.size(); j++) {
            for (int t = 0; t < passThrough.size(); t++) {
                if (i == passThrough[t]) {
                    skip = true;
                    break;
                }
            }
            if (skip) {
                skip = false;
                continue;
            }
            Point c2 = centers[j];
            int a = c1.y - c2.y;
            int b = c2.x - c1.x;
            int c = c1.x * c2.y - c2.x * c1.y;
            vector<int> passesThrough;
            for (int k = 0; k < filtered_contours.size(); k++) {
                for (int t = 0; t < passThrough.size(); t++) {
                    if (i == passThrough[t]) {
                        skip = true;
                        break;
                    }
                }
                if (skip) {
                    skip = false;
                    continue;
                }
                if (intersectsCircle(a, b, c, centers[k].x, centers[k].y, CIRCLE_SIZE)) {
                    passesThrough.push_back(k);
                }
            }
            if (passesThrough.size() > passThrough2.size()) {
                line2.first = i;
                line2.second = j;
                passThrough2 = passesThrough;
                cout << "CURRENT I AND J " << i << " " << j << endl;
                cout << "size: " << passesThrough.size();
            }
        }
    }
    //line(img, centers[line1.first], centers[line1.second],Scalar(255,0,0),4);
    //extendLine(&thresh, centers[line1.first], centers[line1.second]);
    //drawing lines
    extendLine(&img, centers[line1.first], centers[line1.second]);

    extendLine(&img, centers[line2.first], centers[line2.second]);


    //imshow("Display window", img);

    //imshow("thresh", thresh);

    //saving file as answer.png
    imwrite("answer.png", img);
    //imwrite("thresh.png", thresh);
    int k = waitKey(0); // Wait for a keystroke in the window
    return 0;

    //std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
