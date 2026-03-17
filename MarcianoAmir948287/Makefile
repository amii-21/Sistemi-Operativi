CC = gcc
CFLAGS = -Wvla -Wextra -Werror -D_GNU_SOURCE -pthread

all: main operator user

main: main.c config_parser.c common.h
	$(CC) $(CFLAGS) -o main main.c config_parser.c

operator: operator.c config_parser.c common.h
	$(CC) $(CFLAGS) -o operator operator.c config_parser.c

user: user.c config_parser.c common.h
	$(CC) $(CFLAGS) -o user user.c config_parser.c

clean:
	rm -f main operator user