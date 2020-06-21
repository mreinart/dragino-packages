# fanet_pkt_fwd

all: fanet_pkt_fwd

fanet_pkt_fwd: base64.o parson.o radio.o jitqueue.o fanet_pkt_fwd.o
	$(CC) fanet_pkt_fwd.o base64.o parson.o radio.o jitqueue.o $(LIBS) -luci -lrt -ldl -lpthread -o fanet_pkt_fwd

fanet_pkt_fwd.o: fanet_pkt_fwd.c
	$(CC) $(CFLAGS) -c fanet_pkt_fwd.c

jitqueue.o: jitqueue.c
	$(CC) $(CFLAGS) -c jitqueue.c

base64.o: base64.c
	$(CC) $(CFLAGS) -c base64.c

parson.o: parson.c
	$(CC) $(CFLAGS) -c parson.c

radio.o: radio.c
	$(CC) $(CFLAGS) -c radio.c

clean:
	rm *.o fanet_pkt_fwd
