
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]) {

char buf[1];

    int pid;

    int status;

    int fd1[2];

    // int fd2[2];

    if(pipe(fd1) == -1){ //创建管道，并把文件描述符保存在fd[0]和fd[1]中。
                exit(1);
        }

        //     if(pipe(fd2) == -1){ //创建管道，并把文件描述符保存在fd[0]和fd[1]中。
        //         exit(1);
        // }

    pid = fork();

    if(pid == 0){
        sleep(10);
        
read(fd1[0], buf, sizeof(buf));
printf("%d: received ping\n", getpid());
write(fd1[1], buf, 1);
exit(5);


    } else {
        write(fd1[1], buf, 1);
        wait(&status);

        
        read(fd1[0], buf, sizeof(buf));
        printf("%d: received pong\n", getpid());
exit(0);
    }


    exit(0);

}