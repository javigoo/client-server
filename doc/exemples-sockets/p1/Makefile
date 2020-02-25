

#
#
LDFLAGS=	
SOLARISLD= -lnsl -lsocket

all: 
	@echo "Per compilar executar: "
	@echo "		make sistema	"
	@echo "On sistema es un de: linux,solaris,altres"


linux: info_host.c
	gcc -o info_host info_host.c -DLINUX $(LDFLAGS)
	
solaris: info_host.c
	gcc -o info_host info_host.c -DSOLARIS $(LDFLAGS) $(SOLARISLD)
	
altres: info_host.c
	gcc -o info_host info_host.c $(LDFLAGS)


clean:
	-rm -fr info_host

