#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <boost/circular_buffer.hpp>
#include <sys/stat.h> 
#include <fstream>
#include <curl/curl.h>
#include <time.h>
#include <iostream>
#include <tuple>
#include <iostream>
#include <vector>

#define rtsp_url "rtsp://admin:Adminadmin@192.168.2.2:554/cam/realmonitor?channel=1&subtype=1"

using namespace cv;
using namespace std;
using namespace boost;

string server_url;

int capture_frames(string url){

    //initialize video capture
    VideoCapture cap(url);
    // open the default camera using default API
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    //generate and display frames
    while(true){
        Mat frame;
        cap >> frame; //take frame from camera
        if(frame.empty()){
            break; // end of video stream
        }
        cout << "capturing frames" << endl;
        buffer = imencode('.jpg', frame);
        frame = buffer.tobytes();

        //FIXME: python to c++
        //concat frame one by one and show result
        //gera frames enquanto está vivo
        //display frames
        //yield (b'--frame\r\n'b'Content-Type: image/jpg\r\n\r\n' + frame + b'\r\n')
        //b' ' + frame + b' ' concatenates 2 strings with frame -> b = bytes
        char b1[] = "--frame\r\n";
        char b2[] = "Content-Type: image/jpg\r\n\r\n";
        char b3[] = "\r\n";
        co_yield b1 + b2 + frame + b3;
        
    }

    return 0;
}


int send_frames(string ip){
    string url;

    if(ip == "NULL"){
        url = server_url;
    }else{
        url = ip;
    }
    
    //initialize some of the libcurl functionality globally (do ONCE)
    //TODO: curl_global_init(CURL_GLOBAL_ALL); //initialize all known internal sub modules
    
    //EASY INTERFACE
    //EASY HANDLE -- use one handle for every thread you plan to use for transferring
    // -- never share the same handle in multiple threads
    CURL *curl = curl_easy_init();
    
    if (curl) {
        //set properties and options for this handle
        //how the tranfers will be made
        //url identifies a remote resource -- rtsp url -- video
        curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());

        //pass all data to this function: capture_frames
        //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_frames);

        //control data
        //curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &internal_struct);

        //upload data to remote site
        //1-curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
        //2-curl_easy_setopt(easyhandle, CURLOPT_READFUNCTION, read_function);
        //3-curl_easy_setopt(easyhandle, CURLOPT_UPLOAD, 1L);

        //connect to remote site, do stuff and receive the transfer
        response = curl_easy_perform(curl);
        /* Check for errors */
        if (response != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(response));

        // as per libcurl documentation it is imperative to clean all the object used
        curl_easy_cleanup(curl);
        curl_formfree(header);
        curl_slist_free_all(headerlist);
    }

    //HTTP requests needs headers
    /*struct curl_httppost *header = NULL; // initializing header to NULL is required per libcurl documentation
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] =  "Expect:"; // request begins with "Expect: "


    // set up the header
    curl_formadd(&header,
        &lastptr,
        CURLFORM_COPYNAME, "content-type:",
        CURLFORM_COPYCONTENTS, "multipart/form-data",
        CURLFORM_END);

    curl_formadd(&header, &lastptr,
        CURLFORM_COPYNAME, "id",
        CURLFORM_COPYCONTENTS, accident_id.c_str(),
        CURLFORM_END);


    curl_formadd(&header, &lastptr,
        CURLFORM_COPYNAME, "file",
        CURLFORM_BUFFER, accident_id_file.c_str(),
        CURLFORM_BUFFERPTR, contents.data(),
        CURLFORM_BUFFERLENGTH, contents.size(),
        CURLFORM_END);

    headerlist = curl_slist_append(headerlist, buf);
    */
 

    //TODO: curl_global_cleanup(); //ONCE

    return 0;
}

static void show_usage(std::string name){
    std::cerr << "Usage:  " << name << " <option(s)>\n"
            << "Options:\n"
            << "\t-S. --stream-url \t\t RSTP stream link\n"
            << "\t-A, --api \t\t API endpoint to upload video"
            << endl;
}

int main(int argc, char** argv) {
    
    // Clear flag if there is one
    if(argc < 7){
        show_usage(argv[0]);
    return 0;
    }
    for(int i = 1; i < argc; ++i){
        string arg = argv[i];
        if((arg == "-S" ) || (arg == "--stream-url")){
            if(i + 1 < argc){
                stream_url = argv[i+1];
            }else{
                cerr << "Provide a stream URL" << endl;
                return 0;
            }
            }else if((arg == "-A" ) || (arg == "--api")){
            if(i + 1 < argc){
                server_url = argv[i+1];
            }else{
                cerr << "Provide an API endpoint" << endl;
            }
        }
    }

    remove(info);
    while (1)
    {
        capture_frames(rtsp_url);
    }
    
    return 0;
    

}




