#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(2, "please add args\n");
        exit(1);

    }


    if(argc == 1) {
        fprintf(2, "1 args\n");
        fprintf(1, argv[0]);
        fprintf(1, "\n");
        exit(1);

    }

    char * s = argv[1];
    int n = atoi(s);
    sleep(n);

    exit(0);

}