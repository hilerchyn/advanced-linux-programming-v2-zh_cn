

# Default C compiler options
CFLAGS 		= -Wall -g

# C source files for the server
SOURCES 	= server.c module.c common.c main.c

# corresponding object files
OBJECTS 	= $(SOURCES:.c=.o)

# server module shared library files
MODULES		= time.so



### Rules. ###################################################

.PHONY:		all clean


all:		server $(MODULES) golangShared

clean:		
	rm -f $(OBJECTS) $(MODULES) server golang golang.h golang.so

server:		$(OBJECTS)
	$(CC) $(CFLAGS) -Wl,-export-dynamic -o $@ $^ -ldl


$(OBJECTS): server.h

$(MODULES):	\
	%.so: 		%.c server.h
		$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

golangShared: golang.go
	 go build -buildmode=c-shared -o golang.so $<