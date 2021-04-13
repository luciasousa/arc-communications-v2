// #include "neighbour.c"
#include "uthash.h"
#include "client_functions.c"
#include <json.h>
#include <curl/curl.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>  
#include <pthread.h> 

#define endpoint "http://peiarc.ddns.net/add_accident"
#define local_port "4000"
#define buffer_size 500
#define rsu_port "4000"
#define rsu_port_str ":4000"

typedef struct{
    int rsu_available;
    char ip[INET_ADDRSTRLEN];
} rsu_struc;

rsu_struc rsu;

pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

void *rsu_available(void *arg){
    while(1){
        pthread_mutex_lock(&mx);
        if(get_stronger_rsu_ip(rsu.ip) != 0){
            rsu.rsu_available = 0;
        }else{
            rsu.rsu_available = 1;
        }
        pthread_mutex_unlock(&mx);
        sleep(1);
    }
}

int relay_rsu(char *buffer, char *ip){
    // wave socket here to send buffer

    return 0;

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
    
        /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
        itself */ 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(buffer));
    	
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

void send_emergency(char *buffer){
    //THREAD SAFE ACCESS TO INFO
    pthread_mutex_lock(&mx);
    int rsuavailable = rsu.rsu_available;
    char *ip = rsu.ip;
    pthread_mutex_unlock(&mx);

    char *ipstr = malloc(25);
    strcpy(ipstr, ip);

    json_object * accident = json_tokener_parse(buffer);
    char *id = json_object_get_string(json_object_object_get(accident, "video_id"));
    

    if(rsuavailable){ //if there's an RSU send the information through it
        strcat(ipstr, rsu_port_str);
        printf("here");
        if (create_flag(ipstr, id) != 0){
            printf("Something went wrong warning the camera system.\n");
        }else{
            printf("[Server]\tJust warned the camera\n");
        }
        
        relay_rsu(buffer, ip);
        printf("[Server]\t Relayed info to RSU\n");

    }else{
        if (create_flag("NULL", id) != 0){
            printf("Something went wrong warning the camera system.\n");
        }else{
            printf("[Server]\tJust warned the camera\n");
        }
        send_information(buffer);
        printf("[Server]\t NO RSU -> Direct API Call\n");
    }
}


int send_rescue_message(char *buffer){
    int sock;
 
    int signal_db;
    char ip[INET_ADDRSTRLEN];

    if( get_stronger_neighbour_ip(&signal_db, ip) ) {
        printf("\nError getting neighbour IP\n");
        // no car neighbours. let's try an rsu or LTE. NO VIDEO THIS WAY
        send_emergency(buffer);

    }

    if ((sock = client(ip, local_port)) < 0)
    {
        printf("\nError creating client\n");
        return -1;
    }

    if (send(sock, buffer, strlen(buffer), 0) == -1)
    {
        printf("Error sending message");
        return -1;
    }

    printf("[Server]\tRescue message sent to %s\n", ip);

    return 0;
}


int server()
{
    struct addrinfo hints, *result, *r;
    int sock_base, sock_new;

    struct sockaddr_in client_addr;
    // struct sockaddr_storage client_addr;
    socklen_t client_addr_size;

    int int_to_reusable_socket = 1;
    char s[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //populate addrinfo struct -> use hints as input, results as output
    if (getaddrinfo(NULL, local_port, &hints, &result) != 0)
    {
        printf("Error in getaddrinfo!\n");
        return -1;
    }
 
    // navigate linked list result until sucefull binding (or end of list)
    // navigating linked list
    for (r = result; r != NULL; r = r->ai_next)
    {
        if ((sock_base = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) < 0)
        {
            printf("Error acquiring socket\n");
            return -1;
        }

        //make port reusable
        if (setsockopt(sock_base, SOL_SOCKET, SO_REUSEADDR, &int_to_reusable_socket, sizeof(int)) < 0)
        {
            printf("Error making socket reusable\n");
            return -1;
        }

        // binding to port
        if ((bind(sock_base, r->ai_addr, r->ai_addrlen)) < 0)
        {
            close(sock_base);
            printf("Error binding\n");
            continue;
        }

        break;
    }

    // allocated structure no longer needed
    freeaddrinfo(result);

    // check if binding was successful
    if (r == NULL)
    {
        printf("Error binding port\n");
        return -1;
    }

    if (listen(sock_base, 10) < 0)
    {
        printf("Error listening\n");
        return -1;
    }
   
    printf("Successfully listening\n");

    char buffer[buffer_size];
    int bytes_received;

    // accepting state
    while (1)
    {
        // printf("Entering infinite loop\n");
        client_addr_size = sizeof client_addr;
        sock_new = accept(sock_base, (struct sockaddr *)&client_addr, &client_addr_size);
        if (sock_new < 0)
        {
            printf("Error accepting\n");
            continue;
        }

        inet_ntop(AF_INET,
                  &(client_addr.sin_addr),
                  s, sizeof s);
        printf("[Server]\tconnection from %s\n", s);

        if ((bytes_received = recv(sock_new, buffer, buffer_size - 1, 0)) < 0)
        {
            printf("Error receiving message");
            return -1;
        }

        buffer[bytes_received] = '\0';

        printf("[Server]\tMessage Received: %s\n",buffer);
        // printf("string1 = [[%s]], arg 1 = [[%s]]\n", s, "127.0.0.1");
        
        // int mau = strcmp(s, "127.0.0.1");
        // printf("ai entao %d \n", mau);


        if ( strcmp(s, "127.0.0.1") == 0 ){
            // A message from localhost triggers our rescue message
            send_rescue_message(buffer);
        }else{
            send_emergency(buffer);
        }
        

        
        close(sock_new);
        
    }

    close(sock_base);

    return 0;
}


int main(int argc, char const *argv[])
{   
     
    pthread_t id_thread;
    pthread_create(&id_thread, NULL, rsu_available, NULL);

    // create server
    if (server() < 0)
    {
        printf("\nError creating server\n");
        exit(2);
    }

    return 0;
}
