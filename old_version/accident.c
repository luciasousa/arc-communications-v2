// #include "neighbour.c"
#include "uthash.h"
#include "client_functions.c"
#include "json.h"
#include "time.h"


#define local_port "4000"

char* create_json(){



    json_object *info = json_object_new_object();
    json_object *coords = json_object_new_object();
    json_object *lat = json_object_new_double(-12.9283);
    json_object *lon = json_object_new_double(33.9283);
    json_object *speed = json_object_new_double(120.5);
    json_object *n_cars = json_object_new_int(3);
    json_object *people = json_object_new_int(10);
    json_object *damage = json_object_new_double(3.34);
    json_object *braking = json_object_new_boolean(1);
    json_object *airbag = json_object_new_boolean(1);
    json_object *overturned = json_object_new_boolean(0);
    json_object *id_video = json_object_new_string("fake_id");


    json_object_object_add(coords, "lat", lat);
    json_object_object_add(coords, "lng", lon);
    json_object_object_add(info, "location", coords);
    json_object_object_add(info, "video_id", id_video);
    json_object_object_add(info, "velocity", speed);
    json_object_object_add(info, "n_people", people);
    json_object_object_add(info, "airbag", airbag);
    json_object_object_add(info, "ABS", braking);
    json_object_object_add(info, "overturned", overturned);
    json_object_object_add(info, "damage", damage);



    char *jsontxt = json_object_to_json_string(info);

    return jsontxt;


}


int main()
{
    int sock;

    if ((sock = client("localhost", local_port)) < 0)
    {
        printf("\nError creating client\n");
        exit(2);
    }

    //printf("sock: %d\n", sock);

    char *p = create_json();
    if (send(sock, p, strlen(p), 0) == -1)
    {
        printf("Error sending message");
    }

    return 0;

    
}
