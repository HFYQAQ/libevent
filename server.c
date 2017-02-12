#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "event-internal.h"
#include "event.h"
#include "log.h"

#define PORT 8888
#define BACKLOG 5

struct event_base *base;

int start();
void handle_event(struct event *, int, short, void *, void (*)(int, short, void *));
void listen_cb(int, short, void *);

int main() {
	int listenfd;
	base = event_base_new();

	listenfd = start();
	struct event listenev;
	handle_event(&listenev, listenfd, EV_READ | EV_PERSIST, NULL, listen_cb);

	event_base_loop(base);

	return 0;
}

int start() {
	int listenfd = -1;
	struct sockaddr_in servaddr;
	int opt_reuse = 1;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt_reuse, sizeof(opt_reuse)) < 0)
		event_err(1, EVENT_LOG_HEAD "setsockopt: ", __FILE__, __func__, __LINE__);
	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) < 0)
		event_err(1, EVENT_LOG_HEAD "bind: ", __FILE__, __func__, __LINE__);

	socklen_t len = sizeof(servaddr);
	if (getsockname(listenfd, (struct sockaddr *) &servaddr, &len) < 0)
		event_err(1, EVENT_LOG_HEAD "getsockname: ", __FILE__, __func__, __LINE__);

	if (listen(listenfd, BACKLOG) < 0)
		event_err(1, EVENT_LOG_HEAD "listen: ", __FILE__, __func__, __LINE__);

	event_log("server started! ip: %d:%d", servaddr.sin_addr.s_addr, ntohs(servaddr.sin_port));
	return listenfd;	
}

void handle_event(struct event *ev, int fd, short type, void *arg, void (*cb)(int, short, void *)) {
	event_set(ev, fd, type, arg, cb);
	event_base_set(base, ev);
	event_add(ev, NULL);
}

void listen_cb(int fd, short type, void *arg) {

}