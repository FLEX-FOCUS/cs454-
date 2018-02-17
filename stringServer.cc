/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>

using namespace std;

#define PORT "0"  

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master; 
    fd_set read_fds; 
    int fdmax;       

    int listener;     
    int newfd;        
    struct sockaddr_storage remoteaddr; 
    socklen_t addrlen;

    char buffer[200];  
    int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;       
    int i, j, rv;

	struct addrinfo temp, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&temp, 0, sizeof temp);
	temp.ai_family = AF_UNSPEC;
	temp.ai_socktype = SOCK_STREAM;
	temp.ai_flags = AI_PASSIVE;
	
	rv = getaddrinfo(NULL, PORT, &temp, &ai);
	
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}
	
	char hostname[1024];
    gethostname(hostname,1024);
    cout << "SERVER_ADDRESS " << hostname << endl;
    
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
	getsockname(listener, (struct sockaddr *)&sin, &len);
	cout << "SERVER_PORT "<< ntohs(sin.sin_port) << endl;
	

	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); 

    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    FD_SET(listener, &master);

    fdmax = listener; // so far, it's this one

    while(true) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { 
                if (i == listener) {
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); 
                        if (newfd > fdmax) {   
                            fdmax = newfd;
                        }
                    }
                } else {
                    if ((nbytes = recv(i, buffer, sizeof buffer, 0)) <= 0) {
                        close(i); 
                        FD_CLR(i, &master); 
                    } else {
                            if (FD_ISSET(i, &master)) {
                                buffer[nbytes] = '\0';
				
				
				for(int i=0;i<nbytes; i++){
				
					if(buffer[i]>='A' && buffer[i] <= 'Z'){

						buffer[i] = buffer[i] - 'A' + 'a';
					}
					if(i==0 || (i>1 && buffer[i-1] == ' ')){
						buffer[i] = buffer[i] - 'a' + 'A';

					}
				}
   
                                    if (send(i, buffer, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                            } 
                    }
                }
            } 
        } 
    } 
    
    return 0;
}

