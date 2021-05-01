#include "poll.h"
#include "display.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

int
display_poll(long msec)
{
        int dfd = STDIN_FILENO; // TODO: display_fd()
        int maxfd = dfd; // TODO: max(dfd, vc_maxfd())
        int result;
        fd_set rfds;
	struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(dfd, &rfds);
	if (msec >= 0) {
		tv.tv_sec = msec / 1000;
		tv.tv_usec = (msec % 1000) * 1000;
	}
        result = select(maxfd + 1, &rfds, NULL, NULL, msec < 0 ? NULL : &tv);
        if (result == 0) {
                return 0;
        } else if (result < 0) {
                perror(__func__);
                return -1;
        }
        if (FD_ISSET(dfd, &rfds))
                display_update();
        return 0;
}
