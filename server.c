/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE_LENGTH 100
#define MAX_SERVERS 100
#define MAXDATASIZE 100

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

int num_dept = 1;

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

typedef struct {
    int num;
    char servers[MAX_SERVERS][MAX_LINE_LENGTH];
    int num_servers;
} ServerList;

void readServerList(char* filename, ServerList* serverList) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }
    char line[MAX_LINE_LENGTH];
    int lineNum = 0;
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        char* token;
        token = strtok(line, ",");
        while(token) {
            int num = atoi(token);
            if (num != 0) {
                if (lineNum > 0) {
                    serverList[lineNum - 1] = *serverList;
                    serverList++;
					num_dept++;
                }
                serverList->num = num;
                serverList->num_servers = 0;
            } else {
                if (serverList->num_servers < MAX_SERVERS) {
                    strcpy(serverList->servers[serverList->num_servers], token);
                    serverList->num_servers++;
                }
            }
            token = strtok(NULL, ",");
        }
        lineNum++;
    }
    fclose(file);
}

char * toArray(int number)
{
    int n = log10(number) + 1;
    int i;
    char *numberArray = calloc(n, sizeof(char));
    for (i = n-1; i >= 0; --i, number /= 10)
    {
        numberArray[i] = (number % 10) + '0';
    }
    return numberArray;
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	ServerList serverList[MAX_SERVERS];
	char buf[MAXDATASIZE];
	char sen[MAXDATASIZE];
	int numbytes;
	int flag = 1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	readServerList("list.txt", serverList);
	printf("Total num of Backend Servers: %d:\n", num_dept);
	for (int i = 0; i < num_dept; i++) {
		printf("Backend Servers %d contains %d distinct departments\n", i+1, serverList[i].num_servers);
    }

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}		

		// buf[numbytes] = '\0';

		// printf("client: received '%s'\n",buf);

		for (int i = 0; i < num_dept; i++) {
			for (int j = 0; j < serverList[i].num_servers; j++) {
				if(strcmp(serverList[i].servers[j], buf) == 0){
					char* ans = "<Department Name> is found.";
					if (send(new_fd, ans, strlen(ans), 0) == -1){
						perror("send");
					}
					flag = 0;
					break;
				}
			}
    	}
		if(flag == 1){
			char* ans = "<Department Name> not found.";
			if (send(new_fd, ans, strlen(ans), 0) == -1){
						perror("send");
			}
		}
		flag = 1;
	}

	return 0;
}