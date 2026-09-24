#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define LINE_SIZE 512

static int
is_blank(char c)
{
  return c == ' ' || c == '\t' || c == '\r';
}

static int
run_command(char *command, char *base_argv[], int base_argc,
            char *line, int line_length)
{
  /**
   * 把一行输入转换成参数，然后执行命令。
   * Args:
   *  command: 需要执行的命令，例如 echo
   *  base_argv: 命令原本具有的参数数组
   *  base_argc: 命令原本具有的参数数量
   *  line: 从stdin读取的一行
   *  line_length: line的长度
   */
  char *exec_argv[MAXARG];
  int exec_argc = 0;
  int pos = 0;

  // 复制命令原有参数
  for(int i = 0; i < base_argc; i++)
    exec_argv[exec_argc++] = base_argv[i];

  line[line_length] = '\0';

  // 拆分当前输入行
  while(pos < line_length){
    while(pos < line_length && is_blank(line[pos])){
      line[pos] = '\0';
      pos++;
    }

    if(pos == line_length)
      break;

    if(exec_argc >= MAXARG - 1){
      fprintf(2, "xargs: too many arguments\n");
      return -1;
    }

    exec_argv[exec_argc++] = &line[pos];

    while(pos < line_length && !is_blank(line[pos]))
      pos++;

    if(pos < line_length){
      line[pos] = '\0';
      pos++;
    }
  }

  // 忽略空行
  if(exec_argc == base_argc)
    return 0;

  exec_argv[exec_argc] = 0;

  int pid = fork();
  if(pid < 0){
    fprintf(2, "xargs: fork failed\n");
    return -1;
  }

  // 子进程执行命令
  if(pid == 0){
    exec(command, exec_argv);
    fprintf(2, "xargs: exec %s failed\n", command);
    exit(1);
  }

  wait(0);
  return 0;
}

int
main(int argc, char *argv[])
{
  char line[LINE_SIZE];
  int line_length = 0;
  char c;
  int n;

  if(argc < 2){
    fprintf(2, "Usage: xargs command [arguments ...]\n");
    exit(1);
  }

  if(argc - 1 >= MAXARG){
    fprintf(2, "xargs: too many initial arguments\n");
    exit(1);
  }

  while((n = read(0, &c, 1)) > 0){
    if(c == '\n'){
      if(run_command(argv[1], &argv[1], argc - 1,
                     line, line_length) < 0)
        exit(1);
      line_length = 0;
      continue;
    }

    if(line_length >= LINE_SIZE - 1){
      fprintf(2, "xargs: input line too long\n");
      exit(1);
    }

    line[line_length++] = c;
  }

  if(n < 0){
    fprintf(2, "xargs: read failed\n");
    exit(1);
  }

  if(line_length > 0){
    if(run_command(argv[1], &argv[1], argc - 1,
                   line, line_length) < 0)
      exit(1);
  }

  exit(0);
}

