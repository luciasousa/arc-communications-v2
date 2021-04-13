#include <iostream>
#include <unistd.h>
#include <cstring>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <boost/circular_buffer.hpp>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <time.h>
#include <chrono>

#define buffer_size 120

using namespace cv;
using namespace std;
using namespace boost;


void dump_to_video(circular_buffer<Mat> frame_buffer, int frame_width, int frame_height){


    cout << "Testing Codecs 100 frames at 10fps" << endl;

    //Xvid

    cout << "XVID" << endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    VideoWriter video("xvid.avi", CV_FOURCC('X','V','I','D'),10, Size(frame_width, frame_height));
    for(Mat i : frame_buffer){
        video.write(i);
    }
    video.release();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-begin;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    

    //DIVX
    cout << "DIVX" << endl;

    begin = std::chrono::steady_clock::now();
    VideoWriter video6("divx.avi", CV_FOURCC('D','I','V','X'),10, Size(frame_width, frame_height));
    for(Mat i : frame_buffer){
        video6.write(i);
    }
    video6.release();
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end-begin;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    

    //MJPG
    cout << "MJPG" << endl;

    begin = std::chrono::steady_clock::now();
    VideoWriter video2("mjpg.avi", CV_FOURCC('M','J','P','G'),10, Size(frame_width, frame_height));
    for(Mat i : frame_buffer){
        video2.write(i);
    }
    video2.release();
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end-begin;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    
    //WMV1
    cout << "WMV1" << endl;

    begin = std::chrono::steady_clock::now();
    VideoWriter video3("wmv1.avi", CV_FOURCC('W','M','V','1'),10, Size(frame_width, frame_height));
    for(Mat i : frame_buffer){
        video3.write(i);
    }
    video3.release();
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end-begin;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    
    //WMV2
    cout << "WMV2" << endl;

    begin = std::chrono::steady_clock::now();
    VideoWriter video4("wmv2.avi", CV_FOURCC('W','M','V','2'),10, Size(frame_width, frame_height));
    for(Mat i : frame_buffer){
        video4.write(i);
    }
    video4.release();
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end-begin;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

}

inline bool file_exists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

int capture_video(){

init:
    boost::circular_buffer<Mat> frame_buffer{300};

    Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap("rtsp://admin:Adminadmin@192.168.2.2:554/cam/realmonitor?channel=1&subtype=1");
    // open the default camera using default API
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cout << "capturing frames" << endl;

    for(int i = 0; i < 100; i++){ //infinite loop that saves 1000 frames
        if(cap.read(frame) == false){
            cout << "something went wrong" << endl;
        }
        
       
        frame_buffer.push_back(frame.clone());
            
    }

    dump_to_video(frame_buffer, cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    

    return 0;

}


int main() {

    capture_video();
    return 0;
    

}


