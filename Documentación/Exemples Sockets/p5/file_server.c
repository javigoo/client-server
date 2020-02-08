#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <sys/wait.h>

#include<netinet/in.h>



#define FILENAME	"test.1"
#define BUFFSIZE	100


int		nfills=0;



/* Processador de la senyal de fill mort 
   Descompta 1 al nombre de fills
*/
void contarFills(int test)
{
	nfills--;
}

/* Envia el fitxer a un socket, esperant 2 segons cada 100 bytes */
void servirFitxer(int socket)
{

	FILE   *fitxer;
	int	nbytes;
	char	buffer[BUFFSIZE];
	
	
	fitxer=fopen(FILENAME,"r");
	while(!feof(fitxer))
	{
		nbytes=fread(buffer,1,BUFFSIZE,fitxer);
		if(nbytes>0) write(socket,buffer,nbytes);
		sleep(2);
	}
	fclose(fitxer);
}


/* Escriu un fitxer a disc des d'un socket */
void escriureFitxer(int socket)
{
	FILE   *fitxer;
	int	nbytes=1;
	char	buffer[BUFFSIZE];
	
	
	fitxer=fopen(FILENAME,"w");
	while(nbytes>0)
	{
		nbytes=read(socket,buffer,BUFFSIZE);
		if(nbytes>0) fwrite(buffer,1,nbytes,fitxer);
	}
	fclose(fitxer);
}


int main(int argc,char *argv[])
{

	int 			sock,port,laddr_cli,a;
	int 			ssock_cli,newsock,nbytes,end;
	struct sockaddr_in	addr_server,addr_cli;
        fd_set                  rfds;
        int                     retl,status;
	char 			buffer[BUFFSIZE];




	/* Nombre d'arguments */
	if(argc!=2)
	{
		printf("Usar: \n");
		printf("\t%s <port>\n",argv[0]);
		exit(0);
	}


	/* Nombre de port */
	port=atoi(argv[1]);
	
	/* Avis de port privilegiat */
	if(port < 1024)
	{
		printf("Alerta: Nomes el root pot obrir per sota de 1024\n");
		printf("        S'intentara obrir de totes formes\n");
	}


	/* Opertura de socket TCP */
	sock=socket(AF_INET,SOCK_STREAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		perror(argv[0]);
		exit(-1);
	}

	/* Adreça de binding (escolta) -> totes les disponibles */
	memset(&addr_server,0,sizeof (struct sockaddr_in));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_server.sin_port=htons(port);
	
	if(bind(sock,(struct sockaddr *)&addr_server,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"No puc fer el binding del socket!!!\n");
                perror(argv[0]);
                exit(-2);
	}	


	/* Quan un fill mor executar la funcio contarFills */
	signal(SIGCHLD,contarFills);

	/* Cua d'espera del socket */
	listen(sock,5);

	while(1)
	{
		ssock_cli=sizeof(addr_cli);
		
		/* Accepto la nova connexió */
		newsock=accept(sock,(struct sockaddr*)&addr_cli,&ssock_cli);
		memset(buffer,BUFFSIZE-1,'\0');
		if(newsock>0)
		{
			nbytes=read(newsock,buffer,BUFFSIZE);
			/* Llegeixo el primer paquet */
			if(nbytes>0)
			{
				/* Lectura */
				if(strncmp(buffer,"READ",4)==0)
				{
					if(fork()==0)
					{
						/* Proces fill -> serveixo el fitxer */
						servirFitxer(newsock);	
						exit(1);
					}	
					else
					{
						/* Proces pare, anoto el fill */
						nfills++;
						printf("Serveixo un lector (n'hi ha :%d)\n",nfills);
					}
				}
				else
				{
				/* Escriptura */
					printf("Serveixo un escriptor (hi ha :%d lectors). Espero.\n",nfills);
					if(nfills>0) 	/* Encara hi ha fills, esperare */
					{
						/* Espero la mort dels fills */
						while(nfills>0)
						{
							wait(&status);
							printf("Finalitza lector (:%d lectors).\n",nfills);
						}
					}
					/* Fills morts, puc servir l'escriptor */
					write(newsock,"OK\n",3);
					printf("Serveixo l'escriptor. Escric.\n");
					escriureFitxer(newsock);
				}
			}
		}
		close(newsock);
	}
}
