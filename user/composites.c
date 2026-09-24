#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include <stdarg.h>

int
log_stdout(uint i)
{
  /*
  Description: Redirect stdout to a log file named i.log.
  Example:
          - log_stdout(1); // Redirect the stdout to 1.log and return 0.
  Parameters:
          - i (uint): A number
  Return:
          - 0 (int)
  */
  char log_name[15] = "0.log";
  uint base = 1;
  uint i_temp;

  if(i != 0){
    for(base = 0, i_temp = i; i_temp != 0; ++base, i_temp /= 10)
      ;

    for(uint base_temp = 0, i_temp = i;
        i_temp != 0;
        ++base_temp, i_temp /= 10){
      log_name[base - base_temp - 1] = '0' + i_temp % 10;
    }

    strcpy(log_name + base, ".log");
  }

  close(1);
  if(open(log_name, O_CREATE | O_WRONLY) != 1){
    fprintf(2, "log_stdout: open failed\n");
    return -1;
  }

  return 0;
}

void
sub_process(int p_left[2], int i)
{
  /**
   * 
   */

  int factor;         // 当前进程负责的质数
  int value;          // 当前从左侧管道读到的数字
  int p_right[2];     // 连接下一个子进程的管道
  int child_pid = -1; // 下一个子进程的pid

  close(p_left[1]);

  // 从左侧管道读取第一个整数，并将它作为自己的筛选因子
  // 这个筛选因子一定是质数
  if(read(p_left[0], &factor, sizeof(factor)) != sizeof(factor)){
    close(p_left[0]);
    return;
  }

  // 重定向stdout到i.log
  if(log_stdout(i) != 0){
    close(p_left[0]);
    exit(1);
  }

  // 刚才已经重定向stdout了，这一步会写入i.log
  printf("%d\n", factor);

  // 循环读取剩余数字
  while(read(p_left[0], &value, sizeof(value)) == sizeof(value)){
    // 能整除，就写入当前日志
    if(value % factor == 0){
      printf("%d\n", value);
      continue;
    }
    
    // 不能整除，就创建新进程来处理剩余数字
    if(child_pid == -1){
      // 创建右侧管道
      // 当前进程 ──p_right──> 下一进程
      if(pipe(p_right) < 0){
        fprintf(2, "composites: pipe failed\n");
        close(p_left[0]);
        exit(1);
      }
      
      // 创建下一进程
      child_pid = fork();
      if(child_pid < 0){
        fprintf(2, "composites: fork failed\n");
        close(p_left[0]);
        close(p_right[0]);
        close(p_right[1]);
        exit(1);
      }
      
      // 新的子进程递归处理
      if(child_pid == 0){
        close(p_left[0]);
        sub_process(p_right, i + 1);
        exit(0);
      }

      close(p_right[0]);
    }

    // 把不能整除的数字传下去
    if(write(p_right[1], &value, sizeof(value)) != sizeof(value)){
      fprintf(2, "composites: write failed\n");
      close(p_left[0]);
      close(p_right[1]);
      wait(0);
      exit(1);
    }
  }

  close(p_left[0]);

  if(child_pid != -1){
    close(p_right[1]);
    wait(0);
  }

  return;
}

int
main(int argc, char *argv[])
{
  int p[2];

  if(argc != 1){
    fprintf(2, "Usage: composites\n");
    exit(1);
  }

  /**
   * pipe(p)创建第一条管道，连接主进程与第0个子进程
   * p[0]: 读取端
   * p[1]: 写入端
   */
  if(pipe(p) < 0){
    fprintf(2, "composites: pipe failed\n");
    exit(1);
  }

  /**
   * 创建第0个子进程
   */
  int pid = fork();
  if(pid < 0){
    fprintf(2, "composites: fork failed\n");
    close(p[0]);
    close(p[1]);
    exit(1);
  }

  // 子进程启动流水线
  if(pid == 0){
    sub_process(p, 0);
    exit(0);
  }

  // 父进程关闭读取端
  close(p[0]);

  // 主进程依次将2-35写入管道
  for(int value = 2; value <= 35; value++){
    if(write(p[1], &value, sizeof(value)) != sizeof(value)){
      fprintf(2, "composites: write failed\n");
      close(p[1]);
      wait(0);
      exit(1);
    }
  }

  // 关闭管道并等待
  close(p[1]);
  wait(0);
  exit(0);
}
