#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
usage(void)
{
	fprintf(stderr, "usage: dialexec address cmd args...\n");
	exit(1);
}

int
dialunix(char *path)
{
	int s;
	struct sockaddr_un con;

	s = socket(PF_UNIX, SOCK_STREAM, 0);
	if(s < 0)
		return -1;

	memset(&con, 0, sizeof(con));
	con.sun_family = PF_UNIX;
	memset(con.sun_path, 0, sizeof(con.sun_path));
	memmove(con.sun_path, path, sizeof(con.sun_path)-1);

	if((connect(s, (struct sockaddr *) &con, sizeof(con))) < 0)
		return -1;
	return s;
}

int
dialinet(char *host, int port)
{
	int s;
	struct hostent *hp;
	struct sockaddr_in con;

	hp = gethostbyname(host);
	if(hp == NULL)
		return -1;

	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s < 0)
		return -1;

	memset(&con, 0, sizeof(con));
	con.sin_family = AF_INET;
	con.sin_port = htons(port);
	memmove(&con.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);

	if((connect(s, (struct sockaddr *) &con, sizeof(con))) < 0)
		return -1;
	return s;
}

/*
 * Parse either unix or network address argument and connect accordingly.
 * Return the connected file descriptor.
 */
int
dial(char *arg)
{
	char *p, addr[MAXPATHLEN];
	int i, port;

	if(arg == NULL)
		return -1;

	p = arg;
	i = 0;
	while(*p != '\0' && *p != ':')
		addr[i++] = *p++;
	addr[i] = '\0';

	if(*p == ':'){
		p++;
		port = atoi(p);
		return dialinet(addr, port);
	}
	return dialunix(addr);
}

int
main(int argc, char *argv[])
{
	int fd;

	if(argc < 3)
		usage();

	switch(fork()){
	case -1:
		perror("Failed to create process");
		exit(1);
	case 0:
		fd = dial(argv[1]);
		if(fd == -1){
			perror("Failed to connected");
			exit(1);
		}

		dup2(fd, 0);
		dup2(fd, 1);
		close(fd);
		execvp(argv[2], argv+2);
		perror("Failed to exec");
		exit(1);
	default:
		wait(NULL);
	}
	return 0;
}
