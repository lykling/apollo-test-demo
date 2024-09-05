#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int fd = open("b/README.md", O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    char buf[1024];

    ssize_t n = read(fd, buf, sizeof(buf));
    if (n == -1) {
        perror("read");
        return 1;
    }
    printf("%s\n", buf);
    return EXIT_SUCCESS;
}
