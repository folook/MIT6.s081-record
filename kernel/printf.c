//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

volatile int panicked = 0;

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

static char digits[] = "0123456789abcdef";

static void
printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
printf(char *fmt, ...)
{
  va_list ap;
  int i, c, locking;
  char *s;

  locking = pr.locking;
  if(locking)
    acquire(&pr.lock);

  if (fmt == 0)
    panic("null fmt");

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }
  va_end(ap);

  if(locking)
    release(&pr.lock);
}

void
panic(char *s)
{
  pr.locking = 0;
  printf("panic: ");
  printf(s);
  printf("\n");
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
  pr.locking = 1;
}









// Stack
//                    .
//                    .
//       +->          .
//       |   +-----------------+   |
//       |   | return address  |   |
//       |   |   previous fp ------+
//       |   | saved registers |
//       |   | local variables |
//       |   |       ...       | <-+
//       |   +-----------------+   |
//       |   | return address  |   |
//       +------ previous fp   |   |
//           | saved registers |   |
//           | local variables |   |
//       +-> |       ...       |   |
//       |   +-----------------+   |
//       |   | return address  |   |
//       |   |   previous fp ------+
//       |   | saved registers |
//       |   | local variables |
//       |   |       ...       | <-+
//       |   +-----------------+   |
//       |   | return address  |   |
//       +------ previous fp   |   |
//           | saved registers |   |
//           | local variables |   |
//   $fp --> |       ...       |   |
//           +-----------------+   |
//           | return address  |   |
//           |   previous fp ------+
//           | saved registers |
//   $sp --> | local variables |
//           +-----------------+



void
backtrace() {
  //难点1，如何识别最后一帧？提示的 PGROUNDDOWN(fp) 没看懂,看懂了，kernel stack是一个4k的page，这个page的使用是从高到低的

  printf("backace:\n");
  uint64 *fp = (uint64 *)r_fp(); //从寄存器中取值，把这个值赋给指针变量fp，指针fp指向了最后一个stack frame的顶部
  while (PGROUNDDOWN((uint64)fp) != (uint64)fp)//fp本就是一个uint64，所以强转没影响
  {
    uint64 * retaddr = fp - 1;//指针运算，stack frame的前8byte存着return adress
    printf("%p\n", *retaddr);//使用 * 取值符取出return address并打印
    fp = (uint64 *)(*(fp - 2));//修改fp指向的地址，fp现在的值改为previous fp，注意要把previous fp转为(uint64 *)，才能赋值，这里的强转没有任何影响，因为previous fp本身就是一个地址
  }
 
}

// printf.c  另一种写法，直接使用uint64承接 r_fp()

// void backtrace() {
//   uint64 fp = r_fp();
  
//   while(fp != PGROUNDUP(fp)) { // 如果已经到达栈底
//     uint64 ra = *(uint64*)(fp - 8); // return address
//     printf("ra %p\n", ra);
//     fp = *(uint64*)(fp - 16); // previous fp
//   }
// }