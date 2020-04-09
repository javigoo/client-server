#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>

/* Register packages */
#define REG_REQ 0x00
#define REG_INFO 0x01
#define REG_ACK 0x02
#define INFO_ACK 0x03
#define REG_NACK 0x04
#define INFO_NACK 0x05
#define REG_REJ 0x06

/* Periodic communication packages */
#define ALIVE 0x10
#define ALIVE_REJ 0x11

#define MAX_AUTHORIZED_DEVICES 10
#define ID_SIZE 13

/* Global variables */
bool debug_flag = false;
char configuration_file[] = "server.cfg"; /* A cuanto inicializo configuration_file[] ?*/
char authorized_file[] = "bbdd_dev.dat"; /* A cuanto inicializo authorized_file[] ?*/
char authorized_devices[MAX_AUTHORIZED_DEVICES][ID_SIZE];
struct configuration_data configuration;
int udp_socket, tcp_socket = 0;
int udp_port, tcp_port;
struct sockaddr_in	udp_addr, tcp_addr;

/* Structures */
struct configuration_data
{
  char id[ID_SIZE];
  int udp_port;
  int tcp_port;
};

struct udp_pdu
{
  unsigned char pkg;
  char id[13];
  char rand[9];
  char data[61];
};

/* Functions */
void debug(char msg[]);
void msg(char msg[]);
void parse_args(int argc,char *argv[]);
void read_configuration(struct configuration_data *configuration);
void read_authorized(char authorized_file[]);
void initialize_sockets();
void initialize_threads();
void listen_udp();
void udp_package_process(struct udp_pdu data);
bool check_authorized_device(char device[]);


/* Main function */
int main(int argc,char *argv[])
{
  parse_args(argc, argv);
  read_configuration(&configuration);
  read_authorized(authorized_file);
  initialize_sockets();
  initialize_threads();

  return 1;
}

/* Auxiliary functions */
void msg(char msg[])
{
  time_t timer;
  char msg_buffer[9];
  struct tm* tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(msg_buffer, 9, "%H:%M:%S", tm_info);
  printf("%s: MSG -> %s\n", msg_buffer, msg);
}

void debug(char msg[])
{
  if (debug_flag == true)
  {
    time_t timer;
    char debug_buffer[9];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(debug_buffer, 9, "%H:%M:%S", tm_info);
    printf("%s: DEBUG -> %s\n", debug_buffer, msg);
  }
}

void parse_args(int argc,char *argv[])
{
  /* Lo he hecho de esta forma porque no estoy seguro de poder utilizar <getopt.h> */
  int arg;
  for(arg = 0; arg < argc; arg++)
  {
    char const *opt = argv[arg];
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
            strcpy(authorized_file, argv[arg+1]);
            break;
          }
          fprintf(stderr, "Usage: %s [-u <authorized file>, default=bbdd_dev.dat]\n", argv[0]);
          exit(1);
        default:
          fprintf(stderr, "Usage: %s [-d] "
                          "[-c <configuration file>, default=server.cfg] "
                          "[-u <authorized file>, default=bbdd_dev.dat]\n", argv[0]);
          exit(1);
        }
      }
    }
  }

void read_configuration(struct configuration_data *configuration)
{
  FILE *fp;
  char parameter_buffer[255];
  if ((fp = fopen(configuration_file, "r")) == NULL)
  {
    fprintf(stderr, "Error! opening file %s\n", configuration_file);
    exit(1);
  }

  while (fgets(parameter_buffer, 255, fp))
  {
    char *key_parameter = strtok(parameter_buffer, "= \n");
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

void read_authorized(char authorized_file[])
{
  FILE *fp;
  char device_buffer[255];
  int num_device = 0;
  if ((fp = fopen(authorized_file, "r")) == NULL)
  {
    fprintf(stderr, "Error! opening file %s\n", authorized_file);
    exit(1);
  }

  while (fgets(device_buffer, 255, fp))
  {
    strcpy(authorized_devices[num_device], device_buffer);
    authorized_devices[num_device][ID_SIZE-1] = '\0';
    num_device++;
  }

  fclose(fp);
}

void initialize_sockets()
{
  udp_socket = socket(AF_INET,SOCK_DGRAM,0);
  if(udp_socket<0)
  {
    fprintf(stderr,"Error! opening UDP socket\n");
    exit(1);
  }

  udp_port = configuration.udp_port;

	memset(&udp_addr,0,sizeof (struct sockaddr_in));
	udp_addr.sin_family=AF_INET;
	udp_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	udp_addr.sin_port=htons(udp_port);

  tcp_socket = socket(AF_INET,SOCK_STREAM,0);
  if(tcp_socket<0)
  {
    fprintf(stderr,"Error! opening TCP socket\n");
    exit(1);
  }

  tcp_port = configuration.tcp_port;

	memset(&tcp_addr,0,sizeof (struct sockaddr_in));
	tcp_addr.sin_family=AF_INET;
	tcp_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	tcp_addr.sin_port=htons(tcp_port);

}

void initialize_threads()
{
  debug("initialize_threads() not implemented");
  listen_udp();
}

void listen_udp()
{

  /* Variables temporales */
  int received;
  socklen_t len_addr;
  struct udp_pdu data;
  struct sockaddr_in addr;
  char received_buffer[255];

  debug("UDP socket active");

  if(bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr))<0)
	{
		fprintf(stderr,"Error! binding UDP socket\n");
    exit(-2);
	}

	while(true)
	{
    len_addr = sizeof(struct sockaddr_in);
		received = recvfrom(udp_socket, &data, sizeof(data), 0, (struct sockaddr *) &addr, &len_addr);
		if(received<0)
		{
			fprintf(stderr,"Error! UDP recvfrom\n");
			exit(-2);
		}
    sprintf(received_buffer, "Received: bytes=%i, pkg=%d, id=%s, rand=%s, data=%s", received, data.pkg, data.id, data.rand, data.data);
    debug(received_buffer);

    udp_package_process(data);
	}
}

void udp_package_process(struct udp_pdu data)
{
  char msg_buffer[255];
  if(data.pkg == REG_REQ)
  {
    if(check_authorized_device(data.id))
    {
      if ((strcmp(data.rand, "00000000") == 0) & (strcmp(data.data, "") == 0))
      {
        msg("Attend REG_REQ package");
      }
    }
    else
    {
      sprintf(msg_buffer, "Unauthorized device %s", data.id);
      msg(msg_buffer);
    }
  }
  else
  {
    sprintf(msg_buffer, "Recibido paquete %d", data.pkg);
    msg(msg_buffer);
  }
}

bool check_authorized_device(char device[])
{
  int num_device;
  for (num_device = 0; num_device < MAX_AUTHORIZED_DEVICES; num_device++)
  {
    if(strcmp(authorized_devices[num_device], device) == 0)
    {
      return true;
    }
  }
  return false;
}
