#include <stdio.h>//for printf() & perror()
#include <stdlib.h>//for exit90 & memory constants
#include <string.h>//for memset90
#include <unistd.h>//for close() system call
#include <arpa/inet.h>//for socteradd_in & socket API's

#define PORT 8080
#define BACKLOG 5//mex length of our pending connection queue.

int main(void)
{
    int server_fd,client_fd;//holds our listening socket interger.
    struct sockaddr_in server_addr,client_addr;//holds the IP & port configuration.
    socklen_t client_addr_len = sizeof(client_addr);
    int opt = 1;//Flag used to enable socket reuse.
                //
    printf("starting phase 1 Network Server...\n");
    /*
     *Address Family - AF_INET (this is IP version 4).
     *Type - SOCK_STREAM (this means connection oriented TCP protocol) 
     *Protocol - 0 [ or IPPROTO_IP This is IP protocol]
     */
    server_fd = socket(AF_INET,SOCK_STREAM,0);
    

    if(server_fd == -1)
    {
        perror("socket() creation failed\n");
        exit(EXIT_FAILURE);
    }

    printf("[OK] socket created inside kernel\n"
            "\t\tFileDescriptor (fd) = %d\n",server_fd);
    
    if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) == -1)
    {
        perror("setsockopt() failed\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //zeroing out the memory of the address structre to prevent any garbage values.
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;//uses IPv4.
    server_addr.sin_addr.s_addr = INADDR_ANY;//listen to every local network interface.
    server_addr.sin_port = htons(PORT);//convert port 8080 to network byte order(little to big ENDIANESS).


    //tell the OS to attach our socket to this specific port configuration.
    if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1)
    {
        printf("bind() failed\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[OK] Bind successfull to the port:%d:\n",PORT);

    if(listen(server_fd,BACKLOG) == -1)
    {
        perror("listen() failed\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[OK] listening on port: %d.... waiting for a client to connect\n",PORT);

    //accepting one client connection(blocking).
    //-–----–-
    //the execution will pause right here until a client dails in.
    
    client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_addr_len);
    if(client_fd == -1)
    {
        perror("accept() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //converting the client binary IP layout inot standard readable text.
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,INET_ADDRSTRLEN);

    printf("\n---------connection established--------\n");
    printf("server listening FD(main desk) %d\n",server_fd);
    printf("client dedicated FD(private line) %d\n",client_fd);
    printf("client connection from: %s:%d\n",client_ip,ntohs(client_addr.sin_port));
    printf("----------------\n\n");

    close(client_fd);
    close(server_fd);

    printf("[OK] Both file descriptors closed. server shutting down cleanly\n");
            
    return 0;
}
