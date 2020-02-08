#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>




/* Com actuara el client (L/E) */
#define	READ		0
#define	WRITE		1
#define _GNU_SOURCE

int proto=READ;


#define BUFFSIZE                2048
char                    buffer[BUFFSIZE+2];


int main(int argc,char *argv[])
{
        struct hostent *ent;
	int 			sock,port,laddr_cli,a,nbytes,i=0,end;
	struct sockaddr_in	addr_server,addr_cli;

	/* Nombre d'arguments */
	if(argc!=3 && argc!=4)
	{
		printf("Usar: \n");
		printf("\t%s <destinacio> <port> [w|r]\n",argv[0]);
		exit(0);
	}

	/* Si s'especifica L/E */
	if(argc==4 && (argv[3])[0]=='w') 	proto=WRITE;

	/* Port */
	port=atoi(argv[2]);
	
        ent=gethostbyname(argv[1]);
        if(!ent)
        {
                printf("Error! No trobat: %s \n",argv[1]);
                exit(-1);
        }

	/* Creem un socket TCP */
	sock=socket(AF_INET,SOCK_STREAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		perror(argv[0]);
		exit(-1);
	}

	/* Adreça del servidor */
	memset(&addr_server,0,sizeof (struct sockaddr_in));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr=(((struct in_addr *)ent->h_addr)->s_addr);
	addr_server.sin_port=htons(port);
	

	/* Connectem el socket */
	a=connect(sock,(struct sockaddr*)&addr_server,sizeof(addr_server));
        if(a<0)
          {
              fprintf(stderr,"Error al connect\n");
              perror(argv[0]);
              exit(-2);
          }

	end=0;
	switch(proto)
	{
		case WRITE:
			/* En cas d'escriptura li notifiquem al server que volem escriure */
			write(sock,"WRITE ",5);
			/* Esperem Confirmacio del server */
			nbytes=read(sock,buffer,BUFFSIZE);
			buffer[nbytes]=0;
			/* Escrivim */
			printf("Començo a escriure\n");
			write(sock,"01 Aixo va al fitxer de prova\n",30);
			write(sock,"02 Aixo va al fitxer de prova\n",30);
			sleep(1);
			write(sock,"03 Aixo va al fitxer de prova\n",30);
			write(sock,"04 Aixo va al fitxer de prova\n",30);
			sleep(1);
			write(sock,"05 Aixo va al fitxer de prova\n",30);
			write(sock,"06 Aixo va al fitxer de prova\n",30);
			sleep(1);
			write(sock,"07 Aixo va al fitxer de prova\n",30);
			write(sock,"08 Aixo va al fitxer de prova\n",30);
			sleep(1);
			write(sock,"09 Aixo va al fitxer de prova\n",30);
			write(sock,"10 Aixo va al fitxer de prova\n",30);
			sleep(1);
			write(sock,"11 Aixo va al fitxer de prova\n",30);
			write(sock,"12 Aixo va al fitxer de prova\n",30);
			sleep(1);
			break;
		
		case READ:
		default:
			/* Lectura */
			/* Anem llegint del socket e imprimint */
			while(!end)
			{
				write(sock,"READ ",4);
				nbytes=read(sock,buffer,BUFFSIZE);
				buffer[nbytes]=0;
				if(nbytes>0)
					printf("%s",buffer);
				else
					end=1;
			}
			break;
	}
	
	close(sock);
}
