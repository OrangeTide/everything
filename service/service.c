#define _GNU_SOURCE

/* Standard headers */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* POSIX headers */
#include <pthread.h>
#include <unistd.h>

/* Networking headers */
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>

/* Local includes */
#include "log.h"

#define ERR (-1)
#define OK (0)

#if 0

static int listen_sock;

static void
process(int fd)
{
	fprintf(stderr, "TODO: process #%d\n", fd);
}

static void
init(void)
{
	struct epoll_event ev;
	struct addrinfo *result;

	getaddrinfo(NULL, "12345", NULL, &result);
	// TODO: loop through every entry
	listen_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	bind(listen_sock, result->ai_addr, result->ai_addrlen);
	listen(listen_sock, 6);
	freeaddrinfo(result);

	epollfd = epoll_create1(EPOLL_CLOEXEC);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

}

static void
loop(void)
{
	int nfds, n;

	for (;;) {
		struct epoll_event events[MAX_EVENTS];
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listen_sock) {
				struct epoll_event ev;
				int conn_sock;
				struct sockaddr_storage addr;;
				socklen_t addrlen = sizeof(addr);

				conn_sock = accept4(listen_sock,
					(struct sockaddr *) &addr, &addrlen, SOCK_CLOEXEC);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				process(events[n].data.fd);
			}
		}
	}
}


#endif

/**********************************************************************/

typedef int SOCKET;
#define SOCKET_INVALID (-1)

/**********************************************************************/

struct service_info {
	pthread_t thread;
	SOCKET fd;
};

struct service_threadwrap {
	struct service_info *info;
	char *config;
	int (*start)(struct service_info *info, const char *config);
};

/**********************************************************************/

SOCKET
socket_listen_tcp(struct sockaddr *sa, socklen_t salen)
{
	SOCKET fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd == SOCKET_INVALID)
		return SOCKET_INVALID;

	bind(fd, sa, salen);
	listen(fd, 6);

	return fd;
}

void
socket_close(SOCKET fd)
{
	assert(fd > 0);
	if (fd > 0)
		close(fd);
}

/**********************************************************************/

static struct service_info *
service_info_new(void)
{
	struct service_info *info = calloc(1, sizeof(struct service_info));
	if (!info) {
		log_error("server allocation error: %s", strerror(errno));
		return NULL;
	}

	info->fd = SOCKET_INVALID;

	return info;
}

static void
service_info_free(struct service_info *info)
{
	if (info) {
		if (info->fd > 0) {
			socket_close(info->fd);
		}
		info->fd = SOCKET_INVALID;

		free(info);
	}
}

static void *
service_wrapper_thread(void *_wrap)
{
	struct service_threadwrap *wrap = _wrap;
	if (!wrap || !wrap->info || !wrap->start)
		return "Initialization Error";

	int result = wrap->start(wrap->info, wrap->config);

	free(wrap->config);
	wrap->config = NULL;

	service_info_free(wrap->info);
	wrap->info = NULL;

	free(wrap);

	return result ? "Service Error" : NULL;
}

static int
service_start_one(SOCKET fd, int (*service_thread)(struct service_info *info, const char *config), const char *config)
{
	struct service_info *info = service_info_new();
	if (!info)
		return ERR;

	info->fd = fd;

	struct service_threadwrap *wrap = calloc(1, sizeof(*wrap));

	wrap->info = info;
	wrap->config = strdup(config);
	wrap->start = service_thread;

	int e = pthread_create(&info->thread, NULL, service_wrapper_thread, wrap);

	if (e) {
		log_error("thread create error: %s", strerror(e));
		service_info_free(info);
		return ERR;
	}

	return OK;
}


/* launch a thread for each specified service */
int
service_start(const char *ports, int (*service_thread)(struct service_info *info, const char *config), const char *config)
{
	struct addrinfo *ai_head, *ai;
	int status = OK;

	(void)ports; // TODO: parse ports list
	const char *node = NULL; // TODO: implement parsing from ports list
	const char *port = "8080";  // TODO: implement parsing from ports list

	getaddrinfo(node, port, NULL, &ai_head);

	for (ai = ai_head; ai; ai = ai->ai_next) {
		SOCKET fd = socket_listen_tcp(ai->ai_addr, ai->ai_addrlen);
		if (fd == SOCKET_INVALID) {
			log_error("listen failed: cannot bind to address");
			status = ERR;
			break;
		}

		if (service_start_one(fd, service_thread, config) != OK) {
			log_error("server start failed");
			status = ERR;
			break;
		}
	}

	freeaddrinfo(ai_head);

	return status;

}

/**********************************************************************/

static int
http_thread(struct service_info *info, const char *config)
{
	// TODO: implement

	(void)info; // TODO: implement
	(void)config; // TODO: parse config

	return ERR;
}

int
main()
{

	// TODO: implement
	// 1. initialize
	// 1.1. setup signal handlers
	// 1.2. initialize threads/ports
	const char *http_ports = getenv("HTTP_PORTS");
	if (http_ports)
		if (service_start(http_ports, http_thread, "{}") != OK)
			return 1;
	// 2. loop on joining terminating threads
	// 3. clean-up

	return 0;
}
