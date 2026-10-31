#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd; //file descriptor
    size_t length; //length of string to write
    size_t written = 0; //number of bytes written

    // Open the syslog for logging
    openlog("writer", LOG_PID, LOG_USER);

    // Check if the correct number of arguments is provided
    if (argc < 3) {
        fprintf(stderr, "Error: writefile and writestr are required.\n");
        fprintf(stderr, "Usage: %s <writefile> <writestr>\n", argv[0]);
        closelog();
        return 1;
    }

    // Log the action of writing to the specified file
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);

    // Open the file for writing, creating it if it doesn't exist, and truncating it if it does
    fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        int error = errno;

        syslog(LOG_ERR, "Could not open %s: %s", argv[1], strerror(error));
        fprintf(stderr, "Error: Could not open %s: %s\n", argv[1],
                strerror(error));
        closelog();
        return 1;
    }

    length = strlen(argv[2]); // Get the length of the string to write
    // Write the string to the file in a loop to handle partial writes
    while (written < length) {
        ssize_t result = write(fd, argv[2] + written, length - written);

        if (result <= 0) {
            if (result == -1 && errno == EINTR) {
                continue;
            }

            if (result == 0) {
                errno = EIO;
            }
            {
                int error = errno;

                syslog(LOG_ERR, "Could not write to %s: %s", argv[1],
                       strerror(error));
                fprintf(stderr, "Error: Could not write to %s: %s\n", argv[1],
                        strerror(error));
            }
            close(fd);
            closelog();
            return 1;
        }

        written += (size_t)result;
    }

    // Close the file descriptor and handle any errors
    if (close(fd) == -1) {
        int error = errno;

        syslog(LOG_ERR, "Could not close %s: %s", argv[1], strerror(error));
        fprintf(stderr, "Error: Could not close %s: %s\n", argv[1],
                strerror(error));
        closelog();
        return 1;
    }

    // close the syslog and return success
    closelog();
    return 0;
}