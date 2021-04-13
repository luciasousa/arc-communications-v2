
#include <fstream>
#include <curl/curl.h>
#include <time.h>
#include <iostream>

#define server_url "http://peiarc.ddns.net/add_video"

using namespace std;

int send_file(string accident_id, string accident_id_file){

    
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

int main(int argc, char* argv[])
{

send_file("abc123", "abc123.avi");
return 0;

}