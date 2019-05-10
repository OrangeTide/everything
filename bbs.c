/* vim: set noet sw=4 sts=4 ts=4 tw=80: */
#define _GNU_SOURCE	/* for NI_MAXSERV and NI_MAXHOST */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>	/* for getifaddrs() and struct ifaddrs */
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#define log_error(m, ...) fprintf(stderr, "ERROR:" m, ## __VA_ARGS__)
#define log_info(m, ...) printf(m, ## __VA_ARGS__)
#define log_debug(m, ...) fprintf(stderr, "DEBUG:" m, ## __VA_ARGS__)

// #define ROUNDUP(x, y) ((x) + (y) - ((x) % (y)))

#define NET_BACKLOG 6 /* our default backlog for listen() */

typedef int SOCKET;
#define SOCKET_ERROR (-1)

typedef void eventcallback_t(SOCKET s, void *p);

struct event_class {
	eventcallback_t *evread, *evwrite, *evclose;
};

struct event_entry {
	const struct event_class *class;
	SOCKET s;
	unsigned read_ready:1, write_ready:1;
	void *p;
};

/* we use a flat array of entries indexed by file descriptor */
static struct event_entry *evinfo;
static unsigned evinfo_max;
static fd_set event_rfds, event_wfds;
static SOCKET event_fdmax;

/* set or clear the read ready state for a client */
static void
read_ready(SOCKET s, int set)
{
	if (set)
		FD_SET(s, &event_rfds);
	else
		FD_CLR(s, &event_rfds);
}

/* set or clear the write ready state for a client */
static void
write_ready(SOCKET s, int set)
{
	if (set)
		FD_SET(s, &event_wfds);
	else
		FD_CLR(s, &event_wfds);
}

static void
update_fdmax(SOCKET fd)
{
	if (event_fdmax < fd)
		event_fdmax = fd;
}

static int
net_add(SOCKET s, void *p, const struct event_class *class)
{
	struct event_entry ev = {
		.class = class,
		.s = s,
		.p = p,
		.read_ready = class->evread ? 1 : 0,
		.write_ready = class->evwrite ? 1 : 0,
	};

	read_ready(s, ev.read_ready);
	write_ready(s, ev.write_ready);

#if 1 /* assumes s is a small integer file descriptor - Unix/BSD */

	/* is the slot empty? */
	// TODO: bounds check
	if (evinfo[s].class) {
		log_error("event slot #%d is not empty!\n", s);
		return -1;
	}

	evinfo[s] = ev;
	update_fdmax(s);
#else
#error TODO: implement WinSock version
#endif
	return 0;
}

void
net_close(SOCKET s)
{
	// TODO: bounds check
	struct event_entry *e = &evinfo[s];
	if (e->class && e->class->evclose)
		e->class->evclose(s, e->p);
	read_ready(s, 0);
	write_ready(s, 0);
	close(s);
	memset(e, 0, sizeof(*e));
}

static int
welcome_msg(SOCKET s, void *p)
{
	const char m[] = "Welcome ...\r\n";
	size_t n = strlen(m);

	return send(s, m, n, 0);
}

/* handler for client's read() */
static void
h_read(SOCKET s, void *p)
{
	char scratch[64];
	ssize_t n;

	n = recv(s, scratch, sizeof(scratch), 0);
	if (!n)
		net_close(s);
}

/* handler for client's write() */
static void
h_write(SOCKET s, void *p)
{
	welcome_msg(s, p);
	write_ready(s, 0); /* clear the write ready state */
}

/* handler for a server's accept(). Applied to a read event. */
static void
h_accept(SOCKET s, void *p)
{
	struct sockaddr_storage ss;
	socklen_t len;
	SOCKET fd;
	static const struct event_class client_class = {
		.evread = h_read,
		.evwrite = h_write,
	};

	fd = accept(s, (struct sockaddr*)&ss, &len);
	if (fd == SOCKET_ERROR)
		return;
	log_debug("FAMILY=%d\n", ss.ss_family);
	if (net_add(fd, NULL, &client_class)) {
		log_error("%s()\n", __func__);
		close(fd);
	}
}

static int
net_bind(const struct sockaddr *addr, socklen_t len)
{
	static const struct event_class server_class = {
		.evread = h_accept,
	};
	SOCKET s;

	s = socket(addr->sa_family, SOCK_STREAM, 0);
	if (s == SOCKET_ERROR)
		return -1;
	log_debug("SOCKET:fd=%d\n", s);

	int flag = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)))
		goto fail;

	if (bind(s, addr, len))
		goto fail;

	if (listen(s, NET_BACKLOG))
		goto fail;

	if (fcntl(s, F_SETFL, O_NONBLOCK))
		goto fail;

	if (net_add(s, NULL, &server_class))
		goto fail;

	return 0;
fail:
	perror(__func__);
	close(s);
	return -1;
}

/**
 * addr: socket address from getifaddrs() or getaddrinfo()
 * if_hint: string for the related interface, NULL is unknown interface.
 */
static int
server_add(const struct sockaddr *addr, const char *if_hint, unsigned port)
{
	socklen_t len;
	char host[NI_MAXHOST], serv[NI_MAXSERV];
	int result;
	struct sockaddr_in in;
	struct sockaddr_in6 in6;

	if (addr->sa_family == AF_INET) {
		len = sizeof(struct sockaddr_in);
		in = *(struct sockaddr_in*)addr;
		in.sin_port = htons(port);
		addr = (struct sockaddr*)&in;
	} else if (addr->sa_family == AF_INET6) {
		len = sizeof(struct sockaddr_in6);
		in6 = *(struct sockaddr_in6*)addr;
		in6.sin6_port = htons(port);
		addr = (struct sockaddr*)&in6;
	} else {
		return -1;
	}


	result = getnameinfo(addr, len, host, NI_MAXHOST, serv, NI_MAXSERV,
			NI_NUMERICHOST | NI_NUMERICSERV);
	if (result) {
		log_error("%s\n", gai_strerror(result));
		return -1;
	}

	if (if_hint)
		log_info("Server: %s/%s (%s)\n", host, serv, if_hint);
	else
		log_info("Server: %s/%s\n", host, serv);

	if (net_bind(addr, len))
		return -1;

	return 0;
}

static int
interface_add(const char *match_if, unsigned port)
{
	struct ifaddrs *ifaddr, *cur;
	int result = 0;

	getifaddrs(&ifaddr);
	for (cur = ifaddr; cur; cur = cur->ifa_next) {
		if (!cur->ifa_addr)
			continue;
		if (!match_if || !strcmp(cur->ifa_name, match_if)) {
			if (server_add(cur->ifa_addr, cur->ifa_name, port))
				result = -1;
		}
	}
	freeifaddrs(ifaddr);

	return result;
}

/* copies a partial set of fds */
static void
copyfdset(fd_set *d, fd_set *s, int fd_max) {
#if 1 /* almost certainly faster */
	unsigned len = ((fd_max + 1) + __NFDBITS - 1) / 8;
	memcpy(d->fds_bits, s->fds_bits, len);
#else
	unsigned i, len = ((fd_max + 1) + __NFDBITS - 1) / __NFDBITS;
	for (i = 0; i < len; i++) {
		d->fds_bits[i] = s->fds_bits[i];
	}
#endif
}

static void
loop(void)
{
	fd_set rfds, wfds;
	int n;
	struct timeval tv;

	while (1) {
		log_debug("fdmax=%d\n", event_fdmax);

		/* populate fd_set */
		copyfdset(&rfds, &event_rfds, event_fdmax);
		copyfdset(&wfds, &event_wfds, event_fdmax);

		// TODO: fill in the time to next event
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		n = select(event_fdmax + 1, &rfds, &wfds, 0, &tv);
		if (n < 0) {
			perror("select()");
			return;
		}

		// TODO: loop through rfds->fds_bits
		int i;
		for (i = 0; i <= event_fdmax; i++) {
			if (!evinfo[i].class)
				continue;
			SOCKET fd = evinfo[i].s;
			if (FD_ISSET(fd, &rfds)) {
				eventcallback_t *cb = evinfo[i].class->evread;
				if (cb)
					cb(fd, evinfo[i].p);
				else /* no callback? drop from set */
					FD_CLR(fd, &event_rfds);
			}
			if (FD_ISSET(fd, &wfds)) {
				eventcallback_t *cb = evinfo[i].class->evwrite;
				if (cb)
					cb(fd, evinfo[i].p);
				else /* no callback? drop from set */
					FD_CLR(fd, &event_wfds);
			}
		}
	}
}

int
main(int argc, char **argv)
{
	// TODO: parse arguments

	/* setup socket event entries */
	evinfo_max = 1024;
	evinfo = calloc(evinfo_max, sizeof(*evinfo));

	// TODO: accept list of interfaces to bind
	interface_add(NULL, 4444);

	loop();

	return 0;
}

//// BUGS:
// loop() spins as soon as something connects - FIXED - event_len was 0
//
//
