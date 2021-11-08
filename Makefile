CLI = client.o
SRV = server.o
OBJS = 
CFLAGS = -g 

all: ircd irc bot sender

bot: bot.o $(OBJS) 
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

sender: sender.o $(OBJS) 
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

irc: $(CLI) $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

ircd: $(SRV) $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

clean:
	rm -rf *.o ircd irc *~ bot sender
