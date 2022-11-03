#include "kernel/types.h"
#include "user/user.h"
#include <stddef.h>

void process(int p[]) {
    int prime;
    //子进程首先关闭管道的写
    close(p[1]);

    //从管道中读取数据,read会返回独到的字节数,使用 read 返回值作为读完的标志
    if(read(p[0], &prime, sizeof(int)) > 0) {
        //每个进程读到的第一个数字一定是素数
        fprintf(1, "prime %d\n", prime);

        int p2[2];
        pipe(p2);
       //开始 fork
       //父进程要 write
       if(fork() > 0) {
            //关闭父进程 read 端
            close(p2[0]);
            //继续往下一个进程写
            int i;
            while(read(p[0], &i, sizeof(int)) > 0) {
                // fprintf(1, "next proocess write %d\n", i);
                if(i % prime != 0){
                    write(p2[1], &i, sizeof(int));
                }
            }
            //写完之后顺手关闭
            close(p2[1]);
            wait(0);

       }
       else{

            // 关闭上一个进程的读端，用不到了，这里不容易想到
            close(p[0]);
            process(p2);

       }

       exit(0);

    }

}

int main(int argc, char* argv[]) {

    int p[2];
    pipe(p);
    int pid = fork();

    if(pid > 0) {
                //父进程关闭管道读端
        close(p[0]);
        fprintf(1, "prime 2\n");
        //父进程往管道中写
        for(int i = 3; i <= 35; i++) {
            if(i % 2 != 0) {
                //write函数：将指针指向的内存地址处的内容写入文件描述符指向的文件
                write(p[1], &i, sizeof(int));
            }
        }
        //写完后关闭写端
        close(p[1]);
        wait(0); 
    } 
    else{
        process(p);
    }
    exit(0);

}


