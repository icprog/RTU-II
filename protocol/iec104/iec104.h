#ifndef IEC104_H
#define	IEC104_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <signal.h>

#define		IEC104_MAX_GROUP		64 	/*发送的最大二位数组缓存大小。*/ 
#define 	IEC104_RECV_BUFF		255	/*接收缓存的大小*/ 
#define		IEC104_MAX_K			12 	/*未被确认的最大I帧数目。*/ 
#define		IEC104_MAX_CLIENT		5 	/*允许的最大链接数量。*/ 

#define		IEC104_PER_SECOND		20 	/*利用Select函数每50ms产生一次超时，所有一秒种计数20次。*/ 
#define		IEC104_TIMER1_VALUE		15 * IEC104_PER_SECOND
#define		IEC104_TIMER2_VALUE		10 * IEC104_PER_SECOND
#define		IEC104_TIMER3_VALUE		20 * IEC104_PER_SECOND

#define 	IEC104_YX_NB		 	50 	/*遥信量的数量*/ 
#define 	IEC104_YC_NB		 	50 	/*遥测量的数量*/ 
#define 	IEC104_YM_NB		 	50 	/*电度量的数量*/ 
#define 	IEC104_YT_NB		 	50 	/*遥调量的数量*/ 
#define 	IEC104_YK_NB		 	50 	/*遥控量的数量*/ 

#define 	OFFSET_DEVICE_ADDR		0x00	/*配置信息表中设备地址的偏移量（即协议的公共体地址）*/ 


/* 
 * =====================================================================================
 *         			Local Defines
 * =====================================================================================
 */

#define 	COMM_TIME_OUT			0x00	
#define 	DATA_OK					0x01
#define 	DATA_ERROR				0x02
#define 	SOCKET_FAILURE			0x04	
#define 	SELECT_FAILURE			0x05	

#define		TIMER1_TIMEOUT			-0x01
#define		TIMER2_TIMEOUT			-0x02
#define		K_UNCONFIRM				-0x03
#define		SEND_SN_ILLEGAL			-0x04
#define		RECV_SN_ILLEGAL			-0x05

#define 	TRUE				0x01	
#define 	FALSE				0x00	

#define 	ON				0x01	
#define 	OFF				0x00	


//typedef struct{
//
//
//} IEC104_Param_T;


typedef struct{
        int16_t fd;						/*Socket文件描述符*/
        uint8_t debug; 						/*调试标志位*/
	uint8_t k_value;					/*K值，未被确认的最大数目 */
	uint8_t timer1_flag; 					/*定时器1的标志位*/
	uint8_t timer2_flag; 					/*定时器2的标志位*/
	uint8_t timer3_flag; 					/*定时器3的标志位*/

	uint16_t timer1_count; 					/*定时器1的计数值*/
	uint16_t timer2_count; 					/*定时器2的计数值*/
	uint16_t timer3_count; 					/*定时器3的计数值*/

	uint16_t send_SN; 					/*发送序列号 */
	uint16_t recv_SN;					/*接收序列号 */
	uint16_t has_send_nb; 					/*已经发送帧数量*/
	uint16_t need_send_nb; 					/*需要发送帧数量*/
	uint16_t recv_length; 					/*接收到的信息帧长度*/
	uint8_t recv_buff[IEC104_RECV_BUFF];			/*接收数据缓存区*/
	uint8_t group_send_buff[IEC104_MAX_GROUP][256]; 	/*需要分组发送缓冲器*/
	uint8_t group_send_length[IEC104_MAX_GROUP]; 		/*需要分组发送数据长度*/
} IEC104_Param_T;

uint16_t iec104_client_nb;   	/*记录当前客户端的链接数量*/ 

uint16_t config_buff[10]; 	/*模拟的配置信息表,从中读取设备的地址信息。*/ 

uint8_t  yx_buff[100]; 		/*模拟的遥信量信息表*/ 
uint16_t yc_buff[100]; 		/*模拟的遥测量信息表*/ 
uint16_t ym_buff[100]; 		/*模拟的电能值信息表*/ 
uint16_t yt_buff[100]; 		/*模拟的遥调量信息表*/ 
uint16_t yk_buff[100]; 		/*模拟的遥控量信息表*/ 

uint8_t *yx_ptr; 		/*指向遥信数据表的指针*/ 
uint16_t *yc_ptr; 		/*指向遥测数据表的指针*/ 
uint16_t *ym_ptr; 		/*指向电能数据表的指针*/ 
uint16_t *yt_ptr; 		/*指向遥调数据表的指针*/ 
uint16_t *yk_ptr; 		/*指向遥控数据表的指针*/ 

int16_t IEC104_tcp_listen(struct sockaddr_in *addr, socklen_t *addrlen);
uint16_t IEC104_recv(IEC104_Param_T *IEC104_param, uint8_t *data);

#ifdef	__cplusplus
}
#endif

#endif	/* IEC104_H */

