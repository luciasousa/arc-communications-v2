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

/*   while True:
    ret, frame = cap.read()

    cv2.imshow('frame', frame)
    if cv2.waitKey(1) and 0xFF == ord('q'):

        frame_im = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        pil_im = Image.fromarray(frame_im)
        stream = StringIO()
        pil_im.save(stream, format="JPEG")
        stream.seek(0)
        img_for_post = stream.read()    
        files = {'image': img_for_post}
        response = requests.post(
            url='/api/path-to-your-endpoint/',
            files=files
        )

        break

cap.release()
cv2.destroyAllWindows()*/

int send_frames(string ip){
    string url;

    if(ip == "NULL"){
        url = server_url;
    }else{
        url = ip;
    }
    
    FILE *log = fopen("libcurl_log.txt", "wb");
    
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
    CURL *curl = curl_easy_init();


    headerlist = curl_slist_append(headerlist, buf);
    if (curl) {
        //define url
        curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
        
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

    return 0;
}

/*void dump_to_video(circular_buffer<Mat> frame_buffer, int frame_width, int frame_height, int fps){
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
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

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-begin;
    std::cout << "Creating the video: " << elapsed_seconds.count() << "s\n";


    std::chrono::steady_clock::time_point begin2 = std::chrono::steady_clock::now();

    send_file(accident_id, ip, accident_id_file);
    remove(accident_id.c_str());

    std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds2 = end2-begin2;
    std::cout << "Uploading the video: " << elapsed_seconds2.count() << "s\n";
    
}*/

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




