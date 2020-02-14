

#
#
LDFLAGS=	# -lnsl -lsocket

all: primo_client primo_server


primo_client: primo_client.c
	gcc -o primo_client primo_client.c $(LDFLAGS)

primo_server: primo_server.c
	gcc -o primo_server primo_server.c $(LDFLAGS)

clean:
	-rm -fr primo_client primo_server

