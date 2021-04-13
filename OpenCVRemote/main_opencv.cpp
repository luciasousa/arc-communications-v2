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
#include <fstream>

#define buffer_size 120
#define info "/tmp/flag"

using namespace cv;
using namespace std;
using namespace boost;


string read_accident_id(){

    ofstream info_file;
    info_file.open(info);


    if(info_file.is_open()){
        return getline(info_file);
    }

    cerr << "Error handling file" << endl;


}


void dump_to_video(circular_buffer<Mat> frame_buffer, int frame_width, int frame_height){

    string accident_id = read_accident_id();
      

    VideoWriter video(accident_id + ".avi", CV_FOURCC('D','I','V','X'),10, Size(frame_width, frame_height));
    for(Mat i : frame_buffer){
        video6.write(i);
    }
    video.release();
    
    remove("/tmp/flag");
    video.release();
    cout << "done dumping video" << endl;

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

    for(;;){ //infinite loop that saves 1000 frames
        if(cap.read(frame) == false){
            cout << "something went wrong" << endl;
        }
        
        if(!frame.empty()){
            frame_buffer.push_back(frame.clone());
            if(file_exists("/tmp/flag")){
                cout << "warning received" << endl;
                dump_to_video(frame_buffer, cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
                return 0;
            }
        }else{
            cout << "empty frame" << endl;
        
        }   
    }
    return 0;
}


int main() {
    while (1){
        capture_video();
    }
    return 0;
    

}


