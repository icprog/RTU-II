
#include "includes.h"
#include "iec104.h"
//#include "./protocol/iec104/iec104.h"

#include	<time.h>
#include	<string.h>
#include	<sys/socket.h>
#include 	<netinet/in.h>

#define		ERROR_SELECT_RET	-1 		/*SELECT函数返回错误*/ 
#define		ERROR_RECV_RET		-2 		/*接收函数返回错误*/ 
#define		ERROR_TIMEOUT		-3 		/*通讯超时错误*/ 
#define		ERROR_RECV_DATA		-4 		/*接收到异常数据帧*/ 
#define		ERROR_CONNECT_SHUT	-5 		/*远端关闭链接处理*/ 

uint8_t thread01_send_buff[THREAD01_SEND_BUFF_SIZE];
uint8_t thread01_recv_buff[THREAD01_RECV_BUFF_SIZE];

int16_t thread01_send_size;
int16_t thread01_recv_size;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_socket_open
 *  Description:  打开一个新的SOCKET，并且绑定端口进入服务器模式。
 * =====================================================================================
 */
int16_t thread01_net1_socket_open(uint8_t *ip, uint16_t port){

	int16_t ret;
	int16_t socket_fd;
	fd_set read_fd;

	struct sockaddr_in	server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; 			/*地址族*/ 
	server_addr.sin_port = htons(port); 			/*需要绑定的服务器端口。*/ 
//	server_addr.sin_addr = inet_addr("192.168.0.1"); 	/**/ 
//	server_addr.sin_addr = htonl(INADDR_ANY); /**/ 

	socket_fd = socket(AF_INET, SOCK_STREAM, 0); /*AF_INET:IPv4协议*/  /*SOCK_STREAM:字节流套接字*/ 
	if(socket_fd == -1){
//		fprintf(stderr,"Socket error:%s\n\a",strerror(errno));  /*向显示器输出错误信息。*/ 
		printf("Socket Error! \r\n");
		exit(1); 
	}else{
		printf("Socket OK! \r\n");
	}

	ret = bind(socket_fd,(struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret == -1){
		printf("Bind Error! \r\n");
		exit(1); 
	}else{
		printf("Bind OK! \r\n");
	}

	ret = listen(socket_fd, 1); 				/*只允许一个客户端的链接。*/ 
	if(ret == -1){
		printf("Listen Error! \r\n");
		exit(1); 
	}else{
		printf("Listen OK! \r\n");
	}

	return socket_fd;
}		/* -----  end of function thread01_net1_socket_open  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_socket_shut
 *  Description:  关闭已经建立的SOCKET。
 * =====================================================================================
 */
void thread01_net1_socket_shut(uint16_t socket_fd){

        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);

	return ;
}		/* -----  end of function thread01_net1_socket_shut  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_accept_open
 *  Description:  
 * =====================================================================================
 */
uint16_t thread01_net1_accept_open(uint16_t socket_fd){

	socklen_t len;
	int8_t buff[50];
	int16_t accept_fd;
	struct sockaddr_in	client_addr;

	len = sizeof(client_addr);
	accept_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &len);
	printf("Accept OK! \r\n");
	printf("connection from %s, port %d\n",
			inet_ntop(AF_INET, &client_addr.sin_addr, buff, sizeof(buff)),
			ntohs(client_addr.sin_port));

	return accept_fd;
}		/* -----  end of function thread01_net1_accept_open  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_accept_shut
 *  Description:  关闭已经接收到的链接。
 * =====================================================================================
 */
void thread01_net1_accept_shut(int16_t accept_fd){

	shutdown(accept_fd, SHUT_RDWR);
	printf("shutdown accept_fd OK! \r\n");

	return ;
}		/* -----  end of function thread01_net1_accept_shut  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_error_treat
 *  Description:  线程1的异常错误处理函数。
 * =====================================================================================
 */
static void thread01_net1_error_treat(int16_t code, const char *string){

        printf("\nERROR %s (%d)\n", string, code);

	switch(code){
		case ERROR_SELECT_RET:	
			break;
		case ERROR_RECV_RET:	
			break;
		case ERROR_TIMEOUT:	
			break;
		case ERROR_RECV_DATA:	
			break;
		case ERROR_CONNECT_SHUT:	
			break;
		default:	
			break;
	}				/* -----  end switch  ----- */

	return ;
}		/* -----  end of function thread01_net1_error_treat  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_data_send
 *  Description:  网络数据发送函数。
 * =====================================================================================
 */
void thread01_net1_data_send(void){

	return ;
}		/* -----  end of function thread01_net1_data_send  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_data_recv
 *  Description:  网络数据接收函数。
 * =====================================================================================
 */
//void thread01_net1_data_recv(int16_t accept_fd, uint16_t timeout){
//
//	uint16_t ret;
//	int16_t select_ret, recv_ret;
//	struct timeval tv;
//	fd_set read_fd;
//
//	FD_ZERO(&read_fd);
//	FD_SET(accept_fd, &read_fd);
//
//	tv.tv_sec = 0;
//	tv.tv_usec = timeout * 1000; /*将毫秒转化为微妙用于select函数。*/
//
//	while(1){
//		select_ret = select(accept_fd+1, &read_fd, NULL, NULL, &tv);
//		switch(select_ret){
//			case -1:	 	/*返回异常。*/
//			case 0:	 		/*等待超时。*/
//				thread01_net1_socket_error_treat(accept_fd, 0);
//				accept_fd = thread01_net1_accept(socket_fd);
//				break;
//			default:		/*所监视的文件描述符接收到数据。*/
//				recv_ret = recv(accept_fd, &data[recv_ptr], IEC104_RECV_BUFF, 0);
//				if(recv_ret <= 0){
//					return SOCKET_FAILURE;
//				}
//
//				recv_length += read_ret;
//				if(recv_ptr == 0){ 		/*第一次接收到数据，则根据协议第二个字节判断帧数据的长度。*/
//					frame_length = data[OFFSET_LENGTH] + 2;
//				}
//
//				if(read_ret == frame_length){ 	/*接收数据长度等于帧长度，数据接收完成。*/
//					if(IEC104_param->debug == TRUE){
//						for (i = 0; i < recv_length; i++)
//							printf("[%.2X]", data[i]);
//						printf("\n\n");
//					}
//					IEC104_param->recv_length = recv_length;
//					return DATA_OK;
//				}
//
//				if(read_ret > frame_length){	/*接收数据长度大于帧长度，可能多帧连在一起。*/
//					if(IEC104_param->debug == TRUE){
//						for (i = 0; i < recv_length; i++)
//							printf("[%.2X]", data[i]);
//						printf("\n\n");
//					}
//					IEC104_param->recv_length = recv_length;
//					return DATA_ERROR;
//				}
//
//				if(read_ret < frame_length){ 	/*接收数据长度小于帧长度，继续接收数据。*/
//					tv.tv_sec = 0;
//					tv.tv_usec = 50000;
//					recv_ptr += read_ret;
//					WAIT_DATA();
//					if(select_ret == 0){
//						IEC104_param->recv_length = recv_length;
//						return DATA_ERROR;
//					}
//				}
//
//				ret = thread01_net1_socket_read(accept_fd);
//				if(ret <= 0){
//					thread01_net1_socket_error_treat(accept_fd, 0);
//					accept_fd = thread01_net1_accept(socket_fd);
//				}
//				break;
//		}				/* -----  end switch  ----- */
//	}
//
//	return ;
//}		/* -----  end of function thread01_net1_data_recv  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  thread01_net1_data_comm
 *  Description:  线程1上行数据通讯主函数。
 * =====================================================================================
 */
void *thread01_net1_data_comm(void){

	uint16_t i;
	uint16_t j;
	int16_t socket_fd;
	int8_t ret;
	IEC104_Param_T IEC104_param;
	memset(&IEC104_param, 0, sizeof(IEC104_Param_T));

	printf("Thread_01 start OK! \r\n");

	IEC104_init_param(&IEC104_param);
//	socket_fd = thread01_net1_socket_open();
	IEC104_param.fd = thread01_net1_accept_open(socket_fd);

	while(1){
		ret = IEC104_recv(&IEC104_param, IEC104_param.recv_buff);
		switch(ret){
			case COMM_TIME_OUT:	 /*通讯超时*/ 
				IEC104_timer_analy(&IEC104_param);
				break;
			case DATA_ERROR:	 /*接收到异常数据*/
				for(i=0; i<IEC104_param.recv_length; i+=1){
					if(IEC104_param.recv_buff[i] != 0x68){
						break;
					}
					IEC104_analy(&IEC104_param, &IEC104_param.recv_buff[i], (IEC104_param.recv_buff[i + 1] + 2));
					i += IEC104_param.recv_buff[i + 1] + 1;
				}
				IEC104_param.timer3_count = 0; 
				break;
			case DATA_OK:	 	 /*接收到正确数据*/ 
				IEC104_analy(&IEC104_param, IEC104_param.recv_buff, IEC104_param.recv_length);
				IEC104_param.timer3_count = 0;
				break;
			case SOCKET_FAILURE:	 /*Socket错误，重新链接*/ 
				thread01_net1_accept_shut(IEC104_param.fd);
				IEC104_param.fd = thread01_net1_accept_open(socket_fd);
				break;
			default:	
				break;
		}				/* -----  end switch  ----- */
	}

	return NULL;
}		/* -----  end of function thread01_net1_data_comm  ----- */

