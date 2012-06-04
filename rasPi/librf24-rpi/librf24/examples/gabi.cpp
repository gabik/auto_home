
#include <cstdlib>
#include <iostream>

#include "../RF24.h"
#define MAX_RETRIES 3
#define MSLEEP __msleep(5)
#define RESPONSE_TIMEOUT 500

/* Old 8 bit
#define SEQ_BIT 5
#define W 0b00010000 // W = ....1....
#define ON 0b0000001 // ON = ....0001
#define OFF 0b0000000 // OFF = ....0000

#define R 0b00000000 // R = ...0....
#define PWR_STT 0b00000000 // Read Power State = ....0000
#define P_TIME_HOUR 0b00001110 // Read Power On Timer - Hours
#define P_TIME_MIN 0b00000110 // Read Power On Timer - Minutes (/2)
#define R_STATUS 0b00001111 // Alive status = ....1111

#define CMD_MASK 0x0F
#define ID_MASK 0x0F
*/

#define SEQ_BIT 28
#define RW_BIT 23
#define ID_BIT 24
#define CMD_BIT 0

#define CMD_MASK 0xF
#define ID_MASK 0xF
#define RW_MASK 1
#define SEQ_MASK 0xF

#define MINS_BIT 0
#define HOUR_BIT 6
#define MINS_MASK 0x3F
#define HOUR_MASK 0x7

#define STAT_BIT 0
#define STAT_MASK 1

#define TEMP_MASK 0x7F // 7 bit
#define TEMP_BIT 0

//commands:
//	send temp	seq- id-- r/w                  ... ....			.=Temperture (0-127)
//  send stat   seq- Id-- r/w                         .			 .=On/Off (1/0)
//  send time   seq- Id-- r/w               | ||.. ....			 |=hours, .=minutes
//  to me       seq- Id-- r/w                      -cmd
//              3322 2222 2222 1111 1111 11 
//              1098 7654 3210 9876 5432 1098 7654 3210
// payload=4 0x 0000 0000 0000 0000 0000 0000 0000 0000

#define W 0x1 // W = 0001
#define ON 0x1 // ON = 0001
#define OFF 0x0 // OFF = 0000

#define R 0x0 // R = 0000
#define PWR_STT 0x0 // Read Power State = ....0000
#define P_TIME 0b0110 // Read Power On Timer 
#define R_STATUS 0xF // Alive status = ....1111
#define R_TEMP	0b1000	// Get temperture = ....1000

int write_command(int slave_id, uint8_t command);
int read_command(int slave_id, uint8_t query, uint32_t *data_buffer);
int get_seq(unsigned long pkg);
int get_pkg_data(unsigned long pkg);
int get_response(int need_data_readback_check, uint32_t *data_readback, int seq, unsigned int timeout);


// Hardware configuration
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

//RF24 radio(9,10);
RF24 radio("/dev/spidev0.0",2000000 , 25);  //spi device, speed and CSN,only CSN is NEEDED in RPI

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
unsigned int package;

void setup(void)
{
setvbuf (stdout, NULL, _IONBF, 0);
  printf("\n\rRpi Controller\n\r");

  // Setup and configure rf radio
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(4);

  radio.setChannel(107);
  radio.setPALevel(RF24_PA_MAX);

  // Open pipes to other nodes for communication

  // This simple sketch opens two pipes for these two nodes to communicate back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);

  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
//  radio.printDetails();
}

void print_seq_id(unsigned long pkg) 
{
    printf("Seq: %i, Data: %i.   ", get_seq(pkg), get_pkg_data(pkg));
}

int get_seq(unsigned long pkg)
{
    return ((pkg >> SEQ_BIT) & SEQ_MASK);
}

int get_pkg_data(unsigned long pkg)
{
    return ((pkg >> CMD_BIT) & CMD_MASK);
}

bool send_data_retries(unsigned long pkg)
{
	printf("(Sent %lu) ",pkg);
	return  radio.write( &pkg, sizeof(unsigned long) );
}

unsigned int check_alive(unsigned int slave_id)  // Return 1 = alive, 0=dead
{
//	int alive_back;
//	alive_back = start_communicate_with_slave(slave_id);
//	if (alive_back == 0) {
//	uint32_t tmp_payload;
//	tmp_payload = ((R & RW_MASK) << RW_BIT) | ((R_STATUS & CMD_MASK) << CMD_BIT);
	uint32_t data_buff=0;
	int alive_back = read_command(slave_id, R_STATUS, &data_buff);
	if (alive_back>=0) if ((data_buff & STAT_MASK)>>STAT_BIT) return 1;
	return 0;
}

/* Old
int start_communicate_with_slave(unsigned int slave_id) // return 0=OK, -1=data not sent, -2=non seq. -3=not data readback -4=Timeout
{
	//return 0=ok, -1=timeput
	uint32_t pkg = 0;
	radio.stopListening();
	pkg |= (1<<SEQ_BIT);
	pkg |= ((slave_id & ID_MASK) << ID_BIT);
	print_seq_id(pkg);
	bool ok = send_data_retries(pkg);
	radio.startListening();
	if (! ok)
		return -1;
printf(" Sent-seq1-OK "); //GABI
        int ok_ack = get_response(slave_id, 1, RESPONSE_TIMEOUT);
if (ok_ack>=0) printf("seq1-ACK ");
        return ok_ack;
}
*/

int write_command(int slave_id, uint8_t command)
{
	int seq=1;
	uint32_t pkg = 0;
	pkg |= ((seq & SEQ_MASK) << SEQ_BIT);
	pkg |= ((slave_id & ID_MASK) << ID_BIT);
	pkg |= ((command & CMD_MASK) << CMD_BIT);
	pkg |= ((W & RW_MASK) << RW_BIT);
//	int seq=2;
//	unsigned int pkg = 0;
	radio.stopListening();
//	pkg |= (seq<<SEQ_BIT);
//	pkg |= command;
	print_seq_id(pkg);
	bool ok = send_data_retries(pkg);
	radio.startListening();
	if (! ok)
		return -1;
	printf(" Sent-OK "); //GABII	
	int ok_ack = get_response(1, &pkg, seq, RESPONSE_TIMEOUT);
	if (ok_ack>=0) 
		printf("Sent-ACK ");
		//ok_ack &= CMD_MASK;
	return ok_ack;
}

int read_command(int slave_id, uint8_t query, uint32_t *data_buffer)
{
        int seq=1;
        uint32_t pkg = 0;
        pkg |= ((seq & SEQ_MASK) << SEQ_BIT);
        pkg |= ((slave_id & ID_MASK) << ID_BIT);
        pkg |= ((query & CMD_MASK) << CMD_BIT);
	pkg |= ((R & RW_MASK) << RW_BIT);
        radio.stopListening();
        print_seq_id(pkg);
        bool ok = send_data_retries(pkg);
        radio.startListening();
        if (! ok)
                return -1;
	printf(" Sent-OK "); //GABI
	//uint32_t data_back=0;
        int ok_ack = get_response(0, data_buffer, seq, RESPONSE_TIMEOUT);
        if (ok_ack>=0)
                printf("Sent-ACK ");
        return ok_ack;

//        int seq=2;
//        unsigned int pkg = 0;
//       radio.stopListening();
//        pkg |= (seq<<SEQ_BIT);
//        pkg |= query;
//        print_seq_id(pkg);
//        bool ok = send_data_retries(pkg);
//        radio.startListening();
//        if (! ok)
//                return -1;
//printf(" Sent-seq2-OK "); //GABI
//        int ok_ack = get_response(0, seq, RESPONSE_TIMEOUT);
//	if (ok>=0) { 
//		printf("seq2-ACK ");
//		ok_ack &= CMD_MASK;
//	}
//        return ok_ack;
}

int send_power_on(unsigned int slave_id)
{
	//int comm_stat = 0;
	//comm_stat = start_communicate_with_slave(slave_id);
	//if (comm_stat < 0)
	//{
	//	printf(" slave failed init com(power-on %i). ", comm_stat);
	//	return comm_stat;
	//}
	//printf(" slave init. ");
	int comm_stat = write_command(slave_id, ON);
	if (comm_stat < 0)
	{
		printf(" slave failed on power on. (status %i)", comm_stat);
		return comm_stat;
	}
        printf(" slave powered on. ");
	return 0;
}

int send_power_off(unsigned int slave_id)
{
        int comm_stat = write_command(slave_id, OFF);
        if (comm_stat < 0)
        {
                printf(" slave failed on power on. (status %i)", comm_stat);
                return comm_stat;
        }
        printf(" slave powered off. ");
        return 0;
/*
        int comm_stat = 0;
        comm_stat = start_communicate_with_slave(slave_id);
        if (comm_stat < 0)
        {
                printf(" slave failed init com(power-off). ");
                return comm_stat;
        }
        printf(" slave init. ");
        unsigned char power_command = ((W)|(OFF));
        comm_stat = write_command(power_command);
        if (comm_stat < 0)
        {
                printf(" slave failed on power off. ");
                return comm_stat;
        }
	printf(" slave powered off. ");
        return 0;
*/
}

/* Old
int check_power_minutes(unsigned int slave_id)
{
        int comm_stat = 0;
        comm_stat = start_communicate_with_slave(slave_id);
        if (comm_stat < 0)
        {
                printf(" slave failed init com(check_minutes). ");
                return comm_stat;
        }
        printf(" slave init. ");
        unsigned char read_query = ((R)|(P_TIME_MIN));
        comm_stat = read_command(read_query);
        if (comm_stat < 0)
        {
                printf(" slave failed on get status. ");
                return comm_stat;
        }
        printf(":%i ", (comm_stat*2-1));
	if (comm_stat==0) comm_stat++;
        return (comm_stat*2-1);
}

int check_power_hours(unsigned int slave_id)
{
        int comm_stat = 0;
        comm_stat = start_communicate_with_slave(slave_id);
        if (comm_stat < 0)
        {
                printf(" slave failed init com(check_hours). ");
                return comm_stat;
        }
        printf(" slave init. ");
        unsigned char read_query = ((R)|(P_TIME_HOUR));
        comm_stat = read_command(read_query);
        if (comm_stat < 0)
        {
                printf(" slave failed on get status. ");
                return comm_stat;
        }
        printf("Uptime-%i:", comm_stat);
        return comm_stat;
}
*/

int check_temperture(unsigned int slave_id)
{
	uint32_t data_buff = 0;
	int comm_stat = read_command(slave_id, R_TEMP, &data_buff);
	if (comm_stat < 0)
        {
                printf(" slave failed on get Temperture. ");
                return comm_stat;
        }
	int tempC = ((data_buff >> TEMP_BIT) & TEMP_MASK);
        printf("(got %zu) Temperture %i°C", comm_stat, tempC);
        return tempC;
}

int check_power_time(unsigned int slave_id)
{
	uint32_t data_buff = 0;
	int comm_stat = read_command(slave_id, P_TIME, &data_buff);
	if (comm_stat < 0)
        {
                printf(" slave failed on get time. ");
                return comm_stat;
        }
	uint32_t p_hours = ((data_buff >> HOUR_BIT) & HOUR_MASK);
	uint32_t p_mins = ((data_buff >> MINS_BIT) & MINS_MASK);
        printf("(got %zu) Uptime %zu:%zu", comm_stat, p_hours, p_mins);
	uint8_t total_mins = ((uint8_t) p_hours)*60 + ((uint8_t) p_mins);
        return total_mins;

/*
	int total_minutes;
	int comm_stat = 0;
	sleep(1);
	comm_stat = check_power_hours(slave_id);
	if (comm_stat < 0) 
	{
		printf(" Failed on UPTIME check ");
		return comm_stat;
	}
	total_minutes=(comm_stat*60);
	sleep(1);
	comm_stat = check_power_minutes(slave_id);
        if (comm_stat < 0)
	{
                printf(" Failed on UPTIME check ");
                return comm_stat;
        }
	total_minutes+=comm_stat;
	return total_minutes;
*/
}

int check_power_status(unsigned int slave_id)
{
	uint32_t data_buff = 0;
        int comm_stat = read_command(slave_id, PWR_STT, &data_buff);
        if (comm_stat < 0)
        {
                printf(" slave failed on get time. ");
                return comm_stat;
        }
        uint32_t power_stt = ((data_buff & STAT_MASK) << STAT_BIT);
        printf(" slave powered status:%i ", (int)power_stt);
        return (int)power_stt;
/*
        int comm_stat = 0;
        comm_stat = start_communicate_with_slave(slave_id);
        if (comm_stat < 0)
        {
                printf(" slave failed init com(power status). ");
                return comm_stat;
        }
        printf(" slave init. ");
        unsigned char read_query = ((R)|(PWR_STT));
        comm_stat = read_command(read_query);
        if (comm_stat < 0)
        {
                printf(" slave failed on get status. ");
                return comm_stat;
        }
        printf(" slave powered status:%i ", comm_stat);
	//if (comm_stat == 1) printf("Uptime: %i",check_power_time(slave_id)); 
        return 0;
*/
}


int get_response(int need_data_readback_check, uint32_t *data_readback, int seq, unsigned int timeout)
{
	//ret values: 0=OK, -2=non seq. -3=not data readback -4=Timeout
	uint32_t res_data_readback;
	int res_seq;
	unsigned long started_waiting_at = __millis();
	bool timeout_bool = false;
	while ( ! radio.available() && ! timeout_bool ) {
		MSLEEP; //add a small delay to let radio.available to check payload
		if (__millis() - started_waiting_at > timeout )
			timeout_bool = true;
	}

	if ( timeout_bool )
		return -4;
	else
	{
		unsigned long response=0;
		radio.read( &response, sizeof(unsigned long) );
		res_seq = get_seq(response);
		res_data_readback = response; //get_pkg_data(response);
printf(" response=%lu rb=%lu res_rb=%lu seq=%i, res_seq=%i ",response, *data_readback, res_data_readback, seq, res_seq);
		if (res_seq != seq) return -2;
		if ((need_data_readback_check > 0) && (*data_readback != res_data_readback)) return -3;
		if (need_data_readback_check == 0) *data_readback = res_data_readback;
	}
	return 0;
}

void loop(void)
{
	printf("\n\r");
}

int main(int argc, char** argv)
{
        setup();
	char choice;
	int slave_id;
	if ( argc != 3 ) /* argc should be 2 for correct execution */
	{
		/* We print argv[0] assuming it is the program name */
		printf( "usage: %s [Status/oN/ofF/aliVe/Minutes/Hours] Slave_ID\n\r", argv[0] );
		return -1;
	}


	choice = argv[1][0];
	slave_id = atoi(argv[2]);
	if (slave_id > ID_MASK || slave_id < 0 ) { 
		printf("Wrong Slave ID \n\r");
		return -2;
	}
	
	
        printf("\n\r");
        int max_retry = 5;
        int cur_retry = 0;
        int not_done = 1;

	switch(choice) {
                case 't':
                {
                        cur_retry = 0;
                        not_done = -1;
                        while ((not_done<0) && (cur_retry < max_retry)) {
                                not_done = check_power_time(slave_id);
                                cur_retry++;
                                printf("\n\r");
                                sleep(1);
                        }
			printf("%i",not_done);
                        break;
                }
		case 's':
		{
			cur_retry = 0;
			not_done = -1;
			while ((not_done < 0) && (cur_retry < max_retry)) {
				not_done = check_power_status(slave_id);
				cur_retry++;
				printf("\n\r");
				sleep(1);
			}
			printf("%i",not_done);
			break;
		}
		case 'n':
		{
			cur_retry = 0;
			not_done = 1;
			while ((not_done) && (cur_retry < max_retry)) {
				not_done = send_power_on(slave_id);
				cur_retry++;
				printf("\n\r");
				sleep(1);
			}
			printf("%i",not_done);
			break;
		}
		case 'f':
		{
			cur_retry = 0;
			not_done = 1;
			while ((not_done) && (cur_retry < max_retry)) {
				not_done = send_power_off(slave_id);
				cur_retry++;
				printf("\n\r");
				sleep(1);
			}
			printf("%i",not_done);
			break;
		}
		case 'v':
		{
			int alive = -1;
			printf("Alive Check \n\r");
			for (int i=0; i<=ID_MASK ; i++)
			{
				printf("Checking %i. ", i);
				alive=check_alive(i);
				if (alive) printf("%i alive. ", i);
				printf("\n\r");
			}
			break;
		}
		case 'C':
		{
			cur_retry = 0;
			not_done = -1;
			while ((not_done<0) && (cur_retry < max_retry)) {
				not_done = check_temperture(slave_id);
				cur_retry++;
				printf("\n\r");
				sleep(1);
			}
			printf("%i",not_done);
			break;
		}
		default:
		{
			printf("Wrong command");
			return -3;
			break;
		}
	}

        return 0;
}


// vim:cin:ai:sts=2 sw=2 ft=cpp
