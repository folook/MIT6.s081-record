#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


//此函数主要实现提取目标文件名的功能
//输入/abc,输出 abc；输入 /abc/de 输出 de；输入 abc 输出 abc;
//主要思想史使用游标 p 从后往前遍历，直到遇到第一个'/'字符
char*
fmtname(char *path)
{
  //文件名没超过 15 字节，将文件名拷贝到 buf，后面填充 0，超过 15byte 就用原指针
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  //path是指针，path + 4 是指针运算，p 指向 path 指针字符串的结束符
  //循环判断条件：p 地址大于 path 地址 && p 指向的字符不是'/'
  //在 path 有'/'的情况下，循环结束后，p 指向'/'
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  //把 p 加回来，p 指向「最后一个的'/'后的第一个字符」，或 path（path 没'/'的情况）
  p++;

  // Return blank-padded name.
  //strlen从 p 指针位置开始扫描，直到碰到第一个字符串结束符'\0'为止（这个结束符是 path 设置的）
  //检查文件的名字是否大于 14 个字节
  if(strlen(p) >= DIRSIZ)
    return p;
  //从 p 复制 n 个字符到 buf(把文件名复制到 buf)
  memmove(buf, p, strlen(p));
  //复制字符 ' '到参数 buf+strlen(p) 所指向的字符串的前 DIRSIZ-strlen(p) 个字符。
  // 把 buf长度为 14+1 字节，前strlen(p) + 1字节为文件名 + 字符串结束符，后 14-strlen(p)为空格
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;//这个指的是文件的统计信息（kernel/stat.h），包含文件类型/大小/引用数/存放fs的disk dev

  //打开文件，第二个参数指示的是打开方式，0代表的是O_RDONLY只读的形式。
  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }
  //xv6 的系统调用，将一个打开的文件的信息放入 st 中，需要以指针作为参数 
  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  //拿到文件信息后判断文件类型
  switch(st.type){
  case T_DEVICE://注意这里没有 break，说明遇到设备类型的以文件类型处理
  case T_FILE: //如ls echo或者 ls /echo 命令
    //打印出 文件名、文件类型（1目录 2文件）、Inode number、（uint64：unsigned long）类型的 byte
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR://遇到目录类型
  // printf("目录类型\n");
  //如果 路径长度 + 1 + 14 + 1 > 512（说白了 strlen("/abc/def/...) <= 496
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    // printf("path 为：%s\n", path);
    strcpy(buf, path); //path 字符串复制到 buf
    p = buf+strlen(buf); //p 指向字符串结束符'\0'
    //存疑，但结果是成功拼接字符'/'到 path 字符串后，buf 字符串由 : abc -> abc/
    *p++ = '/';//“*”优先级低于“++”，语义上等价于“*(p++)”
    //访问目录内容。每次read只是read一个de的大小（也就是一个目录项），只有read到最后一个目录项的下一次read才会返回0，也就不满足while循环条件退出循环，
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)//此文件夹无文件，continue操作后进行下一次read 
      {
        continue;
      }  
      memmove(p, de.name, DIRSIZ);//将de.name的内容复制到p指针中，比 memcpy() 更安全,这里最关键！
      p[DIRSIZ] = 0;//字符串末尾加 0？
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
       }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
