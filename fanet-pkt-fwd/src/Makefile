# fanet_pkt_fwd

all: fanet_single_rx_tx

fanet_single_rx_tx: radio.o fanet_single_rx_tx.o
	$(CC) fanet_single_rx_tx.o radio.o  $(LIBS) -lrt -ldl -lpthread -lm -o fanet_single_rx_tx

fanet_single_rx_tx.o: fanet_single_rx_tx.c
	$(CC) $(CFLAGS) -c fanet_single_rx_tx.c

jitqueue.o: jitqueue.c
	$(CC) $(CFLAGS) -c jitqueue.c

base64.o: base64.c
	$(CC) $(CFLAGS) -c base64.c

parson.o: parson.c
	$(CC) $(CFLAGS) -c parson.c

radio.o: radio.c
	$(CC) $(CFLAGS) -c radio.c

clean:
	rm *.o fanet_single_rx_tx
