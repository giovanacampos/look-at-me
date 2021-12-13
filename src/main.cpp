#include <cmath>
#include <dlib/cmd_line_parser.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/image_transforms/draw.h>
#include <dlib/opencv.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

struct Euler {
    double yaw;
    double pitch;
    double roll;
};

double font_scale = 1.0;
int font_face = cv::FONT_HERSHEY_DUPLEX;

cv::Scalar magenta(255, 0, 255);
cv::Scalar white(255, 255, 255);

dlib::cv_image<dlib::bgr_pixel> crop_image(cv::Mat image, dlib::rectangle rect) {
    cv::Point p1(rect.left(), rect.top());
    cv::Point p2(rect.right(), rect.bottom());
    return image(cv::Rect(p1, p2));
}

void put_text(cv::Mat image, std::string text) {
    int baseline = 0;
    cv::getTextSize(text, font_face, font_scale, 1, &baseline);
    cv::Point p(1, image.size().height - baseline);
    cv::putText(image, text, p, font_face, font_scale, magenta, 1);
}

void save_image(cv::Mat image, std::string path) {
    bool ok = cv::imwrite(path, image);
    if (ok == false) {
        std::cout << "Unable to save image " << path << std::endl;
    }
}

std::vector<cv::Point3d> get_face_model() {
    return {
        cv::Point3d(0.0, 0.0, 0.0),
        cv::Point3d(0.0, -330.0, -65.0),
        cv::Point3d(-225.0, 170.0, -135.0),
        cv::Point3d(225.0, 170.0, -135.0),
        cv::Point3d(-150.0, -150.0, -125.0),
        cv::Point3d(150.0, -150.0, -125.0),
    };
}

cv::Mat get_camera_matrix(cv::Mat temp) {
    double focal_length = temp.cols;
    cv::Point2d center(temp.cols/2, temp.rows/2);
    return (cv::Mat_<double>(3, 3) << focal_length, 0, center.x, 0, focal_length, center.y, 0, 0, 1);
}

cv::Mat_<double> get_euler_matrix(cv::Mat rotation) {
    cv::Mat_<double> r(3, 3);
    cv::Rodrigues(rotation, r);

    double m00 = r.at<double>(0, 0);
    double m01 = r.at<double>(0, 1);
    double m02 = r.at<double>(0, 2);
    double m10 = r.at<double>(1, 0);
    double m11 = r.at<double>(1, 1);
    double m12 = r.at<double>(1, 2);
    double m20 = r.at<double>(2, 0);
    double m21 = r.at<double>(2, 1);
    double m22 = r.at<double>(2, 2);

    // set up output variables
    Euler euler_out;
    Euler euler_out2;

    if (abs(m20) >= 1)
    {
        euler_out.yaw = 0;
        euler_out2.yaw = 0;

        // From difference of angles formula
        if (m20< 0)  //gimbal locked down
        {
            double delta = atan2(m01, m02);
            euler_out.pitch = M_PI / 2.0;
            euler_out2.pitch = M_PI / 2.0;
            euler_out.roll = delta;
            euler_out2.roll = delta;
        }
        else // gimbal locked up
        {
            double delta = atan2(-m01, -m02);
            euler_out.pitch = -M_PI / 2.0;
            euler_out2.pitch = -M_PI / 2.0;
            euler_out.roll = delta;
            euler_out2.roll = delta;
        }
    }
    else
    {
        euler_out.pitch = -asin(m20);
        euler_out2.pitch = M_PI - euler_out.pitch;

        euler_out.roll = atan2(m21 / cos(euler_out.pitch), m22 / cos(euler_out.pitch));
        euler_out2.roll = atan2(m21 / cos(euler_out2.pitch), m22 / cos(euler_out2.pitch));

        euler_out.yaw = atan2(m10 / cos(euler_out.pitch), m00 / cos(euler_out.pitch));
        euler_out2.yaw = atan2(m10 / cos(euler_out2.pitch), m00 / cos(euler_out2.pitch));
    }

    // return result
    return (cv::Mat_<double>(1, 3) << euler_out.yaw, euler_out.roll, euler_out.pitch);
}

int sign(double x) {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}

void draw_head_pose(cv::Mat temp, std::vector<cv::Point2d> image_points) {
    cv::Mat camera_matrix = get_camera_matrix(temp);
    cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type);

    cv::Mat rotation_vector, translation_vector;
    std::vector<cv::Point3d> model_points = get_face_model();

    cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);

    std::vector<cv::Point3d> nose_end_3d;
    std::vector<cv::Point2d> nose_end_2d;
    nose_end_3d.push_back(cv::Point3d(0, 0, 1000));

    cv::projectPoints(nose_end_3d, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_2d);

    for (unsigned long i = 0; i < image_points.size(); i++) {
        cv::circle(temp, image_points[i], 3, cv::Scalar(0, 0, 255));
    }

    cv::line(temp, image_points[0], nose_end_2d[0], cv::Scalar(255, 0, 0), 2);

    cv::Mat_<double> euler = get_euler_matrix(rotation_vector);
    double yaw = 180 * euler.at<double>(0, 2) / M_PI;
    double pitch = 180 * euler.at<double>(0, 1) / M_PI;
    double roll = 180 * euler.at<double>(0, 0) / M_PI;
    pitch = sign(pitch) * 180 - pitch;
    std::cout << "yaw=" << yaw << " pitch=" << pitch << std::endl;
    if (yaw >= -25 && yaw <= 25 && pitch >= -15 && pitch <= 15)
        put_text(temp, "LOOKING!!!");
}

int main(int argc, char **argv) {
    try {
        dlib::command_line_parser parser;
        parser.add_option("e", "Extract images of faces from the video source.");
        parser.add_option("h", "Display this help message.");
        parser.add_option("m", "Define model path to use for face landmarks.", 1);
        parser.add_option("v", "Video file location, if not provided camera is used.", 1);
        parser.parse(argc, argv);

        if (parser.option("h")) {
            parser.print_options();
            return 0;
        }

        // If video is provided then try to load it or set capture device to camera.
        cv::VideoCapture cap;
        if (parser.option("v")) {
            cap = cv::VideoCapture(parser.option("v").argument());
        } else {
            cap = cv::VideoCapture(0);
        }
        if (!cap.isOpened())
        {
            std::cerr << "Unable to connect to video source" << std::endl;
            return 1;
        }

        // Load face detection and pose estimation models.
        dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
        dlib::shape_predictor predictor;
        if (parser.option("m")) {
            dlib::deserialize(parser.option("m").argument()) >> predictor;
        } else {
            dlib::deserialize("data/models/shape_predictor_68_face_landmarks.dat") >> predictor;
        }

        cv::Mat temp;
        dlib::image_window win, win_faces;

        // Grab and process frames until the main window is closed by the user.
        int frame_count = 0;
        unsigned long w, h;
        long x, y;
        while (!win.is_closed()) {
            frame_count++;

            // Grab a frame from the video source.
            if (!cap.read(temp)) {
                break;
            }

            dlib::cv_image<dlib::bgr_pixel> cimg(temp); // Convert CV for dlib.
            std::vector<dlib::rectangle> faces = detector(cimg, 1.0); // Detect faces from the image.

            // Find the pose of each face and generate images.
            std::vector<dlib::cv_image<dlib::bgr_pixel>> images;
            dlib::full_object_detection shape;
            std::vector<dlib::full_object_detection> shapes;
            for (unsigned long i = 0; i < faces.size(); ++i) {
                shape = predictor(cimg, faces[i]);
                shapes.push_back(shape);

                // Export the frame to a JPEG file to the images/ folder.
                if (parser.option("e")) {
                    std::string path = "images/" + std::to_string(frame_count) + ".jpg";
                    save_image(dlib::toMat(cimg), path);
                }

                std::vector<cv::Point2d> image_points = {
                    cv::Point2d(shape.part(30).x(), shape.part(30).y()), // Nose tip
                    cv::Point2d(shape.part(8).x(), shape.part(8).y()), // Chin
                    cv::Point2d(shape.part(36).x(), shape.part(36).y()), // Left eye left corner
                    cv::Point2d(shape.part(45).x(), shape.part(45).y()), // Right eye right corner
                    cv::Point2d(shape.part(48).x(), shape.part(48).y()), // Left mouth corner
                    cv::Point2d(shape.part(54).x(), shape.part(54).y()), // Right mouth corner
                };

                draw_head_pose(dlib::toMat(cimg), image_points);

                images.push_back(crop_image(dlib::toMat(cimg), faces[i]));
            }

            // Display it all on the screen.
            win.clear_overlay();
            win.set_image(cimg);
            win.add_overlay(dlib::render_face_detections(shapes));

            // Get the position of the video source window.
            win.get_size(w, h);
            win.get_pos(x, y);

            // We can also extract copies of each face that are cropped, rotated upright,
            // and scaled to a standard size as shown here:
            if (images.size() > 0) {
                win_faces.set_image(dlib::tile_images(images));
            }

            // Move the faces window below the video source window.
            win_faces.set_pos(x+w, y);
        }
    }
    catch (dlib::serialization_error& e) {
        std::cout << "You need dlib's default face landmarking model file to run this example." << std::endl;
        std::cout << "You can get it from the following URL: " << std::endl;
        std::cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << std::endl;
        std::cout << std::endl << e.what() << std::endl;
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
