#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Global variables */
bool debug_flag = false;

/* Functions */
void parse_args(int argc,char *argv[]);
void debug(char msg[]);
void msg(char msg[]);

/* Main function */
int main(int argc,char *argv[])
{
  parse_args(argc, argv);
  debug("Debug activado");
  msg("Mensajes activados");

  return 1;
}

/* Auxiliary functions */
void parse_args(int argc,char *argv[])
{
  int arg;
  for(arg = 0; arg < argc; arg++)
  {
    char const *opt = argv[arg];
    if (opt[0]== '-')
    {
      switch (opt[1])
      {
        case 'd':
          debug_flag = true;
          break;
        case 'c':
          printf("cccccc\n");
          break;
        case 'u':
          printf("uuuuuu\n");
          break;
        default:
          fprintf(stderr, "Usage: %s [-d] "
                          "[-c <configuration file>, default=server.cfg] "
                          "[u <authorized file>, default=bbdd_dev.dat]\n", argv[0]);
          exit(0);
        }
      }
    }
  }

void debug(char msg[])
{
  if (debug_flag == true)
  {
    time_t timer;
    char buffer[9];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 9, "%H:%M:%S", tm_info);
    printf("%s: DEBUG -> %s\n", buffer, msg);
  }
}

void msg(char msg[])
{
  time_t timer;
  char buffer[9];
  struct tm* tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(buffer, 9, "%H:%M:%S", tm_info);
  printf("%s: MSG -> %s\n", buffer, msg);
}
