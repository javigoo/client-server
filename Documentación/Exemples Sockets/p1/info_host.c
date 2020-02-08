#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/in.h>


#ifdef SOLARIS
#include <sys/socket.h>			/* Les constants AF_proto estan a aquest include */
#endif


/**
	Imprimeix com s'usa el programa
*/
void imprimirUs(char *nom)
{
	printf("   Per usar:  \n");
	printf("           %s    <nom_host>\n",nom);
	
}

/**
	Funcio principal.
*/
int main(int argc,char *argv[])
{
	struct hostent *ent;
	struct in_addr *a;


	
	/* Comprobo l'us correcte del programa */
	if (argc!=2)
	{
		imprimirUs(argv[0]);
		exit(0);
	}

	/* Demano l'entrada del host per nom */
        ent=gethostbyname(argv[1]);	
	if(!ent)	/* No trobo el host */
	{
		printf("Error! No trobat: %s \n",argv[1]);
		exit(-1);
	}

	/* Host trobat */
	printf("Resultat: \n");
	
	/* Nom del host */
	printf("Nom Oficial:     %s\n",ent->h_name);

	/* Alias del host */
	printf("Alias: \n");
	while (*ent->h_aliases)
		printf(" - %s\n",*(ent->h_aliases)++);

	/* Tipus d'adreça IPv4, IPv6, IPX, etc. Aqui hi ha algunes coses no
	   molt portables. S'ha provat en Linux i Solaris i hi ha protocols
	   que no estan a Solaris definits */
	printf("Tipus d'adreça: ");
	switch(ent->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET -> IPv4\n");
			break;
	

		case AF_IPX:
			printf("Novell IPX\n");
			break;

		case AF_APPLETALK:
		        printf("AppleTalk DDP\n");
			break;

		case AF_DECnet:
			printf("DECnet\n");
			break;
#ifdef LINUX
		/* Aquests 2 protocols no estan als .h de Solaris */
		case AF_NETBEUI:
			printf("NetBIOS/NetBEUI Addressing Scheme\n");
			break;

		case AF_INET6:
			printf("AF_INET6 -> IPng/IPv6\n");
			break;
#endif
		default:
			printf("Desconeguda\n");

	}
	
	/* Numero de bytes de l'adreça: IPv4 -> 4, IPv6->16, etc. */
        printf("Longitud d'adreça: %d bytes\n",ent->h_length);


	/* Adreçes de la maquina */
	printf("Adreces: \n");
	while (*ent->h_addr_list)
	{
		printf(" - %s\n",inet_ntoa(*(struct in_addr *)*(ent->h_addr_list)++));
	}

}
