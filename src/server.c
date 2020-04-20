#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

/* Client states */
#define DISCONNECTED 0xa0
#define NOT_REGISTERED 0xa1
#define WAIT_ACK_REG 0xa2
#define WAIT_INFO 0xa3
#define WAIT_ACK_INFO 0xa4
#define REGISTERED 0xa5
#define SEND_ALIVE 0xa6

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

#define MAX_DEVICES 10
#define ID_SIZE 13
#define MAX_COMMAND_SIZE 4

/* Structures */
struct configuration_data
{
  char id[ID_SIZE];
  int udp_port;
  int tcp_port;
};

struct client_info
{
  char id[ID_SIZE];
  int state;
  char rand[9];
  int alives_not_received;
  char elements[61];
  char ip[9];
};

struct udp_pdu
{
  unsigned char pkg;
  char id[ID_SIZE];
  char rand[9];
  char data[61];
};

/* Global variables */
bool debug_flag = false;
char configuration_file[] = "server.cfg";
char authorized_file[] = "bbdd_dev.dat";
char authorized_devices[MAX_DEVICES][ID_SIZE];
struct client_info clients[MAX_DEVICES];
struct configuration_data configuration;
struct sockaddr_in	addr_server_udp, addr_server_tcp;
int udp_socket, tcp_socket = 0;
int udp_port, tcp_port;
bool thread_flag = true;
pthread_t udp_thread;

/* Functions */
void debug(char msg[]);
void msg(char msg[]);
void parse_args(int argc,char *argv[]);
void read_configuration(struct configuration_data *configuration);
void read_authorized(char authorized_file[]);
void initialize_sockets();
void initialize_threads();
void read_commands();
void command_quit();
void command_list();
void *listen_udp();
void process_udp_package(struct udp_pdu data, struct sockaddr_in addr);
bool check_authorized_device(char device[]);
void reg_req_pkg(struct udp_pdu data, struct sockaddr_in addr);
void reg_info_pkg(int client_num);
void wait_first_alive(struct udp_pdu data, struct sockaddr_in addr);
void alive_pkg(struct udp_pdu data, struct sockaddr_in addr);
void keep_alive(struct udp_pdu data, struct sockaddr_in addr);
int get_client(char id[ID_SIZE]);
char *get_string_state(int state);

/* Main function */
int main(int argc,char *argv[])
{
  srand(time(NULL));

  parse_args(argc, argv);
  read_configuration(&configuration);
  read_authorized(authorized_file);
  initialize_sockets();
  initialize_threads();
  read_commands();

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
  printf("%s: MSG\t-> %s\n", msg_buffer, msg);
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
    printf("%s: DEBUG\t-> %s\n", debug_buffer, msg);
  }
}

void parse_args(int argc,char *argv[])
{
  /*(Lo he hecho de esta forma porque no estoy seguro de poder utilizar <getopt.h>)*/
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
  int rand_number;
  char rand_number_str[9];

  if ((fp = fopen(authorized_file, "r")) == NULL)
  {
    fprintf(stderr, "Error! opening file %s\n", authorized_file);
    exit(1);
  }

  while (fgets(device_buffer, 255, fp))
  {
    strcpy(authorized_devices[num_device], device_buffer);
    authorized_devices[num_device][ID_SIZE-1] = '\0';

    /* Setting client info */
    strcpy(clients[num_device].id, authorized_devices[num_device]);
    clients[num_device].state = DISCONNECTED;
    clients[num_device].alives_not_received = 0;

    /* Set random number */
    rand_number = (rand() % 1000000 + 9999999);
    sprintf(rand_number_str, "%i", rand_number);
    strcpy(clients[num_device].rand, rand_number_str);

    num_device++;
  }

  fclose(fp);
}

int get_client(char id[ID_SIZE])
{
  int num_client;
  for (num_client = 0; num_client < MAX_DEVICES; num_client++)
  {
    if (strcmp(clients[num_client].id, id) == 0) {
      return num_client;
    }
  }
  return -1;
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

	memset(&addr_server_udp,0,sizeof (struct sockaddr_in));
	addr_server_udp.sin_family=AF_INET;
	addr_server_udp.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_server_udp.sin_port=htons(udp_port);

  tcp_socket = socket(AF_INET,SOCK_STREAM,0);
  if(tcp_socket<0)
  {
    fprintf(stderr,"Error! opening TCP socket\n");
    exit(1);
  }

  tcp_port = configuration.tcp_port;

	memset(&addr_server_tcp,0,sizeof (struct sockaddr_in));
	addr_server_tcp.sin_family=AF_INET;
	addr_server_tcp.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_server_tcp.sin_port=htons(tcp_port);

}

void initialize_threads()
{
  pthread_t udp_thread;

  debug("UDP thread initialized");
  pthread_create(&udp_thread, NULL, listen_udp, NULL);

}

void read_commands()
{
  char command[MAX_COMMAND_SIZE];

  while(true)
  {
    scanf("%4s", command);

    if(strcmp(command, "quit") == 0)
    {
      command_quit();
    }
    else if(strcmp(command, "list") == 0)
    {
      command_list();
    }
  }
}

void command_quit()
{
  close(udp_socket);
  close(tcp_socket);
  pthread_cancel(udp_thread);
  exit(1);
}

void command_list()
{
  int device;
  printf("ID\t\tRNDM\t\tIP\t\tSTATE\t\tELEMENTS\n");
  for (device = 0; device < MAX_DEVICES; device++) {
    if (strcmp(clients[device].id, "") != 0) {
      printf("%s\t%s\t%s\t%s\t%s\n",clients[device].id, clients[device].rand, "127.0.0.1", get_string_state(clients[device].state), clients[device].elements);
    }
  }
}

char *get_string_state(int state)
{
  switch (state)
  {
    case DISCONNECTED:
      return "DISCONNECTED";
    case NOT_REGISTERED:
      return "NOT_REGISTERED";
    case WAIT_ACK_REG:
      return "WAIT_ACK_REG";
    case WAIT_INFO:
      return "WAIT_INFO";
    case WAIT_ACK_INFO:
      return "WAIT_ACK_INFO";
    case REGISTERED:
      return "REGISTERED";
    case SEND_ALIVE:
      return "SEND_ALIVE";
    default:
      return NULL;
  }
}

void *listen_udp()
{
  int received;
  struct udp_pdu data;
  struct sockaddr_in addr;
  socklen_t len_addr;

  debug("UDP socket active");

  if(bind(udp_socket, (struct sockaddr *)&addr_server_udp, sizeof(addr_server_udp))<0)
	{
		fprintf(stderr,"Error! binding UDP socket\n");
    exit(-2);
	}

	while(thread_flag)
	{
    len_addr = sizeof(struct sockaddr_in);
		received = recvfrom(udp_socket, &data, sizeof(data), 0, (struct sockaddr *) &addr, &len_addr);
		if(received<0)
		{
			fprintf(stderr,"Error! UDP recvfrom\n");
			exit(-2);
		}
    process_udp_package(data, addr);
	}
  return NULL;
}

void process_udp_package(struct udp_pdu data, struct sockaddr_in addr)
{
  char msg_buffer[255];

  if(data.pkg == REG_REQ)
  {
    sprintf(msg_buffer, "Received: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(data), "REG_REQ", data.id, data.rand, data.data);
    debug(msg_buffer);

    reg_req_pkg(data, addr);

  }
  else if(data.pkg == ALIVE)
  {
    alive_pkg(data, addr);
  }
}

bool check_authorized_device(char device[])
{
  int num_device;
  for (num_device = 0; num_device < MAX_DEVICES; num_device++)
  {
    if(strcmp(authorized_devices[num_device], device) == 0)
    {
      return true;
    }
  }
  return false;
}

void reg_req_pkg(struct udp_pdu data, struct sockaddr_in addr)
{
  int sent;
  socklen_t len_addr;
  struct udp_pdu reg_ack_pdu;
  char msg_buffer[255];
  struct sockaddr_in	new_addr_server_udp;
  char tmp_port[255];
  int client_num;

  if(check_authorized_device(data.id))
  {
    /* get_client seria mejor si devolviese un struct client_info */
    client_num = get_client(data.id);

    if ((strcmp(data.rand, "00000000") == 0) & (strcmp(data.data, "") == 0))
    {
      if (clients[client_num].state == DISCONNECTED)
      {
        /* Open new UDP port */
        /*(Como obtener el nuevo puerto aleatorio?)*/
        memset(&new_addr_server_udp,0,sizeof (struct sockaddr_in));
        new_addr_server_udp.sin_family=AF_INET;
        new_addr_server_udp.sin_addr.s_addr=htonl(INADDR_ANY);
        new_addr_server_udp.sin_port = 0;
        /*printf("%i\n", new_addr_server_udp.sin_port);*/

        reg_ack_pdu.pkg = REG_ACK;
        strcpy(reg_ack_pdu.id, configuration.id);
        strcpy(reg_ack_pdu.rand, clients[client_num].rand);

        /* Temporalmente utilizare el mismo puerto*/
        sprintf(tmp_port, "%i", configuration.udp_port);
        strcpy(reg_ack_pdu.data, tmp_port);

        len_addr = sizeof(addr);
        sent = sendto(udp_socket, &reg_ack_pdu, sizeof(reg_ack_pdu), 0, (struct sockaddr *) &addr, len_addr);

        if(sent<0)
        {
          fprintf(stderr,"Error! UDP sendto\n");
          exit(-2);
        }

        sprintf(msg_buffer, "Sent: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(reg_ack_pdu), "REG_ACK", reg_ack_pdu.id, reg_ack_pdu.rand, reg_ack_pdu.data);
        debug(msg_buffer);

        clients[client_num].state = WAIT_INFO;
        sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "WAIT_INFO");
        msg(msg_buffer);

        reg_info_pkg(client_num);

      }
      else
      {
        clients[client_num].state = DISCONNECTED;
        sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "DISCONNECTED");
        msg(msg_buffer);

        /*(Finalizar registro. exit()?)*/
      }
    }
    else
    {
      clients[client_num].state = DISCONNECTED;
      sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "DISCONNECTED");
      msg(msg_buffer);

      reg_ack_pdu.pkg = REG_REJ;
      strcpy(reg_ack_pdu.id, configuration.id);
      strcpy(reg_ack_pdu.rand, "00000000");
      strcpy(reg_ack_pdu.data, "Discrepancy with identification data");

      len_addr = sizeof(addr);
      sent = sendto(udp_socket, &reg_ack_pdu, sizeof(reg_ack_pdu), 0, (struct sockaddr *) &addr, len_addr);
    }
  }
  else
  {
    reg_ack_pdu.pkg = REG_REJ;
    strcpy(reg_ack_pdu.id, configuration.id);
    strcpy(reg_ack_pdu.rand, "00000000");
    strcpy(reg_ack_pdu.data, "Unauthorized device");

    len_addr = sizeof(addr);
    sent = sendto(udp_socket, &reg_ack_pdu, sizeof(reg_ack_pdu), 0, (struct sockaddr *) &addr, len_addr);
  }
}

void reg_info_pkg(int client_num)
{
  int s, retval, sent;
  fd_set rfds;
  struct timeval tv;
  int received;
  struct udp_pdu data;
  struct sockaddr_in addr;
  socklen_t len_addr;
  char msg_buffer[255];
  struct udp_pdu reg_info_pdu;
  char tmp_port[255];
  char *elements;

  /* Timers and thresholds */
  s = 2;

  /* Watch stdin (fd 0) to see when it has input. */
  FD_ZERO(&rfds);
  FD_SET(0, &rfds);
  FD_SET(udp_socket, &rfds);

  /* Wait up to 's' seconds. */
  tv.tv_sec = s;
  tv.tv_usec = 0;
  retval = select(udp_socket+1, &rfds, NULL, NULL, &tv);

  if (retval == -1)
  {
      perror("select()");
      exit(-1);
  }
  else if (retval)
  {
      if (FD_ISSET(udp_socket, &rfds))
      {
        len_addr = sizeof(struct sockaddr_in);
        received = recvfrom(udp_socket, &data, sizeof(data), 0, (struct sockaddr *) &addr, &len_addr);
        if(received<0)
        {
          fprintf(stderr,"Error! UDP recvfrom\n");
          exit(-2);
        }
      }

      if(data.pkg == REG_INFO)
      {
        sprintf(msg_buffer, "Received: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(data), "REG_INFO", data.id, data.rand, data.data);
        debug(msg_buffer);

        if(check_authorized_device(data.id) & (strcmp(data.rand, clients[client_num].rand) == 0))
        {
          clients[client_num].state = REGISTERED;
          sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "REGISTERED");
          msg(msg_buffer);

          /* Guardamos los datos*/
          strtok(data.data, ",");
          elements = strtok(NULL, ",");
          strcpy(clients[client_num].elements, elements);

          reg_info_pdu.pkg = INFO_ACK;
          strcpy(reg_info_pdu.id, configuration.id);
          strcpy(reg_info_pdu.rand, clients[client_num].rand);
          sprintf(tmp_port, "%i", configuration.tcp_port);
          strcpy(reg_info_pdu.data, tmp_port);

          len_addr = sizeof(addr);
          sent = sendto(udp_socket, &reg_info_pdu, sizeof(reg_info_pdu), 0, (struct sockaddr *) &addr, len_addr);

          if(sent<0)
          {
            fprintf(stderr,"Error! UDP sendto\n");
            exit(-2);
          }

          sprintf(msg_buffer, "Sent: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(reg_info_pdu), "INFO_ACK", reg_info_pdu.id, reg_info_pdu.rand, reg_info_pdu.data);
          debug(msg_buffer);

          alive_pkg(data, addr);
        }
        else
        {
          clients[client_num].state = DISCONNECTED;
          sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "DISCONNECTED");
          msg(msg_buffer);

          reg_info_pdu.pkg = INFO_NACK;
          strcpy(reg_info_pdu.id, configuration.id);
          strcpy(reg_info_pdu.rand, clients[client_num].rand);
          strcpy(reg_info_pdu.data, "REG_INFO not correct");

          len_addr = sizeof(addr);
          sent = sendto(udp_socket, &reg_info_pdu, sizeof(reg_info_pdu), 0, (struct sockaddr *) &addr, len_addr);

          if(sent<0)
          {
            fprintf(stderr,"Error! UDP sendto\n");
            exit(-2);
          }

          sprintf(msg_buffer, "Sent: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(reg_info_pdu), "INFO_NACK", reg_info_pdu.id, reg_info_pdu.rand, reg_info_pdu.data);
          debug(msg_buffer);
          exit(1);
        }
      }
  }
  else
  {
    clients[client_num].state = DISCONNECTED;
    sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "DISCONNECTED");
    msg(msg_buffer);

    exit(1);
  }
}

void alive_pkg(struct udp_pdu data, struct sockaddr_in addr)
{
  int w, x, retval, sent, client_num;
  fd_set rfds;
  struct timeval tv;
  int received;
  socklen_t len_addr;
  char msg_buffer[255];
  struct udp_pdu alive_pdu;

  /* Timers and thresholds */
  w = 3;
  x = 3;

  client_num = get_client(data.id);
  while (clients[client_num].state == REGISTERED || clients[client_num].state == SEND_ALIVE) {

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(udp_socket, &rfds);

    /* Wait up to 's' seconds. */
    tv.tv_sec = w;
    tv.tv_usec = 0;
    retval = select(udp_socket+1, &rfds, NULL, NULL, &tv);

    if (retval == -1)
    {
        perror("select()");
        exit(-1);
    }
    else if (retval)
    {
      if (FD_ISSET(udp_socket, &rfds))
      {
        len_addr = sizeof(struct sockaddr_in);
        received = recvfrom(udp_socket, &data, sizeof(data), 0, (struct sockaddr *) &addr, &len_addr);
        if(received<0)
        {
          fprintf(stderr,"Error! UDP recvfrom\n");
          exit(-2);
        }
        client_num = get_client(data.id);
      }

      if(data.pkg == ALIVE)
      {
        sprintf(msg_buffer, "Received: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(data), "ALIVE", data.id, data.rand, data.data);
        debug(msg_buffer);

        if(check_authorized_device(data.id) & (strcmp(data.rand, clients[client_num].rand) == 0))
        {

          /* PRIMER PAQUETE - state = REGISTERED */
          if(clients[client_num].state == REGISTERED)
          {
            clients[client_num].state = SEND_ALIVE;
            sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "SEND_ALIVE");
            msg(msg_buffer);
          }

          /* MANTENER ALIVE - state = SEND_ALIVE */
          if(clients[client_num].state == SEND_ALIVE)
          {
            /*Enviar paquete alive*/
            alive_pdu.pkg = ALIVE;
            strcpy(alive_pdu.id, configuration.id);
            strcpy(alive_pdu.rand, clients[client_num].rand);
            strcpy(alive_pdu.data, clients[client_num].id);

            len_addr = sizeof(addr);
            sent = sendto(udp_socket, &alive_pdu, sizeof(alive_pdu), 0, (struct sockaddr *) &addr, len_addr);

            if(sent<0)
            {
              fprintf(stderr,"Error! UDP sendto\n");
              exit(-2);
            }

            sprintf(msg_buffer, "Sent: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(alive_pdu), "ALIVE", alive_pdu.id, alive_pdu.rand, alive_pdu.data);
            debug(msg_buffer);
          }
        }
      }
      else
      {
        clients[client_num].state = DISCONNECTED;
        sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "DISCONNECTED");
        msg(msg_buffer);

        /* Use another pdu? */
        alive_pdu.pkg = ALIVE_REJ;
        strcpy(alive_pdu.id, configuration.id);
        strcpy(alive_pdu.rand, clients[client_num].rand);
        strcpy(alive_pdu.data, "ALIVE not correct");

        len_addr = sizeof(addr);
        sent = sendto(udp_socket, &alive_pdu, sizeof(alive_pdu), 0, (struct sockaddr *) &addr, len_addr);

        if(sent<0)
        {
          fprintf(stderr,"Error! UDP sendto\n");
          exit(-2);
        }

        sprintf(msg_buffer, "Sent: bytes=%li, pkg=%s, id=%s, rand=%s, data=%s", sizeof(alive_pdu), "ALIVE_REJ", alive_pdu.id, alive_pdu.rand, alive_pdu.data);
        debug(msg_buffer);
      }
    }
    else
    {
      if(clients[client_num].state == SEND_ALIVE && clients[client_num].alives_not_received < x)
      {
        clients[client_num].alives_not_received = clients[client_num].alives_not_received + 1;
      }
      else
      {
        clients[client_num].state = DISCONNECTED;
        sprintf(msg_buffer, "Device %s goes to state %s", clients[client_num].id, "DISCONNECTED");
        msg(msg_buffer);
      }
    }
  }
}
