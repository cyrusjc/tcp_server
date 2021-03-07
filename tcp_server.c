// Server side C program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>        // for multithreading
#include <dirent.h>

#define PORT 4000
#define BACKLOG 10

#define BUFSIZE 40000


int createSocket(){
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

void bindSocket(int socket,struct sockaddr_in address){
    
    
    if (bind(socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    
}

void listFiles(int socket){
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {   
        char msg[1000] = "The following files are on the directory:\n";
        while ((dir = readdir(d)) != NULL)
        {
            
            strcat(msg, dir->d_name);
            strcat(msg, " ");
        }
        
        printf("%s\n", msg);
        send(socket , &msg , strlen(msg),0);
        closedir(d);
    } 
    return;
}

void printFile(int socket){
    
    char buffer[BUFSIZE];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATH_MAX+1];
    
    char *msg = "Which file would you like to read? ";
    send(socket, msg, strlen(msg), 0);
    
    
    recv(socket, buffer, sizeof(buffer),0);
    printf("\nFile is:  %s\n", buffer);
    
    // checking for path validity
    if (realpath(buffer, actualpath) == NULL){
        
        char error = 1;
        send(socket, &error, 1,0); 
        printf("Bad Path: %s\n", buffer);
        return;
    }
  
    send(socket, msg, strlen(msg), 0); 
    

    
    FILE *fp = fopen(actualpath, "r");
    if (fp == NULL){
        printf(" can't open %s\n", buffer);
        return;
    } 

    
    while( (bytes_read = fread(buffer,1,BUFSIZE,fp))>0 ){
        printf("sending %zu bytes\n", bytes_read);
        send(socket,buffer,bytes_read,0);
    }
    
    memset(buffer, 0, sizeof(buffer));
    fflush(stdout);
    fclose(fp);
    
    return;
}

void uploadFile (int socket){
    int size = 10;
    int finish;
    char actualpath[PATH_MAX+1];
    
    char send_buffer[BUFSIZE], read_buffer[256];
    
    char *msg = "What would you like to download?";
    send(socket, msg, strlen(msg), 1);
    
    memset(read_buffer, '\0', sizeof(read_buffer));
    recv(socket, read_buffer, sizeof(read_buffer),0);
    printf("\nFile is:  %s\n", read_buffer);
    
    // checking for path validity
    if (realpath(read_buffer, actualpath) == NULL){
        
        size = 0;
        send(socket, (void*)&size, sizeof(int),0); 
        printf("Bad Path: %s\n", read_buffer);
        return;
    }
    
    FILE *fp = fopen(actualpath, "r");
    if (fp == NULL){
        printf("Can't open %s\n", read_buffer);
        return;
    } 

    
    fseek(fp,0,SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    send(socket, (void*)&size, sizeof(int), 0); 
     
    
    while (!feof(fp)){
        size = fread(send_buffer,1,sizeof(send_buffer)-1, fp);
        
        do{
            finish = write(socket,send_buffer,size);
        } while(finish < 0);
        
        printf("Packet Size: %i\n", size);
        memset(send_buffer, '\0' ,sizeof(send_buffer));

    }
    return;
    
    
}

void * handler(void *p_socket){
    
        int socket = *((int*)p_socket);
        long valread;
        int cont = 1;
        
        char buffer[30000] = {0};
        char *hello = "What would you like to do?\n 1. List\n 2. Read File\n 3. Upload \n 0. Terminate";
        send(socket , hello , strlen(hello), 0);
        
        while(cont){
            
            recv( socket , buffer, 30000, 0);
            printf("recieved: %s\n",buffer );
            switch(buffer[0])
            {
                case '1': 
                    //send(socket, "Sending Files", strlen("Sending Files"), 0);
                    listFiles(socket);
                    break;
                case '2':
                    printFile(socket);
                    break;
                case '3':
                    uploadFile(socket);
                    break;
                case '0':
                    cont = 0;
                    break;
                default:
                    send(socket, "Please enter another number: ", 32,0 );
                    break;
            }
            memset(buffer, 0, sizeof(buffer));
        }
        
        printf("Terminating socket\n");
        free(p_socket);
        close(socket);
        return NULL;
}

int main()
{
 
    // Creating socket file descriptor
    // STEP 1, CREATING SOCKET
    int server_fd = createSocket();
    // STEP 2, BINDING THE SOCKET
    // declare struct sockaddr_in and fill
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    bindSocket(server_fd, address);
    

    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    
    int addrlen = sizeof(address);
    int new_socket; 
    
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        puts("Connected\n");
        
        
        pthread_t thread;
        int *p_new_socket = malloc(sizeof(int));
        *p_new_socket = new_socket;
        pthread_create(&thread, NULL, handler, p_new_socket);
        //handler(&new_socket);

    }
    
    return 0;
}

