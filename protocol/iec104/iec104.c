
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iec104.h"


//以下是接收的I帧每个信息点的地址偏移量。
#define 	OFFSET_LENGTH       		1 	/*I帧中表示帧长度字节偏移量*/ 
#define 	OFFSET_TYPE	    		6 	/*I帧中表示类型标识字节偏移量*/ 
#define 	OFFSET_REASON	 		8 	/*I帧中表示传输原因字节偏移量*/ 
#define 	OFFSET_DEV_ADDR     		10 	/*I帧中表示公共地址字节偏移量*/ 
#define 	OFFSET_VER_ADDR     		12 	/*I帧中表示信息体地址字节偏移量*/ 
#define 	OFFSET_YK_SCO 	 		15 	/*I帧中表示遥控命令字节偏移量*/ 
#define 	OFFSET_YT_QL 	 		17 	/*I帧中表示遥调命令字节偏移量*/ 
#define 	OFFSET_MS_LO	 		15 	/*I帧中表示校时毫秒低字节偏移量*/ 
#define 	OFFSET_MS_HI	 		16 	/*I帧中表示校时毫秒高字节偏移量*/ 
#define 	OFFSET_MINUTE	 		17 	/*I帧中表示校时分钟字节偏移量*/ 
#define 	OFFSET_HOUR	 		18 	/*I帧中表示校时小时字节偏移量*/ 
#define 	OFFSET_DAY	 		19 	/*I帧中表示校时日期字节偏移量*/ 
#define 	OFFSET_MONTH	 		20 	/*I帧中表示校时月份字节偏移量*/ 
#define 	OFFSET_YEAR	 		21 	/*I帧中表示校时年份字节偏移量*/ 

#define 	REASON_SPONT	 		3 	/*传送原因，突发。*/ 
#define 	REASON_QUERY_ASK	 	5 	/*传送原因，请求或者被请求。*/ 
#define 	REASON_ACTIVE		 	6 	/*传送原因，激活。*/ 
#define 	REASON_ACTIVE_CONFIRM	 	7 	/*传送原因，激活确认。*/ 
#define 	REASON_STOP_ACTIVE	 	8 	/*传送原因，停止激活。*/ 
#define 	REASON_ACTIVE_OVER	 	10 	/*传送原因，激活停止。*/ 
#define 	REASON_TOTAL_QUERY	 	20 	/*传送原因，响应总召唤。*/ 
#define 	REASON_UNKNOWN_TYPE	 	44 	/*传送原因，未知类型标识。*/ 
#define 	REASON_UNKNOWN_REASON	 	45 	/*传送原因，未知传送原因。*/ 
#define 	REASON_UNKNOWN_DEV_ADDR	 	46 	/*传送原因，未知设备地址。*/ 
#define 	REASON_UNKNOWN_VER_ADDR	 	47 	/*传送原因，未知信息地址。*/ 

#define 	TYPE_YX			 	1 	/*类型标识，单点遥信。*/ 
#define 	TYPE_YC			 	11 	/*类型标识，标度化测量值（不带时标）。*/ 
#define 	TYPE_YM			 	15 	/*类型标识，累计量（电能值,不带时标）。*/ 
#define 	TYPE_YK			 	45 	/*类型标识，单点命令（遥控）。*/ 
#define 	TYPE_YT			 	49 	/*类型标识，设定值（标度化遥调）。*/ 
#define 	TYPE_TOTAL_QUERY	 	100 	/*类型标识，总召唤。*/ 
#define 	TYPE_YM_QUERY		 	101 	/*类型标识，计量值总召唤（YM召唤）。*/ 
#define 	TYPE_TIMING		 	103 	/*类型标识，时钟同步命令。*/ 

#define 	YK_CHOICE_OPEN		 	0x81 	/*遥控命令，预置打开。*/ 
#define 	YK_CHOICE_CLOSE		 	0x80 	/*遥控命令，预置关闭。*/ 
#define 	YK_ACTIVE_OPEN		 	0x01 	/*遥控命令，执行打开。*/ 
#define 	YK_ACTIVE_CLOSE		 	0x00 	/*遥控命令，执行关闭。*/ 

#define 	YT_CHOICE		 	0x80 	/*遥调命令，预置。*/ 
#define 	YT_ACTIVE		 	0x00 	/*遥调命令，执行。*/ 

/* 
 * =====================================================================================
 * 以下是对104协议内一帧最多可以传送多少点信息的定义，
 * 此处定义全部以地址连续为准，YC，YM量每个信息体中含有一个字节品质描述词,目前默认为0 
 * =====================================================================================
 */
#define 	FRAME_MAX_YM_NB		 	48 	/*一帧最多能传送的电度量*/ 
#define 	FRAME_MAX_YX_NB		 	240 	/*一帧最多能传送的遥信量*/ 
#define 	FRAME_MAX_YC_NB		 	80 	/*一帧最多能传送的遥测量*/ 

#define 	START_ADDR_YX		 	0x0001 	/*遥信量的起始地址*/ 
#define 	START_ADDR_YC		 	0x4001 	/*遥测量的起始地址*/ 
#define 	START_ADDR_YM		 	0x6401 	/*电度量的起始地址*/ 
#define 	START_ADDR_YK		 	0x6001 	/*遥控量的起始地址*/ 
#define 	START_ADDR_YT		 	0x6201 	/*遥调量的起始地址*/ 

/* 
 * =====================================================================================
 *         			Local Constants
 * =====================================================================================
 */

/* 
 * =====================================================================================
 *         			Local Global Variables
 * =====================================================================================
 */


/* 
 * =====================================================================================
 *         			Local Function Prototypes
 * =====================================================================================
 */

static uint16_t IEC104_send(IEC104_Param_T *IEC104_param, uint8_t *data, uint16_t nb);





/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_tcp_listen
 *  Description:  
 * =====================================================================================
 */
int16_t IEC104_tcp_listen(struct sockaddr_in *addr, socklen_t *addrlen){

	int16_t ret;
	int16_t socket_fd;

        socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socket_fd < 0){
                perror("socket");
		return -1;
        }else{
		printf("Socket OK\n");
        }

	addr->sin_family = AF_INET;
	addr->sin_port = htons(2404);
        addr->sin_addr.s_addr = INADDR_ANY;
	memset(&(addr->sin_zero), '\0', 8);

        ret = bind(socket_fd, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
        if (ret < 0) {
                perror("bind");
		shutdown(socket_fd, SHUT_RDWR);
                close(socket_fd);
		return -1;
        } else {
                printf("Bind OK\n");
        }

        ret = listen(socket_fd, IEC104_MAX_CLIENT);
        if (ret != 0) {
                perror("listen");
                close(socket_fd);
		return -1;
        } else {
                printf("Listen OK\n");
        }

	return socket_fd;
}		/* -----  end of function IEC104_tcp_listen  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_set_system_time
 *  Description:  设置系统时间函数，校时函数用到。
 * =====================================================================================
 */
void IEC104_set_system_time(IEC104_Param_T *IEC104_param, uint8_t *recv_buff){

	uint8_t i;
	uint8_t time[6] = {0};
	int16_t fd;
	uint16_t ms_temp = 0;

	ms_temp = recv_buff[OFFSET_MS_HI]; 			/*读取毫秒*/ 
	ms_temp = (ms_temp << 8) + recv_buff[OFFSET_MS_LO];

	time[0] = recv_buff[OFFSET_YEAR]   & 0x7F;
	time[1] = recv_buff[OFFSET_MONTH]  & 0x0F;
	time[2] = recv_buff[OFFSET_DAY]    & 0x1F;
	time[3] = recv_buff[OFFSET_HOUR]   & 0x1F;
	time[4] = recv_buff[OFFSET_MINUTE] & 0x3F;
	time[5] = ms_temp/1000; 				/*将毫秒转换成秒*/ 

	for(i=0; i<6; i++){ 					/*将16进制数转换成BCD码*/ 
		time[i] = ((time[i]/10)<<4) + (time[i]%10);
	}

	fd = open("/dev/atop_rtc", O_RDWR);

	printf("Set Sysetm Time:20%02x/%02x/%02x %02x:%02x:%02x\n", time[0], time[1], time[2],
			time[3], time[4], time[5]);

	write(fd, time, 6);
	close(fd);
	return ;
}		/* -----  end of function IEC104_set_system_time  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_get_system_time
 *  Description:  获取系统时间函数。
 * =====================================================================================
 */
void IEC104_get_system_time(IEC104_Param_T *IEC104_param){

	int16_t fd;
	uint8_t rtc_tm[6] ={0};

	fd = open("/dev/atop_rtc", O_RDWR);
	read(fd, rtc_tm, 6);
	printf("Get System Time:20%02x/%02x/%02x %02x:%02x:%02x\n", rtc_tm[0], rtc_tm[1], rtc_tm[2],
			rtc_tm[3], rtc_tm[4], rtc_tm[5]);
	close(fd);
	return ;
}		/* -----  end of function IEC104_get_system_time  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_init_param
 *  Description:  初始化相关参数。
 * =====================================================================================
 */
void IEC104_init_param(IEC104_Param_T *IEC104_param){

	uint16_t i;

	IEC104_param->debug = OFF;
	IEC104_param->timer3_flag = TRUE;

	for ( i = 0; i < IEC104_YX_NB; i += 1 ) { /*测试使用，可注释掉*/ 
		yx_buff[i] = 0x01; 
	}

	for ( i = 0; i < IEC104_YC_NB; i += 1 ) {/*测试使用，可注释掉*/ 
		yc_buff[i] = i; 
	}

	for ( i = 0; i < IEC104_YM_NB * 2; i += 1 ) {/*测试使用，可注释掉*/ 
		ym_buff[i] = 0x01; 
	}

	yx_ptr = &yx_buff[0]; 		/*指向遥信数据表*/ 
	yc_ptr = &yc_buff[0]; 		/*指向遥测数据表*/ 
	ym_ptr = &ym_buff[0]; 		/*指向遥脉数据表*/ 
	yt_ptr = &yt_buff[0]; 		/*指向遥调数据表*/ 
	yk_ptr = &yk_buff[0]; 		/*指向遥控数据表*/ 

	signal(SIGPIPE, SIG_IGN); /*忽略SIGPIPE信号，否则当SOCKET关闭状态下发送数据会导致进程退出。*/ 
	return ;
}		/* -----  end of function IEC104_init_param  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_error_treat
 *  Description:  错误处理函数，当检测到异常情况下处理。
 * =====================================================================================
 */
void IEC104_error_treat(IEC104_Param_T *IEC104_param, uint8_t *string, uint8_t code){

	printf("\nERROR %s (%d)\n", string, code);

//	switch(code){
//		case :	
//			break;
//		case :	
//			break;
//		case :	
//			break;
//		default:	
//			break;
//	}				/* -----  end switch  ----- */

        shutdown(IEC104_param->fd, SHUT_RDWR);
        close(IEC104_param->fd);

	return ;
}		/* -----  end of function IEC104_error_treat  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_add_SN
 *  Description:  给要发送的I帧添加发送和接收序列号。
 * =====================================================================================
 */
void IEC104_add_SN(IEC104_Param_T *IEC104_param){

	uint16_t send_count = ((IEC104_param->send_SN) << 1) & 0XFFFE;
	uint16_t recv_count = ((IEC104_param->recv_SN) << 1) & 0XFFFE;

	//发送序列号
	IEC104_param->group_send_buff[IEC104_param->has_send_nb][2] = send_count & 0x00FF;
	IEC104_param->group_send_buff[IEC104_param->has_send_nb][3] = (send_count >> 8) & 0x00FF;
	//接收序列号
	IEC104_param->group_send_buff[IEC104_param->has_send_nb][4] = recv_count & 0x00FF;
	IEC104_param->group_send_buff[IEC104_param->has_send_nb][5] = (recv_count >> 8) & 0x00FF;

	return ;
}		/* -----  end of function IEC104_add_SN  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_send
 *  Description:  发送函数，发送数据。
 * =====================================================================================
 */
static uint16_t IEC104_send(IEC104_Param_T *IEC104_param, uint8_t *data, uint16_t nb){

        uint16_t i;
        int16_t ret;
        
        if (IEC104_param->debug) {
                for (i = 0; i < nb; i++)
                        printf("{%.2X}", data[i]);
		printf("\n\n");
        }
        
	ret = send(IEC104_param->fd, data, nb, 0);

        if ((ret == -1) || (ret != nb)) {
                ret = SOCKET_FAILURE;
		IEC104_error_treat(IEC104_param, "Send Socket Failure!", ret);
        }
        
        return ret;
}		/* -----  end of function IEC104_send  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_send_group
 *  Description:  
 * =====================================================================================
 */
void IEC104_send_group(IEC104_Param_T *IEC104_param){

	uint8_t ret;
	uint8_t *send_buff;
	uint16_t send_nb;

	send_buff = IEC104_param->group_send_buff[IEC104_param->has_send_nb];
	send_nb = IEC104_param->group_send_length[IEC104_param->has_send_nb];

	//已经发送的数据超过12帧未确认。断开连接。
	if((IEC104_param->k_value) > IEC104_MAX_K){
		IEC104_error_treat(IEC104_param, "More Than Max K Value Unconfirmed!", K_UNCONFIRM);
		return ;
	}

	//为即将发送的帧添加发送和接收序列号。
	IEC104_add_SN(IEC104_param);
	ret = IEC104_send(IEC104_param, send_buff, send_nb);

	IEC104_param->send_SN ++;
	if(IEC104_param->send_SN > 32767){
		IEC104_param->send_SN = 0;
	}

	IEC104_param->k_value ++;

	IEC104_param->has_send_nb ++;
	if(IEC104_param->has_send_nb >= IEC104_param->need_send_nb){
		IEC104_param->has_send_nb = 0;
		IEC104_param->need_send_nb = 0;
	}

	return ;
}		/* -----  end of function IEC104_send_group  ----- */

#define WAIT_DATA(){                                                                	\
    while ((select_ret = select(IEC104_param->fd+1, &rfds, NULL, NULL, &tv)) == -1) {  	\
	    IEC104_error_treat(IEC104_param, "Select Failure!", SELECT_FAILURE);	\
    }                                                                              	\
}
						     
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_recv
 *  Description:  接收函数，采用阻塞方式，有数据接收到就解析,同时做帧判断。
 * =====================================================================================
 */
uint16_t IEC104_recv(IEC104_Param_T *IEC104_param, uint8_t *data){

	uint16_t i;
	uint16_t recv_ptr = 0;
	int16_t read_ret = 0;
	int16_t select_ret = 0;
	uint16_t recv_length = 0;
	uint16_t frame_length= 0;
        fd_set rfds;
        struct timeval tv;
	
	tv.tv_sec = 0;
	tv.tv_usec = 50000; /*定义超时时间为50ms,超过50ms没有收到数据则返回。*/ 
             
	FD_ZERO(&rfds);
	FD_SET(IEC104_param->fd, &rfds);

	WAIT_DATA();
	if(select_ret == 0){ /*Select函数返回等待超时，直接返回。*/ 
		return COMM_TIME_OUT;
	}

	while(select_ret){
		read_ret = recv(IEC104_param->fd, &data[recv_ptr], IEC104_RECV_BUFF, 0);
		if(read_ret <= 0){
			return SOCKET_FAILURE;	
		}

		recv_length += read_ret;
		if(recv_ptr == 0){ 		/*第一次接收到数据，则根据协议第二个字节判断帧数据的长度。*/ 
			frame_length = data[OFFSET_LENGTH] + 2;
		}

		if(read_ret == frame_length){ 	/*接收数据长度等于帧长度，数据接收完成。*/ 
			if(IEC104_param->debug == TRUE){
				for (i = 0; i < recv_length; i++)
					printf("[%.2X]", data[i]);
				printf("\n\n");		
			}
			IEC104_param->recv_length = recv_length;
			return DATA_OK;
		}

		if(read_ret > frame_length){	/*接收数据长度大于帧长度，可能多帧连在一起。*/
			if(IEC104_param->debug == TRUE){
				for (i = 0; i < recv_length; i++)
					printf("[%.2X]", data[i]);
				printf("\n\n");		
			}
			IEC104_param->recv_length = recv_length;
			return DATA_ERROR;
		}

		if(read_ret < frame_length){ 	/*接收数据长度小于帧长度，继续接收数据。*/ 
			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			recv_ptr += read_ret; 
			WAIT_DATA();
			if(select_ret == 0){
				IEC104_param->recv_length = recv_length;
				return DATA_ERROR;
			}
		}
	}
}		/* -----  end of function IEC104_recv  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_yx
 *  Description:  
 * =====================================================================================
 */
void IEC104_build_yx(IEC104_Param_T *IEC104_param){

	uint8_t i;
	uint8_t j;
	uint8_t frame_nb;

	uint16_t index = 0;
	uint16_t data_nb = 0;
	uint16_t data_addr = 0;
	uint16_t data_offset = 0;
	uint8_t *data_ptr;
	uint16_t group_index = IEC104_param->need_send_nb;

	if(IEC104_YX_NB % FRAME_MAX_YX_NB == 0){
		frame_nb = IEC104_YX_NB/FRAME_MAX_YX_NB;
	}else{
		frame_nb = IEC104_YX_NB/FRAME_MAX_YX_NB + 1;
	}

	for(i=0; i<frame_nb; i+=1){

		index = 0;
		if(group_index >= IEC104_MAX_GROUP){ /*当前发送缓存已满，不再添加，否则数组溢出。*/ 
			return;
		}
		//起始字0x68
		IEC104_param->group_send_buff[group_index][index++] = 0x68;
		//APDU长度，最后用index填写。
		index++;
		//发送序列号，在发送前添加，否则容易出现序列号异常。
		index++;
		index++;
		//接收序列号，在发送前添加，否则容易出现序列号异常。
		index++;
		index++;
		//类型标识
		IEC104_param->group_send_buff[group_index][index++] = TYPE_YX;
		//可变结构限定词
		if(i == (frame_nb - 1)){
			data_nb = IEC104_YX_NB % FRAME_MAX_YX_NB;
		}else{
			data_nb = FRAME_MAX_YX_NB;
		}
		IEC104_param->group_send_buff[group_index][index++] = data_nb | 0x80;
		//传输原因
		IEC104_param->group_send_buff[group_index][index++] = REASON_TOTAL_QUERY;
		IEC104_param->group_send_buff[group_index][index++] = 0x00;
		//公共体地址
		IEC104_param->group_send_buff[group_index][index++] = config_buff[OFFSET_DEVICE_ADDR] & 0x00ff;
		IEC104_param->group_send_buff[group_index][index++] = (config_buff[OFFSET_DEVICE_ADDR] >> 8) & 0x00ff;
		//信息体地址。
		data_addr = START_ADDR_YX + i * FRAME_MAX_YX_NB;
		IEC104_param->group_send_buff[group_index][index++] = data_addr & 0x00FF;
		IEC104_param->group_send_buff[group_index][index++] = (data_addr >> 8) & 0x00FF;
		IEC104_param->group_send_buff[group_index][index++] = 0x00;

		//从数组中取出信息放入信息体中
		for(j=0; j<data_nb; j+=1){
			data_offset = i * FRAME_MAX_YX_NB + j;
			data_ptr = yx_ptr + data_offset;
			if(*data_ptr == 0x00){
				IEC104_param->group_send_buff[group_index][index++] = 0x00;
			}else{
				IEC104_param->group_send_buff[group_index][index++] = 0x01;
			}
		}
		IEC104_param->group_send_buff[group_index][1] = index - 2;
		IEC104_param->group_send_length[group_index] = index;
		group_index ++;
		IEC104_param->need_send_nb ++;
	}

	return ;
}		/* -----  end of function IEC104_build_yx  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_yc
 *  Description:  
 * =====================================================================================
 */
void IEC104_build_yc(IEC104_Param_T *IEC104_param){

	uint8_t i;
	uint8_t j;
	uint8_t frame_nb;

	uint16_t index = 0;
	uint16_t data_nb = 0;
	uint16_t data_addr = 0;
	uint16_t data_offset = 0;
	uint16_t *data_ptr;
	uint16_t group_index = IEC104_param->need_send_nb;

	if(IEC104_YC_NB % FRAME_MAX_YC_NB == 0){
		frame_nb = IEC104_YC_NB/FRAME_MAX_YC_NB;
	}else{
		frame_nb = IEC104_YC_NB/FRAME_MAX_YC_NB + 1;
	}

	for(i=0; i<frame_nb; i+=1){

		index = 0;
		if(group_index >= IEC104_MAX_GROUP){ /*当前发送缓存已满，不再添加，否则数组溢出。*/ 
			return;
		}
		//起始字0x68
		IEC104_param->group_send_buff[group_index][index++] = 0x68;
		//APDU长度，最后用index填写。
		index++;
		//发送序列号，在发送前添加，否则容易出现序列号异常。
		index++;
		index++;
		//接收序列号，在发送前添加，否则容易出现序列号异常。
		index++;
		index++;
		//类型标识
		IEC104_param->group_send_buff[group_index][index++] = TYPE_YC;
		//可变结构限定词
		if(i == (frame_nb - 1)){
			data_nb = IEC104_YC_NB % FRAME_MAX_YC_NB;
		}else{
			data_nb = FRAME_MAX_YC_NB;
		}
		IEC104_param->group_send_buff[group_index][index++] = data_nb | 0x80;
		//传输原因
		IEC104_param->group_send_buff[group_index][index++] = REASON_TOTAL_QUERY;
		IEC104_param->group_send_buff[group_index][index++] = 0x00;
		//公共体地址
		IEC104_param->group_send_buff[group_index][index++] = config_buff[OFFSET_DEVICE_ADDR] & 0x00ff;
		IEC104_param->group_send_buff[group_index][index++] = (config_buff[OFFSET_DEVICE_ADDR] >> 8) & 0x00ff;
		//信息体地址。
		data_addr = START_ADDR_YC + i * FRAME_MAX_YC_NB;
		IEC104_param->group_send_buff[group_index][index++] = data_addr & 0x00FF;
		IEC104_param->group_send_buff[group_index][index++] = (data_addr >> 8) & 0x00FF;
		IEC104_param->group_send_buff[group_index][index++] = 0x00;

		//从数组中取出信息放入信息体中
		for(j=0; j<data_nb; j+=1){
			data_offset = i * FRAME_MAX_YC_NB + j;
			data_ptr = yc_ptr + data_offset;
			IEC104_param->group_send_buff[group_index][index++] = (*data_ptr) & 0x00ff;
			IEC104_param->group_send_buff[group_index][index++] = (*data_ptr >> 8) & 0x00ff;
			//品质描述词。
			IEC104_param->group_send_buff[group_index][index++] = 0x00;
		}
		IEC104_param->group_send_buff[group_index][1] = index - 2;
		IEC104_param->group_send_length[group_index] = index;
		group_index ++;
		IEC104_param->need_send_nb ++;
	}

	return ;
}		/* -----  end of function IEC104_build_yc  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_ym
 *  Description:  
 * =====================================================================================
 */
void IEC104_build_ym(IEC104_Param_T *IEC104_param){

	uint8_t i;
	uint8_t j;
	uint8_t frame_nb;

	uint16_t index = 0;
	uint16_t data_nb = 0;
	uint16_t data_addr = 0;
	uint16_t data_offset = 0;
	uint16_t *data_ptr = 0;
	uint16_t group_index = IEC104_param->need_send_nb;

	if(IEC104_YM_NB % FRAME_MAX_YM_NB == 0){
		frame_nb = IEC104_YM_NB/FRAME_MAX_YM_NB;
	}else{
		frame_nb = IEC104_YM_NB/FRAME_MAX_YM_NB + 1;
	}

	for(i=0; i<frame_nb; i+=1){

		index = 0;
		if(group_index >= IEC104_MAX_GROUP){ /*当前发送缓存已满，不再添加，否则数组溢出。*/ 
			return;
		}
		//起始字0x68
		IEC104_param->group_send_buff[group_index][index++] = 0x68;
		//APDU长度最后用index填写。
		index++;
		//发送序列号
		index++;
		index++;
		//接收序列号
		index++;
		index++;
		//类型标识
		IEC104_param->group_send_buff[group_index][index++] = TYPE_YM;
		//可变结构限定词
		if(i == (frame_nb - 1)){
			data_nb = IEC104_YM_NB % FRAME_MAX_YM_NB;
		}else{
			data_nb = FRAME_MAX_YM_NB;
		}
		IEC104_param->group_send_buff[group_index][index++] = data_nb | 0x80;
		//传输原因
		IEC104_param->group_send_buff[group_index][index++] = REASON_QUERY_ASK;
		IEC104_param->group_send_buff[group_index][index++] = 0x00;
		//公共体地址
		IEC104_param->group_send_buff[group_index][index++] = config_buff[OFFSET_DEVICE_ADDR] & 0x00ff;
		IEC104_param->group_send_buff[group_index][index++] = (config_buff[OFFSET_DEVICE_ADDR] >> 8) & 0x00ff;
		//信息体地址。
		data_addr = START_ADDR_YM + i * FRAME_MAX_YM_NB;
		IEC104_param->group_send_buff[group_index][index++] = data_addr & 0x00FF;
		IEC104_param->group_send_buff[group_index][index++] = (data_addr >> 8) & 0x00FF;
		IEC104_param->group_send_buff[group_index][index++] = 0x00;

		//从数组中取出信息放入信息体中
		for(j=0; j<data_nb; j+=1){
			data_offset = i * FRAME_MAX_YM_NB + j;
			data_ptr =  ym_ptr + data_offset * 2;

			IEC104_param->group_send_buff[group_index][index++] = *(data_ptr + 1) & 0x00ff;
			IEC104_param->group_send_buff[group_index][index++] = (*(data_ptr + 1) >> 8) & 0x00ff;
			IEC104_param->group_send_buff[group_index][index++] = *(data_ptr) & 0x00ff;
			IEC104_param->group_send_buff[group_index][index++] = (*(data_ptr) >> 8) & 0x00ff;
			//品质描述词。
			IEC104_param->group_send_buff[group_index][index++] = 0x00;
		}
		IEC104_param->group_send_buff[group_index][1] = index - 2;
		IEC104_param->group_send_length[group_index] = index;
		group_index ++;
		IEC104_param->need_send_nb ++;
	}

	return ;
}		/* -----  end of function IEC104_build_ym  ----- */

/* * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_S
 *  Description:  组建要发送的S格式帧。
 * =====================================================================================
 */
void IEC104_build_S(IEC104_Param_T *IEC104_param){

	uint8_t ret;
	uint8_t response[6];
	uint16_t recv_count = IEC104_param->recv_SN;

	recv_count = (recv_count << 1) & 0XFFFE;

	response[0] = 0x68;
	response[1] = 0x04;
	response[2] = 0x01;
	response[3] = 0x00;
	response[4] = recv_count & 0x00FF;
	response[5] = (recv_count >> 8) & 0x00FF;

	ret = IEC104_send(IEC104_param, response, 6);
	return ;
}		/* -----  end of function IEC104_build_S  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_U
 *  Description:  组建要发送的U格式帧。
 * =====================================================================================
 */
void IEC104_build_U(IEC104_Param_T *IEC104_param, uint8_t ctrl_code){

	uint8_t ret;
	uint8_t response[6];

	response[0] = 0x68;
	response[1] = 0x04;
	response[2] = ctrl_code;
	response[3] = 0x00;
	response[4] = 0x00;
	response[5] = 0x00;

	ret = IEC104_send(IEC104_param, response, 6);
	return ;
}		/* -----  end of function IEC104_build_U  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_confirm
 *  Description:  组建响应主站召唤的起始帧。
 * =====================================================================================
 */
void IEC104_build_confirm(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb){

	uint16_t i;
	uint16_t group_index = IEC104_param->need_send_nb;

	if(group_index >= IEC104_MAX_GROUP){
		return;
	}

	for(i=0; i<recv_nb; i+=1){
		IEC104_param->group_send_buff[group_index][i] = recv_buff[i];
	}

	//返回镜像，但传输原因字节不相同。
	IEC104_param->group_send_buff[group_index][OFFSET_REASON] = REASON_ACTIVE_CONFIRM;
	IEC104_param->group_send_length[group_index] = recv_nb;

	IEC104_param->need_send_nb ++;

	return ;
}		/* -----  end of function IEC104_build_confirm  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_over
 *  Description:  组建响应主站召唤的结束帧。
 * =====================================================================================
 */
void IEC104_build_over(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb){

	uint16_t i;
	uint16_t group_index = IEC104_param->need_send_nb;

	if(group_index >= IEC104_MAX_GROUP){
		return;
	}
	for(i=0; i<recv_nb; i+=1){
		IEC104_param->group_send_buff[group_index][i] = recv_buff[i];
	}

	//返回镜像，但传输原因字节不相同。
	IEC104_param->group_send_buff[group_index][OFFSET_REASON] = REASON_ACTIVE_OVER;
	IEC104_param->group_send_length[group_index] = recv_nb;

	IEC104_param->need_send_nb ++;

	return ;
}		/* -----  end of function IEC104_build_over  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_build_error
 *  Description:  组建主站召唤的错误帧(I帧内含异常错误)。
 * =====================================================================================
 */
void IEC104_build_error(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb, uint8_t error_byte){

	uint16_t i;
	uint16_t group_index = IEC104_param->need_send_nb;

	if(group_index >= IEC104_MAX_GROUP){
		return;
	}

	for(i=0; i<recv_nb; i+=1){
		IEC104_param->group_send_buff[group_index][i] = recv_buff[i];
	}

	//返回镜像，但传输原因字节不相同。
	IEC104_param->group_send_buff[group_index][OFFSET_REASON] = error_byte;
	IEC104_param->group_send_length[group_index] = recv_nb;

	IEC104_param->need_send_nb ++;

	return ;
}		/* -----  end of function IEC104_build_error  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_active_yt
 *  Description:  
 * =====================================================================================
 */
void IEC104_active_yt(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb, uint16_t ver_addr){

	uint8_t ret = TRUE;
	
	switch(recv_buff[OFFSET_YT_QL]){
		case YT_CHOICE:	
			//TODO:此处添加判断是否可以执行遥调代码。 
			//
			if(ret == TRUE){
				IEC104_build_confirm(IEC104_param, recv_buff, recv_nb); /*判断可以执行，返回客户端确认。*/ 
			}else{
				IEC104_build_over(IEC104_param, recv_buff, recv_nb); /*返回客户端执行结束帧。*/ 
			}
			break;
		case YT_ACTIVE:	
			//TODO:此处添加执行遥调代码(最好也先判断一下是否可以执行)。 
			//
			IEC104_build_over(IEC104_param, recv_buff, recv_nb); /*返回客户端执行结束帧。*/ 
			break;
		default:	
			break;
	}				/* -----  end switch  ----- */

	return ;
}		/* -----  end of function IEC104_active_yt  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_active_yk
 *  Description:  
 * =====================================================================================
 */
void IEC104_active_yk(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb, uint16_t ver_addr){

	uint8_t ret;

	switch(recv_buff[OFFSET_YK_SCO]){
		case YK_ACTIVE_CLOSE:	
			//TODO:此处添加执行遥控关闭代码(最好也先判断一下是否可以执行)。 
			//
			IEC104_build_over(IEC104_param, recv_buff, recv_nb); /*返回客户端执行结束帧。*/ 
			break;
		case YK_ACTIVE_OPEN:	
			//TODO:此处添加执行遥控打开代码(最好也先判断一下是否可以执行)。 
			//
			IEC104_build_over(IEC104_param, recv_buff, recv_nb); /*返回客户端执行结束帧。*/ 
			break;
		case YK_CHOICE_CLOSE:	
			//TODO:此处添加判断是否可以遥控关闭代码。 
			//
			if(ret == TRUE){
				IEC104_build_confirm(IEC104_param, recv_buff, recv_nb); /*判断可以执行，返回客户端确认。*/ 
			}else{
				IEC104_build_over(IEC104_param, recv_buff, recv_nb); /*判断不可以执行，返回客户端不能遥控*/ 
			}
			break;
		case YK_CHOICE_OPEN:	
			//TODO:此处添加判断是否可以遥控打开代码。 
			//
			if(ret == TRUE){
				IEC104_build_confirm(IEC104_param, recv_buff, recv_nb); /*判断可以执行，返回客户端确认。*/ 
			}else{
				IEC104_build_over(IEC104_param, recv_buff, recv_nb); /*判断不可以执行，返回客户端不能遥控*/ 
			}
			break;
		default:	
			break;
	}				/* -----  end switch  ----- */

	return ;
}		/* -----  end of function IEC104_active_yk  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_analy_yk
 *  Description:  
 * =====================================================================================
 */
void IEC104_analy_yk(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb){

	uint16_t ver_addr;

	ver_addr = recv_buff[OFFSET_VER_ADDR + 1];
	ver_addr = (ver_addr << 8) & 0xff00;
	ver_addr = (ver_addr + recv_buff[OFFSET_VER_ADDR]);

	/*判断遥控的变量地址是否正确，如果不正确则返回客户端告知信息体地址不正确。*/ 
	if((ver_addr >= START_ADDR_YK + IEC104_YK_NB) || (ver_addr < START_ADDR_YK)){
		IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_VER_ADDR);
		return ;
	}

	switch(recv_buff[OFFSET_REASON]){
		case REASON_ACTIVE:		//激活或者执行确认。
			IEC104_active_yk(IEC104_param, recv_buff, recv_nb, ver_addr);
			break;
		case REASON_STOP_ACTIVE:	//停止激活
			IEC104_build_over(IEC104_param, recv_buff, recv_nb);
			break;
		default:	
			IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_REASON);
			break;
	}				/* -----  end switch  ----- */
	return ;
}		/* -----  end of function IEC104_analy_yk  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_analy_yt
 *  Description:  
 * =====================================================================================
 */
void IEC104_analy_yt(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb){

	uint16_t ver_addr;

	ver_addr = recv_buff[OFFSET_VER_ADDR + 1];
	ver_addr = (ver_addr << 8) & 0xff00;
	ver_addr = (ver_addr + recv_buff[OFFSET_VER_ADDR]);

	if((ver_addr >= START_ADDR_YT + IEC104_YT_NB) || (ver_addr < START_ADDR_YT)){
		IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_VER_ADDR);
		return;
	}

	switch(recv_buff[OFFSET_REASON]){
		case REASON_ACTIVE:		//激活或者执行确认。
			IEC104_active_yt(IEC104_param, recv_buff, recv_nb, ver_addr);
			break;
		case REASON_STOP_ACTIVE:	//停止激活
			IEC104_build_over(IEC104_param, recv_buff, recv_nb);
			break;
		default:	
			IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_REASON);
			break;
	}				/* -----  end switch  ----- */

	return ;
}		/* -----  end of function IEC104_analy_yt  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_I_check
 *  Description:  
 * =====================================================================================
 */
uint8_t IEC104_I_check(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb){

	uint16_t local_send_nb;
	uint16_t local_recv_nb;

	uint16_t client_send_nb;
	uint16_t client_recv_nb;

	local_send_nb = IEC104_param->send_SN;
	local_recv_nb = IEC104_param->recv_SN;

	client_send_nb = recv_buff[3];
	client_send_nb = ((client_send_nb << 8) + recv_buff[2]) >> 1;
	client_send_nb &= 0x7fff;

	client_recv_nb = recv_buff[5];
	client_recv_nb = ((client_recv_nb << 8) + recv_buff[4]) >> 1;
	client_recv_nb &= 0x7fff;

	//接收序列号加一。
	IEC104_param->recv_SN ++;
	if(IEC104_param->recv_SN > 0x7FFF){
		IEC104_param->recv_SN = 0;
	}

#if 0
	if(recv_buff[OFFSET_DEV_ADDR] != (config_buff[0] & 0xff)){  /*判断公共地址是否正确。*/ 
		IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_DEV_ADDR);
		return FALSE;
	}
#endif

	if((local_send_nb - client_recv_nb) > IEC104_MAX_K){ /*判断接收的I帧接收序列号是否正常。*/ 
		IEC104_error_treat(IEC104_param, "I Frame Recv SN Error!", RECV_SN_ILLEGAL);
		return FALSE;
	}else{
		IEC104_param->k_value = local_send_nb - client_recv_nb;
	}

//	if((client_send_nb - local_recv_nb) > IEC104_MAX_K){ /*判断接收的I帧发送序列号是否正常。*/ 
//
//		printf("\nclient_send_nb:%d\n", client_send_nb);
//		printf("\nlocal_recv_nb:%d\n", local_recv_nb);
//		IEC104_error_treat(IEC104_param, "I Frame Send SN Error!", SEND_SN_ILLEGAL);
//		return FALSE;
//	}

	return TRUE;
}		/* -----  end of function IEC104_I_check  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_S_analy
 *  Description:  
 * =====================================================================================
 */
void IEC104_S_analy(IEC104_Param_T *IEC104_param, uint8_t *recv_buff){

	uint16_t local_send_nb;
	uint16_t client_recv_nb;

	local_send_nb = IEC104_param->send_SN;

	client_recv_nb = recv_buff[5];
        client_recv_nb = (client_recv_nb << 8) & 0xff00;
	client_recv_nb = (client_recv_nb + recv_buff[4]) >> 1;
	client_recv_nb &= 0x7fff;

	if((local_send_nb - client_recv_nb) > IEC104_MAX_K){
		IEC104_error_treat(IEC104_param, "S Frame Receive SN Error!", RECV_SN_ILLEGAL);
	}else{
		if(IEC104_param->timer2_flag == TRUE){
				IEC104_param->timer2_flag == FALSE;
		}
		IEC104_param->k_value = local_send_nb - client_recv_nb;
	}	

	return ;
}		/* -----  end of function IEC104_S_analy  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_I_analy
 *  Description:  
 * =====================================================================================
 */
void IEC104_I_analy(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb){

	uint8_t ret;
	uint8_t type_flag = recv_buff[OFFSET_TYPE];

	ret = IEC104_I_check(IEC104_param, recv_buff, recv_nb);

	if(ret == FALSE){
		return ;
	}

	switch(type_flag){
		case TYPE_TOTAL_QUERY:	//总召唤
			if(recv_buff[OFFSET_REASON] != REASON_ACTIVE){
				IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_REASON);
				return;
			}
			IEC104_build_confirm(IEC104_param, recv_buff, recv_nb);
			IEC104_build_yx(IEC104_param);
			IEC104_build_yc(IEC104_param);
			IEC104_build_over(IEC104_param, recv_buff, recv_nb);
			break;
		case TYPE_YM_QUERY:	//电度总召唤
			if(recv_buff[OFFSET_REASON] != REASON_ACTIVE){
				IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_REASON);
				return;
			}
			IEC104_build_confirm(IEC104_param, recv_buff, recv_nb);
			IEC104_build_ym(IEC104_param);
			IEC104_build_over(IEC104_param, recv_buff, recv_nb);
			break;
		case TYPE_YK:		//单点遥控
			IEC104_analy_yk(IEC104_param, recv_buff, recv_nb);
			break;
		case TYPE_YT:		//单点遥调
			IEC104_analy_yt(IEC104_param, recv_buff, recv_nb);
			break;
		case TYPE_TIMING:	//校时命令
			if(recv_buff[OFFSET_REASON] != REASON_ACTIVE){
				IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_REASON);
				return;
			}
			IEC104_set_system_time(IEC104_param, recv_buff);
			IEC104_build_confirm(IEC104_param, recv_buff, recv_nb);
			break;
		default:
			IEC104_build_error(IEC104_param, recv_buff, recv_nb, REASON_UNKNOWN_TYPE);
			break;
	}

	return ;
}		/* -----  end of function IEC104_I_analy  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_U_analy
 *  Description:  
 * =====================================================================================
 */
void IEC104_U_analy(IEC104_Param_T *IEC104_param, uint8_t *recv_buff){

	switch(recv_buff[2]){
		case 0x43:		//客户端发送的测试帧，返回确认
			IEC104_build_U(IEC104_param, 0x83);
			break;
		case 0x83:		//客户端返回确认，关闭定时器。
			IEC104_param->timer1_flag = FALSE;
			break;
		case 0x13:		//客户端发送的停止命令，返回确认。
			IEC104_build_U(IEC104_param, 0x23);
			IEC104_param->send_SN = 0;
			IEC104_param->recv_SN = 0;
			IEC104_param->has_send_nb = 0;
			IEC104_param->need_send_nb = 0;
			break;
		case 0x23:		//客户端返回的停止确认，不会出现。
			break;
		case 0x07:		//客户端发送的启动命令，返回确认。
			IEC104_build_U(IEC104_param, 0x0B);
			IEC104_param->send_SN = 0;
			IEC104_param->recv_SN = 0;
			break;
		case 0x0b:		//客户端返回的启动停止，不会出现。
			break;
		default:
			break;
	}
	return ;
}		/* -----  end of function IEC104_U_analy  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_timer_analy
 *  Description:  
 * =====================================================================================
 */
void IEC104_timer_analy(IEC104_Param_T *IEC104_param){

	uint8_t ret = 0;

	if(IEC104_param->timer1_flag == TRUE){
		IEC104_param->timer1_count ++;
		if(IEC104_param->timer1_count >= IEC104_TIMER1_VALUE){
			IEC104_param->timer1_count = 0;
			IEC104_error_treat(IEC104_param, "Timer1 Timeout!", TIMER1_TIMEOUT);
		}
	}

	if(IEC104_param->timer2_flag == TRUE){
		IEC104_param->timer2_count ++;
		if(IEC104_param->timer2_count >= IEC104_TIMER2_VALUE){
			IEC104_param->timer2_count = 0;
			IEC104_error_treat(IEC104_param, "Timer2 Timeout!", TIMER2_TIMEOUT);
		}
	}

	if(IEC104_param->timer3_flag == TRUE){
		IEC104_param->timer3_count ++;
		if(IEC104_param->timer3_count >= IEC104_TIMER3_VALUE){
			IEC104_param->timer3_count = 0;
			IEC104_param->timer1_flag = TRUE;
			IEC104_build_U(IEC104_param, 0x43);
		}
	}

	//判断当前是否有数据需要发送。
	if((IEC104_param->need_send_nb) > 0){
		IEC104_send_group(IEC104_param);
	}

	return ;
}		/* -----  end of function IEC104_timer_analy  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_analy
 *  Description:  数据帧类型判断，解析接收到时I，S，U帧。
 * =====================================================================================
 */
void IEC104_analy(IEC104_Param_T *IEC104_param, uint8_t *recv_buff, uint16_t recv_nb){


	uint8_t frame_type = recv_buff[2] & 0x03;

	switch(frame_type){
		case 0x00://I帧
			IEC104_I_analy(IEC104_param, recv_buff, recv_nb);
			break;
		case 0X01://S帧
			IEC104_S_analy(IEC104_param, recv_buff);
			break;
		case 0x02://I帧
			IEC104_I_analy(IEC104_param, recv_buff, recv_nb);
			break;
		case 0x03://U帧
			IEC104_U_analy(IEC104_param, recv_buff);
			break;
		default:
			break;
	}	
	return ;
}		/* -----  end of function IEC104_analy  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread_for_IEC104
 *  Description:  创建的IEC104协议通讯线程。
 * =====================================================================================
 */
//void *thread_for_IEC104(void * data){
//
//	uint16_t i;
//	uint16_t j;
//	int8_t ret;
//	IEC104_Param_T IEC104_param;
//	memset(&IEC104_param, 0, sizeof(IEC104_Param_T));
//
//	IEC104_param.fd = *((int16_t *)data);
//	IEC104_init_param(&IEC104_param);
//
//	while(1){
//		ret = IEC104_recv(&IEC104_param, IEC104_param.recv_buff);
//
//		switch(ret){
//			case COMM_TIME_OUT:	 /*通讯超时*/ 
//				IEC104_timer_analy(&IEC104_param);
//				break;
//			case DATA_ERROR:	 /*接收到异常数据*/
//				for(i=0; i<IEC104_param.recv_length; i+=1){
//					if(IEC104_param.recv_buff[i] != 0x68){
//						break;
//					}
//					IEC104_analy(&IEC104_param, &IEC104_param.recv_buff[i], (IEC104_param.recv_buff[i + 1] + 2));
//					i += IEC104_param.recv_buff[i + 1] + 1;
//				}
//				IEC104_param.timer3_count = 0; 
//				break;
//			case DATA_OK:	 	 /*接收到正确数据*/ 
//				IEC104_analy(&IEC104_param, IEC104_param.recv_buff, IEC104_param.recv_length);
//				IEC104_param.timer3_count = 0;
//				break;
//			case SOCKET_FAILURE:	 /*Socket错误，重新链接*/ 
//				IEC104_error_treat(&IEC104_param, "Receive Socket Error!", SOCKET_FAILURE);
//				break;
//			default:	
//				break;
//		}				/* -----  end switch  ----- */
//	}
//	return ;
//}		/* -----  end of function thread_for_IEC104  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  IEC104_process
 *  Description:  IEC104协议处理的接口函数.周期性调用.
 * =====================================================================================
 */
uint8_t IEC104_process(IEC104_param_t *iec104_param, uint8_t *send_buff, uint16_t send_size, uint8_t* recv_buff, uint16_t recv_size){

	return ;
}		/* -----  end of function IEC104_process  ----- */

