

#
#
LDFLAGS=	# -lnsl -lsocket

all: talk_client talk_server


talk_client: talk_client.c
	gcc -o talk_client talk_client.c $(LDFLAGS)

talk_server: talk_server.c
	gcc -o talk_server talk_server.c $(LDFLAGS)

clean:
	-rm -fr talk_client talk_server

