#include <stdio.h> 
#include <stdlib.h> 
#include "json.h"
#include "time.h"
#include <unistd.h>
#include <math.h>
#include <curl/curl.h>

#define endpoint "http://peiarc.ddns.net/add_accident"


int randoms(int lower, int upper) 
{ 
    return (rand() % (upper - lower + 1)) + lower;    
} 

double * coordinates(){

    double coords[10][2] = {{40.3896857,-7.6901815},
                            {40.189050, -8.812179},
                            {40.249183, -8.778549},
                            {40.426346, -8.707009},
                            {40.593766, -8.596150},
                            {40.8586811,-8.3765283},
                            {40.6103477,-7.9387466},
                            {40.3896857,-7.6901815},
                            {40.6103477,-7.9387466},
                            {41.1471592,-8.639845}};


    int pos_return = randoms(0,9);
    

    double *temp = malloc(sizeof(double)*2);
    temp[0] = floor(coords[pos_return][0]*100000)/100000;
    temp[1] = floor(coords[pos_return][1]*100000)/100000;

    return temp;
}

char* create_json(int random){

    double *coords_value = coordinates();

    json_object *info = json_object_new_object();
    json_object *coords = json_object_new_object();
    json_object *lat = json_object_new_double(coords_value[0]);
    json_object *lon = json_object_new_double(coords_value[1]);
    json_object *speed = json_object_new_double(randoms(40, 150));
    json_object *people = json_object_new_int(randoms(1,5));
    json_object *damage = json_object_new_double(randoms(10, 50));
    json_object *braking = json_object_new_boolean(randoms(0,1));
    json_object *airbag = json_object_new_boolean(randoms(0,1));
    json_object *overturned = json_object_new_boolean(randoms(0,1));
    json_object *id_video = json_object_new_int((int)(time(NULL)));
    json_object *temp = json_object_new_int(randoms(25, 50));


    json_object_object_add(coords, "lat", lat);
    json_object_object_add(coords, "lng", lon);
    json_object_object_add(info, "location", coords);
    json_object_object_add(info, "video_id", id_video);
    json_object_object_add(info, "velocity", speed);
    json_object_object_add(info, "n_people", people);
    json_object_object_add(info, "temperature", temp);
    json_object_object_add(info, "airbag", airbag);
    json_object_object_add(info, "ABS", braking);
    json_object_object_add(info, "overturned", overturned);
    json_object_object_add(info, "damage", damage);



    char *jsontxt = json_object_to_json_string(info);

    return jsontxt;
}

int send_information(char *buffer){
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, endpoint);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
    
        /* Perform the request, res will get the return code */ 
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    
        /* always cleanup */ 
        curl_easy_cleanup(curl);
    }
    return 0;


}


void accident(){
    for(;;){
        char *p = create_json(0);
        send_information(p);
        
    }
}

int main()
{
    
    accident();
    return 0;

}
