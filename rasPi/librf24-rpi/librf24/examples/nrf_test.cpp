
#include <cstdlib>
#include <iostream>

#include "../RF24.h"

#define MSLEEP __msleep(5)
#define DATA_PAYLOAD 2
#define nrf_PAYLOAD 2

void nrf_send(uint8_t& slave_id, uint8_t& num)
{
    uint8_t xorb;
    uint8_t raw[4];
    xorb=slave_id;
    for (int i=0; i<DATA_PAYLOAD; i++) xorb^=num;
    raw[0]=slave_id;
    raw[1]=num;
    raw[2]=num;
    raw[3]=xorb;
    radio.stopListening();
    prinf("Sending ");
    for (int i=0 ;i<nrf_PAYLOAD, i++) printf("%i ");
    bool stt = radio.write(raw, nrf_PAYLOAD);
    printf(stt ? "true" : "false");
    printf("\n\r");
}

void nrf_get(uint8_t& data)
{
    radio.write(&data, nrf_PAYLOD);
}

RF24 radio("/dev/spidev0.0",2000000 , 25);
const uint64_t pipes[2] = { 0xF0F0F0F0F0LL, 0xF1F1F1F1F1LL }; // TX: RX=0 TX=1 , RX: RX=1, TX=0

void setup(char choice)
{
    setvbuf (stdout, NULL, _IONBF, 0);
    printf("\n\rRpi Controller\n\r");
    radio.begin();
    radio.setAutoAck(false);
    radio.setPayloadSize(4);
    radio.setChannel(2);
    radio.setPALevel(RF24_PA_MAX);
    if (choice == 't')
    {
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(0,pipes[1]);
    } else {
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(0,pipes[0]);
    }
    radio.startListening();
}

int main(int argc, char** argv)
{
    bool tx_ok, tx_fail, rx_ready;
    if (argc != 2) 
    {
        printf( "usage: %s rx/tx\n\r", argv[0] );
        return -1;
    }
   choice = argv[1][0];

   setup(choice);
   char choice;
   uint8_t slave_id = 1;

   if (choice == "tx")
   {
        for (uint8_t i=0 ; i<100 ; i++)
        {
            nrf_send(slave_id, i);
            sleep(1)
        }
   } else {
        rx_ready = false;
        while (!rx_ready) { radio.whatHappened(tx_ok, tx_fail, rx_ready); }
        uint8_t data;
        nrf_get(data);
        printf("Got %i\n\r",data);
   }
}
