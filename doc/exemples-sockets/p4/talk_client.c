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






#define BUFFSIZE                2048
char                    buffer[BUFFSIZE+2];


/*
	Funcio principal
*/

int main(int argc,char *argv[])
{
        struct hostent *ent;
	int 			sock,port,laddr_cli,a,nbytes,i=0,end;
	struct sockaddr_in	addr_server,addr_cli;
	time_t			temps;
        fd_set 			rfds;
        int 			retl;

	/* Numero de parametres */
	if(argc!=3)
	{
		printf("Usar: \n");
		printf("\t%s <destinacio> <port> \n",argv[0]);
		exit(0);
	}

	/* Agafem el port */
	port=atoi(argv[2]);
	
        ent=gethostbyname(argv[1]);
        if(!ent)
        {
                printf("Error! No trobat: %s \n",argv[1]);
                exit(-1);
        }

	/* Obrim el socket INET+STREAM -> TCP */
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
	
	/* Connectem amb el server */
	a=connect(sock,(struct sockaddr*)&addr_server,sizeof(addr_server));
        if(a<0)
          {
              fprintf(stderr,"Error al connect\n");
              perror(argv[0]);
              exit(-2);
          }

	end=0;
	/* Anem llegint de teclat i del socket  i transmeten/impr dades */
	while(!feof(stdin) && !end)
	{
           /* Controlar la disponibilitat de dades o a stdin o al socket */
           FD_ZERO(&rfds);
           FD_SET(0, &rfds);
           FD_SET(sock, &rfds);
           retl = select(sock+1, &rfds, NULL, NULL, NULL);
           if (retl)
			{
                if(FD_ISSET(0, &rfds))
				{ 
				/* Hi ha dades a stdin (Teclat) */
					fgets((char*)buffer,BUFFSIZE,stdin);
					write(sock,buffer,strlen(buffer));
				}
		
                if(FD_ISSET(sock, &rfds))
				{ 
				/* Hi ha dades al socket */
					nbytes=read(sock,buffer,BUFFSIZE);
					buffer[nbytes]=0;
			if(nbytes>0)
				printf("%s",buffer);
			else
			/* Socket tancat */
			   	end=1;
		}
	   }
	}
	
	close(sock);
}
