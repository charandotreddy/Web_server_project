#include <stdio.h>//for printf() & perror()
#include <stdlib.h>//for exit() & memory constants
#include <string.h>//for memset()
#include <unistd.h>//for close() system call
#include <arpa/inet.h>//for socteraddr_in & socket API's
#include <sys/socket.h>

#define PORT 8080
#define BACKLOG 5//mex length of our pending connection queue.

int main(void)
{
    printf("------------------------------------------------------------------\n");
    int server_fd,client_fd;//holds our listening socket interger.
    struct sockaddr_in server_addr,client_addr;//holds the IP & port configuration.
    int opt = 1;//Flag used to enable socket reuse.
                //
    printf("Starting Network Server...\n");
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
        perror("bind() failed\n");
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

    // ------------------------------------------------------------------
    // STRUCTURAL STARTUP DASHBOARD PANEL (Beautification)
    // ------------------------------------------------------------------
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                C WEB SERVER — PHASE 5                    ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  STATUS   : -> RUNNING                                   ║\n");
    printf("║  PORT     : %-4d                                         ║\n", PORT);
    printf("║  LOCAL    : http://127.0.0.1:%-4d                        ║\n", PORT);
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  (|) OPEN IN BROWSER : http://127.0.0.1:%-4d             ║\n", PORT);
    printf("║  </> TERMINAL TEST   : curl -v http://127.0.0.1:%-4d     ║\n", PORT);
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  AVAILABLE ROUTES:                                       ║\n");
    printf("║    [] GET /           → serves public/index.html         ║\n");
    printf("║    [] GET /about      → serves public/about.html         ║\n");
    printf("║    () GET /shutdown   → graceful server termination      ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║   [$] Press Ctrl+C to stop the server manually           ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    //accepting one client connection(blocking).
    //-–----–-
    //the execution will pause right here until a client dails in.
    
    /*
     * -----------------------------------------------------------
     *  THE TIME LOOP: THIS LOOP KEEPS RUNNIG FOREVER
     *----------------------------------------------------------
     */
    int keep_running = 1;
    while(keep_running)
    {
        socklen_t client_addr_len = sizeof(client_addr);
    
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

    
        /*
        * ----------------------------------------------------------------------
        *  phase:2 reading raw data from client
        *  -----------------------------------------------------------------------
       */
        char buffer[1024];      //1kB network buffer stack frame.
        memset(buffer,0,sizeof(buffer));

        /*reading incomming stream data using socket  specific recv()*/
        ssize_t bytes_read = recv(client_fd,buffer,sizeof(buffer)-1,0);

        if(bytes_read > 0)
        {
            /*terminating right at the end of the data received*/
            buffer[bytes_read] = '\0';

            /*----------------------------------------------------------
             * Phase-3: Pharsing the HTTP request line.
             *-------------------------------------------------------------
             */
            char method[16];
            char path[256];
            char version[16];

            /*scanf reads tokens separated by whitespace from the first line of the buffer.*/
            int items_parsed = sscanf(buffer,"%s %s %s",method,path,version);
        
            if(items_parsed == 3)
            {
                printf("\nParsed HTTP Request Details\n");
                printf("Method      :%s\n",method);
                printf("Path        :%s\n",path);
                printf("version     :%s\n",version);
                printf("\n\n");
            }
            else
            {
                printf("[warning] Failed to fully parse the HTTP request line\n");
                close(client_fd);

            }

            /*
             *Phase-4: Mapping HTTP paths to read Disk Files.
             */
            char file_path[512];

            /*Root paht mapping rules*/
            if(strcmp(path,"/") == 0)
            {
                snprintf(file_path,sizeof(file_path),"public/index.html");
            }
            else if(strcmp(path,"/about") == 0)
            {
                snprintf(file_path,sizeof(file_path),"public/about.html");
            }
            else if(strcmp(path,"/shutdown") == 0)
            {
                /*
                 * Graceful shutdown cmd detected.
                 */
                printf("[SHUTDOWN]Graceful shutdown request received from browser.\n");

                const char *shutdown_body = "<html><body><h1>server shutting down</h1><p>The C backend loop has exited gracefully.You can terminate now.</p></body></html>";
                int body_length = strlen(shutdown_body);
                
                char response[1024];
                snprintf(response,sizeof(response),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %d\r\n"
                        "Connection:close\r\n"
                        "\r\n"
                        "%s",
                        body_length,shutdown_body);

                /*sending the confirmation page back before breaking lines.*/
                send(client_fd,response,strlen(response),0);
                /*breaking the while condition*/
                keep_running = 0;
                close(client_fd);
            }   
            else
            {
                /*safely chaining relative paths to the public directory*/
                snprintf(file_path,sizeof(file_path),"public%s",path);
            }

            /*Attempting to open the requested resource*/
            FILE *file = fopen(file_path,"r");

            if(file != NULL)
            {
                /*---------------
                 * CASE A: File Found (200 OK execution track)
                 * -------------
                 *  calculate total byte footprint via file seekers
                 */
    
                fseek(file,0,SEEK_END);
                long file_size = ftell(file);
                fseek(file,0,SEEK_SET);/*resetting file pointer back to the beginning*/

                /*Allocating a memory buffer to hold the raw content stream*/
                char *file_buffer = malloc(file_size+1);
                if(file_buffer == NULL)
                {
                    perror("memory allocation failure for file reading\n");
                    fclose(file);
                    close(client_fd);
                    continue;             
                }

                /*Read the data sterams from disk into our memeory footprint*/
                ssize_t bytes_read_from_file = fread(file_buffer,1,file_size,file);
                file_buffer[bytes_read_from_file] ='\0';
                fclose(file);

                /*construct HTTP standard response header*/
                char response_header[512];
                snprintf(response_header,sizeof(response_header),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %ld\r\n"
                        "Connection:close\r\n"
                        "\r\n",
                        file_size);

                /*Transmitting header frame*/
                send(client_fd,response_header,strlen(response_header),0);

                /*Transmitting content body frame*/
                send(client_fd,file_buffer,file_size,0);

                printf("[SUCCESS] served %s (%ld bytes) back to client \n",file_path,file_size);
                free(file_buffer);
            }

            else
            {
                /*
                 * CASE B: File not found(404 ERROR Execution Track)
                */
                printf("[404 Error] Requested file path not found:%s\n",file_path);
                
                const char *not_found_body = "<html><body><h1>404 not found</h1><p>The requested page does not exist in the server</p></body></html>";

                int body_length = strlen(not_found_body);

                char response[1024];
                snprintf(response,sizeof(response),
                        "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %d\r\n"
                        "Connection:close\r\n"
                        "\r\n"
                        "%s",body_length,not_found_body);

                send(client_fd,response,strlen(response),0);
            }
        }
        /*closing the client communication */
        close(client_fd);

        printf("[OK] connection processed cleanly.Waiting for the next connection.......\n");
        printf("\n");
    }

    close(server_fd);

    printf("[OK] Both file descriptors closed. server shutting down cleanly\n");

            
    return 0;
}
