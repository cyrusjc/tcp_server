// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


// MY SERVER'S PUBLIC IP ADDRESS 54.148.173.123 (Lightsail)
// MY SERVER'S PRIVATE IP ADDRESS 172.26.0.75
#define PORT 4000
#define BUFSIZE 40000
#define IP_ADDR "54.148.173.123"
void clearMsg(char *buffer){

    memset(buffer, '\0', sizeof(buffer));
    return;
}

void downloadFile(int sock){
    long valread;  
    char buffer[BUFSIZE] = {0};
    int size = 0;
    char reply[20];

    //recieving reply
    memset(buffer, '\0', sizeof(buffer));
    recv(sock, buffer, BUFSIZE, 0); //
    printf("%s ", buffer);

    //file to recieve from server
    memset(reply, '\0', sizeof(reply));
    scanf("%s", reply);
    send(sock , &reply , strlen(reply) , 0 );

    recv(sock, (void*)&size, sizeof(int), 0);
    printf("size is %d\n", size);
    if(size==0){
        printf("Not Valid path\n");
        return;
    }
    
    FILE *file = fopen(reply, "wb");
    if(file == NULL){
        puts("cannot open file");
        return;
    }

    int len;
    int remain_data = size;

        while ((remain_data > 0) && ((len = recv(sock, buffer, BUFSIZ, 0)) > 0))
        {
                fwrite(buffer, sizeof(char), len, file);
                remain_data -= len;
                fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
        }

    fclose(file);
    printf("File Recieved\n");
    return;
}
int main(int argc, char const *argv[])
{
    // STEP ONE DECLARING A SOCKET
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    // ==============================STEP TWO, CONNECTING A SOCKET (BINDING)============
    // =================================================================================
    /*============IPv4 AF_INET sockets:================================
    struct sockaddr_in {
        short            sin_family;   // e.g. AF_INET, AF_INET6
        unsigned short   sin_port;     // e.g. htons(3490)
        struct in_addr   sin_addr;     // see struct in_addr, below
        char             sin_zero[8];  // zero this if you want to

    };
    
    struct in_addr {
        unsigned long s_addr;          // load with inet_pton()
    };

    struct sockaddr {
        unsigned short    sa_family;    // address family, AF_xxx
        char              sa_data[14];  // 14 bytes of protocol address
    };
    
    ===================================================================*/
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    // ============================================================================
    // =======================STEP 3 Send data over socket=========================
    // ============================================================================
    
    long valread;  
    char buffer[BUFSIZE] = {0};
    char *hello = "1";
    int cont = 1;

    char reply[10];

    recv(sock, buffer, BUFSIZE, 0);

    printf("%s\n",buffer );

    while(cont){
        printf("\n\nWhat would you like to do? ");
        scanf("%s", reply);
        send(sock , &reply , strlen(reply) , 0 );
        switch(reply[0]){
            case '0':
                cont = 0;
                break;
            case '1':
                recv(sock, buffer, BUFSIZE, 0);
                printf("%s", buffer);
                break;
            case '2':

                //recieving reply
                memset(buffer, '\0', sizeof(buffer));
                recv(sock, buffer, 30000, 0);
                printf("%s ", buffer);

                //file to recieve from server
                memset(reply, '\0', sizeof(reply));
                clearMsg(reply);
                scanf("%s", reply);
                send(sock , &reply , strlen(reply) , 0 );

                // check if file is good
                memset(buffer, '\0', sizeof(buffer));
                recv(sock, buffer, 30000, 0);
                if (buffer[0] == 1){
                    printf("\nNo such file name\n");
                    break;
                }                
                //recive file
                //clearMsg(buffer);
                memset(buffer, '\0', sizeof(buffer));
                recv(sock, buffer, 30000, 0);
                printf("%s",buffer);
                break;

            case '3':
                downloadFile(sock);
                break;
            default:
                printf("Please enter another number\n");
                break;
        }
        memset(buffer, '\0', sizeof(buffer));
        memset(reply, '\0', sizeof(reply));
        fflush(stdout);
        //clearMsg(buffer);
        //(reply);

    }
    
    printf("\n%s\n",buffer);
    printf("\nTERMINATING SOCKET123\n");
    
    close(sock);
    return 0;
}
