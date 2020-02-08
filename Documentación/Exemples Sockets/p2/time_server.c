#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
 
#include<netinet/in.h>


#define LONGDADES	100

/* Funcio principal */
int main(int argc,char *argv[])
{

	int 			sock,port,laddr_cli,a;
	struct sockaddr_in	addr_server,addr_cli;
	char			dadcli[LONGDADES];
	time_t			temps;



	/* Comprovacio del nombre de parametres */
	if(argc!=2)
	{
		printf("Usar: \n");
		printf("\t%s <port>\n",argv[0]);
		exit(0);
	}

	/* Port a escoltar */
	port=atoi(argv[1]);
	
	/* Avis per obertura de ports privilegiats */
	if(port < 1024)
	{
		printf("Alerta: Nomes el root pot obrir per sota de 1024\n");
		printf("        S'intentara obrir de totes formes\n");
	}


	/* Opertura del socket INET+DGRAM=> UDP */
	sock=socket(AF_INET,SOCK_DGRAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		perror(argv[0]);
		exit(-1);
	}

	/* Omplim l'estructura addr amb les adreces per les que acceptarem entrades */	
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

	/* Bucle de servei */
	while(1)
	{
		laddr_cli=sizeof(struct sockaddr_in);
		/* Rebem un paquet del client */
		a=recvfrom(sock,dadcli,LONGDADES,0,(struct sockaddr *)&addr_cli,&laddr_cli);
		if(a<0)
		{
			fprintf(stderr,"Error al recvfrom\n");
			perror(argv[0]);
			exit(-2);
		}
		
		/* Contestem amb el temps */
		temps=time(NULL);
		sprintf(dadcli,"%s\n",ctime(&temps));
		/* Enviem la resposta a la mateixa adreça d'on ens l'han demanat */
		a=sendto(sock,dadcli,strlen(dadcli)+1,0,(struct sockaddr*)&addr_cli,laddr_cli);
                if(a<0)
                {
                        fprintf(stderr,"Error al sendto\n");
                        perror(argv[0]);
                        exit(-2);
                }

	}
}
