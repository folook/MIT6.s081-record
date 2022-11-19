#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

  if (fu(atoi(argv[1])) < 0) {
    fprintf(2, "%s: fu failed\n", argv[0]);
    exit(1);
  }
  
  exit(0);
}
