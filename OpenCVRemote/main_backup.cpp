#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <boost/circular_buffer.hpp>
#include <sys/stat.h> 
#include <fstream>
#include <curl/curl.h>
#include <time.h>
#include <iostream>
#include <tuple>




#define buffer_size 150
#define info "/tmp/flag"
#define stream "rtsp://admin:Adminadmin@192.168.2.2:554/cam/realmonitor?channel=1&subtype=1"
#define server_url "http://peiarc.ddns.net/add_video"
#define interval 500


using namespace cv;
using namespace std;
using namespace boost;





int send_file(string accident_id, string ip, string accident_id_file){
    string url;

    if(ip == "NULL"){
        url = server_url;
    }else{
        url = ip;
    }
    
    FILE *log = fopen("libcurl_log.txt", "wb");
    
    cout << url << endl;

    std::string contents;
    std::ifstream in(accident_id_file, std::ios::in | std::ios::binary);

    // using a stream to read binary data from the movie
    if (in)
    {
        in.seekg(0, std::ios::end); // going to the end of the file to see its size
        contents.resize(in.tellg()); //resizing the string to the size of the binary
        in.seekg(0, std::ios::beg); //back to the beggining
        in.read(&contents[0], contents.size()); //read the file into the string
        in.close();
    }
    


    CURL *curl;
    CURLcode response;

    struct curl_httppost *header = NULL; // initializing header to NULL is required per libcurl documentation
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] =  "Expect:"; // request begins with "Expect: "

    curl_global_init(CURL_GLOBAL_ALL);

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

    

    //init curl object
    curl = curl_easy_init();

    headerlist = curl_slist_append(headerlist, buf);
    if (curl) {
        //define url
        
        curl_easy_setopt(curl, CURLOPT_URL, server_url);
       
        
        //appending header to request
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, header);
        // send the request, receive response
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

}


std::tuple<string,string> read_accident_id(){
    // reads flag file and returns the accident id which is written by another service

    std::tuple<string, string> inf;
    inf = make_tuple("a", "b");
    ifstream info_file;
    info_file.open(info);
    
    if(info_file.is_open()){
        getline(info_file, get<0>(inf));
        getline(info_file, get<1>(inf));
    }

    info_file.close();

    return inf;
    

}


void dump_to_video(circular_buffer<Mat> frame_buffer, int frame_width, int frame_height, int fps){
    //receives the frame buffer and several settings
    //dumps everything into an .avi file and sends it
    std::tuple<String, String> inf = read_accident_id();
    

    String accident_id = get<0>(inf);
    String ip = get<1>(inf);

    
    cout << accident_id << endl;
    cout << ip << endl;
    
    String accident_id_file = accident_id + ".avi";
    

    cout << fps << endl;      

    VideoWriter video(accident_id_file, CV_FOURCC('D','I','V','X'),fps, Size(frame_width, frame_height));
    for(Mat i : frame_buffer){
        video.write(i);
    }
    video.release();
    
    remove("/tmp/flag");
    cout << "done dumping video" << endl;

    send_file(accident_id, ip, accident_id_file);
    remove(accident_id.c_str());
    
}

inline bool file_exists (const std::string& name) {
    //according to the internet this is the fastest way to check for file existence
    struct stat buffer;   
    return (stat (name.c_str(), &buffer) == 0); 
}

int capture_video(){
    //brains of the operation. Open the stream, capture frames, save frames, detect the flag
    // start the dump/send routine

    boost::circular_buffer<Mat> frame_buffer{buffer_size};

    Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap(stream);
    // open the default camera using default API
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cout << "capturing frames" << endl;

    for(;;){ //infinite loop that saves 100 frames
        if(cap.read(frame) == false){
            cout << "something went wrong" << endl;
        }
        
        if(!frame.empty()){
            frame_buffer.push_back(frame.clone());
            if(file_exists("/tmp/flag")){
                cout << "warning received" << endl;
                dump_to_video(frame_buffer, cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT)
                , cap.get(CV_CAP_PROP_FPS));
                return 0;
            }
        }else{
            cout << "empty frame" << endl;
        
        }
        
           
    }
    

    return 0;


}


int main() {
    
    // Clear flag if there is one

    remove(info);
  
    while (1)
    {
        capture_video();
    }
    
    return 0;
    

}


