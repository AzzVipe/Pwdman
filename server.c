#include <sys/un.h>
#include <sys/stat.h>
#include <error.h>
#include <sock_lib.h>
#include <database.h>
#include <pwdman.h>
#include <crypto.h>

void server_init(void);

int main(int argc, char *argv[])
{
	int is_set;
	int listenfd, connfd, bytes_read, maxfd;
	int clients[FD_SETSIZE], nready;
	char buf[MAXLINE];
	fd_set allset, rset;

	if (!crypto_init()) {
		fprintf(stderr, "Failed to initialise crypto\n");
		return -1;
	}

	server_init();

	// Daemon_init(argv[0]);

	listenfd = Uxd_listen(UNIXPATH);
	
	if (listenfd == -1) {
		perror("listenfd");
		exit(-1);
	}

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	maxfd = listenfd;

	for (int i = 0; i < FD_SETSIZE; ++i)
		clients[i] = -1;

	for (;;) {
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {
			if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0) {
				perror("accept error");
				continue;
			}

			for (int i = 0; i < FD_SETSIZE; i++) {
				if (clients[i] == -1) {
					clients[i] = connfd;
					if (connfd > maxfd)
						maxfd = connfd;
					FD_SET(connfd, &allset);
					break;
				}
			}
			nready--;
			puts("Client request accepted!");
		}

		for (int i = 0; i < FD_SETSIZE && nready != 0; i++) {
			if (clients[i] == -1)
				continue;

			if (FD_ISSET(clients[i], &allset)) {
				if ((bytes_read = read(clients[i], buf, sizeof(buf))) > 0) {
					buf[bytes_read] = 0;
					fprintf(stderr, "%s\n", buf);
					pwdman_request_handle(buf, clients[i]);
				}

				FD_CLR(clients[i], &allset);
				close(clients[i]);
				clients[i] = -1;
				nready--;
			}
		}
	}

	return 0;
}

void server_init(void)
{
	struct stat stats;
	char *filename = database_get_name();

	if (stat(filename, &stats) == -1 && errno == ENOENT) {
		database_create_app();
	}
}