#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>

char * trim(const char * str) {

    size_t len = strlen(str) - 1;
    size_t newlen = 0;
    for (int i = 0; i < len; i++) {
        if (!isspace(str[i])) {
            newlen++;
        }
    }

    char *newstr = malloc(newlen+1 * sizeof(char));
    memset(newstr, 0x0, newlen+1);
    for (int i = 0, ix = 0; i < len; i++) {
        if (!isspace(str[i])) {
            newstr[ix] = str[i];
            ix++;
        }
    }

    return newstr;

}

const char * auth_req = "AUTHENTICATE";
const char * auth_resp_ok = "OK";

bool authAllowed(int sockfd, struct sockaddr_in * server) {

    if (connect(sockfd, (struct sockaddr *)server, sizeof(*server)) < 0) {
        return false;
    }

    char incoming[64] = {0};
    if (send(sockfd, auth_req, strlen(auth_req), 0) < 0) {
        return false;
    }
    if (recv(sockfd, incoming, sizeof(incoming), 0) < 0) {
        return false;
    }

    char * incomingStripped = trim(incoming);

    if (strcmp(incomingStripped, auth_resp_ok) == 0) {
        free(incomingStripped);
        return true;
    }
    
    else {
        free(incomingStripped);
        return false;
    }

}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {

    if (argc < 2) {
        return PAM_PERM_DENIED;
    }

    struct sockaddr_in server;

    int sockfd;

    server.sin_family = AF_INET;
    if (!inet_pton(AF_INET, argv[0], &server.sin_addr)) {
        return PAM_PERM_DENIED;
    }
    int port = 0;
    port = atoi(argv[1]);
    if (port < 1 || port > 65535) {
        return PAM_PERM_DENIED;
    }
    server.sin_port = htons(port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bool retVal = authAllowed(sockfd, &server);

    close(sockfd);

    if (retVal == true) {
        return PAM_SUCCESS;
    }
    else {
        return PAM_PERM_DENIED;
    }
}

PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
	return PAM_SUCCESS;
}
