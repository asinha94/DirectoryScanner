#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>

int
main(int argc, char **argv)
{
    int fd = inotify_init();//inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1 failed");
        return 1;
    }

    int dir_fd = inotify_add_watch(fd, ".", IN_CREATE);
    if (dir_fd == -1) {
        perror("Could watch fd for .");
        return 1;
    }

    char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;
    char *ptr;

    for (;;) {
        len = read(fd, buf, sizeof buf);
        if (len == -1 && errno != EAGAIN) {
            perror("Error during read()");
            return 1;
        }

        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len){
            event = (const struct inotify_event *) ptr;

            if (event->mask & IN_CREATE) {
                printf("Created file %s\n", event->name);
            }
        }

        
    }

    
    

    
    return 0;
}



