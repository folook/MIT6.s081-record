#include "kernel/sysinfo.h"
#include "user/user.h"
#include "kernel/param.h"
#include "kernel/types.h"

int main(int argc, char* argv[]) 
{
    if(argc != 1) {
        fprintf(2, "sysinfo need not param\n", argv[0]);
        exit(1);
    
    }
    fprintf(0, "sysinfo in\n");
    struct sysinfo sinfo;
    sysinfo(&sinfo);
    printf("free space: %d, used process num: %d\n", sinfo.freemem, sinfo.nproc);
    
    exit(0);

}