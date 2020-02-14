#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>


 

#define LONGDADES	100

/**
	Funció Principal.
	
*/
int main(int argc,char *argv[])
{
        struct hostent *ent;
	int 			sock,port,laddr_cli,a;
	struct sockaddr_in	addr_server,addr_cli;
	char			dadcli[LONGDADES];
	time_t			temps;


	/* 
		Comprova el nombre de parametres passats a la funció
	*/
	if(argc!=3)
	{
		printf("Usar: \n");
		printf("\t%s <destinacio> <port>\n",argv[0]);
		exit(0);
	}
	/* Recupera el port */
	port=atoi(argv[2]);
	
	/* Recupera el nom de host */
        ent=gethostbyname(argv[1]);
        if(!ent)
        {
                printf("Error! No trobat: %s \n",argv[1]);
                exit(-1);
        }

	/* Crea un socket INET+DGRAM -> UDP */
	sock=socket(AF_INET,SOCK_DGRAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		perror(argv[0]);
		exit(-1);
	}
	/* Ompla l'estructrura d'adreça amb les adreces on farem el binding (acceptem
	   per qualsevol adreça local */
	memset(&addr_cli,0,sizeof (struct sockaddr_in));
	addr_cli.sin_family=AF_INET;
	addr_cli.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_cli.sin_port=htons(0);
	
	/* Fem el binding */
	if(bind(sock,(struct sockaddr *)&addr_cli,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"No puc fer el binding del socket!!!\n");
                perror(argv[0]);
                exit(-2);
	}	

	/* Ompla l'estructrura d'adreça amb l'adreça del servidor on enviem les dades */
	memset(&addr_server,0,sizeof (struct sockaddr_in));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr=(((struct in_addr *)ent->h_addr)->s_addr);
	addr_server.sin_port=htons(port);
	
	/* Paquet per disparar la resposta amb el temps. */
	strcpy(dadcli,"DUMMY");
	a=sendto(sock,dadcli,strlen(dadcli)+1,0,(struct sockaddr*)&addr_server,sizeof(addr_server));
        if(a<0)
          {
              fprintf(stderr,"Error al sendto\n");
              perror(argv[0]);
              exit(-2);
          }


	/* Paquet de resposta amb el temps al servidor */
	a=recvfrom(sock,dadcli,LONGDADES,0,(struct sockaddr *)0,(int *)0);
	if(a<0)
	{
		fprintf(stderr,"Error al recvfrom\n");
		perror(argv[0]);
		exit(-2);
	}
	dadcli[a]='\0';	
	printf("%s\n",dadcli);
}
