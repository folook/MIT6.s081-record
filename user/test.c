#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
 
char*
fmtname(char *path) //格式化名字，把名字变成前面没有左斜杠/，仅仅保存文件名
{
  static char buf[DIRSIZ+1];
  char *p;
 
  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
 
  // Return blank-padded name.
  memmove(buf, p, strlen(p) + 1);
  return buf;
}
 
void
find(char *path, char* findName)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
 
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
 
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
 
  switch(st.type){
  case T_FILE:// 如果是文件类型，那么比较，文件名是否匹配，匹配则输出
    if(strcmp(fmtname(path), findName) == 0)
      printf("%s\n", path);
    break;
  case T_DIR://如果是目录则递归去查找
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';//buf是一个绝对路径，p是一个文件名，并通过加"/"前缀拼接在buf的后面
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0) {
        continue;
        }
      memmove(p, de.name, DIRSIZ);//memmove, 把de.name信息复制p,其中de.name是char name[255],代表文件名
      p[strlen(de.name)] = 0; // 设置文件名结束符
        if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                continue;
        }
        find(buf, findName);
    }
    break;
  }
  close(fd);
}
 
int
main(int argc, char *argv[])
{
 
  if(argc < 3){
        printf("error argc num");
    exit(0);
  }
  find(argv[1], argv[2]);
  exit(0);
}
