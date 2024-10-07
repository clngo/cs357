#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <curses.h>
#include <netdb.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>

#include "talk.h"

#define MAX_BUFFER_SIZE 1024
#define MIN_PORT 1024
#define MAX_PORT 65535
#define NAME_BUFFER 253
#define HOSTARGC 3
#define HOSTARGV 2
#define VERBOSE 'v'
#define ACCEPT 'a'
#define WINDOWS 'N'
#define OPTDASH '-'
#define NULLCHAR '\0'

#define BACKLOG_LIMIT 5
#define POLLTIMEOUT -1
#define POLL_EVENTS 0
#define NUMPOLLS 2
#define RESPONSESIZE 2

void serverMode(int port, int opt_verbose, int opt_accept, int opt_windows);
void clientMode(const char *hostname, int port, int opt_verbose, int opt_accept, int opt_windows);

int main(int argc, char *argv[]) {
    int opt;
    int opt_verbose = 0;
    int opt_accept = 0;
    int opt_windows = 1;

    /* Parse command line options */
    while ((opt = getopt(argc, argv, "vaN")) != -1) {
        switch (opt) {
            case VERBOSE:
                opt_verbose++;
                break;
            case ACCEPT:
                opt_accept++;
                break;
            case WINDOWS:
                opt_windows--;
                break;
            default:
                fprintf(stderr, "Usage: %s [ -v ] [ -a ] [ -N ] [ hostname ] port\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /*  Parse the port argument */
    int port = atoi(argv[argc - 1]);
    if (port <= MIN_PORT || port > MAX_PORT) {
        fprintf(stderr, "Invalid port number: %d\n", port);
        exit(EXIT_FAILURE);
    }

    /* Check if a hostname is provided */
    if (argc >= HOSTARGC && argv[argc - HOSTARGV] && 
    (argv[argc - HOSTARGV][0] != OPTDASH)) {
        clientMode(argv[argc - HOSTARGV], port, opt_verbose, opt_accept, opt_windows);
    } else {
        serverMode(port, opt_verbose, opt_accept, opt_windows);
    }

    return 0;
}

void serverMode(int port, int opt_verbose, int opt_accept, int opt_windows) {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    if (opt_verbose) {
        printf("Options:\n");
        printf("    int       opt_verbose = %i\n", opt_verbose);
        printf("    talkmode  opt_mode    = server\n");
        printf("    int       opt_port    = %i\n", port);
        printf("    char     *opt_host    = (none)\n");
        printf("    int       opt_accept  = %i\n", opt_accept);
        printf("    int       opt_windows = %i\n", opt_windows);
        printf("VERB: Waiting for connection...\n");
    }

    /* Create socket */ 
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    /* Set up server address struct */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    /* Bind socket */
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, 
    sizeof(serverAddr)) == -1) {
        perror("Error binding socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    /* Listen for connections */
    if (listen(serverSocket, BACKLOG_LIMIT) == -1) {
        perror("Error listening");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    /* Accept a connection */
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, 
    &clientLen)) == -1) {
        perror("Error accepting connection");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    /* Wait to receive username */
    struct pollfd* pollResponse;
    if ((pollResponse = calloc(sizeof(struct pollfd), 1)) < 0) {
        perror("calloc");
        close(serverSocket);
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    pollResponse[0].fd = clientSocket;
    pollResponse[0].events = POLLIN;
    poll(pollResponse, 1, POLLTIMEOUT);

    char username[NAME_BUFFER];
    ssize_t bytesRead = recv(clientSocket, username, sizeof(username) - 1, 0);
    if (bytesRead <= 0) {
        perror("Error receiving username from server");
        close(serverSocket);
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    username[bytesRead] = NULLCHAR;
    free(pollResponse);

    if (!opt_accept) {
        char hostname[NAME_BUFFER];
        if (getnameinfo((struct sockaddr*)&clientAddr, sizeof(clientAddr), 
        hostname, NAME_BUFFER, NULL, 0, 0)) {
            perror("Can't receive hostname");
            exit(EXIT_FAILURE);
        }

        printf("Mytalk request from %s@%s. Accept (y/n)? ", username, hostname);

        char response;
        scanf(" %c", &response);
        if (!(response == 'y' || response == 'Y')) {
            send(clientSocket, "no", RESPONSESIZE, 0);
            close(serverSocket);
            close(clientSocket);
            exit(EXIT_SUCCESS);
        }
    } 
    struct pollfd* pollData;
    if ((pollData = calloc(sizeof(struct pollfd), NUMPOLLS)) < 0) {
        perror("calloc");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    pollData[0].fd = STDIN_FILENO;
    pollData[0].events = POLLIN;
    pollData[1].fd = clientSocket;
    pollData[1].events = POLLIN;
    send(clientSocket, "ok", RESPONSESIZE, 0);
    if (opt_windows) {
        start_windowing();
    }
    /* Communication loop */ 
    int closeMessageWritten = 0;
    while (!(has_hit_eof())) {
        poll(pollData, NUMPOLLS, POLLTIMEOUT);

        if (pollData[0].revents == POLLIN) {
            /* Creating a line */
            pollData[0].revents = POLL_EVENTS;
            update_input_buffer();

            char line[MAX_BUFFER_SIZE];
            if (has_whole_line()) {
                /* Send a line to the client */
                read_from_input(line, sizeof(line));
                send(clientSocket, line, strlen(line), 0);
            }
        }
        
        else if (pollData[1].revents == POLLIN) {
            pollData[1].revents = POLL_EVENTS;
            /* Receive a line from the client */

            char response[MAX_BUFFER_SIZE];
            bytesRead = recv(clientSocket, response, sizeof(response) - 1, MSG_DONTWAIT);
            if (bytesRead > 0) {
                response[bytesRead] = NULLCHAR;
                /* Write the response from the client to the window */
                write_to_output(response, bytesRead);
            } 

            else if (bytesRead == 0 && !(closeMessageWritten)) {
                char *closeMessage = "Connection closed. ^C to terminate";
                write_to_output(closeMessage, strlen(closeMessage));
                closeMessageWritten = 1;                

                struct pollfd eventpoll;
                eventpoll.fd = clientSocket;
                eventpoll.events = POLL_EVENTS;
                eventpoll.revents = POLL_EVENTS;

                struct pollfd pollarr[1] = {eventpoll};
                poll(pollarr, 1, POLLTIMEOUT);
            } 

            else if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("Error receiving response from client");
                if (opt_windows) {
                    stop_windowing();
                }
                close(serverSocket);
                close(clientSocket);
                exit(EXIT_FAILURE);
            }
        } 
    }
    free(pollData);   
    if (opt_windows) {
        stop_windowing();
    }
        close(serverSocket);
        close(clientSocket);
}

void clientMode(const char *hostname, int port, int opt_verbose, int opt_accept, int opt_windows) {
    int clientSocket;
    struct sockaddr_in serverAddr;
    struct hostent *serverInfo;

    serverInfo = gethostbyname(hostname);
    if (!serverInfo) {
        fprintf(stderr, "Error: Cannot resolve hostname %s\n", hostname);
        exit(EXIT_FAILURE);
    }

    /* Create socket */ 
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    /* Set up server address */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr, serverInfo->h_addr, serverInfo->h_length);

    /* Connect to server */
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error connecting to server");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    if (opt_verbose) {
        printf("Options:\n");
        printf("    int       opt_verbose = %i\n", opt_verbose);
        printf("    talkmode  opt_mode    = client\n");
        printf("    int       opt_port    = %i\n", port);
        printf("    char     *opt_host    = %s\n", hostname);
        printf("    int       opt_accept  = %i\n", opt_accept);
        printf("    int       opt_windows = %i\n", opt_windows);
    }

    /* Send the username to the server */
    char usernamePacket[MAX_BUFFER_SIZE];
    const char *user = getenv("USER");
    if (!user) {
        perror("Can't receive username");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    strncpy(usernamePacket, user, sizeof(usernamePacket) - 1);
    usernamePacket[sizeof(usernamePacket) - 1] = NULLCHAR;

    
    send(clientSocket, usernamePacket, strlen(usernamePacket), 0);

    /* Wait for response */
    printf("Waiting for response from  %s\n", hostname);
    struct pollfd* pollResponse;
    if ((pollResponse = calloc(sizeof(struct pollfd), 1)) < 0) {
        perror("calloc");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    pollResponse[0].fd = clientSocket;
    pollResponse[0].events = POLLIN;
    poll(pollResponse, 1, POLLTIMEOUT);

    char response[MAX_BUFFER_SIZE];
    ssize_t bytesRead = recv(clientSocket, response, sizeof(response) - 1, 0);

    if (bytesRead <= 0) {
        perror("Error receiving response from server");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    response[bytesRead] = NULLCHAR;

    if (strncmp(response, "ok", RESPONSESIZE) == 0) {
        struct pollfd* pollData;
        if ((pollData = calloc(sizeof(struct pollfd), NUMPOLLS)) < 0) {
            perror("calloc");
            close(clientSocket);
            exit(EXIT_FAILURE);
        }

        pollData[0].fd = STDIN_FILENO;
        pollData[0].events = POLLIN;
        pollData[1].fd = clientSocket;
        pollData[1].events = POLLIN;


        if (opt_windows) {
            start_windowing();
        }
        
        /* Communication loop */
        int closeMessageWritten = 0;
        while ((!(has_hit_eof()))) {
            poll(pollData, NUMPOLLS, POLLTIMEOUT);

            if (pollData[0].revents == POLLIN) {
                /* Creating a line */
                pollData[0].revents = POLL_EVENTS;
                update_input_buffer();
                
                if (has_whole_line() && !(closeMessageWritten)) {
                    char line[MAX_BUFFER_SIZE];
                    /* Send a line to the server */
                    read_from_input(line, sizeof(line));
                    
                    send(clientSocket, line, strlen(line), 0);
                }
            } 

            else if (pollData[1].revents == POLLIN) {
                /* Receive a line from the server */
                pollData[1].revents = POLL_EVENTS;
                bytesRead = recv(clientSocket, response, sizeof(response) - 1, MSG_DONTWAIT);
                if (bytesRead > 0) {
                    response[bytesRead] = NULLCHAR;
                    write_to_output(response, bytesRead);
                } 
                else if (bytesRead == 0 && !(closeMessageWritten)) {
                    char *closeMessage = "Connection closed. ^C to terminate";
                    write_to_output(closeMessage, strlen(closeMessage));
                    struct pollfd eventpoll;
                    eventpoll.fd = clientSocket;
                    eventpoll.events = POLL_EVENTS;
                    eventpoll.revents = POLL_EVENTS;
                    struct pollfd pollarr[1] = {eventpoll};
                    poll(pollarr, 0, POLLTIMEOUT);
                    closeMessageWritten = 1;
                }
                else if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        perror("Error receiving response from client");
                        if (opt_windows) {
                            stop_windowing();
                        }
                        close(clientSocket);
                        exit(EXIT_FAILURE);
                }
            }
            
        }

        free(pollData); 
        if (opt_windows) {
            stop_windowing();
        }
    }
    else {
        printf("%s declined connection.\n", hostname);
    }

    close(clientSocket);
}
