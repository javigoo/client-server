#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

/* Global variables */
bool debug_flag = false;
char configuration_file[] = "server.cfg"; /* A cuanto inicializo configuration[] ?*/

/* Structures */
struct configuration_data
{
  char id[13];
  int udp_port;
  int tcp_port;
};

/* Functions */
void debug(char msg[]);
void msg(char msg[]);
void parse_args(int argc,char *argv[]);
void read_configuration(struct configuration_data *configuration);


/* Main function */
int main(int argc,char *argv[])
{
  struct configuration_data configuration;

  msg("Server start");

  parse_args(argc, argv);
  read_configuration(&configuration);

  printf("\nConfiguration:\n");
  printf("id = %s\n", configuration.id);
  printf("udp_port = %i\n", configuration.udp_port);
  printf("tcp_port = %i\n", configuration.tcp_port);

  return 1;
}

/* Auxiliary functions */
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

void parse_args(int argc,char *argv[])
{
  /* Lo he hecho de esta forma porque no estoy seguro de poder utilizar <getopt.h> */
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
          if (argc-arg >= 2)
          {
            strcpy(configuration_file, argv[arg+1]);
            break;
          }
          fprintf(stderr, "Usage: %s [-c <configuration file>, default=server.cfg]\n", argv[0]);
          exit(1);
        case 'u':
          if (argc-arg >= 2)
          {
            printf("Reading '%s' authorized file\n", argv[arg+1]);
            msg("Select authorized file - Not Implemented");
            break;
          }
          fprintf(stderr, "Usage: %s [u <authorized file>, default=bbdd_dev.dat]\n", argv[0]);
          exit(1);
        default:
          fprintf(stderr, "Usage: %s [-d] "
                          "[-c <configuration file>, default=server.cfg] "
                          "[u <authorized file>, default=bbdd_dev.dat]\n", argv[0]);
          exit(1);
        }
      }
    }
  }

void read_configuration(struct configuration_data *configuration)
{
  char buffer[255];
  FILE *fp;
  if ((fp = fopen(configuration_file, "r")) == NULL)
  {
    fprintf(stderr, "Error! opening file %s\n", configuration_file);
    exit(1);
  }

  while (fgets(buffer, 255, fp))
  {
    char *key_parameter = strtok(buffer, "= \n");
    if (key_parameter)
    {
      char *parameter_value = strtok(NULL, "= \n");
      if (parameter_value)
      {
        if(strcmp(key_parameter, "Id") == 0)
        {
          strcpy(configuration->id, parameter_value);
        }
        if (strcmp(key_parameter, "UDP-port") == 0)
        {
          configuration->udp_port = atoi(parameter_value);
        }
        if (strcmp(key_parameter, "TCP-port") == 0)
        {
          configuration->tcp_port = atoi(parameter_value);
        }
      }
    }
  }

  fclose(fp);
}
