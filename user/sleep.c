#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if(argc == 0) {
        write(1, "please add args", 16);

    }

    char * s = argv[0];
    int n = atoi(s);
    sleep(n);

    exit(0);

}