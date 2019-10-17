#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/inotify.h>


void
watch_directory(int watch_fd, const char *directory)
{
    // start watching directory itself
    int dir_flags = IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_TO | IN_MOVED_FROM | IN_MOVE_SELF;
    int dir_fd = inotify_add_watch(watch_fd, directory, dir_flags);
    if (dir_fd == -1) {
        perror("Couldnt watch fd for directory");
        return;
    }

    DIR *dir;
    struct dirent *dp;

    // Start watching contents
    dir = opendir(directory);
    if (dir == NULL) {
        perror("Couldn't read directory");
        return;
    }
    
    // Check all of the files in this directory
    // in a DFS manner
    while (( dp = readdir(dir) ) != NULL) {



        // Recursivley start watching a subdir
        if (dp->d_type == DT_DIR) {

            // Dont include '.' and '..'
            if (dp->d_name[0] == '.' || dp->d_name[1] == '.') {
                continue;
            }

            // Legit directory
            printf("Found dir %s\n", dp->d_name);
            watch_directory(watch_fd, dp->d_name);
        }
    }
    closedir(dir);
    return;
}

int
watch_dir(const char *filepath) {
    int watch_fd = inotify_init();//inotify_init1(IN_NONBLOCK);
    if (watch_fd == -1) {
        perror("inotify_init failed");
        exit(EXIT_FAILURE);
    }

    // watch subdirs and files
    watch_directory(watch_fd, filepath);
    return watch_fd;
}

int
main(int argc, char **argv)
{
    int watch_fd;
    char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;
    char *ptr;

    watch_fd = watch_dir(".");

    for (;;) {
        len = read(watch_fd, buf, sizeof buf);
        if (len == -1 && errno != EAGAIN) {
            perror("Error during read()");
            return 1;
        }

        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len){
            event = (const struct inotify_event *) ptr;

            switch (event->mask)
            {
            case IN_CREATE:
                printf("Created file %s\n", event->name);
                break;
            
            case IN_DELETE:
                printf("Deleted file %s\n", event->name);
                break;

            case IN_DELETE_SELF:
                printf("Deleted dir %s\n", event->name);
                break;

            case IN_MOVED_TO:
                printf("Moved file %s\n", event->name);
                break;

            case IN_MOVED_FROM:
                printf("Moved file %s into dir\n", event->name);
                break;

            case IN_MOVE_SELF:
                printf("Dir %s file moved out of watch dir\n", event->name);
                break;
            
            default:
                printf("Unknown event returned from Inotify. MASK %u\n", event->mask);
                break;
            }

        }

    }

    return 0;
}



