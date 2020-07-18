/* vim: set noet sw=4 sts=4 ts=4 tw=80: */
#define _GNU_SOURCE	/* for NI_MAXSERV and NI_MAXHOST */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>	/* for getifaddrs() and struct ifaddrs */
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

#define log_error(m, ...) fprintf(stderr, "ERROR:" m, ## __VA_ARGS__)
#define log_info(m, ...) printf(m, ## __VA_ARGS__)
#define log_debug(m, ...) fprintf(stderr, "DEBUG:%s():" m, __func__, ## __VA_ARGS__)

// #define ROUNDUP(x, y) ((x) + (y) - ((x) % (y)))

#define NET_BACKLOG 6 /* our default backlog for listen() */

typedef int SOCKET;
#define SOCKET_ERROR (-1)

typedef void eventcallback_t(SOCKET s, void *p);

struct event_class {
	eventcallback_t *evread, *evwrite, *evclose, *evinit;
	void *(*newdata)(const struct event_class*); /* allocate event_entry.p */
	void (*freedata)(void *p);
	const void *extra;
};

struct event_entry {
	const struct event_class *class;
	SOCKET s;
	unsigned read_ready:1, write_ready:1;
	void *p;
};

struct server_info {
	const struct event_class *clientclass;
};

struct buffer {
	unsigned start, end;
	unsigned max;
	unsigned char data[];
};

struct client_info {
	enum { CLIENT_STATE_LOGIN, CLIENT_STATE_COMMAND, } state;
	struct buffer *outbuf, *inbuf;
};

/******************************************************************************/
/* global configuration */

static int verbose = 0;

/******************************************************************************/
/* network socket event API */

/* we use a flat array of entries indexed by file descriptor */
static struct event_entry *evinfo;
static unsigned evinfo_max;
static fd_set event_rfds, event_wfds;
static SOCKET event_fdmax;
static int current_entry = -1; /* track the currently "selected" entry */
static int current_fd = -1; /* track the currently "selected" entry */

/******************************************************************************/

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

	/* handle the init event */
	struct event_entry *e = &evinfo[s];
	log_info("[%d] init event!\n", e->s);
	if (e->class && e->class->evinit)
		e->class->evinit(e->s, e->p);

	return 0;
}

void
net_close(SOCKET s)
{
	// TODO: bounds check
	struct event_entry *e = &evinfo[s];
	log_info("[%d] close event!\n", e->s);
	if (e->class && e->class->evclose)
		e->class->evclose(s, e->p);
	read_ready(s, 0);
	write_ready(s, 0);
	close(s);
	if (e->class && e->class->freedata) {
		e->class->freedata(e->p);
		e->p = NULL;
	}
	memset(e, 0, sizeof(*e));
}

static int
net_bind(const struct sockaddr *addr, socklen_t len, const struct event_class *class)
{
	SOCKET s;

	s = socket(addr->sa_family, SOCK_STREAM, 0);
	if (s == SOCKET_ERROR)
		return -1;
	log_debug("[%d] socket created\n", s);

	int flag = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)))
		goto fail;

	if (bind(s, addr, len))
		goto fail;

	if (listen(s, NET_BACKLOG))
		goto fail;

	if (fcntl(s, F_SETFL, O_NONBLOCK))
		goto fail;

	void *data = class->newdata ? class->newdata(class) : NULL;
	if (net_add(s, data, class))
		goto fail;

	return 0;
fail:
	perror(__func__);
	close(s);
	return -1;
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

	// TODO: terminate if there are no more sockets
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
			current_entry = i;
			current_fd = fd;
			if (FD_ISSET(fd, &rfds)) {
				log_info("[%d] read event!\n", fd);
				eventcallback_t *cb = evinfo[i].class->evread;
				if (cb) {
					cb(fd, evinfo[i].p);
				} else { /* no callback? this is bad */
					log_error("[%d] no read callback\n", fd);
					net_close(fd);
				}
			}
			if (FD_ISSET(fd, &wfds)) {
				log_info("[%d] write event!\n", fd);
				eventcallback_t *cb = evinfo[i].class->evwrite;
				if (cb) {
					cb(fd, evinfo[i].p);
				} else { /* no callback? this is bad */
					log_error("[%d] no write callback\n", fd);
					net_close(fd);
				}
			}
		}
		current_entry = -1;
		current_fd = -1;
	}
}

/******************************************************************************/
/* buffer API */

static void
buf_free(struct buffer *buf)
{
	free(buf);
}

static struct buffer *
buf_new(unsigned max)
{
	struct buffer *b = calloc(1, sizeof(*b) + max);
	b->max = max;
	return b;
}

/* a read-only pointer to the data */
static const void *
buf_data(struct buffer *buf, unsigned *len_out)
{
	assert(buf->end >= buf->start);
	unsigned n = buf->end - buf->start;
	if (len_out)
		*len_out = n;
	return n ? buf->data + buf->start : NULL;
}

/* a write-able pointer to the end of the buffer */
static void *
buf_prepare(struct buffer *buf, unsigned *len_out)
{
	assert(buf->end >= buf->start);
	unsigned n = buf->end - buf->start;
	if (len_out)
		*len_out = n;
	return n ? buf->data + buf->end : NULL;
}

/* move entire buffer to start of array to get the most free space */
static void
buf_pack(struct buffer *buf)
{
	assert(buf->end >= buf->start);
	if (buf->start) {
		unsigned n = buf->end - buf->start;
		memmove(buf->data, buf->data + buf->start, n);
		buf->end -= buf->start;
		buf->start = 0;
	}
}

/* called after writing data to the buffer */
static void
buf_update(struct buffer *buf, unsigned len)
{
	buf->end += len;
}

static void
buf_consume(struct buffer *buf, unsigned len)
{
	buf->start += len;
}

static int
buf_add(struct buffer *buf, const void *data, size_t len)
{
	if (buf->max < len)
		return -1; /* never will fit */

	unsigned avail = buf->max - buf->end;
	if (avail < len)
		buf_pack(buf);

	memcpy(buf->data + buf->end, data, len);
	buf->end += len;

	return 0;
}

static int
buf_printf(struct buffer *buf, const char *fmt, ...)
{
	va_list ap;
	int n;

	buf_pack(buf);
	assert(buf->max >= buf->end);
	int len = buf->max - buf->end;
	va_start(ap, fmt);
//	log_debug("start=%d end=%d max=%d\n", buf->start, buf->end, buf->max);
	n = vsnprintf((char*)buf->data + buf->end, len, fmt, ap);
	va_end(ap);
	if (n > len)
		return -1;
	buf_update(buf, n);

	return n;
}

/* grabs the next line, requires buf_consume() to clean up */
static const void *
buf_line(struct buffer *buf, unsigned *mark)
{
	assert(buf->end >= buf->start);
	unsigned n = buf->end - buf->start;

	/* bail early on an empty buffer */
	if (!n) {
		if (mark)
			*mark = 0;
		return NULL;
	}

	unsigned char *start = buf->data + buf->start,
				  *end = memchr(start, '\n', n);

	/* no line found */
	if (!*end) {
		if (mark)
			*mark = 0;
		return NULL;
	}

	n = end - start; /* save for buf_consume() */

	/* deal with possible CR LF case */
	if (end > start && end[-1] == '\r')
		end[-1] = 0;
	else
		end[0] = 0;

	if (mark)
		*mark = n;

	return n ? buf->data + buf->start : NULL;
}

/******************************************************************************/
/* Utility functions */

// TODO: grow()

/******************************************************************************/
/* Property list API */

typedef char *property_t;

struct prop_iter {
	property_t *base;
	int i;
	int max;
};

struct property_list {
	unsigned len, max;
	property_t *list; /* entries are stored as "key\0value\0" */
};

static inline const char *
prop_get_name(const property_t p)
{
	return p;
}

static inline const char *
prop_get_value(const property_t p)
{
	return p + strlen(p) + 1;
}

static inline void
prop_free(property_t p)
{
	free(p);
}

/* initializes a prop_iter for iterating through an array of property_t.
 * the base pointer passed through iter should not be modified between calls of
 * this function or prop_iter_next(). */
static inline void
prop_iter_init(struct prop_iter *iter, property_t *base, unsigned max)
{
	iter->base = base;
	iter->i = 0;
	iter->max = max;
}

property_t
prop_new(const char *name, const char *value)
{
	if (!name) /* unnamed properties are not valid */
		return NULL;
	if (!value) /* treat NULL as an empty string */
		value = "";
	size_t n = strlen(name) + 1, v = strlen(value) + 1;
	property_t p = malloc(n + v);
	if (!p)
		return NULL; /* allocation error */
	memcpy(p, name, n);
	memcpy(p + n, value, v);
	return p;
}

void
plist_free(struct property_list *plist)
{
	unsigned i;
	unsigned len = plist->len;
	for (i = 0; i < len; i++) {
		property_t *ptr = &plist->list[i];
		prop_free(*ptr);
		*ptr = NULL;
	}
	free(plist->list);
}

/* compare two property names */
static int
prop_compar(const void *a, const void *b)
{
        return strcmp(*(property_t *)a, *(property_t *)b);
}

/* return offset or -1 on error. */
int
prop_find_slot(property_t *prop, unsigned prop_len, const char *name)
{
        if (!prop_len)
                return -1; /* no match because list is empty */

        property_t *res = bsearch(&name, prop, prop_len,
		sizeof(*prop), prop_compar);

        if (res)
                return res - prop;

        return -1; /* no match */
}

int
plist_set(struct property_list *plist, const char *name, const char *value)
{
	property_t p = prop_new(name, value);
	if (!p)
		return -1;

        int ofs = prop_find_slot(plist->list, plist->len, name);
        if (ofs >= 0) {
		property_t *ptr = &plist->list[ofs];
		/* entry was found, replace into the same slot */
                free(*ptr);
                *ptr = p;
		/* no need to qsort() because name hasn't changed */
                return 0;
        }

	/* else add a new entry on the end */
	ofs = plist->len;
	unsigned new_size = plist->len + 1;
	if (new_size >= plist->max) { /* make space for the entry */
		// BUG: TODO: arguments for grow() have changed
		if (grow(&plist->list, &plist->max, new_size,
				sizeof (*plist->list))) {
			prop_free(p);
			return -1; /* allocation error */
		}
		plist->len = new_size;
	}
	plist->list[ofs] = p;

	/* the bsearch() requires the array to be sorted */
	qsort(plist->list, plist->len, sizeof(*plist->list), prop_compar);

	return 0;
}

const char *
plist_get(struct property_list *plist, const char *name)
{
        int ofs = prop_find_slot(plist->list, plist->len, name);
	if (ofs < 0)
		return NULL;
	property_t p = plist->list[ofs];
	return prop_get_value(p);
}

/* gets the next property name-value pair.
 * return false when there are no more properties.
 * the base pointer passed through iter should not be modified between calls of
 * this function.
 */
bool
prop_iter_next(struct prop_iter *iter,
	const char **name, const char **value)
{
	int i = iter->i;
	if (i >= iter->max)
		return false;
	const property_t p = iter->base[i];
	if (name)
		*name = prop_get_name(p);
	if (value)
		*value = prop_get_value(p);
	return true;
}


/******************************************************************************/
/* DB API */

typedef struct db_handle *db_handle_t;

struct db_handle {
	int fd;
	int readonly;
	int state; // 0=header, 1=data
};

static int db_dirfd = -1;

static int
db_init(const char *dbpath)
{
	db_dirfd = open(dbpath, O_DIRECTORY);
	if (db_dirfd == -1) {
		perror(dbpath);
		return -1;
	}

	return 0;
}

static int
db_close(db_handle_t h)
{
	// TODO: check that this is read-only
	close(h->fd);
	return 0;
}

static int
db_commit(db_handle_t h)
{
	abort(); // TODO: implement this
}

/* don't commit a transaction */
static int
db_discard(db_handle_t h)
{
	abort(); // TODO: implement this
}

static db_handle_t
db_handle_alloc(void)
{
	db_handle_t h;

	return calloc(1, sizeof(*h));
}

/* open an existing resource */
static int
db_open(const char *resource, db_handle_t *h_out)
{
	if (db_dirfd == -1)
		return -1;
	int fd = openat(db_dirfd, resource, O_RDONLY);

	db_handle_t h = db_handle_alloc();

	h->fd = fd;
	h->state = 0;
	h->readonly = 1;

	if (h_out) {
		*h_out = h;
	} else {
		db_discard(h);
	}

	return 0;
}

/* create or overwrite a resource */
static int
db_create(const char *resource, db_handle_t *h_out)
{
	// TODO: open with O_CREAT | O_EXCL
	// TODO: open a temp file

	abort(); // TODO: implement this
}

/* read resource's meta-data */
static int
db_headers(db_handle_t h, void *data, unsigned max, unsigned *len_out)
{
	abort(); // TODO: implement this
}

/* read the data area */
static int
db_read(db_handle_t h, void *data, unsigned max, unsigned *len_out)
{
	// TODO: skip over any remaining headers section
	abort(); // TODO: implement this
}

/* write/append to data area */
static int
db_write(db_handle_t h, const void *data, unsigned len)
{
	abort(); // TODO: implement this
}

/******************************************************************************/
/* user record */
struct user_info {
	char name[32];
	unsigned uid;
	unsigned groups[15];
	char cipher[496];
};

static struct user_info user_current;

static int
user_begin(const char *name)
{
	char path[256];

	if (snprintf(path, sizeof(path), "users/%s", name) >= sizeof(path)) {
		log_error("Username too long\n");
		return -1;
	}

	db_handle_t h;
	int result = db_open(path, &h);
	if (result)
		return result; /* probably user not found */

	// TODO: read user headers. there won't be any data section.

	db_close(h);

	return 0;
}

static int
user_check_password(const char *cleartext)
{
	// TODO: implement this
	abort();
}

/* complete the transaction */
static void
user_end(void)
{
	// TODO: implement this
	abort();
}

/******************************************************************************/
/* Client API */

static void
show_file(SOCKET s, struct client_info *cinfo, const char *resource)
{
	if (strcmp(resource, "WELCOME") == 0) {
		// TODO: access the database
		buf_printf(cinfo->outbuf, "Welcome ...");
		buf_add(cinfo->outbuf, "\r\n", 2);
		write_ready(s, 1);
	} else if (strcmp(resource, "ERROR") == 0) {
		buf_printf(cinfo->outbuf, "Fatal ERROR!");
		buf_add(cinfo->outbuf, "\r\n", 2);
		write_ready(s, 1);
	} else {
		buf_printf(cinfo->outbuf, "ERROR: Unknown resource!");
		buf_add(cinfo->outbuf, "\r\n", 2);
		write_ready(s, 1);
	}
}

/******************************************************************************/
/* Client handling implementation */

static void
client_process(SOCKET s, struct client_info *cinfo)
{
	if (!cinfo) {
		log_error("[%d] missing client info\n", s);
		net_close(s);
		return;
	}


	// TODO: process lines until there are no more
	while (1) {
		unsigned mark;
		const char *line = buf_line(cinfo->inbuf, &mark);
		if (!line)
			break;

		switch (cinfo->state) {
		case CLIENT_STATE_LOGIN:
			// TODO: go to next state
			log_info("[%d] USER: \"%s\"\n", s, line);
			cinfo->state = CLIENT_STATE_COMMAND;
			show_file(s, cinfo, "MAINMENU");
			break;
		case CLIENT_STATE_COMMAND:
			log_info("[%d] COMMAND: \"%s\"\n", s, line);
			break;
		default:
			log_error("[%d] invalid state %d\n", s, cinfo->state);
			show_file(s, cinfo, "ERROR");
			// TODO: flush then disconnect
			return;
		}
		buf_consume(cinfo->outbuf, mark);
	}
}

/* handler for client's read() */
static void
client_handle_read(SOCKET s, void *p)
{
	struct client_info *cinfo = p;
	unsigned len;
	ssize_t n;
	void *data;

	buf_pack(cinfo->inbuf);
	data = buf_prepare(cinfo->inbuf, &len);
	if (!data) {
		log_error("[%d] unable create buffer\n", s);
		net_close(s);
		return;
	}
	if (len <= 1) {
		log_error("[%d] buffer filled, but not draining\n", s);
		net_close(s);
		return;
	}

	n = recv(s, data, len, 0);
	if (!n) {
		net_close(s);
		return;
	} else if (n < 0) {
		log_error("[%d] socket error %d\n", s, errno);
		net_close(s);
		return;
	}
	buf_update(cinfo->inbuf, n);
	client_process(s, cinfo);
}

/* handler for client's write() */
static void
client_handle_write(SOCKET s, void *p)
{
	struct client_info *cinfo = p;
	unsigned len;
	const void *data = buf_data(cinfo->outbuf, &len);
	if (len) {
		ssize_t cnt = send(s, data, len, 0);
		if (cnt < 0) {
			log_error("[%d] socket error %d\n", s, errno);
		} else if (cnt > 0) {
			buf_consume(cinfo->outbuf, cnt);
			len -= cnt;
		}
	}
	write_ready(s, len != 0);
}

static void
client_freedata(void *p)
{
	struct client_info *cinfo = p;

	if (cinfo) {
		buf_free(cinfo->outbuf);
		cinfo->outbuf = NULL;
		buf_free(cinfo->inbuf);
		cinfo->inbuf = NULL;
		free(cinfo);
	}
}

static void *
client_newdata(const struct event_class *class)
{
	struct client_info *cinfo;

	(void)class; /* unused */

	cinfo = calloc(1, sizeof(*cinfo));
	cinfo->outbuf = buf_new(131072);
	cinfo->inbuf = buf_new(4096);
	cinfo->state = CLIENT_STATE_LOGIN;

	return cinfo;
}

static void
client_handle_init(SOCKET s, void *p)
{
	struct client_info *cinfo = p;

	assert(cinfo != NULL);
	assert(cinfo->outbuf != NULL);
	assert(cinfo->inbuf != NULL);

	show_file(s, cinfo, "WELCOME");
	show_file(s, cinfo, "LOGIN");
}

/******************************************************************************/
/* Server handling implementation */

/* handler for a server's accept(). Applied to a read event. */
static void
server_handle_accept(SOCKET s, void *p)
{
	struct sockaddr_storage ss;
	socklen_t len;
	SOCKET fd;
	struct server_info *sinfo = p;
	const struct event_class *clientclass = sinfo->clientclass;

	fd = accept(s, (struct sockaddr*)&ss, &len);
	if (fd == SOCKET_ERROR)
		return;
	log_debug("[%d] FAMILY=%d\n", fd, (int)ss.ss_family);
	void *data = clientclass->newdata ? clientclass->newdata(clientclass) : NULL;
	if (net_add(fd, data, clientclass)) {
		log_error("%s()\n", __func__);
		close(fd);
	}
}

static void *
server_newdata(const struct event_class *class)
{
	struct server_info *sinfo = calloc(1, sizeof(struct server_info));

	sinfo->clientclass = class->extra;

	return sinfo;
}

/**
 * addr: socket address from getifaddrs() or getaddrinfo()
 * if_hint: string for the related interface, NULL is unknown interface.
 */
static int
server_add(const struct sockaddr *addr, const char *if_hint, unsigned port, const struct event_class *class)
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

	if (net_bind(addr, len, class))
		return -1;

	return 0;
}

static int
interface_add(const char *match_if, unsigned port, const struct event_class *class)
{
	struct ifaddrs *ifaddr, *cur;
	int result = 0;

	getifaddrs(&ifaddr);
	for (cur = ifaddr; cur; cur = cur->ifa_next) {
		if (!cur->ifa_addr)
			continue;
		if (!match_if || !strcmp(cur->ifa_name, match_if)) {
			if (server_add(cur->ifa_addr, cur->ifa_name, port, class))
				result = -1;
		}
	}
	freeifaddrs(ifaddr);

	return result;
}

/******************************************************************************/
/* main program init */

static int server_count; /* used to see if a default server should be created */

static const struct event_class client_class = {
	.evread = client_handle_read,
	.evwrite = client_handle_write,
	.evinit = client_handle_init,
	.newdata = client_newdata,
	.freedata = client_freedata,
};

static const struct event_class server_class = {
	.evread = server_handle_accept,
	.newdata = server_newdata,
	.freedata = free,
	.extra = &client_class,
};

static int
do_flag(const char *option, const char *arg)
{
	if (strcmp(option, "verbose") == 0) {
		verbose++;
		return 0;
	} else if (strcmp(option, "quiet") == 0) {
		verbose = 0;
		return 0;
	} else if (strcmp(option, "port") == 0) {
		unsigned port = atoi(arg);
		// TODO: accept list of interfaces to bind
		interface_add(NULL, port, &server_class);
		server_count++;
		return 0;
	} else {
		fprintf(stderr, "Unknown argument '-%s'\n", option);
		return -1;
	}
}

static int
parse_args(int argc, char **argv)
{
	int i;
	i = 1;
	while (i < argc) {
		const char *s = argv[i++];
		if (*s == '-') {
			int used = do_flag(s + 1, i < argc ? argv[i]: NULL);
			if (used < 0)
				return -1;
			i += used;
		}
	}
	return 0;
}

int
main(int argc, char **argv)
{
	/* setup socket event entries */
	evinfo_max = 1024;
	evinfo = calloc(evinfo_max, sizeof(*evinfo));

	if (parse_args(argc, argv))
		return 1;

	if (server_count == 0) {
		interface_add(NULL, 4444, &server_class);
		server_count++;
	}

	loop();

	return 0;
}

//// BUGS:
//
// - loop() spins as soon as something connects - FIXED - event_len was 0
//
//
//// TODO
//
//
//
//// Design
//
// Concepts:
//
// process - i/o endpoint, timer element, ...
// port - waits on one or more events
//
//// VM
//
// Instructions:
// SYS(id,argc)		system call
// CALL(addr,argc)	call subroutine
// RET				return from subroutine
// JMP(addr)		jump to address / unconditional branch
// B(addr,cond,a,b)	compare and branch
// MOV(dst,src)		move / copy value
// ADD(dst,a,b)		addition
// SUB(dst,a,b)		subtraction
// AND(dst,a,b)		bitwise AND
// OR(dst,a,b)		bitwise OR
// XOR(dst,a,b)		exclusive-OR
// NOT(dst,src)		bitwise NOT / complement
// DIV(q,r,a,b)		division with quotient and remainder
// MUL(dst,a,b)		multiplication
// NEG(dst,src)		negation
// ??? LEA(dst,expr)	load effective address
//
// Registers
// P				program counter
// S				stack pointer
// F				frame pointer / local variable table
// L				link register
// G				global variable table
//
// Addressing
// local variable - frame relative
// absolute
// global variable - 'G' relative
// stack relative - call arguments
// indirect ???
