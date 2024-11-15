#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(void)
{
    int fd1,fd2,i,pos;
    char c;
    

    fd1 = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    close(fd1);
    for (i=1; i < 10; i++) {
        pos = (i*5)+(i-1)*5;
        if (fork() == 0) {
            /* Child */
            char buffer[6];
            fd1 = open("output.txt", O_RDWR);
            sprintf(buffer, "%d", i*11111);
            lseek(fd1, pos, SEEK_SET);
            write(fd1, buffer, 5);
            close(fd1);
            exit(0);
        } else {
            /* Parent */
            fd2 = open("output.txt", O_RDWR);
            int posPadre=(i-1)*5+(i-1)*5;
            lseek(fd2, posPadre, SEEK_SET);
            write(fd2, "00000", 5);
            close(fd2);
          
        }
    }

	//wait for all childs to finish
    while (wait(NULL) != -1);

    fd1 = open("output.txt", O_RDWR);
    lseek(fd1, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0)
        printf("%c", (char) c);
    printf("\n");
    close(fd1);
    exit(0);
}
