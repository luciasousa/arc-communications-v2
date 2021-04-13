#include "uthash.h"
#include "client_functions.c"
#include <stdio.h> 
#include <stdlib.h> 
#include "json.h"
#include "time.h"
#include <unistd.h>
#include <math.h>

#define local_port "4000"


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

void accident(){
    for(;;){
        int sock;

        if ((sock = client("localhost", local_port)) < 0)
        {
            printf("\nError creating client\n");
            exit(2);
        }

        //printf("sock: %d\n", sock);

        char *p = create_json(0);
        if (send(sock, p, strlen(p), 0) == -1)
        {   
            printf("Error sending message");
        }
        sleep(7);
    }
}

void local_test(){
    for(;;){
        char* p = create_json(0);
        printf(p);
        printf("\n");
        sleep(3);
    }
}

void sequential_generator(){

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
    
    int i;

    for(i = 1; i<10; i++){
        double x, y;
        x = coords[i][0];
        y = coords[i][1];

        json_object *info = json_object_new_object();
        json_object *coords = json_object_new_object();
        json_object *lat = json_object_new_double(x);
        json_object *lon = json_object_new_double(y);
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

        int sock;

        if ((sock = client("localhost", local_port)) < 0)
        {
            printf("\nError creating client\n");
            exit(2);
        }

        //printf("sock: %d\n", sock);
        if (send(sock, jsontxt, strlen(jsontxt), 0) == -1)
        {   
            printf("Error sending message");
        }
        sleep(30);


    }




}

int main()
{
    
    sequential_generator();
    return 0;

}
