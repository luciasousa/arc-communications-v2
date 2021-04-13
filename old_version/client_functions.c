#include <stdio.h>
#include <string.h>
#include "uthash.h"
#include "neighbour.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAC_LEN 18
#define INACTIVE_TIME 2000

void strip(char *s)
{
    int len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
    {
        s[--len] = '\0';
    }
}

int get_stronger_neighbour_mac(int *result_db, char *result_mac)
{
    struct Hash *a = get_neighbours();

    struct Hash *s;

    if (a == NULL)
    {
        return -1;
    }

    *result_db = a->data.signal;
    strcpy(result_mac, a->data.mac_address);

    for (s = a; s != NULL; s = (struct Hash *)(s->hh.next))
    {
        if (s->data.signal < *result_db)
        {
            *result_db = s->data.signal;
            strcpy(result_mac, a->data.mac_address);
        }
    }

    return 0;
}

int get_ip_from_mac(char *mac, char *ip)
{
    FILE *file;
    file = fopen("../knownhosts.txt", "r");

    if (file == NULL)
    {
        printf("Error opening hosts\n");
        return -1;
    }

    char buff[100];

    while (fgets(buff, sizeof(buff), file) != NULL)
    {
        strip(buff);

        if (strcmp(buff, mac) == 0)
        {
            break;
        }

        // printf("string1 = [[%s]], arg 1 = [[%s]]\n", buff, mac);    //cool trick to check linebreaks
    }

    if (fgets(buff, sizeof(buff), file) == NULL)
    {
        return -1;
    }

    strip(buff);

    // *ip = "a10";
    strcpy(ip, buff);

    fclose(file);
    return 0;
}

int get_stronger_neighbour_ip(int *result_db, char *result_ip){
    // gets all wave neighbours
    // for each one checks if inactive time is < 2000ms and if its a car
    // returns the first one fullfills
    int debug = 0;
    // int stronger_db;
    char stronger_mac[MAC_LEN];

    if (get_stronger_neighbour_mac(result_db, stronger_mac) < 0)
    {
        printf("\nNo neighbours\n");
        return -1;
    }


    if (debug) printf("Mac: %s | Signal: %d dB\n", stronger_mac, *result_db);

    // char stronger_ip[16];

    if (get_ip_from_mac(stronger_mac, result_ip) < 0)
    {
        printf("\nError converting from MAC to IP\n");
        return -1;
    }

    if (debug) printf("Stronger IP: %s | Signal: %d dB\n", result_ip, *result_db);


    return 0;
}

int get_ip_from_mac_rsu(char *mac, char *ip)
{
    FILE *file;
    file = fopen("../knownhosts_rsu.txt", "r");

    if (file == NULL)
    {
        printf("Error opening rsu hosts\n");
        return -1;
    }

    char buff[100];

    while (fgets(buff, sizeof(buff), file) != NULL)
    {
        strip(buff);

        if (strcmp(buff, mac) == 0)
        {
            break;
        }

        // printf("string1 = [[%s]], arg 1 = [[%s]]\n", buff, mac);    //cool trick to check linebreaks
    }

    if (fgets(buff, sizeof(buff), file) == NULL)
    {
        return -1;
    }

    strip(buff);

    // *ip = "a10";
    strcpy(ip, buff);

    fclose(file);
    return 0;
}

int get_stronger_rsu_ip(char *result_ip)
{
    struct Hash *a = get_neighbours();
 

    struct Hash *s;

    if (a == NULL)
    {
        return -1;
    }

    
    

    for (s = a; s != NULL; s = (struct Hash *)(s->hh.next))
    {
        if (s->data.inac_time < INACTIVE_TIME)
        {
            // check if mac address if from a rsu
            if (get_ip_from_mac_rsu(s->data.mac_address, result_ip)==0)
            {
                return 0;
            }
            
        }
    }

    // no mac address from a rsu
    return -1;
}


int client(char *ip, char *port){
    struct addrinfo hints, *result, *r;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, port, &hints, &result) != 0)
    {
        printf("Error in getaddrinfo!\n");
        return -1;
    }

    int sock;
    for(r = result; r!=NULL; r=r->ai_next){
        if((sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol))<0){
            printf("Error acquiring socket\n");
           return -1;
        }

        if ( (connect(sock, r->ai_addr, r->ai_addrlen))   <0){
            close(sock);
            printf("Error connecting\n");
           return -1;
        }

        break;

    }

    return sock;
}

int create_flag(char *ip, char *id){
    char mac_add[MAC_LEN];
    FILE *file_mac;
    file_mac = fopen("/sys/class/net/wlan0/address", "r");

    if (file_mac == NULL)
    {
        printf("Error getting mac address\n");
        return -1;
    }

    fgets(mac_add, sizeof(mac_add), file_mac);
    
    fclose(file_mac);
    


    FILE *file;
    file = fopen("/tmp/flag", "w");

    if (file == NULL)
    {
        printf("Error opening flag\n");
        return -1;
    }

    fprintf(file, "%s\n", id); //mac_add gives error?
    fprintf(file, "%s\n", ip);
    fclose(file);

    return 0;


}
