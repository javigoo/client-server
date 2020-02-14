#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include<netinet/in.h>



#define BUFFSIZE		2048

char			buffer[BUFFSIZE+2];


/* Principal */
int main(int argc,char *argv[])
{

	int 			sock,port,laddr_cli,a;
	int 			ssock_cli,newsock,nbytes,end;
	struct sockaddr_in	addr_server,addr_cli;
        fd_set                  rfds;
        int                     retl;


	
	/* Nombre d'arguments */
	if(argc!=2)
	{
		printf("Usar: \n");
		printf("\t%s <port>\n",argv[0]);
		exit(0);
	}

	/* Port */
	port=atoi(argv[1]);
	
	if(port < 1024)
	{
		printf("Alerta: Nomes el root pot obrir per sota de 1024\n");
		printf("        S'intentara obrir de totes formes\n");
	}


	/* Creem el socket TCP */
	sock=socket(AF_INET,SOCK_STREAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		perror(argv[0]);
		exit(-1);
	}

	/* Adreça de binding (acceptem per qualsevol adr. )*/
	memset(&addr_server,0,sizeof (struct sockaddr_in));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_server.sin_port=htons(port);
	
	/* Fem el binding */
	if(bind(sock,(struct sockaddr *)&addr_server,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"No puc fer el binding del socket!!!\n");
                perror(argv[0]);
                exit(-2);
	}	

	/* Cua d'espera de l'accept-> 5*/
	listen(sock,5);

	while(1)
	{
		ssock_cli=sizeof(addr_cli);
		/* Rebem una nova connexió */
		newsock=accept(sock,(struct sockaddr *)&addr_cli,&ssock_cli);
		memset(buffer,BUFFSIZE-1,'\0');
		if(newsock>0)
		{
			nbytes=1; end=0;
					
			/* Anem llegint de teclat i socket */
			while(!feof(stdin) && !end)
			{
			   /* Controlar la disponibilitat de dades o a stdin o al socket */
			   FD_ZERO(&rfds);
			   FD_SET(0, &rfds);
			   FD_SET(newsock, &rfds);

			   retl = select(newsock+1, &rfds, NULL, NULL, NULL);
			   if (retl)
			   {
				if(FD_ISSET(0, &rfds))
				{
					fgets((char*)buffer,BUFFSIZE,stdin);
					write(newsock,buffer,strlen(buffer));
				}

				if(FD_ISSET(newsock, &rfds))
				{
					nbytes=read(newsock,buffer,BUFFSIZE);
					buffer[nbytes]=0;
					if(nbytes>0)
						printf("%s",buffer);
					else
						end=1;
				}
			   }
			}

		}
		close(newsock);
	}
}
