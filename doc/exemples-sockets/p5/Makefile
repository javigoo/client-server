

#
#
# Per solaris descomentar -lnsl -lsocket 
LDFLAGS=	# -lnsl -lsocket

all: file_client file_server


file_client: file_client.c
	gcc -o file_client file_client.c $(LDFLAGS)

file_server: file_server.c
	gcc -o file_server file_server.c $(LDFLAGS)

clean:
	-rm -fr file_client file_server

