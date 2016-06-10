#ifndef UTILS_H_
#define UTILS_H_
/* searches for the given pattern s2 into the string s1 */
int xstrsearch ( char * s1, char * s2 );

/** GET POSIX time from a server
 * Performs a http GET request by IP and port to a server that responds with
 * POSIX time in a http header with format Posix-time:
 *
 * @param ip_addr       A pointer to an array of host's ip address
 * @param port          A pointer to a integer of host's port
 *
 * @returns an integer with POSIX time
 */
int http_get_time (char *ip_addr, int *port);

#endif
