#include <string.h>

#include "FreeRTOS.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "utils.h"

int xstrsearch ( char * s1, char * s2 )
{
    int i, j, k ;
    int l1 = strlen ( s1 ) ;
    int l2 = strlen ( s2 ) ;

    for ( i = 0 ; i <= l1 - l2 ; i++ )
    {
        j = 0 ;
        k = i ;
        while ( ( s1[k] == s2[j] ) && ( j < l2 ) )
        {
            k++ ;
            j++ ;
        }
        if ( j == l2 )
            return i ;
    }
    return -1 ;
}

int 
http_request_by_ip(
        char *ip, 
        int port, 
        http_request *request, 
        char *response
        )
{
    int socket_fd, request_size, r;
    struct sockaddr_in server;
    static char recv_buf[256];

    socket_fd = socket(AF_INET , SOCK_STREAM , 0);
    if(socket_fd < 0) {
       printf("Failed to create socket\n");
       return -1;
    }
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );

    if (connect(socket_fd , (struct sockaddr *)&server , sizeof(server)) < 0) {
        printf("connect error\n");
        close(socket_fd);
        return -2;
    }

    request_size = 41 + sizeof(request);
    if(xPortGetFreeHeapSize() < request_size) {
            printf("No memory to handle request");
        close(socket_fd);
        return -5;
    }
    char * request_str = pvPortMalloc(request_size);

    //TODO manage user headers. new F??
    /*
     * $VERB $URI HTTP/$VERSION\r\n
     * Host: $IP\r\n
     * User Agent: esp-8266\r\n
     * \r\n
     * $MESSAGE
     *
     * template_lenght = 41
     */

     strcat(request_str, request->verb);
     strcat(request_str, " ");
     strcat(request_str, request->uri);
     strcat(request_str, " ");
     strcat(request_str, "HTTP/");
     strcat(request_str, request->version);
     strcat(request_str, "\r\n");

     strcat(request_str, "Host: ");
     strcat(request_str, ip);
     strcat(request_str, "\r\n");

     strcat(request_str, "User Agent: esp-8266\r\n");
     strcat(request_str, "\r\n");

     if(request->message != NULL || strlen(request->message) > 0) {
         strcat(request_str, request->message);
         strcat(request_str, "\r\n");
     }
     strcat(request_str, "\r\n");

     if( send(socket_fd , request_str, strlen(request_str) , 0) < 0) {
        printf("Failed to send request\n");
        close(socket_fd);
        return -3;
     }

     if(xPortGetFreeHeapSize() < 1000) {
         printf("No memory to handle response");
         close(socket_fd);
         return -4;
     }
     char * recv_response = pvPortMalloc(1000);
     do {
         bzero(recv_buf, 256);
         r = read(socket_fd, recv_buf, 255);
         if(r > 0 && r + strlen(response) < 1000) {
             response = strcat(recv_response, recv_buf);
         }
     } while(r > 0);

     response = recv_response;
     return strlen(recv_response);

}

int http_get_time (char *ip_addr, int *port)
{
    int socket_fd, time_header_pos, r;
    struct sockaddr_in server;
    static char recv_buf[256], date[11];


   socket_fd = socket(AF_INET , SOCK_STREAM , 0);
   if(socket_fd < 0) {
       printf("Failed to create socket\n");
       return -1;
   }

   //printf("socket was created\n");

   server.sin_addr.s_addr = inet_addr(ip_addr);
   server.sin_family = AF_INET;
   server.sin_port = htons( *port );

   if (connect(socket_fd , (struct sockaddr *)&server , sizeof(server)) < 0) {
       printf("connect error\n");
       close(socket_fd);
       return -2;
   }
   //printf("Connected...\n");

   const char* request = "GET / HTTP/1.0\r\n\r\n";
   if( send(socket_fd , request, strlen(request) , 0) < 0) {
       printf("Failed to send request\n");
       close(socket_fd);
       return -3;
   }

   if(xPortGetFreeHeapSize() < 1000) {
        printf("No memory to handle response");
        close(socket_fd);
        return -4;
   }
   char * response = pvPortMalloc(1000);
   printf("\n\nLast data received: %d", strlen(response));
   do {
       bzero(recv_buf, 256);
       r = read(socket_fd, recv_buf, 255);
       if(r > 0 && r + strlen(response) < 1000) {
           response = strcat(response, recv_buf);
       }
   } while(r > 0);

   time_header_pos = xstrsearch(response, "Posix-time:");
   if (time_header_pos > 0) {
       r = time_header_pos + 12;
       for(int i=0; i < 10; i++) {
           date[i] = response[r + i];
       }
   }

   //printf("Heap status BE: %d\n", xPortGetFreeHeapSize());
   vPortFree(response);
   close(socket_fd);
   response[0] = '\0';
   //printf("Closed...\n");
   //printf("Heap status AE: %d", xPortGetFreeHeapSize());
   return (int) date;

}

int 
http_post_temperature(
    http_request *request,
    char *ip,
    int port
    )
{
    char * response;
    int response_code;

    http_request_by_ip(ip, port, request, response);
    //TODO PARSE response to return HTTP code
    response_code = 200;
    return response_code;
}

