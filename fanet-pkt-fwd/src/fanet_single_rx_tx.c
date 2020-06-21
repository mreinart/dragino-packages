/**
 * Author: Dragino / MR
 * Date: 16/01/2018 / 2020-06-21
 * 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.dragino.com
 *
 * 
*/

#include <signal.h>		/* sigaction */
#include <errno.h>		/* error messages */

#include "radio.h"

#define TX_MODE 0
#define RX_MODE 1
#define RADIOHEAD 1
#define HEXBINARY 2

#define RADIO1    "/dev/spidev1.0"
#define RADIO2    "/dev/spidev2.0"

static char ver[8] = "0.1";

/* lora configuration variables */
static char sf[8] = "7";
static char bw[8] = "250000";
static char cr[8] = "5";
static char wd[8] = "241";
static char prlen[8] = "8";
static char power[8] = "14";
static char freq[16] = "868200000";            /* frequency of radio */
static char radio[16] = RADIO2  ;
static char filepath[32] = {'\0'};
static int mode = TX_MODE;
static int payload_format = 0; 
static int device = 50;
static bool getversion = false;

/* signal handling variables */
volatile bool exit_sig = false; /* 1 -> application terminates cleanly (shut down hardware, close open files, etc) */
volatile bool quit_sig = false; /* 1 -> application terminates without shutting down the hardware */

/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

static void sig_handler(int sigio) {
    if (sigio == SIGQUIT) {
	    quit_sig = true;;
    } else if ((sigio == SIGINT) || (sigio == SIGTERM)) {
	    exit_sig = true;
    }
}

void print_help(void) {
    printf("Usage: fanet_single_rx_tx  [-d radio_dev] select radio 1 or 2 (default:2) \n");
    printf("                           [-t] set as tx\n");
    printf("                           [-r] set as rx\n");
    printf("                           [-H] save as HEX format\n");
    printf("                           [-f frequency] (default:868200000)\n");
    printf("                           [-s spreadingFactor] (default: 7)\n");
    printf("                           [-b bandwidth] default: 250k \n");
    printf("                           [-w syncword] default: 241(0xF1)reserved for FANET\n");
    printf("                           [-c coderate] default: 5(4/5), range 5~8(4/8)\n");
    printf("                           [-p PreambleLength] default: 8, range 6~65535\n");
    printf("                           [-m message ]  message to send\n");
    printf("                           [-P power ] Transmit Power (min:5; max:20) \n");
    printf("                           [-o filepath ] payload output to file\n");
    printf("                           [-R] Transmit in Radiohead format\n");
    printf("                           [-v] show version \n");
    printf("                           [-h] show this help and exit \n");
}

int DEBUG_INFO = 1;

/* -------------------------------------------------------------------------- */
/* --- MAIN FUNCTION -------------------------------------------------------- */

int main(int argc, char *argv[])
{

    struct sigaction sigact; /* SIGQUIT&SIGINT&SIGTERM signal handling */
	
    int c, i;

    char input[128] = {'\0'};

    FILE *fp = NULL;

    // Make sure only one copy of the daemon is running.
    //if (already_running()) {
      //  MSG_DEBUG(DEBUG_ERROR, "%s: already running!\n", argv[0]);
      //  exit(1);
    //}

    while ((c = getopt(argc, argv, "trHd:m:p:c:f:s:b:w:P:o:Rvh")) != -1) {
        switch (c) {
            case 'v':
                getversion = true;
                break;
            case 'd':
	        if (optarg)
	            device = optarg[0];
	        else {
	            print_help();
		    exit(1);
	        }
                break;
            case 't':
                mode = TX_MODE;
                break;
            case 'r':
                mode = RX_MODE;
                break;
            case 'H':
                payload_format = HEXBINARY;
                break;
            case 'R':
                payload_format = RADIOHEAD;
                break;
            case 'f':
                if (optarg)
                    strncpy(freq, optarg, sizeof(freq));
                else {
                    print_help();
                    exit(1);
                }
                break;
            case 's':
                if (optarg)
                    strncpy(sf, optarg, sizeof(sf));
                else {
                    print_help();
                    exit(1);
                }
                break;
            case 'b':
                if (optarg)
                    strncpy(bw, optarg, sizeof(bw));
                else {
                    print_help();
                    exit(1);
                }
                break;
            case 'w':
                if (optarg)
                    strncpy(wd, optarg, sizeof(wd));
                else {
                    print_help();
                    exit(1);
                }
                break;
            case 'p':
                if (optarg)
                    strncpy(prlen, optarg, sizeof(prlen));
                else {
                    print_help();
                    exit(1);
                }
            case 'c':
                if (optarg)
                    strncpy(cr, optarg, sizeof(cr));
                else {
                    print_help();
                    exit(1);
                }
            case 'm':
                if (optarg)
                    strncpy(input, optarg, sizeof(input));
                else {
                    print_help();
                    exit(1);
                }
                break;
            case 'P':
                if (optarg)
                    strncpy(power, optarg, sizeof(power));
                else {
                    print_help();
                    exit(1);
                }
                break;
            case 'o':
                if (optarg)
                    strncpy(filepath, optarg, sizeof(filepath));
                else {
                    print_help();
                    exit(1);
                }
                break;
            case 'h':
            case '?':
            default:
                print_help();
                exit(0);
        }
    }

	
    if (getversion) {
		printf("fanet_single_rx_tx ver: %s\n",ver);
        exit(0);
    }	

	
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    /* radio device SPI_DEV init */
    radiodev *loradev;
    loradev = (radiodev *) malloc(sizeof(radiodev));

    if (device == 49){
        loradev->nss = 15;
        loradev->rst = 8;
        loradev->dio[0] = 7;
        loradev->dio[1] = 6;
        loradev->dio[2] = 0;
        strncpy(radio, RADIO1, sizeof(radio));
    }
    else if ( device == 50){
        loradev->nss = 24;
        loradev->rst = 23;
        loradev->dio[0] = 22;
        loradev->dio[1] = 20;
        loradev->dio[2] = 0;
        strncpy(radio, RADIO2, sizeof(radio));
    }

    loradev->spiport = lgw_spi_open(radio);

    if (loradev->spiport < 0) { 
        printf("opening %s error!\n",radio);
        goto clean;
    }

    loradev->freq = atol(freq);
    loradev->sf = atoi(sf);
    loradev->bw = atol(bw);
    loradev->cr = atoi(cr);
    loradev->power = atoi(power);
    loradev->syncword = atoi(wd);
    loradev->nocrc = 1;  /* crc check */
    loradev->prlen = atoi(prlen);
    loradev->invertio = 0;
    strcpy(loradev->desc, "RFDEV");	

    MSG("Radio struct: spi_dev=%s, spiport=%d, freq=%ld, sf=%d, bw=%ld, cr=%d, pr=%d, wd=0x%2x, power=%d\n", radio, loradev->spiport, loradev->freq, loradev->sf, loradev->bw, loradev->cr, loradev->prlen, loradev->syncword, loradev->power);

    if(!get_radio_version(loradev))  
        goto clean;

    /* configure signal handling */
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = sig_handler;
    sigaction(SIGQUIT, &sigact, NULL); /* Ctrl-\ */
    sigaction(SIGINT, &sigact, NULL); /* Ctrl-C */
    sigaction(SIGTERM, &sigact, NULL); /* default "kill" command */

    setup_channel(loradev);

    if ( mode == TX_MODE ){
		uint8_t payload[256] = {'\0'};
		uint8_t payload_len;

		if (strlen(input) < 1)
			strcpy(input, "Hello DRAGINO-FANET");

		if ( payload_format == RADIOHEAD ) {
			input[strlen((char *)input)] = 0x00;
			payload_len = strlen((char *)input) + 5;			
			payload[0] = 0xff;
			payload[1] = 0xff;
			payload[2] = 0x00;
			payload[3] = 0x00;
			for (int i = 0; i < payload_len - 4; i++){
				payload[i+4] = input[i];
			}
		}
		else {
			snprintf(payload, sizeof(payload), "%s", input);
			payload_len = strlen((char *)payload);
		}

		printf("Transmit Data(ASCII): %s\n", payload);
		printf("Transmit Data(HEX): ");
		for(int i = 0; i < payload_len; i++) {
            printf("%02x", payload[i]);
        }
		printf("\n");
		single_tx(loradev, payload, payload_len);
    } else if ( mode == RX_MODE){

        struct pkt_rx_s rxpkt;

        FILE * chan_fp = NULL;
        char tmp[256] = {'\0'};
        char chan_path[256] = {'\0'};
        char *chan_id = NULL;
        char *chan_data = NULL;
        char fanet_id[7] = {'\0'};    // VVuuuu
        unsigned long device_id = 0;
        static int id_found = 0;
        static unsigned long next = 1, count_ok = 0, count_err = 0;
        int i, data_size;

        rxlora(loradev, RXMODE_SCAN);

        if (strlen(filepath) > 0) 
            fp = fopen(filepath, "w+");

        MSG("\nListening at SF%i on %.6lf Mhz. port%i\n", loradev->sf, (double)(loradev->freq)/1000000, loradev->spiport);
        fprintf(stdout, "REC_OK: %d,    CRCERR: %d\n", count_ok, count_err);
        while (!exit_sig && !quit_sig) {
            if(digitalRead(loradev->dio[0]) == 1) {
                memset(rxpkt.payload, 0, sizeof(rxpkt.payload));
                if (received(loradev->spiport, &rxpkt)) {

                    data_size = rxpkt.size;
                    MSG("\n-- Received %d bytes\n", data_size);

                    memset(tmp, 0, sizeof(tmp));

                    for (i = 0; i < rxpkt.size; i++) {
                        tmp[i] = rxpkt.payload[i];
                    }

                    if (fp) {
                        if ( payload_format == HEXBINARY ) {
                            for (i = 0; i < rxpkt.size; i++) {
                                fprintf(fp, "%02x", rxpkt.payload[i]);
                            }
                            fprintf(fp, "\n");
                        } else {
                            //fprintf(fp, "%s\n", rxpkt.payload);
                            fwrite(rxpkt.payload, rxpkt.size, 1, fp);
                        }
                        fflush(fp);
                    }

                    // get FANET ID from packet payload byte0 = type - addrr: bytes 1-3
                    device_id = rxpkt.payload[1] << 16 + rxpkt.payload[3] << 8 + rxpkt.payload[2];
                    sprintf(fanet_id, "%02X%02X%02X\0", rxpkt.payload[1], rxpkt.payload[3], rxpkt.payload[2]);
                    next = time(0);
                    if ( payload_format == HEXBINARY )
                        sprintf(chan_path, "/var/fanet/packets/FANET-Packet-%ld-%s.hex", next, fanet_id);
                    else
                        sprintf(chan_path, "/var/fanet/packets/FANET-Packet-%ld-%s.bin", next, fanet_id);
                    fprintf(stdout, "-- Channel file path: %s\n", chan_path);

                    chan_fp  = fopen(chan_path, "w+");
                    if ( NULL !=  chan_fp ) {
                        //fprintf(chan_fp, "%s\n", chan_data);
                        //fwrite(chan_data, sizeof(char), data_size, chan_fp);
                        if ( payload_format == HEXBINARY ) {
                            for (i = 0; i < rxpkt.size; i++) {
                                fprintf(chan_fp, "%02x", rxpkt.payload[i]);
                            }
                            fprintf(chan_fp, "\n");
                        } else {
                            fwrite(rxpkt.payload, rxpkt.size, 1, chan_fp);
                        }
                        fflush(chan_fp);
                        fclose(chan_fp);
                    } else 
                        fprintf(stderr, "ERROR~ canot open file path: %s\n", chan_path); 

                    fprintf(stdout, "count_OK: %d, count_CRCERR: %d\n", ++count_ok, count_err);
                } else                                             
                    fprintf(stdout, "REC_OK: %d, CRCERR: %d\n", count_ok, ++count_err);
            }
        }

    }

clean:
    if(fp)
        fclose(fp);

    free(loradev);
	
    MSG("INFO: Exiting %s\n", argv[0]);
    exit(EXIT_SUCCESS);
}
