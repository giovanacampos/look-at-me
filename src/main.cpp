#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include "LookAtMe.h"
#include "errors.hpp"

std::vector<cv::Rect> detect_faces(cv::Mat frame);
std::vector<cv::Rect> detect_eyes(cv::Mat frame);
void draw_eyes(cv::Mat frame, cv::Rect face, std::vector<cv::Rect>, cv::Scalar color);
void draw_faces(cv::Mat frame, std::vector<cv::Rect>, cv::Scalar color);

cv::CascadeClassifier face_cascade;
cv::CascadeClassifier eyes_cascade;

cv::Scalar blue = cv::Scalar(255, 0, 0);
cv::Scalar green = cv::Scalar(0, 255, 0);
cv::Scalar red = cv::Scalar(0, 0, 255);

int main(int argc, char *argv[]) {
    std::string major = std::to_string(LookAtMe_VERSION_MAJOR);
    std::string minor = std::to_string(LookAtMe_VERSION_MINOR);
    std::string name = "LookAtMe "+major+"."+minor;

    cv::CommandLineParser parser(argc, argv,
        "{help h||}"
        "{face_cascade f|data/haarcascades/haarcascade_frontalface_default.xml|Path to the face cascade.}"
        "{eyes_cascade e|data/haarcascades/haarcascade_eye.xml|Path to the eyes cascade.}"
        "{video v|data/videos/man-1.mp4|Path to video file or 0 for camera.}"
        );
    parser.about("\n"+name+"\nDetect faces, eyes, and direction of face.\n\n");
    parser.printMessage();

    std::string face_path = parser.get<std::string>("face_cascade");
    std::string eyes_path = parser.get<std::string>("eyes_cascade");
    std::string video_path = parser.get<std::string>("video");

    // Load the face cascader from the local data folder
    if (!face_cascade.load(face_path)) {
        std::cout << "Error loading the face cascade" << std::endl;
        return ERR_FACE_CASCADE;
    }

    // Load the eye cascader from the local data folder
    if (!eyes_cascade.load(eyes_path)) {
        std::cout << "Error loading the eye cascade" << std::endl;
        return ERR_EYE_CASCADE;
    }

    // If the input is the web camera, pass 0 instead of the video file name
    cv::VideoCapture cap(0);

    // Check if camera opened successfully
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return ERR_VID_FILE;
    }

    cv::Mat frame;

    while (true) {
        // Capture frame-by-frame
        cap >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break;

        // Setup grayscale frame.
        cv::Mat frame_gray;
        cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(frame_gray, frame_gray);

        std::vector<cv::Rect> faces = detect_faces(frame_gray);
        draw_faces(frame, faces, blue);

        cv::Rect face;
        for (size_t i = 0; i < faces.size(); i++) {
            face = faces[i];
            cv::Mat faceROI = frame_gray(cv::Rect(face.x, face.y, face.width, face.height/2));
            std::vector<cv::Rect> eyes = detect_eyes(faceROI);
            draw_eyes(frame, face, eyes, green);
        }

        // Display the resulting frame
        cv::imshow("Frame", frame);

        // Press  ESC on keyboard to exit
        char c = (char)cv::waitKey(25);
        if (c == 27)
            break;
    }

    // When everything done, release the video capture object
    cap.release();

    // Closes all the frames
    cv::destroyAllWindows();

    return OK;
}

std::vector<cv::Rect> detect_faces(cv::Mat frame_gray) {
    // Detect faces.
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(frame_gray, faces);

    return faces;
}

std::vector<cv::Rect> detect_eyes(cv::Mat frame_gray) {
    // Detect eyes.
    std::vector<cv::Rect> eyes;
    eyes_cascade.detectMultiScale(frame_gray, eyes);

    return eyes;
}

void draw_eyes(cv::Mat frame, cv::Rect face, std::vector<cv::Rect> rects, cv::Scalar color) {
    cv::Rect r;
    for (size_t i = 0; i < rects.size(); i++) {
        r = rects[i];
        cv::Point center(face.x+r.x+r.width/2, face.y+r.y+r.height/2);
        ellipse(frame, center, cv::Size(r.width/2, r.height/2), 0, 0, 360, color, 4);
    }
}

void draw_faces(cv::Mat frame, std::vector<cv::Rect> rects, cv::Scalar color) {
    cv::Rect r;
    for (size_t i = 0; i < rects.size(); i++) {
        r = rects[i];
        cv::rectangle(frame, cv::Point(r.x, r.y), cv::Point(r.x+r.width, r.y+r.height), color, 4);
    }
}