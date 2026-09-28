/*
 * udp_func.c
 *
 *  Created on: 14 Sept 2026
 *      Author: naor
 */


#include "udp_func.h"




volatile uint8_t callback_flag=0;
struct udp_pcb* upcb;
ip_addr_t dest_ipaddr;
u16_t dest_port=0;
u16_t incomming_len=0;
struct incomming_struct m1;
struct result_struct a1;

int flag_timer_prif=0;
int flag_uart_prif=0;
int flag_spi_prif=0;
int flag_i2c_prif=0;
int flag_adc_prif=0;

volatile int flag_r=0;
volatile int flag_t=0;
volatile int flag_tim1=0;
volatile int flag_tim4=0;
volatile uint32_t count_tim1=0,count_tim4=0;
volatile int spi_rx=0;
volatile int spi_tx=0;
volatile int i2c_rx=0;
volatile int i2c_tx=0;
volatile int flag_tmp=0;

enum which_prif{
	TIMER_p=1,
	UART_p=2,
	SPI_p=4,
	I2C_p=8,
	ADC_p=16
};


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	flag_r=1;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	flag_t=1;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	spi_rx=1;
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	spi_tx=1;
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){
	i2c_tx=1;
}
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c){
	i2c_rx=1;
}
//timers jump after 1 sec
//tim1 is Master
//tim4 is slave


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance==TIM1){
		count_tim1 = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
		flag_tim1=1;
	}
	else if(htim->Instance==TIM4){
		count_tim4 = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
		flag_tim4=1;
	}
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	flag_tmp=1;
}



void udp_receive_callback(void* arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port)
{
	//checks if we are free to receive new struct(callback_flag==0 is needed)
	if(callback_flag == 1)
	{
		pbuf_free(p);
		return;
	}
	//save sender information
	ip_addr_copy(dest_ipaddr, *addr);
	dest_port = port;
	memcpy(&m1 , p->payload,p->len);
	incomming_len = p->len;
	callback_flag = 1;
	//free pbuf
	pbuf_free(p);
}


err_t send_packet(struct udp_pcb* pcb, const void* payload, size_t payload_size, const ip_addr_t* ipaddr, u16_t port)
{
    err_t err;
    struct pbuf* p;

    // allocate a pbuf for the payload
    p = pbuf_alloc(PBUF_TRANSPORT, payload_size, PBUF_RAM);
    if (!p) {
        // failed to allocate pbuf
        return ERR_MEM;
    }

    // copy the payload into the pbuf
    memcpy(p->payload, payload, payload_size);

    // send the packet
    err = udp_sendto(pcb, p, ipaddr, port);

    // free the pbuf
    pbuf_free(p);

    return err;
}


void udp_server_init(void) {
	//create control block structure
	upcb = udp_new();
	callback_flag = 0;
	//checks bind status
	err_t err = udp_bind(upcb, IP_ADDR_ANY, SERVER_PORT);
	if (err == ERR_OK) {
		udp_recv(upcb, udp_receive_callback, NULL);
	}
	else {
		udp_remove(upcb);
   }
}

//simple suitable implementation for uint8_t of strlen
uint8_t my_strlen(uint8_t *str){
	uint8_t count=0;
	while(str[count]!='\0'){
		count++;
	}
	return count;
}

//simple suitable implementation for uint8_t of strcmp
int my_strcmp(uint8_t *str1,uint8_t *str2){
	int count=0;
	while(str1[count]!='\0' || str2[count]!='\0'){
		if(str1[count]!=str2[count]){
			return 1;
		}
		count++;
	}
	return 0;
}
void peripheral_being_tested(uint8_t per_tested){
	uint8_t mask=1;
	for(int i=0;i<5;i++){
		if((per_tested&(mask<<i))!=0 ){
			switch(i){
				case 0:
					flag_timer_prif=TIMER_p;
					break;
				case 1:
					flag_uart_prif=UART_p;
					break;
				case 2:
					flag_spi_prif=SPI_p;
					break;
				case 3:
					flag_i2c_prif=I2C_p;
					break;
				case 4:
					flag_adc_prif=ADC_p;
					break;
			}
		}
	}
}

void reset_prif_flags(void){
	flag_timer_prif=0;
	flag_uart_prif=0;
	flag_spi_prif=0;
	flag_i2c_prif=0;
	flag_adc_prif=0;
}


//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
//	flag_r=1;
//}
//
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
//	flag_t=1;
//}


//int uart_test_function(void){
//	int result=-1;
//	uint8_t buf4[256]={0};
//	uint8_t buf5[256]={0};
//	uint32_t crc_val=0,crc_cmp_val=0;
//	HAL_StatusTypeDef stat;
//	uint8_t len_buf5=0,len_buf4=0;
//	struct loop_steps l_steps[3]={
//			{&huart5,&huart4,buf5,m1.message,m1.mess_len},
//			{&huart4,&huart5,buf4,buf5,len_buf5},
//			{&huart5,&huart4,buf5,buf4,len_buf4}
//	};
//	if(m1.mess_len>100){
//		crc_val=HAL_CRC_Calculate(&hcrc, (uint32_t *)m1.message, m1.mess_len);
//	}
//	for(uint8_t i=0;i<m1.itr;i++){
//		for(uint8_t j=0;j<3;j++){
//			if(j==1){
//				//update len_buf5 (another indication buf5 was really transfered)
//				len_buf5=my_strlen(buf5);
//				l_steps[j].data_len=len_buf5;
//			}
//			else if(j==2){
//				//update len_buf4 (another indication buf4 was really transfered)
//				len_buf4=my_strlen(buf4);
//				l_steps[j].data_len=len_buf4;
//				//reset buf5 for making sure, the data is really came from buf4
//				memset(buf5,0,len_buf5);
//			}
//			stat=HAL_UART_Receive_DMA(l_steps[j].rec_prif, l_steps[j].rec_data,l_steps[j].data_len);
//			if(stat!=HAL_OK){
//				return 0;
//			}
//			stat=HAL_UART_Transmit_DMA(l_steps[j].send_prif,l_steps[j].send_data,l_steps[j].data_len);
//			if(stat!=HAL_OK){
//				return 0;
//			}
//			//making sure data transmitted and received
//			while((flag_r==0) &&(flag_t==0)){
//
//			}
//			flag_r=0;
//			flag_t=0;
//
//		}
//
//		if(m1.mess_len>100){
//			crc_cmp_val=HAL_CRC_Calculate(&hcrc, (uint32_t *)buf5, m1.mess_len);
//			if(crc_val==crc_cmp_val){
//				result=1;
//			}
//			else{
//				result=0;
//				return result;
//			}
//		}
//		else{
//			if(strcmp(buf5,m1.message)==0){
//				result=1;
//			}
//			else{
//				result=0;
//				return result;
//			}
//
//		}
//		//reset buf5 and buf4 before every external iteration
//		memset(buf5,0,len_buf5);
//		memset(buf4,0,len_buf4);
//
//	}
//	return result;
//}


//timers jump after 1 sec
//tim1 is Master
//tim4 is slave
//volatile int flag_tim1=0;
//volatile int flag_tim4=0;
//volatile uint32_t count_tim1=0,count_tim4=0;
//void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
//	if(htim->Instance==TIM1){
//		count_tim1 = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
//		flag_tim1=1;
//	}
//	else if(htim->Instance==TIM4){
//		count_tim4 = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
//		flag_tim4=1;
//	}
//}


//checking if advance timer (timer1) count to the same ARR as general purpose does
//both timers use output compare mode
//one pulse mode is enabled,making sure every iteration each timer counts once.
int timer_test_function(void){
	HAL_StatusTypeDef stat;
	MX_TIM1_Init();
	MX_TIM4_Init();
	int result=-1;
	for(uint8_t i=0;i<m1.itr;i++){
		stat=HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
		if(stat!=HAL_OK){
			return 0;
		}
		stat=HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
		if(stat!=HAL_OK){
			return 0;
		}
		uint32_t start=HAL_GetTick();
		while( !( (flag_tim1==1)&&(flag_tim4==1) ) ){
			if((HAL_GetTick()-start)>500){
				break;
			}
		}
		flag_tim1=0;flag_tim4=0;
		//reset one pulse mode for next iterations
		htim4.State=HAL_TIM_STATE_READY;
		htim1.State=HAL_TIM_STATE_READY;
		htim4.ChannelState[0]=HAL_TIM_CHANNEL_STATE_READY;
		htim1.ChannelState[0]=HAL_TIM_CHANNEL_STATE_READY;

		if( ((count_tim1) == (count_tim4)) &&(count_tim1!=0) ){
			result=1;
		}

	}
	return result;
}


//volatile int spi_rx=0;
//volatile int spi_tx=0;
//void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
//	spi_rx=1;
//}
//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
//	spi_tx=1;
//}

//int spi_test_function(void){
//	int result=-1;
//	MX_SPI1_Init();
//	MX_SPI4_Init();
//	uint8_t buf_m1[256]={0};
//	uint8_t buf_s4[256]={0};
//	uint32_t crc_val=0,crc_cmp_val=0;
//	HAL_StatusTypeDef stat;
//	uint8_t len_buf_m1=0,len_buf_s4=0;
//	struct loop_steps l_steps[3]={
//			{&hspi4,&hspi1,buf_s4,m1.message,m1.mess_len},
//			{&hspi4,&hspi1,buf_m1,buf_s4,len_buf_s4},
//			{&hspi4,&hspi1,buf_s4,buf_m1,len_buf_m1}
//	};
//	if(m1.mess_len>100){
//		crc_val=HAL_CRC_Calculate(&hcrc, (uint32_t *)m1.message, m1.mess_len);
//	}
//	for(uint8_t i=0;i<m1.itr;i++){
//		for(uint8_t j=0;j<3;j++){
//			if(j==1){
//				//update len_buf_s4(another indication buf_s4 was really transfered)
//				len_buf_s4=my_strlen(buf_s4);
//				l_steps[j].data_len=len_buf_s4;
//			}
//			else if(j==2){
//				//update len_buf4 (another indication buf4 was really transfered)
//				len_buf_m1=my_strlen(buf_m1);
//				l_steps[j].data_len=len_buf_m1;
//				//reset buf_s4 for making sure, the data is really came from buf_m1
//				memset(buf_s4,0,len_buf_s4);
//			}
//			stat=HAL_SPI_Receive_DMA(l_steps[j].rec_prif, l_steps[j].rec_data,l_steps[j].data_len);
//			if(stat!=HAL_OK){
//				return 0;
//			}
//			stat=HAL_SPI_Transmit_DMA(l_steps[j].send_prif,l_steps[j].send_data,l_steps[j].data_len);
//			if(stat!=HAL_OK){
//				return 0;
//			}
//			while((spi_rx==0)&&(spi_tx==0)){
//
//			}
//			spi_rx=0;
//			spi_tx=0;
//
//
//
//		}
//
//		if(m1.mess_len>100){
//			crc_cmp_val=HAL_CRC_Calculate(&hcrc, (uint32_t *)buf_s4, m1.mess_len);
//			if(crc_val==crc_cmp_val){
//				result=1;
//			}
//			else{
//				result=0;
//				return result;
//			}
//		}
//		else{
//			if(strcmp(buf_s4,m1.message)==0){
//				result=1;
//			}
//			else{
//				result=0;
//				return result;
//			}
//
//		}
//		//reset buf5 and buf4 before every external iteration
//		memset(buf_s4,0,len_buf_s4);
//		memset(buf_m1,0,len_buf_m1);
//
//	}
//	return result;
//}



//test function for UART, SPI and I2C
int prif_test_function(void* prif_rec,void* prif_send,uint16_t addr_master,uint16_t addr_slave,int prif,volatile int *flag_r,volatile int *flag_t){
	int result=0;
	uint8_t buf1[256]={0};
	uint8_t buf2[256]={0};
	uint32_t crc_val=0,crc_cmp_val=0;
	HAL_StatusTypeDef stat;
	uint8_t len_buf1=0,len_buf2=0;
//	MX_UART5_Init();
//	MX_UART7_Init();
//	MX_DMA_Init();
//	MX_SPI1_Init();
//	MX_SPI4_Init();
//	MX_I2C1_Init();
//	MX_I2C4_Init();
	struct loop_steps l_steps[3];
	//default setting suitable to uart case
	l_steps[0]=(struct loop_steps){prif_rec,prif_send,addr_master,addr_slave,buf1,m1.message,m1.mess_len};
	l_steps[1]=(struct loop_steps){prif_send,prif_rec,addr_master,addr_slave,buf2,buf1,len_buf1};
	l_steps[2]=(struct loop_steps){prif_rec,prif_send,addr_master,addr_slave,buf1,buf2,len_buf2};

	switch (prif){

	case SPI_p:{

//		l_steps[0]=(struct loop_steps){prif_rec,prif_send,addr_master,addr_slave,buf1,m1.message,m1.mess_len};
		l_steps[1]=(struct loop_steps){prif_rec,prif_send,addr_master,addr_slave,buf2,buf1,len_buf1};
//		l_steps[2]=(struct loop_steps){prif_rec,prif_send,addr_master,addr_slave,buf1,buf2,len_buf2};
//		MX_SPI1_Init();
//		MX_SPI4_Init();
//		MX_DMA_Init();

		break;
	}
	case I2C_p:{
//		l_steps[0]=(struct loop_steps){prif_rec,prif_send,addr_master,addr_slave,buf1,m1.message,m1.mess_len};
		l_steps[1]=(struct loop_steps){prif_send,prif_rec,addr_slave,addr_master,buf2,buf1,len_buf1};
//		l_steps[2]=(struct loop_steps){prif_rec,prif_send,addr_master,addr_slave,buf1,buf2,len_buf2};
//		MX_I2C1_Init();
//		MX_I2C4_Init();
		break;
	}
	}

	if(m1.mess_len>100){
		MX_CRC_Init();
		crc_val=HAL_CRC_Calculate(&hcrc, (uint32_t *)m1.message, m1.mess_len);
	}
	for(uint8_t i=0;i<m1.itr;i++){
		for(uint8_t j=0;j<3;j++){
			if(j==1){
				//update len_buf1(another indication buf1 was really transfered)
				len_buf1=my_strlen(buf1);
				l_steps[j].data_len=len_buf1;
			}
			else if(j==2){
				//update len_buf2 (another indication buf2 was really transfered)
				len_buf2=my_strlen(buf2);
				l_steps[j].data_len=len_buf2;
				//reset buf1 for making sure, the data is really came from buf2
				memset(buf1,0,len_buf1);
			}
			if(prif==UART_p){
				stat=HAL_UART_Receive_DMA(l_steps[j].rec_prif, l_steps[j].rec_data,l_steps[j].data_len);
				if(stat!=HAL_OK){
					return 0;
				}
				stat=HAL_UART_Transmit_DMA(l_steps[j].send_prif,l_steps[j].send_data,l_steps[j].data_len);
				if(stat!=HAL_OK){
					return 0;
				}
			}
			else if(prif==SPI_p){
				stat=HAL_SPI_Receive_DMA(l_steps[j].rec_prif, l_steps[j].rec_data,l_steps[j].data_len);
				if(stat!=HAL_OK){
					return 0;
				}
				stat=HAL_SPI_Transmit_DMA(l_steps[j].send_prif,l_steps[j].send_data,l_steps[j].data_len);
				if(stat!=HAL_OK){
					return 0;
				}

			}
			else if(prif==I2C_p){
				stat=HAL_I2C_Slave_Receive_IT(l_steps[j].rec_prif, l_steps[j].rec_data, l_steps[j].data_len);
				if(stat!=HAL_OK){
					return 0;
				}
				stat=HAL_I2C_Master_Transmit_IT(l_steps[j].send_prif,(l_steps[j].addr_slave)<<1, l_steps[j].send_data,l_steps[j].data_len);
				if(stat!=HAL_OK){
					return 0;
				}
			}
			//giving up to half second waiting to each transmit and receive
			uint32_t start=HAL_GetTick();
			while((*(flag_r)==0)&&(*(flag_t)==0)){
				if((HAL_GetTick()-start)>500){
					break;
				}
			}
			//unsuccessful transmitting or receiving consider as a failure
			if( (*(flag_r)==0)||(*(flag_t)==0) ){
				return 0;
			}
			*flag_r=0;
			*flag_t=0;

		}

		if(m1.mess_len>100){
			crc_cmp_val=HAL_CRC_Calculate(&hcrc, (uint32_t *)buf1, m1.mess_len);
			if(crc_val==crc_cmp_val){
				result=1;
			}
		}
		else{
			if(my_strcmp(buf1,m1.message)==0){
				result=1;
			}
		}
		if(result==0){
			return result;
		}
		//reset buf1 and buf2 before every external iteration
		memset(buf1,0,len_buf1);
		memset(buf2,0,len_buf2);

	}
	return result;
}

//Converte analog input from built in sensor on board
//return temperature value
double measure_temp(void){
	HAL_StatusTypeDef stat;
	volatile uint16_t adc_raw_temp = 0;
	double vsense=0.0;
	double temp=0.0;
	stat=HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_raw_temp, 1);
	if(stat!=HAL_OK){
		return 0.0;
	}
	while(flag_tmp==0){

	}
	flag_tmp=0;
	vsense=(hadc1.Instance->DR)*POWER_SUPPLAY_VOLTAGE/MAX_DIGIT_VAL;
	temp=((vsense-V25)/(AVG_SLOPE))+25;
	stat=HAL_ADC_Stop_DMA(&hadc1);
	if(stat!=HAL_OK){
		return 0.0;
	}
	return temp;
}

//Checks the temperature of STM32 using build-in Sensor
//Temp will be ok if it is between 25 to 50 celsius degrees (C)
int adc_test_function(void){
//	MX_ADC1_Init();
	int result=0;
	double temp;
	for(uint8_t i=0;i<m1.itr;i++){
		temp=measure_temp();
		if(temp>=25 && temp<=50){
			result=1;
		}
		else{
			result=0;
			break;
		}
	}
	return result;
}



int test_function(void){
	int result=0;
	if(m1.per_tested==0){
		return 0;
	}
	if(flag_timer_prif==TIMER_p){
		result=timer_test_function();
		if(result==0){
			return result;
		}
	}
	if(flag_uart_prif==UART_p){
		result=prif_test_function(&huart5, &huart7,0,0, UART_p, &flag_r, &flag_t);
		if(result==0){
			return result;
		}
	}
	if(flag_spi_prif==SPI_p){
		result=prif_test_function(&hspi4, &hspi1,0,0, SPI_p, &spi_rx, &spi_tx);
		if(result==0){
			return result;
		}
	}
	if(flag_i2c_prif==I2C_p){
		result=prif_test_function(&hi2c4, &hi2c1,0x10,0x20, I2C_p, &i2c_rx, &i2c_tx);
		if(result==0){
			return result;
		}
	}
	if(flag_adc_prif==ADC_p){
		result=adc_test_function();
		if(result==0){
			return result;
		}
	}
	reset_prif_flags();
	return result;


}
