/*
 * udp_func.h
 *
 *  Created on: 14 Sept 2026
 *      Author: naor
 */

#ifndef INC_UDP_FUNC_H_
#define INC_UDP_FUNC_H_



#include <inttypes.h>
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include <string.h>

#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"


#define SERVER_PORT 12345
#define MAX_BUF_LEN 100
#define MAX_MESS_LEN 256
//for ADC:
#define POWER_SUPPLAY_VOLTAGE 3300 //mV
#define MAX_DIGIT_VAL 4095 //for 12 bit
#define V25 760//mV
#define AVG_SLOPE 2.5 //mV/C

struct __attribute__((packed)) incomming_struct{
	uint32_t test_id;
	uint8_t per_tested;
	uint8_t itr;
	uint8_t mess_len;
	uint8_t message[MAX_MESS_LEN];
};

struct __attribute__((packed)) result_struct{
	uint32_t test_id;
	uint8_t test_result;
};


struct loop_steps{
	void *rec_prif;
	void *send_prif;
	uint16_t addr_master; //only for I2C
	uint16_t addr_slave;  //only for I2C
	uint8_t *rec_data;
	uint8_t *send_data;
	uint8_t data_len;
};



extern struct incomming_struct m1;
extern struct result_struct a1;
extern struct loop_steps l_steps;
extern volatile uint8_t callback_flag;

extern struct udp_pcb* upcb;
extern ip_addr_t dest_ipaddr;
extern u16_t dest_port;
extern u16_t incomming_len;

extern int flag_timer_prif;
extern int flag_uart_prif;
extern int flag_spi_prif;
extern int flag_i2c_prif;
extern int flag_adc_prif;


void udp_receive_callback(void* arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port);
err_t send_packet(struct udp_pcb* pcb, const void* payload, size_t payload_size, const ip_addr_t* ipaddr, u16_t port);
void udp_server_init(void);
void peripheral_being_tested(uint8_t per_tested);
int test_function(void);
void reset_prif_flags(void);
#endif /* INC_UDP_FUNC_H_ */
