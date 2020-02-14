

#
#
LDFLAGS=	# -lnsl -lsocket

all: time_client time_server


time_client: time_client.c
	gcc -o time_client time_client.c $(LDFLAGS)

time_server: time_server.c
	gcc -o time_server time_server.c $(LDFLAGS)

clean:
	-rm -fr time_client time_server

