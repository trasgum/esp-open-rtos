#ifndef UTILS_H_
#define UTILS_H_

typedef struct {
    char * verb;
    char * uri;
    char * version;
    char * message;
} http_request;

/** searches for the given pattern s2 into the string s1 
 *
 * @param s1            String which can cotaints the pattern
 * @param s2            String which contains the pattern
 *
 * @returns if s2 matches an integer with the first character index in s1 or -1*/
int xstrsearch ( char * s1, char * s2 );

/** Performs a HTTP request following this template
 *
 *    * $VERB $URI HTTP/$VERSION\r\n
 *    * Host: $IP\r\n
 *    * User Agent: esp-8266\r\n
 *    * \r\n
 *    * $MESSAGE
 *
 * @param ip            A pointer to an array of host's ip address
 * @param port          A integer of host's port
 * @param request       A pointer to http_request structure
 * @param response      A pointer to a string to store server response
 *
 * @returns an integer with the response length
 */
int http_request_ip(char *ip, int port, http_request request, char *response);

/** GET POSIX time from a server
 * Performs a http GET request by IP and port to a server that responds with
 * POSIX time in a http header with format Posix-time. The cannot exceed 1000B
 *
 * @param ip_addr       A pointer to an array of host's ip address
 * @param port          A pointer to a integer of host's port
 *
 * @returns an integer with POSIX time
 */
int http_get_time (char *ip_addr, int *port);

#endif
