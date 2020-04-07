#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

/* Global variables */
bool debug_flag = false;
char configuration[] = "server.cfg"; /* A cuanto inicializo configuration[] ?*/

/* Structures */
struct configuration_data
{
  char id[13];
  int udp_port;
  int tcp_port;
};

/* Functions */
void parse_args(int argc,char *argv[]);
void debug(char msg[]);
void msg(char msg[]);
void read_configuration(struct configuration_data *configuration);


/* Main function */
int main(int argc,char *argv[])
{
  struct configuration_data configuration;

  msg("Server start");

  parse_args(argc, argv);
  read_configuration(&configuration);

  return 1;
}

/* Auxiliary functions */
void parse_args(int argc,char *argv[])
{
  int arg;
  for(arg = 0; arg < argc; arg++)
  {
    char const *opt = argv[arg];  /* char const ? */
    if (opt[0] == '-')
    {
      switch (opt[1])
      {
        case 'd':
          debug_flag = true;
          break;
        case 'c':
          if (argc-arg == 2)
          {
            printf("Reading '%s' configuration file\n", argv[arg+1]);
            strcpy(configuration, argv[arg+1]);
            break;
          }
          fprintf(stderr, "Usage: %s [-c <configuration file>, default=server.cfg]\n", argv[0]);
          exit(0);
        case 'u':
          if (argc-arg >= 2)
          {
            printf("Reading '%s' authorized file\n", argv[arg+1]);
            msg("Select authorized file - Not Implemented");
            break;
          }
          fprintf(stderr, "Usage: %s [u <authorized file>, default=bbdd_dev.dat]\n", argv[0]);
          exit(0);
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

void read_configuration(struct configuration_data *configuration)
{
  debug("Reading configuration...");
}
