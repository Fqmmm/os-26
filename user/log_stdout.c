#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include <stdarg.h>

char buf[1024];

int read_stdin(char *buf)
{
  /*
  Description: Read stdin into buf
  Example:
          - read_stdin(); // Read the stdin into buf
  Parameters:
          - buf (char*): A buffer to store all characters
  Return:
          - 0 (int)
  */
  int total = 0;

  while(total < 1023){
    /**
     * int read(int fd, void *buf, int n);
     * 从文件描述符 fd 对应的数据源中（0 表示 stdin）
     * 最多读取 n 个字节
     * 并保存到 buf 指向的内存中
     * 返回实际读到的字节数
     */
    int n = read(0, buf + total, 1023 - total);
    if(n < 0){
      fprintf(2, "read_stdin: read failed\n");
      return -1;
    }
    
    // EOF
    if(n == 0)
      break;

    total += n;
  }

  buf[total] = '\0';
  return 0;
}

int log_stdout(uint i)
{
  /*
  Description: Redirect stdout to a log file named i.log
  Example:
          - log_stdout(1); // Redirect the stdout to 1.log and return 0
  Parameters:
          - i (uint): A number
  Return:
          - 0 (int)
  */
  char log_name[15] = "0.log";
  uint base;
  uint i_temp;

  if(i != 0){
    // 计算数字位数
    base = 0;
    i_temp = i;
    while (i_temp != 0) {
      base++;
      i_temp /= 10;
    }

    // 将每一位转换为字符
    for(uint base_temp = 0, i_temp = i;
        i_temp != 0;
        ++base_temp, i_temp /= 10){
      log_name[base - base_temp - 1] = '0' + i_temp % 10;
    }

    // 添加 .log
    strcpy(log_name + base, ".log");
  }

  // 关闭 stdout
  close(1); 

  // 打开 .log，使其成为新的 stdout
  if(open(log_name, O_CREATE | O_WRONLY) != 1){
    fprintf(2, "log_stdout: open failed\n");
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  if(argc != 2){
    fprintf(2, "Usage: log_stdout number\n");
    exit(1);
  }
  if(log_stdout(atoi(argv[1])) != 0){
    fprintf(2, "log_stdout: log_stdout failed\n");
    exit(1);
  }
  if(read_stdin(buf) != 0){
    fprintf(2, "log_stdout: read_stdin failed\n");
  }
  printf(buf);
  exit(0);
}
