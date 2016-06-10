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

   //TODO check realloc ??
   char * response = pvPortMalloc(1000);
   printf("\n\nLast data received: %d", strlen(response));
   do {
       bzero(recv_buf, 256);
       r = read(socket_fd, recv_buf, 255);
       if(r > 0 && r + strlen(response) < 1000) {
           response = strcat(response, recv_buf);
           if(response == NULL) {
               printf("No memory to handle response");
               close(socket_fd);
               vPortFree(response);
               response[0] = '\0';
               return -4;
           }
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

