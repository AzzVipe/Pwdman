#include <sock_lib.h>
#include <request.h>
#include <command.h>

int main(int argc, char const *argv[])
{
	int sockfd, maxfd, nready, buflen, nread;
	fd_set rset, allset;
	// char buf[] =
	// 	"GET /find\n"
	// 	"content-len: 10\n\n"
	// 	"id=ab\n"
	// 	"type=id\n\n";
	struct request req = {0};
	char buf[MAXLINE], read_buf[MAXLINE];

	if (command_handle(&req, argv, argc, buf) == -1)
		return -1;
	
	
	sockfd = Uxd_connect(UNIXPATH);
	if (sockfd == -1) {
		perror("sockfd");
		exit(-1);
	}
	
	if ((buflen = request_prepare(&req, buf, sizeof(buf))) == -1) {
		fprintf(stderr, "request_prepare : Error\n");
		return -1;
	} 

	puts(buf);

	write(sockfd, buf, strlen(buf));

	while ((nread = read(sockfd, read_buf, sizeof(read_buf) - 1)) > 0) {
		read_buf[nread] = 0;
		printf("%s\n", read_buf);
	}

	if (nread == -1) {
		perror("read error");
		exit(-1);
	}

	return 0;
}