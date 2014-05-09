
#include 	"includes.h"
#include	<unistd.h>

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  hello world!测试函数。
 * =====================================================================================
 */
int main (void){

	//git test
	uint8_t i;
	pthread_t thread[MAX_THREAD_NUM];

	load_all_config_param();

	pthread_create(&thread[0], NULL, (void *)thread01_net1_data_comm, NULL);

	for ( i = 0; i < MAX_THREAD_NUM; i += 1 ) {
		pthread_join(thread[i], NULL);
	}

	sleep(2);
	printf("Exit All Thread !!!\r\n");
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

