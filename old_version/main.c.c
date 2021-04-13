#include "neighbour.c"
#include "uthash.h"
#include <stdio.h>

#define MAC_LEN 18

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

void strip(char *s)
{
    int len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
    {
        s[--len] = '\0';
    }
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

int main(int argc, char const *argv[])
{
    // struct Hash *a = get_neighbours();

    // struct Hash *s;

    // for (s = a; s != NULL; s = (struct Hash *)(s->hh.next))
    // {
    //     printf("Interface: %s | mac: %s | signal: %d dB | txrate: %.1f MBit/s | Throughput: %u.%u |Inactive Time: %u ms \n",
    //            s->data.ifname, s->data.mac_address, s->data.signal, s->data.txrate,
    //            s->data.i_throughput, s->data.d_throughput, s->data.inac_time);
    // }

    int stronger_db;
    char stronger_mac[MAC_LEN];

    if (get_stronger_neighbour_mac(&stronger_db, stronger_mac) < 0)
    {
        printf("\nNo neighbours\n");
        exit(2);
    }

    printf("Stronger: \n");

    printf("Mac: %s | Signal: %d dB\n",
           stronger_mac, stronger_db);

    char stronger_ip[16];

    if (get_ip_from_mac(stronger_mac, stronger_ip) < 0)
    {
        printf("\nError converting from MAC to IP\n");
        exit(2);
    }

    printf("IP: %s\n",
           stronger_ip);

    struct addrinfo hints, *result, *r;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(stronger_ip, "4000", &hints, &result) != 0)
    {
        printf("Error in getaddrinfo!\n");
        exit(2);
    }


    int sock;
    for(r = result; r!=NULL; r=r->ai_next){
        if((sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol))<0){
            printf("Error acquiring socket\n");
            exit(2);
        }

        if ( (connect(sock, r->ai_addr, r->ai_addrlen))   <0){
            close(sock);
            printf("Error connecting\n");
            exit(2);
        }

        break;

    }



    return 0;
}
