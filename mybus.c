
#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h> 
#include <assert.h>

/*sqlite3 ͷ�ļ�*/
//#include <sqlite3.h>  
#include <stddef.h>
/*FIONREAD*/
#include <sys/ioctl.h>

#define  TIME_OUT_TIME 5    //time_out 10s
#define MAXSIZE 1024

unsigned char buff[1024],temp_buff[1024];
unsigned char recv_buff_time[100];
unsigned char recv_buff_versioninfo[100];
unsigned char DEBUG ;
unsigned char debug_buf[5];
unsigned char locate_flag_buf[5];
unsigned char station_buff[5];
int data_length;
int from_uart_len = 0;
unsigned int len_version = 0;
unsigned char from_uart_version_info[100];
int uart_fd;
void init_uart(void);
int open_uart(void);

/*�й���־���ܵĺ�������*/
FILE* openfile(const char *fileName,const char *mode);
int getTime(char *out,int fmt);
int writeFile(FILE *fp,const char *str,int blog);
int closeFile(FILE *fp);
int convet(char *src,char *dest,int length);/*ת��16���Ƶ��ַ���*/
unsigned long convert_atohex(char* str);/*�ַ���ת16����*/
time_t first;/*��ȡ��ʼ��ʱ��*/
time_t heartbeat_t;/*���������*/
time_t time_request_t;/*ʱ��������*/
time_t jinchuzhan_beat;
time_t pos_intval_t;

unsigned int crc16(unsigned char *buf, unsigned int len);
ssize_t nread(int fd,unsigned char *ptr);

unsigned int crc16(unsigned char *buf, unsigned int len);
void *save_msg();
void *send_msg();
void *rec_ask();
void *info_from_service_and_send_to_uart();
void *locate_process();
unsigned int crc16_serial(unsigned char *buf, unsigned int len);
ssize_t xread(int fd,void *ptr,size_t n) ;


/*locate */


/******************************
*Global variable
*******************************/
unsigned char OBU_NUM = 0;/*��ʶ��ͬ��OBU*/
unsigned char departure_flag = 0;
int pos_flag = 0;
int posvalue = 0;
int locate_flag;
int station_flag;
char rssi_pair[9];
unsigned char obu_id_pos[8];
time_t time_intval;/*�������ڵ�����*/
int len_out_queue = 0;
unsigned char pos_send_to_service[50];
unsigned char report_station_to_service[50];
//int MAXSIZE_locate  = 50;
#define MAXSIZE_locate 50
char PTR[60];
int rssi1[32], rssi2[32], rssi3[32], rssi4[32];//������λ��Ҫ
int Isleave;/*����λ����Ϣ�ı�־*/
int Ispos_two;
int Ispos_one;
int Ispos_three;
int rssi_one_num = 0;
int rssi_two_num = 0;
int rssi_three_num = 0;
int rssi_four_num = 0;

int colldata[50][6];
unsigned char chuzhan_notice[15];
unsigned char chuzhan_notice_send_to_uart[15];
int colldata_size = 0;
int colldata_max_zsf = 10;
int colldata_maxsize = 30;
unsigned int Serial_Id_Sql = 0;

char SQL_Insert[200];
char SQL_Delete[200];
int MaxRssiID(int rssi1, int rssi2, int rssi3, int rssi4);
int AverageRssi(int rssi[]);//��ֹ״̬����(rssi������Ϣ�ڶ�λΪ0x01��
int Position(int rssi1[], int rssi2[], int rssi3[], int rssi4[]);//��ֹ״̬�¶�λ
//bool is_LeaveStation(int rssi1, int rssi2, int rssi3, int rssi4);//�Ƚ�·��3��4��rssiֵ��С���ж��Ƿ��վ
/*locate above*/


unsigned int CRC,k;
unsigned int dev_id;
unsigned int data_attr,data_num,data_crc,f_head,f_end;
unsigned long data_rec,data_send;
int sockfd,nbytes;
pthread_t save_msg_pthread,send_msg_pthread,rec_ask_pthread,info_from_service_and_send_to_uart_pthread,locate_process_pthread;
struct sockaddr_in server_addr; 
struct hostent *host; 

unsigned int  CRCTABLE[256]=
{
        0xF078,0xE1F1,0xD36A,0xC2E3,0xB65C,0xA7D5,0x954E,0x84C7,
        0x7C30,0x6DB9,0x5F22,0x4EAB,0x3A14,0x2B9D,0x1906,0x088F,
        0xE0F9,0xF170,0xC3EB,0xD262,0xA6DD,0xB754,0x85CF,0x9446,
        0x6CB1,0x7D38,0x4FA3,0x5E2A,0x2A95,0x3B1C,0x0987,0x180E,
        0xD17A,0xC0F3,0xF268,0xE3E1,0x975E,0x86D7,0xB44C,0xA5C5,
        0x5D32,0x4CBB,0x7E20,0x6FA9,0x1B16,0x0A9F,0x3804,0x298D,
        0xC1FB,0xD072,0xE2E9,0xF360,0x87DF,0x9656,0xA4CD,0xB544,
        0x4DB3,0x5C3A,0x6EA1,0x7F28,0x0B97,0x1A1E,0x2885,0x390C,
        0xB27C,0xA3F5,0x916E,0x80E7,0xF458,0xE5D1,0xD74A,0xC6C3,
        0x3E34,0x2FBD,0x1D26,0x0CAF,0x7810,0x6999,0x5B02,0x4A8B,
        0xA2FD,0xB374,0x81EF,0x9066,0xE4D9,0xF550,0xC7CB,0xD642,
        0x2EB5,0x3F3C,0x0DA7,0x1C2E,0x6891,0x7918,0x4B83,0x5A0A,
        0x937E,0x82F7,0xB06C,0xA1E5,0xD55A,0xC4D3,0xF648,0xE7C1,
        0x1F36,0x0EBF,0x3C24,0x2DAD,0x5912,0x489B,0x7A00,0x6B89,
        0x83FF,0x9276,0xA0ED,0xB164,0xC5DB,0xD452,0xE6C9,0xF740,
        0x0FB7,0x1E3E,0x2CA5,0x3D2C,0x4993,0x581A,0x6A81,0x7B08,
        0x7470,0x65F9,0x5762,0x46EB,0x3254,0x23DD,0x1146,0x00CF,
        0xF838,0xE9B1,0xDB2A,0xCAA3,0xBE1C,0xAF95,0x9D0E,0x8C87,
        0x64F1,0x7578,0x47E3,0x566A,0x22D5,0x335C,0x01C7,0x104E,
        0xE8B9,0xF930,0xCBAB,0xDA22,0xAE9D,0xBF14,0x8D8F,0x9C06,
        0x5572,0x44FB,0x7660,0x67E9,0x1356,0x02DF,0x3044,0x21CD,
        0xD93A,0xC8B3,0xFA28,0xEBA1,0x9F1E,0x8E97,0xBC0C,0xAD85,
        0x45F3,0x547A,0x66E1,0x7768,0x03D7,0x125E,0x20C5,0x314C,
        0xC9BB,0xD832,0xEAA9,0xFB20,0x8F9F,0x9E16,0xAC8D,0xBD04,
        0x3674,0x27FD,0x1566,0x04EF,0x7050,0x61D9,0x5342,0x42CB,
        0xBA3C,0xABB5,0x992E,0x88A7,0xFC18,0xED91,0xDF0A,0xCE83,
        0x26F5,0x377C,0x05E7,0x146E,0x60D1,0x7158,0x43C3,0x524A,
        0xAABD,0xBB34,0x89AF,0x9826,0xEC99,0xFD10,0xCF8B,0xDE02,
        0x1776,0x06FF,0x3464,0x25ED,0x5152,0x40DB,0x7240,0x63C9,
        0x9B3E,0x8AB7,0xB82C,0xA9A5,0xDD1A,0xCC93,0xFE08,0xEF81,
        0x07F7,0x167E,0x24E5,0x356C,0x41D3,0x505A,0x62C1,0x7348,
        0x8BBF,0x9A36,0xA8AD,0xB924,0xCD9B,0xDC12,0xEE89,0xFF00
};

FILE *fop_log;/*��־�ļ�*/
FILE *send_fail_fop;
char log_name[30];/*��־�ļ���*/
char dir_log_name[40];
int portnumber;
unsigned char time_send_to_uart[20];
unsigned char lukoubianhao[9] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
int ntime;

/*ini��صĽṹ��*/
typedef struct item_t {
    char *key;
    char *value;
}ITEM;
char *strtrimr(char *pstr);//���ַ����ҿո�
char *strtriml(char *pstr);//���ַ�����ո�
char *strtrim(char *pstr);//���ַ������߿ո�
int  get_item_from_line(char *line,  ITEM *item);//����
int file_to_items(const char *file,  ITEM *items,  int *num);
int read_conf_value(const char *key, char *value,const char *file);//���ļ�
int write_conf_value(const char *key,char *value,const char *file);//write
unsigned int checkcrc16( unsigned char *databuf, unsigned int datalen );//crc
unsigned int crc_version = 0;
unsigned int crc_time = 0;
char value_port[10];/*��Ŷ�������ֵ*/
char value_ipadd[30];/*��Ŷ�������ֵ*/
char value_device_id[20];/*��Ŷ�������ֵ*/
char real_ip[15];
unsigned long real_device_id;
unsigned char heart_beat[10];//read from config.ini  heart_beat frequency
unsigned int  heart_beat_int;//int feature for heart_beat frequency

unsigned int serial_num = 0;
unsigned char version_info[22];
/*ѭ������*/
typedef struct
{
    char bitch[MAXSIZE][200];
    int font;
    int rear;
    int length[MAXSIZE]; 
}SqQueue;
/*��ʼ��һ���ն���*/
unsigned int InitQueue(SqQueue *Q)
{
    Q->font = 0;
    Q->rear = 0;
    return 0;
}

int QueueLength(SqQueue Q)
{
    return (Q.rear - Q.font)%MAXSIZE;
}

unsigned int ENQueue(SqQueue *Q,char *buffer,int len)
{
    if((Q->rear+1)%MAXSIZE == Q->font)
        return -1;

    memcpy(Q->bitch[Q->rear],buffer,len);
    Q->length[Q->rear] = len;
    Q->rear = (Q->rear+1)%MAXSIZE;
    return 0;
}
/*������Ҳ���и�����*/
unsigned int DeQueue(SqQueue *Q,char *buffer,int len)
{
    if(Q->font == Q->rear)
        return -1;
	 len = Q->length[Q->font];
        memcpy(buffer,Q->bitch[Q->font],len);
        Q->font = (Q->font+1)%MAXSIZE;
        return len;

}
SqQueue *MyQueue ;
unsigned char RSU_heart_beat[50];	
unsigned int RSU_serial_num = 0x77;

/*Ϊ��λ�㷨��������*/

typedef struct
{
    char bitch[MAXSIZE_locate][60];
    int font;
    int rear;
    int length[MAXSIZE_locate]; 
}SqQueue_locate;

/*��ʼ��һ���ն���*/
unsigned int InitQueue_locate(SqQueue_locate *Q)
{
    Q->font = 0;
    Q->rear = 0;
    return 0;
}

int QueueLength_locate(SqQueue_locate Q)
{
    return (Q.rear - Q.font)%MAXSIZE_locate;
}

unsigned int ENQueue_locate(SqQueue_locate *Q,char *buffer,int len)
{
    if((Q->rear+1)%MAXSIZE_locate== Q->font)
        return -1;

    memcpy(Q->bitch[Q->rear],buffer,len);
    Q->length[Q->rear] = len;
    Q->rear = (Q->rear+1)%MAXSIZE_locate;
    return 0;
}
/*������Ҳ���и�����*/
unsigned int DeQueue_locate(SqQueue_locate *Q,char *buffer,int len)
{
    if(Q->font == Q->rear)
        return -1;
	 len = Q->length[Q->font];
        memcpy(buffer,Q->bitch[Q->font],len);
        Q->font = (Q->font+1)%MAXSIZE_locate;
        return len;

}
unsigned int  SqQueue_locate_clear(SqQueue_locate *Q)
{
	Q->font = 0;
	Q->rear = 0;
	memset(Q->length,0,MAXSIZE_locate);
	return 0;

}

SqQueue_locate *MyQueue_locate ;


int main(int argc, char *argv[]) 
{ 
	//char command[20];
	//struct timeval timeout;
	fd_set read_fd;
	
	time_t timep;
	char convet_buff[100];
	FILE* fd;
	int ret_read_conf;
	unsigned char buff_time[50];//��ʼУʱ�õ�

	/*���ݿ�Ķ���*/
/*
  	int ret_sql = 0;   
 	int value_access = 0;
	value_access= access("Rssi_sql.db",F_OK);
		printf("value_access = %d \n",value_access);
	if(value_access!=0)
		{
		
   		   ret_sql = sqlite3_open_v2("Rssi_sql.db", &db,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,NULL);  
   		   if (ret_sql != SQLITE_OK)  
   	 	   {  
	   	        fprintf(stderr, "�޷������ݿ⣺%s\n", sqlite3_errmsg(db));  
	   	        sqlite3_close(db);  
	   	        return 1;  
     		  }  
		    //ִ�н���SQL  
		    ret_sql = sqlite3_exec(db, sSQL1_create_table, _sql_callback, 0, &pErrMsg);  
		    if (ret_sql != SQLITE_OK)  
		    {  
		        fprintf(stderr, "SQL create error: %s\n", pErrMsg);  
		        sqlite3_free(pErrMsg); //��ֹ�ڴ�й©
		        sqlite3_close(db);  
		        return 1;  
		    }  
		    printf("���ݿ�Rssi_table��ɹ���������\n");  

			  //ִ�н���SQL  
		    ret_sql = sqlite3_exec(db, sSQL1_create_local_table, _sql_callback, 0, &pErrMsg);  
		    if (ret_sql != SQLITE_OK)  
		    {  
		        fprintf(stderr, "SQL create error: %s\n", pErrMsg);  
		        sqlite3_free(pErrMsg); //��ֹ�ڴ�й©
		        sqlite3_close(db);  
		        return 1;  
		    }  
		    printf("���ݿ�local_table��ɹ���������\n");  


			
			 sqlite3_close(db);  
			printf("�ر����ݿ⣡\n");  

	}


   ret_sql= sqlite3_open_v2("Rssi_sql.db", &db,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,NULL);  
   if (ret_sql!= SQLITE_OK)  
    {  
        fprintf(stderr, "�޷������ݿ⣺%s\n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
        return 1;  
    }  

	printf("�����ݿ�\n");

*/

	/*�ڴ����*/
	MyQueue = (SqQueue *)malloc(sizeof(SqQueue));
	MyQueue_locate = (SqQueue_locate *)malloc(sizeof(SqQueue_locate));
	
	struct timeval tv,time_out;
	struct tm *p;
	long time_second;

	time_out.tv_sec=1;
	time_out.tv_usec=0;
     /*����һ���о�Ҫ�������ļ�*/

	ret_read_conf = read_conf_value("portnum", value_port,"/root/config.ini");
	 if(ret_read_conf)
	 	{
		//printf("read config.ini error \n");
		send_fail_fop = openfile("/root/send_failed.log","a+");
		writeFile(send_fail_fop, "read config.ini error", 6);
		closeFile(send_fail_fop);
		
	 }
	ret_read_conf = read_conf_value("ipadd", value_ipadd,"/root/config.ini");
	 if(ret_read_conf)
	 	{
		send_fail_fop = openfile("/root/send_failed.log","a+");
		writeFile(send_fail_fop, "read config.ini error", 6);
		closeFile(send_fail_fop);	
	 }
	portnumber = atoi(value_port);
	printf("portnum = %d \n",portnumber);
	printf("ipadd = %s \n",value_ipadd);
ret_read_conf = read_conf_value("device_id", value_device_id,"/root/config.ini");
	 if(ret_read_conf)
	 {
		send_fail_fop = openfile("/root/send_failed.log","a+");
		writeFile(send_fail_fop, "read config.ini error device_id", 6);
		closeFile(send_fail_fop);	
	 }
	printf("device_id is = %s ",value_device_id);
	printf("device_id len is = %d \n",strlen(value_device_id));
	real_device_id = convert_atohex(value_device_id);
	printf("real_device_id len is = %x \n",real_device_id);

	ret_read_conf = read_conf_value("heart_beat", heart_beat,"/root/config.ini");
	 if(ret_read_conf)
	 	{
		send_fail_fop = openfile("/root/send_failed.log","a+");
		writeFile(send_fail_fop, "read config.ini error heart_beat", 6);
		closeFile(send_fail_fop);	
	 }
	heart_beat_int = convert_atohex(heart_beat);
	printf("heart_beat_int = %02x \n",heart_beat_int);
		/*DEBUG*/
	ret_read_conf = read_conf_value("DEBUG", debug_buf,"/root/config.ini");
	 if(ret_read_conf)
	 	{
		send_fail_fop = openfile("/root/send_failed.log","a+");
		writeFile(send_fail_fop, "read config.ini error", 6);
		closeFile(send_fail_fop);	
	 }
	DEBUG= atoi(debug_buf);
	printf("DEBUG = %d \n",DEBUG);


	/*locate_flag*/
	ret_read_conf = read_conf_value("locate_flag", locate_flag_buf,"/root/config.ini");
	 if(ret_read_conf)
	 	{
		send_fail_fop = openfile("/root/send_failed.log","a+");
		writeFile(send_fail_fop, "read config.ini error", 6);
		closeFile(send_fail_fop);	
	 }
	locate_flag= atoi(locate_flag_buf);
	printf("locate_flag = %d \n",locate_flag);


	/*station*/
	ret_read_conf = read_conf_value("station_flag", station_buff,"/root/config.ini");
	 if(ret_read_conf)
	 {
		send_fail_fop = openfile("/root/send_failed.log","a+");
		writeFile(send_fail_fop, "read config.ini error", 6);
		closeFile(send_fail_fop);	
	 }
	station_flag= atoi(station_buff);
	printf("**********station_flag = %d \n",station_flag);
/*	switch(station_flag)
	 {
		 case 0x02://ʯ��������
		 printf("**********station_flag = 0x02\n");
			 	 
				 break;
			
		 case 0x03://ʯ��������
		 printf("**********station_flag = 0x03\n");
				 
				 break;  
				
		 case 0x04://�ڶ�����
		 printf("**********station_flag = 0x04\n");
			 
				 break;
		 case 0x05://�ڶ�����
		 printf("**********station_flag = 0x05\n");
			 	 
				 break;
		 case 0x06://ʦ���ߴ�����
			 	 
				 break;
		 case 0x07://ʦ���ߴ�����
				 
				 break;
		 case 0x08://�����³�����
			 	 
				 break;
		 case 0x09://�����³�����
		 printf("**********station_flag = 0x09\n");
			 	 
				 break;
		 case 0x0A://��������
		 printf("**********station_flag = 0x0A\n");
				 
				 break;
		 case 0x0B://��������
		 printf("**********station_flag = 0x0B\n");
			 	 
				 break;
		 case 0x0C://ѧԺ����
		 printf("**********station_flag = 0x0C\n");
			 
				 break;
		 case 0x0D://ѧԺ����
		 printf("**********station_flag = 0x0D\n");
			 	 
				 break;
		 case 0x0E://���´�����
		 printf("**********station_flag = 0x0E\n");
				 
				 break;
		 case 0x0F://���´�����
		 printf("**********station_flag = 0x0F\n");
				 
				 break;
		 case 0x10://�Ķ�����
		 printf("**********station_flag = 0x10\n");
				 
				 break;
		 case 0x11://�Ķ�����
		 printf("**********station_flag = 0x11\n");
				 
				 break;
		 case 0x12://������������
		 printf("**********station_flag = 0x12\n");
			 
				 break;
		 case 0x13://������������
		 printf("**********station_flag = 0x13\n");
				 
				 break;
		 case 0x14://��������
		 printf("**********station_flag = 0x14\n");
				 
				 break;
		 case 0x15://��������
		 printf("**********station_flag = 0x15\n");
			 
				 break;
		 case 0x16://����������
		 printf("**********station_flag = 0x16\n");
			 
				 break;
		 case 0x17://����������
		 printf("**********station_flag = 0x17\n");
			 
				 break;
		 case 0x18://�ƴ�����
		 printf("**********station_flag = 0x18\n");
			
				 break;
		 case 0x19://�ƴ�����
		 printf("**********station_flag = 0x19\n");
			 
				 break;
		 case 0x1A://��������
		 printf("**********station_flag = 0x1A\n");
			  
				 break;
		 case 0x1B://��������
		 printf("**********station_flag = 0x1B\n");
			 
				 break;
		default:
				break;
	 }
*/

	

	switch(locate_flag)
	{
		case 0:
			printf("��ƽ�ܶ�λ����\n");
			break;
		case 1:
			printf("��������λ����\n");
			break;
		case 2:
			printf("BRT��λ����\n");
			break;
		default:
				printf("��;վ��λ����\n");
				break;
	}

int send_pos_serial_num = 0;
int report_station_serial_num = 0;
	/*λ����Ϣ*/
	pos_send_to_service[0] =*((unsigned char *)&real_device_id+2);//send_id
	pos_send_to_service[1] =*((unsigned char *)&real_device_id+1);
	pos_send_to_service[2] =*((unsigned char *)&real_device_id);

	pos_send_to_service[3] = 0x80;
	pos_send_to_service[4]  = 0x00;
	pos_send_to_service[5] = 0x00;

	pos_send_to_service[6]  = (send_pos_serial_num>>24)|(1<<7);
	pos_send_to_service[7]  = send_pos_serial_num>>16;
	pos_send_to_service[8] =  send_pos_serial_num>>8;
	pos_send_to_service[9] =  send_pos_serial_num;

	pos_send_to_service[10] = 0x00 ;
	pos_send_to_service[11] = 0x23;
//	pos_send_to_service[11] = 43;
	pos_send_to_service[12] = 0x00 ;//check
	pos_send_to_service[13] = 0x00 ;//check
	pos_send_to_service[14] = 0xc3 ;
//	pos_send_to_service[14] = 0xD1 ;

	pos_send_to_service[15] = 0x01 ;//hui ju

	pos_send_to_service[16] = 0x00 ; //obu hou san
	pos_send_to_service[17] =  0x00;//obu hou san
	pos_send_to_service[18] =  0x00;//obu hou san
	//pos_send_to_service[19] =*((unsigned char *)&real_device_id+2);//weizhi
	//pos_send_to_service[20] =*((unsigned char *)&real_device_id+1);//weizhi
	//pos_send_to_service[21] =*((unsigned char *)&real_device_id);	//weizhi

	pos_send_to_service[19] =0x10;//weizhi
	pos_send_to_service[20] =0x90;//weizhi
	pos_send_to_service[21] =0x22;//weizhi


	
	pos_send_to_service[22] =  0x00;
	pos_send_to_service[23] = 0x00 ;
	pos_send_to_service[24] =  0x00;
	pos_send_to_service[25] =  0x00;
	pos_send_to_service[26] = 0x00 ;
	pos_send_to_service[27] =  0x00;
	pos_send_to_service[28] =  0x00;

	pos_send_to_service[29] = 0x00 ;
	pos_send_to_service[30] =  0x00;
	pos_send_to_service[31] =  0x00;
	pos_send_to_service[32] = 0x00 ;
	pos_send_to_service[33] =  0x00;
	pos_send_to_service[34] =  0x00;



/*��վ��Ϣ*/
	report_station_to_service[0] =*((unsigned char *)&real_device_id+2);//send_id
	report_station_to_service[1] =*((unsigned char *)&real_device_id+1);
	report_station_to_service[2] =*((unsigned char *)&real_device_id);

	report_station_to_service[3] = 0x80;
	report_station_to_service[4]  = 0x00;
	report_station_to_service[5] = 0x00;

	report_station_to_service[6]  = (report_station_serial_num>>24)|(1<<7);
	report_station_to_service[7]  = report_station_serial_num>>16;
	report_station_to_service[8] =  report_station_serial_num>>8;
	report_station_to_service[9] =  report_station_serial_num;

	report_station_to_service[10] = 0x00 ;
//	report_station_to_service[11] = 0x23;
	report_station_to_service[11] = 43;
	report_station_to_service[12] = 0x00 ;//check
	report_station_to_service[13] = 0x00 ;//check
//	report_station_to_service[14] = 0xc3 ;
	report_station_to_service[14] = 0xD1 ;

	report_station_to_service[15] = 0x01 ;//hui ju

	report_station_to_service[16] = 0x00 ; //obu hou san
	report_station_to_service[17] =  0x00;//obu hou san
	report_station_to_service[18] =  0x00;//obu hou san
	report_station_to_service[19] =*((unsigned char *)&real_device_id+2);//weizhi
	report_station_to_service[20] =*((unsigned char *)&real_device_id+1);//weizhi
	report_station_to_service[21] =*((unsigned char *)&real_device_id);	//weizhi
	


/*test*/
	chuzhan_notice_send_to_uart[0] = 0x7e;
	chuzhan_notice_send_to_uart[1] = 12;
	chuzhan_notice_send_to_uart[2] = 0x21;
	chuzhan_notice_send_to_uart[3] = 0x00;
	chuzhan_notice_send_to_uart[4] = 0x03;
	chuzhan_notice_send_to_uart[5] = 0x61;
	chuzhan_notice_send_to_uart[6] = 0x03;
	chuzhan_notice_send_to_uart[7] = 0x01;
	chuzhan_notice_send_to_uart[8] = 0x09;
	chuzhan_notice_send_to_uart[9] = 0x00;
	chuzhan_notice_send_to_uart[10] = 0x30;
	chuzhan_notice_send_to_uart[11] = 0x00;/*check*/


	
	if((host=gethostbyname(value_ipadd))==NULL)  /*�����ļ���ipҪ����д*/
	{ 
		fprintf(stderr,"Gethostname error\n"); 
		exit(1); 
	} 

	if((sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)/*tcp��udp*/ 
//	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)   /*SOCK_STREAM  tcp*/
	{ 
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno)); 
		exit(1); 
	}  
	
	bzero(&server_addr,sizeof(server_addr)); 
	server_addr.sin_family=AF_INET;          // IPV4
	server_addr.sin_port=htons(portnumber); 
	server_addr.sin_addr=*((struct in_addr *)host->h_addr); 



	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
			fprintf(stderr,"Connect Error:%s\a\n",strerror(errno)); 
			exit(1); 
	} 



	uart_fd = open_uart();
	init_uart();
	printf("init is OK\r\n");
	if(InitQueue(MyQueue))//MyQueue-> SqQueue ;
		{
		printf("InitQueue error \n");
	}
	if(InitQueue_locate(MyQueue_locate))// MyQueue_locate->SqQueue_locate
		{
		printf("InitQueue_locate error \n");
	}

	
	serial_num ++;
	memcpy(version_info,"20141217V0.1",20);
	/*�汾��Ϣ*/
	buff[0]=*((unsigned char *)&real_device_id+2);//send_id
	buff[1]=*((unsigned char *)&real_device_id+1);
	buff[2]=*((unsigned char *)&real_device_id);
	buff[3]=0x80;//dest_id
	buff[4]=0x00;
	buff[5]=0x00;
	buff[6]=(*((unsigned char *)&serial_num+3) | (1<<7));//serial_num  SET 1
	buff[7]=*((unsigned char *)&serial_num+2);
	buff[8]=*((unsigned char *)&serial_num+1);
	buff[9]=*((unsigned char *)&serial_num);
	buff[10]=0x00;//len
	buff[11]=0x23;
	buff[12]=0x00;//crc
	buff[13]=0x00;//crc
	buff[14] = 0xD0;//attribute
	
	//memcpy(buff+15,version_info,strlen(version_info));//version_info
	memcpy(buff+15,version_info,20);//version_info
	crc_version= checkcrc16(buff,35);
	buff[12] = *((unsigned char *)&crc_version+1);
	buff[13] = *((unsigned char *)&crc_version);
	
	memcpy(buff_time,buff,10);
	/*ʱ������֡*/
	buff_time[10] = 0x00;
	buff_time[11] = 0x15;
	buff_time[12] = 0x00;//crc
	buff_time[13] = 0x00;//crc
	buff_time[14] = 0xD7;//attribute
	buff_time[15]  = 0x00;//·��ڵ�ʱ��
	buff_time[16]  = 0x00;
	buff_time[17]  = 0x00;
	buff_time[18]  = 0x00;
	buff_time[19]  = 0x00;
	buff_time[20]  = 0x00;
	crc_time= checkcrc16(buff_time,0x15);
	buff_time[12] = *((unsigned char *)&crc_time+1);
	buff_time[13] = *((unsigned char *)&crc_time);
	if((nbytes=write(sockfd,buff_time,0x15))==-1)
		{
		printf("write sockfd  send time is error \n ");

	}
	printf("send time success \n");
	printf("write time _nbytes  = %d \n",nbytes);

	/*����Ҫ��д�Ķ���д����־�ļ�����*/
	getTime(log_name, 3);
	sprintf(dir_log_name,"/root/%s.log",log_name);
	fop_log = openfile(dir_log_name,"a+");
	memset(convet_buff,0,100);
	 convet(buff_time,convet_buff,21);
	 writeFile(fop_log, convet_buff, 8);//send to service
/*	 	timeout.tv_sec = 180;
		timeout.tv_usec = 0;
		printf("��ط����������Ƿ������ȴ�ʱ��180�룬180������������ִ��reboot\n");
		if(select(sockfd+1,&read_fd,NULL,NULL,&timeout)==0){
			sprintf(command,"reboot");
			system(command);
		}
		printf("��ط����������ѷ���\n");*/
	if((nbytes=read(sockfd, recv_buff_time,31)) == -1)  //�����ص�ʱ����Ϣ����̨�ر�����������bug�����������Ǽ��
		{
			printf("read time  error \n");
	
	}

		int count = 0;
	printf("�������ֽ���%d \n",nbytes);
	for(count=0;count<nbytes;count++)
		{
	printf("recv_buff_time[%d] = %02x\n",count,recv_buff_time[count]);
	}


	printf("��д���\n");
	
	if(recv_buff_time[14] == 0x7E)
		{
		unsigned char sprintf_time_out[40];
	//	printf("23 = %02x \n",recv_buff_time[25]);//year
	//	printf("23 = %02x \n",recv_buff_time[26]);//month
	//	printf("23 = %02x \n",recv_buff_time[27]);//day
	//	printf("23 = %02x \n",recv_buff_time[28]);//hour
	//	printf("23 = %02x \n",recv_buff_time[29]);//minute
	//	printf("23 = %02x \n",recv_buff_time[30]);//second
		sprintf(sprintf_time_out,"date %02x%02x%02x%02x%02x%02x.%02x",recv_buff_time[26],recv_buff_time[27],recv_buff_time[28],recv_buff_time[29],0x20,recv_buff_time[25],recv_buff_time[30]);
		printf("sprintf_time_out = %s \n",sprintf_time_out);
		system(sprintf_time_out);
		/*�򴮿ڷ�ʱ��ŵ�����һ���߳�����*/
	}
	memset(convet_buff,0,100);
	convet(recv_buff_time,convet_buff,31);
	writeFile(fop_log, convet_buff, 2);
	closeFile(fop_log);

	/*ʱ��У׼֮�����´��ļ�������µ�һ�콫�ᴴ���µ��ĵ�*/
	getTime(log_name, 3);
	sprintf(dir_log_name,"/root/%s.log",log_name);
	fop_log = openfile(dir_log_name,"a+");
	
	/*����������ʱ��*/
	/*�����ڹ����İ汾��Ϣ*/
/*	len_version = nread(uart_fd,from_uart_version_info);
	if(len_version == -1)
		{
			printf("read from_uart_version_info is error \n");
		}
	if(from_uart_version_info[0]==0x1c)
		{
			memcpy(buff,from_uart_version_info+9,0x23);
		}*/
	/*���淢�Ͱ汾��Ϣ*/
	
	if((nbytes=write(sockfd,buff,0x23))==-1)
	{
	      printf("Write Error!\n");
	      exit(1);
       	}
	printf("write _nbytes  = %d \n",nbytes);

	memset(convet_buff,0,100);
	 convet(buff,convet_buff,0x23);
	writeFile(fop_log, convet_buff, 1);
//	printf("read zhiqian \n");
//	int val =fcntl(sockfd,F_GETFL,0);
//	fcntl(sockfd,F_SETFL,val | O_NONBLOCK);


/*2015 06 16ע�͵����������İ汾��Ϣ*/
//	if(-1 == (nbytes=read(sockfd, recv_buff_versioninfo,0x13)))  
// 	{
 // 		printf("read data fail !\r\n");  
    
//	} 

	
	
//	fcntl(sockfd,F_SETFL,val & (~O_NONBLOCK));
	printf("read _nbytes  = 0x%02x \n",nbytes);

	memset(convet_buff,0,100);
	convet(recv_buff_versioninfo,convet_buff,0x13);
	writeFile(fop_log, convet_buff, 2);	//recv from service	


	int pthread_Err = pthread_create(&save_msg_pthread,NULL,save_msg,NULL);
	if (pthread_Err != 0)
	{
	printf("Create thread Failed!\n");
	return EXIT_FAILURE;
	}
	pthread_Err = pthread_create(&send_msg_pthread,NULL,send_msg,NULL);
	if (pthread_Err != 0)
	{
	printf("Create thread Failed!\n");
	return EXIT_FAILURE;
	}
	pthread_Err = pthread_create(&rec_ask_pthread,NULL,rec_ask,NULL);
	if (pthread_Err != 0)
	{
	printf("Create thread Failed!\n");
	return EXIT_FAILURE;
	}
	pthread_Err = pthread_create(&info_from_service_and_send_to_uart_pthread,NULL,info_from_service_and_send_to_uart,NULL);
	if (pthread_Err != 0)
	{
	printf("Create thread Failed!\n");
	return EXIT_FAILURE;
	}
	//locate_process_pthread
	pthread_Err = pthread_create(&locate_process_pthread,NULL,locate_process,NULL);
	if (pthread_Err != 0)
	{
	printf("Create thread Failed!\n");
	return EXIT_FAILURE;
	}



	
	int err=pthread_join(save_msg_pthread,NULL);/*�����ȴ��߳��˳�*/
    	if(err!=0)  
  	{  
        	printf("can not join with thread1:%s\n",strerror(err));  
        	exit(1);  
   	}
	err=pthread_join(send_msg_pthread,NULL);
    	if(err!=0)  
  	{  
        	printf("can not join with thread2:%s\n",strerror(err));  
        	exit(1);  
   	}
	err=pthread_join(rec_ask_pthread,NULL);
    	if(err!=0)  
  	{  
        	printf("can not join with thread3:%s\n",strerror(err));  
        	exit(1);  
   	}

	err=pthread_join(locate_process_pthread,NULL);
    	if(err!=0)  
  	{  
        	printf("can not join with thread3:%s\n",strerror(err));  
        	exit(1);  
   	}
		
    	 err=pthread_join(info_from_service_and_send_to_uart_pthread,NULL);
    	if(err!=0)  
  	{  
        	printf("can not join with thread4:%s\n",strerror(err));  
       	exit(1);  
   	}



		
		close(sockfd);
		exit(0); 
} 



void *save_msg()
{
	unsigned char store_save[250];
	unsigned char rssi_from_uart[150];
	unsigned char rssi_convert[300];
	unsigned char rssi_print[200];
	unsigned char obu[30];
	unsigned char obu_convert[60];
	printf("enter save_msg********************************************************\n");
	unsigned char temp;
	jinchuzhan_beat= 0;
	pos_intval_t = 0;
	char rmlog2[30];
	char rmlog1[30];
	char rmlog3[30];
	char rmlogm[30];
	char out[40];
	time_t timep;
	first = time(NULL);
	unsigned int crc_fix;
	int serial_num_luyou=0x00;/*��ˮ�ų�ʼ��*/
	while(1)
	{	
	//	printf("save ******\n");
	//	sleep(1);
	/*����Ӧ�ü��ļ���***********/
		if(difftime(time(NULL),first) >= 60){  /*�ᱣ��һ��Сʱ���������û��������Ҳ����Ƶ���Ĵ����µ��ļ�*/
			closeFile(fop_log);
			getTime(log_name, 3);
			sprintf(dir_log_name,"/root/%s.log",log_name);
			fop_log = openfile(dir_log_name,"a+");
			first = time(NULL);
			getTime(rmlog2, 4);
			getTime(rmlog1,5);
			getTime(rmlog3,6);
			getTime(rmlogm,7);
			sprintf(out,"rm /root/%s.log",rmlog2);
			system(out);
			sprintf(out,"rm /root/%s.log",rmlog1);
			system(out);
			sprintf(out,"rm /root/%s.log",rmlog3);
			system(out);
			sprintf(out,"rm /root/%s*.log",rmlogm);
			
		}
		
	//	printf("thread save msg is running*****\n");
		memset(store_save,0,250);
		memset(temp_buff,0,1024);
		data_length= nread(uart_fd,temp_buff);/*���ض��������ֽ�����������ݳ��� temp_buff����У��λ*/
		printf("data_length = %d \n",data_length);
		printf("temp_buff[0] = %02x \n",temp_buff[0]);
		for(k = 0;k<data_length;k++)
		{
				printf("%02x ",temp_buff[k]);
		}
		printf("\n");
		switch(temp_buff[0])   /*���������ݵĵ�һ���ֽ�*/
		{ 
			case 0x01://obu��Ϣ�����ϴ�
			memset(obu_convert,0,60);
			convet(temp_buff, obu_convert, 21);
			//writeFile(fop_log, obu_convert, 10);
				break;
			case 0x04://rssi��Ϣ
				//д����־����
				memset(rssi_from_uart,0,150);
				memset(rssi_convert,0,300);
				memset(rssi_print,0,200);
				rssi_from_uart[0] = 0x7e;
				rssi_from_uart[1] = from_uart_len;
				memcpy(rssi_from_uart+2,temp_buff,from_uart_len-2);/*����У��λ*/
				convet(rssi_from_uart, rssi_convert, from_uart_len);/*uart_convet����������ascii*/
				//writeFile(fop_log, rssi_convert, 9);
				printf("\n");
				memcpy(rssi_print,"OBU:",4);
				memcpy(rssi_print+4,rssi_convert+6,16);
				memcpy(rssi_print+20," rsu_id1:",9);
				memcpy(rssi_print+29,rssi_convert+22,16);
				memcpy(rssi_print+45,":",1);
				memcpy(rssi_print+46,rssi_convert+38,2);
				memcpy(rssi_print+48," rsu_id2:",9);
				memcpy(rssi_print+57,rssi_convert+40,16);
				memcpy(rssi_print+73,":",1);
				memcpy(rssi_print+74,rssi_convert+56,2);
				memcpy(rssi_print+76," rsu_id3:",9);
				memcpy(rssi_print+85,rssi_convert+58,16);
				memcpy(rssi_print+101,":",1);
				memcpy(rssi_print+102,rssi_convert+74,2);
				memcpy(rssi_print+104," rsu_id4:",9);
				memcpy(rssi_print+113,rssi_convert+76,16);
				memcpy(rssi_print+129,":",1);
				memcpy(rssi_print+130,rssi_convert+92,2);
				writeFile(fop_log, rssi_print, 9);
				
				break;
			case 0x14://Զ�̰�ο���Ӧ��֡
				memcpy(store_save,temp_buff+9,0x24);
				store_save[14] = 0xD4;
				ENQueue(MyQueue,store_save,0x24);
				break;
			case 0x16://Զ������Ӧ��֡
				memcpy(store_save,temp_buff+9,22);
				ENQueue(MyQueue,store_save,22);
				break;
			case 0x18://Զ�����ò�ѯ֡
				memcpy(store_save,temp_buff+9,41);
				ENQueue(MyQueue,store_save,41);
				break;
			case 0x19://Զ�̵���Ӧ��
				memcpy(store_save,temp_buff+9,0x14);
				store_save[14] = 0xD5;
				ENQueue(MyQueue,store_save,0x14);
				break;
			case 0x1A:  //��վ��Ϣ
				memcpy(store_save,temp_buff+9,39);
				store_save[14] = 0xD1;
				int wdx = 0;
				for(wdx=38;wdx>14;wdx--)//�ճ�4λ���·��id
					{
					store_save[wdx+4] = store_save[wdx];
				}
				store_save[15] = *((unsigned char *)&real_device_id+3);
				store_save[16] = *((unsigned char *)&real_device_id+2);
				store_save[17] = *((unsigned char *)&real_device_id+1);
				store_save[18] = *((unsigned char *)&real_device_id+0);
				ENQueue(MyQueue,store_save,43);
				int cys = 0;
				printf("cys \n");
			//	for(cys=0;cys<39;cys++)
			//		printf("store_save[%d] = %02x\n",cys,store_save[cys]);
				if(temp_buff[24]==0x00)
				{
					printf("��ջ\n");
					writeFile(fop_log, NULL, 18);
				}
				else
					{
						printf("jinzhan\n");
					writeFile(fop_log, NULL, 17);
				}
				
				break;
			case 0x1D://����can������Ϣ�ϱ�
				memcpy(store_save,temp_buff+9,0x2B);
				ENQueue(MyQueue,store_save,0x2B);
				break;

			case 0x1E:
				memcpy(store_save,temp_buff+9,35);//ͨ����Ϣ
				printf("ͨ����Ϣ\n");
				ENQueue(MyQueue,store_save,35);
				break;

			case 0xec:
				memcpy(store_save,temp_buff+6,3);
				store_save[3] = 0x80;
				store_save[4] = 0x00;
				store_save[5] = 0x00;
				store_save[6] = 0x00;/*4 ��ˮ��*/
				store_save[7] = 0x00;
				store_save[8] =0x00;
				store_save[9] = 0x00;
				store_save[10] = 0x00;
				store_save[11] = 41;
				store_save[12] = 0x00;/*check*/
				store_save[13] = 0x00;
				store_save[14] =0xD3;
				store_save[15] =0x00;/*serial*/
				store_save[16] =0x00;
				store_save[17] =0x00;
				store_save[18] =0x01;
				store_save[19] =0xff;
				store_save[20] =0xec;
				memcpy(store_save+21,temp_buff+3,20);
				ENQueue(MyQueue,store_save,41);

				break;
			case 0x20://·������ ��У��
				memcpy(store_save,temp_buff+6,3);
				//printf("store_save[0] = %02x ",store_save[0]);
				//printf("store_save[0] = %02x ",store_save[1]);
				//printf("store_save[0] = %02x ",store_save[2]);
				printf("\n");
				serial_num_luyou++;
				if(serial_num_luyou ==0x7FFFFFFF)
				serial_num_luyou = 1;
				store_save[3] = 0x80;
				store_save[4] = 0x00;
				store_save[5] = 0x00;
				store_save[6] = (*((unsigned char *)&serial_num_luyou+3) | (1<<7));//serial_num  SET 1;
				store_save[7] = *((unsigned char *)&serial_num_luyou+2);
				store_save[8] = *((unsigned char *)&serial_num_luyou+1);
				store_save[9] = *((unsigned char *)&serial_num_luyou);
				store_save[10] = 0x00;
				store_save[11] = 0x0f;
				store_save[12] = 0x00;
				store_save[13] = 0x00;
				store_save[14] =0xD6;
				crc_fix = checkcrc16(store_save,0x0f);
				store_save[12] = *((unsigned char *)&crc_fix+1);
				store_save[13] = *((unsigned char *)&crc_fix);
				ENQueue(MyQueue,store_save,15);
				
				break;
			default:
				break;
			
		}
		
	}
	
	
}
		
void *send_msg()
{
	printf("enter send_msg  ***********\n");

	char j;
	int buffer_send_rsu_len = 0;/*�����еĳ���*/
	//unsigned char buffer_time_request[100];
	unsigned char buffer_send_rsu[200]; /*����*/
	unsigned char error_buffer[200];
	unsigned char sended_info_convert[200];

	while(1)
	{
		
	//	printf("send *****\n");
		sleep(1);
		while(MyQueue->font !=MyQueue->rear)
		{
		
			memset(buffer_send_rsu,0,200);
			buffer_send_rsu_len = DeQueue(MyQueue,buffer_send_rsu,buffer_send_rsu_len);
			printf("buffer_send_rsu_len = %d \n",buffer_send_rsu_len);
			signal(SIGPIPE,SIG_IGN);
			if((nbytes=write(sockfd,buffer_send_rsu,buffer_send_rsu_len))==-1)
			{	
				for(j=0;j<2;j++)                 /*�����д*/
				{
					signal(SIGPIPE,SIG_IGN);
					if((nbytes=write(sockfd,buffer_send_rsu,buffer_send_rsu_len))!=-1)
						/**/
						break;
				}
				printf("send error!!  ");
				/*д����־*//*����д��һ���ļ�����ȽϺ�*/
				send_fail_fop = openfile("/root/send_failed.log","a+");
				convet(buffer_send_rsu, error_buffer, buffer_send_rsu_len);
				writeFile(send_fail_fop, error_buffer, 5);
				closeFile(send_fail_fop);
			}
			else
				{
				printf("buffer_send_rsu:  fasong de shuju **********\n");
				for(k = 0;k<buffer_send_rsu_len;k++)
					{
						printf(" %02x",buffer_send_rsu[k]);
					}
			printf("  send message send nbytes = %d \n ",nbytes);
			//write the sended message into log
		//	convet(buffer_send_rsu, sended_info_convert,strlen(buffer_send_rsu));//��������ǧ������strlen
		//	writeFile(fop_log, sended_info_convert, 1);
			}
			
	
			}
	}
	
}

void *rec_ask()
{
	printf("enter rec_ask  ***********\n");
	unsigned char RSU_heart_beat[50];
	unsigned char buffer_time_request[100];
	unsigned int RSU_serial_num = 0;
	unsigned int time_request_serial_num = 0;
	unsigned char convert_buffer[100];
	heartbeat_t = time(NULL);
	time_request_t = time(NULL);
	unsigned char heart_beat_convert[65];
	int flag = 0;
	int val;
	unsigned char biaozhi_arm = 0x10;/*arm heartbeat biaozhi'*/
	struct timeval tm;
	fd_set set;
	RSU_heart_beat[0] = *((unsigned char *)&real_device_id+2);
	RSU_heart_beat[1] = *((unsigned char *)&real_device_id+1);/*��������*/
	RSU_heart_beat[2] = *((unsigned char *)&real_device_id);
	RSU_heart_beat[3]=0x80;
	RSU_heart_beat[4]=0x00;
	RSU_heart_beat[5]=0x00;
	buffer_time_request[0] = *((unsigned char *)&real_device_id+2);
	buffer_time_request[1] = *((unsigned char *)&real_device_id+1);
	buffer_time_request[2] = *((unsigned char *)&real_device_id);
	buffer_time_request[3] = 0x80;
	buffer_time_request[4] = 0x00;
	buffer_time_request[5] = 0x00;
	while(1){
		//	printf("ask *****\n");
			sleep(1);
			if(difftime(time(NULL),time_request_t) >= 300)
			{
			time_request_t = time(NULL);
			time_request_serial_num ++;
			if(time_request_serial_num == 0x7FFFFFFF)
			time_request_serial_num = 0x01;
			buffer_time_request[6] =(*((unsigned char *)&RSU_serial_num+3) | (1<<7));
			buffer_time_request[7] =*((unsigned char *)&RSU_serial_num+2) ;
			buffer_time_request[8] =*((unsigned char *)&RSU_serial_num+1) ;
			buffer_time_request[9] =*((unsigned char *)&RSU_serial_num) ;
			buffer_time_request[10] = 0x00;
			buffer_time_request[11] = 0x15;
			buffer_time_request[12] = 0x00;//crc
			buffer_time_request[13] = 0x00;//crc
			buffer_time_request[14] = 0xD7;
			buffer_time_request[15] = 0x00;
			buffer_time_request[16] = 0x00;
			buffer_time_request[17] = 0x00;
			buffer_time_request[18] = 0x00;
			buffer_time_request[19] = 0x00;
			buffer_time_request[20] = 0x00;
			if((nbytes=write(sockfd,buffer_time_request,21)) !=21)//=!15 error occured
					{
						printf("write buffer_time_request is error \n");
					}
			printf("buffer_time_request nybtes = %d \n",nbytes);
		//	memset(convert_buffer,0,100);
		//	 convet(buffer_time_request,convert_buffer,21);
		//	 writeFile(fop_log, convert_buffer, 8);//send to service
			memset(recv_buff_time,0,100);
	/*	if(-1 == (nbytes=read(sockfd, recv_buff_time,31)))  //�����ص�ʱ����Ϣ
		{
			printf("read time  error \n");
		}
	
		if(recv_buff_time[14] == 0x7E)
			{
		unsigned char sprintf_time_out[40];
		sprintf(sprintf_time_out,"date %02x%02x%02x%02x%02x%02x.%02x",recv_buff_time[26],recv_buff_time[27],recv_buff_time[28],recv_buff_time[29],0x20,recv_buff_time[25],recv_buff_time[30]);
		printf("sprintf_time_out = %s \n",sprintf_time_out);
		system(sprintf_time_out);
	
		}*/
	//	memset(convert_buffer,0,100);
	//	convet(recv_buff_time,convert_buffer,31);
	//	writeFile(fop_log, convert_buffer, 2);
		//2015 06 16 ע�͵�
	//	closeFile(fop_log);
			 
			
			}
	
		
		if(difftime(time(NULL),heartbeat_t) >= heart_beat_int)   /*interval read from ini */
			{
				heartbeat_t  = time(NULL);
				RSU_serial_num ++;
				if(RSU_serial_num ==0x7FFFFFFF)
				RSU_serial_num = 0x01;
				RSU_heart_beat[3]=0x80;
				RSU_heart_beat[4]=0x00;
				RSU_heart_beat[5]=0x00;
				RSU_heart_beat[6]=(*((unsigned char *)&RSU_serial_num+3) | (1<<7));
				RSU_heart_beat[7]=*((unsigned char *)&RSU_serial_num+2);
				RSU_heart_beat[8]=*((unsigned char *)&RSU_serial_num+1);
				RSU_heart_beat[9]=*((unsigned char *)&RSU_serial_num);
				RSU_heart_beat[10]=0x00;
				RSU_heart_beat[11]=0x0F;//len
				RSU_heart_beat[12]=0x00;
				RSU_heart_beat[13]=0x00;
				RSU_heart_beat[14]=0xD6;

	
				//����SIGPIPE����쳣ѭ����д������
				signal(SIGPIPE,SIG_IGN);
	
			if((nbytes=send(sockfd,RSU_heart_beat,15,0))!=15)//=!15 error occured
				{
				 printf(" enter if \n");
				 printf("nyte = %d \n",nbytes);
				 printf("hahahahhahha   flag = %d \n",flag);
				//	for(;;){
					/*д����־*/
					printf("write  is wrong ********************************************\n");
					close(sockfd);
					sockfd=socket(AF_INET,SOCK_DGRAM,0) ;  /*SOCK_DGRAM  udp*/
	
					
					bzero(&server_addr,sizeof(server_addr)); 
					server_addr.sin_family=AF_INET;      
					server_addr.sin_port=htons(portnumber); 
					server_addr.sin_addr=*((struct in_addr *)host->h_addr); 
					
					val =fcntl(sockfd,F_GETFL,0);
					fcntl(sockfd,F_SETFL,val | O_NONBLOCK);
					int connect_flag;
			connect_flag = connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr));
			sleep(1);
				printf("lianjie fuzhang \n");
				FD_ZERO(&set);
				FD_SET(sockfd,&set);	
				tm.tv_sec = TIME_OUT_TIME;
				tm.tv_usec = 0;
				flag = select(sockfd+1,NULL,NULL,NULL,&tm);
				
				printf("flag is %d \n",flag);
					if(-1 == flag)
						{
							printf("select error \n");
						}
					if(0 == flag)
						{
							printf("time out \n");
						}
		
					sleep(3);
		
				fcntl(sockfd,F_SETFL,val & (~O_NONBLOCK));
				
			}
			else{
				//write the heart-beat into log
				printf("else \n");
				memset(heart_beat_convert,0,65);
				//ע�������� �Ժ�Ҫ����Ϊ�˿�rssi
			//	convet(RSU_heart_beat, heart_beat_convert, 15);
			//	writeFile(fop_log, heart_beat_convert, 7);
			}
		printf("nbytes = %d \n",nbytes);
		printf("sleep \n");
		
	}

}
}

void *info_from_service_and_send_to_uart()
{
	printf("info_from_service_and_send_to_uart********************************************************\n");

	fd_set read_fd;
	struct timeval tm;
	int data_len;//�������·������ݳ���
	int flag = 0;
	unsigned char sprintf_time_out[40];
	unsigned char buff_recv_service[200];
	unsigned char buff_send_uart[200];
	unsigned char buff_uart_ctl_A[100];//Զ�̰��ABC
	unsigned char buff_uart_ctl_B[100];
	unsigned char buff_uart_ctl_C[100];
	unsigned char buff_uart_ctl_A_convert[150];
	unsigned char buff_uart_ctl_B_convert[150];
	unsigned char buff_uart_ctl_C_convert[150];
	unsigned char tongxin_yingda[50];
	unsigned char obu_id_zhongduan[5] = {0x00,0x03,0x61,0x03,0x00};
	unsigned char obu_id_luyou[5] = {0x00,0x03,0x64,0x01,0x00};
	unsigned char obu_id_huiju[5] = {0x00,0x03,0x62,0x01,0x02};//��վ���

	unsigned char send_uart[50] = {0x7e};/* 0xe* */
	unsigned char zuwangchaxun[50];
	int nread;
	unsigned int serial_num_service = 1;
	
	while(1)
	{
		//printf("info_from_service_and_send_to_uart \n");

		if(departure_flag==1){
			if((nbytes = write(uart_fd,chuzhan_notice_send_to_uart,12)) == -1){
								printf("write error \n");}
			departure_flag = 0;
		      sleep(3);
			}
		
		FD_ZERO(&read_fd);
		FD_SET(sockfd,&read_fd);	
		tm.tv_sec = 3;
		tm.tv_usec = 0;
	//	timeout.tv_sec = 1200;
	//	timeout.tv_usec = 0;
	//	if(select(sockfd+1,&read_fd,NULL,NULL,&timeout)==0){
	//		sprintf(command,"killall -9 watchdogd");
		//	system(command);
		//}
		//printf("��ط����������Ƿ������ȴ�ʱ��20����\n");
		
		flag = select(sockfd+1,&read_fd,NULL,NULL,&tm);
		switch(flag)
		{
		case 0:
			printf("time out \n");
		//	sleep(2);
			break;
		case -1:
			printf("select error occoured \n");
			//return (char *)-1;
		default:     //select ���ػὫδ׼���õ����������
			printf("enter default \n");
			if(FD_ISSET(sockfd,&read_fd)){
				ioctl(sockfd,FIONREAD,&nread);//���Ի����������ж��ٸ��ֽڿ��Ա���ȡ��Ȼ����ֽ��������nread����
				if(nread==0){
					//sleep(2);
					break;
					}
				printf("nread service can be readed = %d \n",nread);
				memset(buff_recv_service,0,200);
				nread = read(sockfd,buff_recv_service,nread);
				buff_recv_service[nread] = 0;
				printf("read from service is :\n");
				for(k = 0;k<nread;k++){
				printf(" %02x",buff_recv_service[k]);
					}
				serial_num_service ++;
				if(serial_num_service == 0x7FFFFFFF)
					serial_num_service = 0x01;
				//��÷���������ˮ���ж��Ƿ�ҪӦ��
				if((buff_recv_service[6]>>7)&(1))//��ҪӦ��
					{
					serial_num_service ++;
					if(serial_num_service == 0x7FFFFFFF)
					serial_num_service = 0x01;
					tongxin_yingda[0] = 0x70;//��ʼ�豸id
					tongxin_yingda[1] = 0x00;
					tongxin_yingda[2] = 0x01;
					tongxin_yingda[3] = 0x80;
					tongxin_yingda[4] = 0x00;
					tongxin_yingda[5] = 0x00;
					tongxin_yingda[6] =*((unsigned char *)&serial_num_service+3);//serial num
					tongxin_yingda[7] =*((unsigned char *)&serial_num_service+2);
					tongxin_yingda[8] = *((unsigned char *)&serial_num_service+1);
					tongxin_yingda[9] = *((unsigned char *)&serial_num_service);
					tongxin_yingda[10] = 0x00;
					tongxin_yingda[11] = 0x13;
					tongxin_yingda[12] = 0x00;//crc
					tongxin_yingda[13] = 0x00;
					tongxin_yingda[14] = 0x7F;
					memcpy(tongxin_yingda+15,buff_recv_service+15,4);
					if((nbytes = write(sockfd,tongxin_yingda,0x13)) == -1){
								printf("write error \n");}

				}

				
				switch(buff_recv_service[14]){
					case 0xc0://Զ������֡
					data_len = nread;
					memset(buff_send_uart,0,200);
					buff_send_uart[0] = 0x7E;
					buff_send_uart[1] = data_len+12;//�������ڵĳ���
					buff_send_uart[2] = 0x15;
					printf("from service zheng is %02x \n",buff_recv_service[3]);
					printf("from service is %02x \n",buff_recv_service[3]>>4);
					printf("buff_recv_service[20] = %02x \n",buff_recv_service[20]);
					switch(buff_recv_service[20])
					{
						case 0xe1:
							memset(send_uart,0,50);
							send_uart[0] = 0x7e;
							  send_uart[1] = 14;
							   send_uart[2] = 0xe1;
						
							   memcpy(send_uart+3,buff_recv_service+21,10);
							   send_uart[13] = 0x00;/*check*/

							   	if((nbytes = write(uart_fd,send_uart,14)) == -1){
								printf("write error \n");}
								for(k = 0;k<14;k++)
									{
									printf("send_uart = %02x \n",send_uart[k]);
								}
							break;
						case 0xe2:
								memset(send_uart,0,50);
								send_uart[0] = 0x7e;
								  send_uart[1] = 13;
								   send_uart[2] = 0xe2;
								      memcpy(send_uart+3,buff_recv_service+21,9);
									    send_uart[12] = 0x00;/*check*/

									   	if((nbytes = write(uart_fd,send_uart,13)) == -1){
								printf("write error \n");}
								for(k = 0;k<13;k++)
									{
									printf("send_uart = %02x \n",send_uart[k]);
								}
										
									break;
						case 0xe3:

									memset(send_uart,0,50);
									send_uart[0] = 0x7e;
									  send_uart[1] = 13;
									   send_uart[2] = 0xe3;
									      memcpy(send_uart+3,buff_recv_service+21,9);
									    send_uart[12] = 0x00;/*check*/


										   	if((nbytes = write(uart_fd,send_uart,13)) == -1){
								printf("write error \n");}
								for(k = 0;k<13;k++)
									{
								printf("send_uart = %02x \n",send_uart[k]);
								}
							break;
						case 0xe4:

								memset(send_uart,0,50);
								send_uart[0] = 0x7e;
								  send_uart[1] = 14;
								   send_uart[2] = 0xe4;
								      memcpy(send_uart+3,buff_recv_service+21,10);
									    send_uart[13] = 0x00;/*check*/

										   	if((nbytes = write(uart_fd,send_uart,14)) == -1){
								printf("write error \n");}
								for(k = 0;k<14;k++)
									{
									printf("send_uart = %02x \n",send_uart[k]);
								}
							
							break;
						case 0xe5:

							memset(send_uart,0,50);
								send_uart[0] = 0x7e;
								  send_uart[1] = 12;
								   send_uart[2] = 0xe5;
								      memcpy(send_uart+3,buff_recv_service+21,8);
									    send_uart[11] = 0x00;/*check*/
										   	if((nbytes = write(uart_fd,send_uart,12)) == -1){
								printf("write error \n");}
								for(k = 0;k<12;k++)
									{
									printf("send_uart = %02x \n",send_uart[k]);
								}
							break;
		


					}


				if(buff_recv_service[20]==0x01){	
					switch(buff_recv_service[3]>>4)/*Ŀ���豸id*/
						{
							case 0x01://·�����豸2530  ʵ����;վ�ڵ�
								//buff_send_uart[1] = 0x1c;//����
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
				
								break;
							case 0x02://·�����豸2530  ʵ����վ�ڵ�
								printf("send to huiju **********\n");
								
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								for(k=0;k<data_len+12;k++){
									printf(" %02x",buff_send_uart[k]);
								}
								printf("****************\n");
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								printf("****************nbytes = %d \n",nbytes);
								break;
							case 0x03://·�����豸2530  ����վ��ڵ���Ȥ��
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x04://·�����豸2530 �����ڵ�
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x05://·��·���豸
								memcpy(buff_send_uart+3,obu_id_luyou,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x06://���������ն�
								memcpy(buff_send_uart+3,obu_id_zhongduan,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x07://·�����豸ARM
								printf("enter 0x07 *********\n");
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							default:
								break;
							
						}
					
					}
					break;
					case 0xc1://Զ�̲�ѯ֡
						data_len = nread;
						memset(buff_send_uart,0,200);
						buff_send_uart[0] = 0x7E;
						buff_send_uart[1] = data_len+12;
						buff_send_uart[2] = 0x17;
					//	printf("from service chaxun is %02x \n",buff_recv_service[3]);
						printf("\n from service  chaxun is %02x \n",buff_recv_service[3]>>4);

					if(buff_recv_service[20]==0xec){	   /*����״̬��ѯ*/
						//	zuwangchaxun
								memset(zuwangchaxun,0,50);
						zuwangchaxun[0] = 0x7e;
						zuwangchaxun[1] = 12;
						zuwangchaxun[2] = 0xec;
						zuwangchaxun[3] = 0xFF;
						zuwangchaxun[4] = 0xFF;
						zuwangchaxun[5] = 0xFF;
						zuwangchaxun[6] = 0xFF;
						zuwangchaxun[7] = 0xFF;
						zuwangchaxun[8] = 0xFF;
						zuwangchaxun[9] = 0xFF;
						zuwangchaxun[10] = 0xFF;
						zuwangchaxun[11] = 0x00;
							if((nbytes = write(uart_fd,zuwangchaxun,12)) == -1){
								printf("write error \n");}
								for(k = 0;k<12;k++)
									{
									printf("������ѯ [%d]= %02x \n",k,zuwangchaxun[k]);
								}
								
						}



					if(buff_recv_service[20]==0x01){	
						switch(buff_recv_service[3]>>4){
							case 0x01://·�����豸2530  ʵ����;վ�ڵ�
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								for(k = 0;k<data_len+11;k++)
									{
									printf("buff_send_uart = %02x \n",buff_send_uart[k]);
								}
								break;
							case 0x02://·�����豸2530  ʵ����վ�ڵ�
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
							
								break;
							case 0x03://·�����豸2530  ����վ��ڵ���Ȥ��
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x04://·�����豸2530 �����ڵ�
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x05://·��·���豸
								memcpy(buff_send_uart+3,obu_id_luyou,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x06://���������ն�
								memcpy(buff_send_uart+3,obu_id_zhongduan,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								printf("send to uart \n");
								for(k=0;k<data_len+12;k++)
									{
									printf("%02x ",buff_send_uart[k]);
								}
								printf("\n");
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							case 0x07://·�����豸ARM
								printf("enter 0x07 *********\n");
								memcpy(buff_send_uart+3,obu_id_huiju,5);
								memcpy(buff_send_uart+8,buff_recv_service+3,3);
								memcpy(buff_send_uart+11,buff_recv_service,data_len);
								buff_send_uart[data_len+11] = 0x00;//У��
								for(k = 0;k<data_len+11;k++)
									{
									printf("buff_send_uart = %02x \n",buff_send_uart[k]);
								}
								if((nbytes = write(uart_fd,buff_send_uart,data_len+12)) == -1){
								printf("write error \n");}
								break;
							default:
								break;
							}

					}
						break;
					case 0xc2://Զ�̰�ο���
						data_len = nread;
						memset(buff_uart_ctl_A,0,100);
						memset(buff_uart_ctl_B,0,100);
						memset(buff_uart_ctl_C,0,100);
						buff_uart_ctl_A[0] = 0x7E;
						buff_uart_ctl_B[0] = 0x7E;
						buff_uart_ctl_C[0] = 0x7E;
						buff_uart_ctl_A[1] = 0x38;//��У��λ��0x38
						buff_uart_ctl_B[1] = 0x2c;
						buff_uart_ctl_C[1] = 0x2c;
						buff_uart_ctl_A[2] = 0x11;
						buff_uart_ctl_B[2] = 0x12;
						buff_uart_ctl_C[2] = 0x13;
						if((buff_recv_service[3]>>4)==0x06){
							printf("enter 0x06  ctl zhongduan ***********\n");
							memcpy(buff_uart_ctl_A+3,obu_id_zhongduan,5);
							memcpy(buff_uart_ctl_A+8,buff_recv_service+3,3);
							memcpy(buff_uart_ctl_A+11,buff_recv_service,0x2c);
							memcpy(buff_uart_ctl_B+3,obu_id_zhongduan,5);
							memcpy(buff_uart_ctl_B+8,buff_recv_service+3,3);
							memcpy(buff_uart_ctl_B+11,buff_recv_service+44,0x20);
							memcpy(buff_uart_ctl_C+3,obu_id_zhongduan,5);
							memcpy(buff_uart_ctl_C+8,buff_recv_service+3,3);
							memcpy(buff_uart_ctl_C+11,buff_recv_service+44+32,0x20);
							//У��
							buff_uart_ctl_A[0x37] = 0x00;
							buff_uart_ctl_B[0x2B] = 0x00;
							buff_uart_ctl_C[0x2B] = 0x00;
							buff_uart_ctl_A[25] = 0xE1;
							printf("begin write to uart ********************\n");
							if((nbytes = write(uart_fd,buff_uart_ctl_A,0x38)) == -1){
								printf("write error \n");}
								sleep(1);
							if((nbytes = write(uart_fd,buff_uart_ctl_B,0x2c)) == -1){
								printf("write error \n");}
								sleep(1);
							if((nbytes = write(uart_fd,buff_uart_ctl_C,0x2c)) == -1){
								printf("write error \n");}
								sleep(1);
							/*��ӡ����־*/
								memset(buff_uart_ctl_A_convert,0,150);
								 convet(buff_uart_ctl_A,buff_uart_ctl_A_convert,0x38);
								 writeFile(fop_log, buff_uart_ctl_A_convert, 11);
									
								 memset(buff_uart_ctl_B_convert,0,150);
								 convet(buff_uart_ctl_B,buff_uart_ctl_B_convert,0x2c);
								 writeFile(fop_log, buff_uart_ctl_B_convert, 12);
										
								 memset(buff_uart_ctl_C_convert,0,150);
								 convet(buff_uart_ctl_C,buff_uart_ctl_C_convert,0x2c);
								 writeFile(fop_log, buff_uart_ctl_C_convert, 13);
									
								
							}
						break;
					case 0x7E://ʱ��ظ�֡
					//	unsigned char sprintf_time_out[40];
						sprintf(sprintf_time_out,"date %02x%02x%02x%02x%02x%02x.%02x",buff_recv_service[26],buff_recv_service[27],buff_recv_service[28],buff_recv_service[29],0x20,buff_recv_service[25],buff_recv_service[30]);
						printf("sprintf_time_out = %s \n",sprintf_time_out);
						system(sprintf_time_out);
						break;
					default:
						break;
					
					}

				
				
				}//end if FD_ISSET
			
		}


	}


}




void *locate_process()
{
while(1){
	/*������������*/
	/*test*/
	//	ENQueue(MyQueue, chuzhan_notice_send_to_uart, 12);

	//	if((nbytes = write(uart_fd,chuzhan_notice_send_to_uart,12)) == -1){
	//							printf("write error \n");}
	//	sleep(3);

	
switch(locate_flag)
{
case 0:/*��ƽ��*/
while(QueueLength_locate(*MyQueue_locate)<=colldata_maxsize)
{
	printf("�ռ�����%d ��\n",QueueLength_locate(*MyQueue_locate));
	if(QueueLength_locate(*MyQueue_locate)==1||QueueLength_locate(*MyQueue_locate)==2)
		{
			time_intval = time(NULL);
	}
//	printf("time(NULL)-time_intval = %d \n",time(NULL)-time_intval);
	if(QueueLength_locate(*MyQueue_locate)>1&&time(NULL)-time_intval>=60)
	{
		time_intval = time(NULL);
		printf("��ʱ��ʱ�䵽\n");
		printf("����������%d ������\n",QueueLength_locate(*MyQueue_locate));
		break;
	}
	if(QueueLength_locate(*MyQueue_locate)==0)
		sleep(3);
}

if(QueueLength_locate(*MyQueue_locate)>=colldata_maxsize)
{
	printf("�ռ���30��\n");

	
}
/*�ռ�30�����ݴ���*/


printf("MyQueue_locate->font = %d \n",MyQueue_locate->font);
printf("MyQueue_locate->rear = %d \n",MyQueue_locate->rear);

printf("MyQueue_locate->rear-MyQueue_locate->font= %d \n",MyQueue_locate->rear-MyQueue_locate->font);
/*Ҫ������*/

int tempnum = 0;
//DeQueue_locate(MyQueue_locate, PTR, len_out_queue);


		while(QueueLength_locate(*MyQueue_locate))
		{
		
		DeQueue_locate(MyQueue_locate, PTR, len_out_queue);
		  tempnum = PTR[16];
		 colldata[colldata_size][tempnum] = PTR[17];
		 tempnum = PTR[25];
		 colldata[colldata_size][tempnum] = PTR[26];
		 tempnum = PTR[34];
		 colldata[colldata_size][tempnum] = PTR[35];
		 tempnum  = PTR[43];
		 colldata[colldata_size][tempnum] = PTR[44];
			colldata_size++; 
		}
	printf("���մ������������%d \n",colldata_size);
			int rssi1_ave = 0;
			int rssi2_ave = 0;
			int rssi3_ave = 0;
			int rssi4_ave = 0;
			int sum1 =0;
			int sum2 =0;
			int sum3 =0;
			int sum4 =0;
			int cut_temp;
			for(cut_temp=0;cut_temp<colldata_size;cut_temp++)
			{
				sum1 += colldata[cut_temp][1];
				sum2 += colldata[cut_temp][2];
				sum3 += colldata[cut_temp][3];
				sum4 += colldata[cut_temp][4];
				if(colldata[cut_temp][1]!=0)
					rssi_one_num++;
				if(colldata[cut_temp][2]!=0)
					rssi_two_num++;
				if(colldata[cut_temp][3]!=0)
					rssi_three_num++;
				if(colldata[cut_temp][4]!=0)
					rssi_four_num++;	
			}

			if(rssi_one_num!=0){
			rssi1_ave =sum1/rssi_one_num;
				}
			if(rssi_two_num!=0){
			rssi2_ave =sum2/rssi_two_num;
				}
			if(rssi_three_num!=0){
			rssi3_ave =sum3/rssi_three_num;
				}
			if(rssi_four_num!=0){
			rssi4_ave =sum4/rssi_four_num;
				}
			printf("rssi_one_num = %02x\n",rssi_one_num);
			printf("rssi_two_num = %02x\n",rssi_two_num);
			printf("rssi_three_num = %02x\n",rssi_three_num);
			printf("rssi_four_num = %02x\n",rssi_four_num);


			printf("rssi1_ave = %02x\n",rssi1_ave);
			printf("rssi2_ave = %02x\n",rssi2_ave);
			printf("rssi3_ave = %02x\n",rssi3_ave);
			printf("rssi4_ave = %02x\n",rssi4_ave);
			
			if(rssi_one_num<colldata_maxsize/2)
				{
				printf("���ݲ��� ����\n");
				rssi1_ave = 0;
				}
			if(rssi_two_num<colldata_maxsize/2){
				printf("���ݲ��� ����\n");
				rssi2_ave = 0;}
			if(rssi_three_num<colldata_maxsize/2)	{
				printf("���ݲ��� ����\n");
				rssi3_ave = 0;}
			if(rssi_four_num<colldata_maxsize/2)	{
				printf("���ݲ��� ����\n");
				rssi4_ave = 0;}
				


			int max = rssi1_ave;
			int max_index = 1;
			if(max<rssi2_ave){
				max = rssi2_ave;
				max_index = 2;}
			if(max<rssi3_ave){
				max = rssi3_ave;
				max_index = 3;}
			if(max<rssi4_ave){
				max=rssi4_ave;
				max_index =3;}

			printf("rssi���ֵ��%02x    \n",max);
			printf("Stay at position %d\n", max_index);
			pos_send_to_service[19] =0x10;
	pos_send_to_service[20] =0x90;
	if(max_index==1)
	pos_send_to_service[21] =0x11;
	if(max_index==2)
	pos_send_to_service[21] =0x12;
	if(max_index==3||max_index==4)
	pos_send_to_service[21] =0x13;
	ENQueue(MyQueue, pos_send_to_service, 35);
			


			
			colldata_size = 0;
			memset(colldata,0,300);
			rssi_four_num = 0;
			rssi_one_num = 0;
			rssi_three_num=0;
			rssi_two_num = 0;
			time_intval =0;
			SqQueue_locate_clear(MyQueue_locate);
		break;

	case 1:     /*��ɽ��*/
		
		

	/*�ռ�10�����ݴ���*/

while(QueueLength_locate(*MyQueue_locate)<=colldata_max_zsf)
{
	printf("�ռ�����%d ��\n",QueueLength_locate(*MyQueue_locate));
	if(QueueLength_locate(*MyQueue_locate)==1||QueueLength_locate(*MyQueue_locate)==2||QueueLength_locate(*MyQueue_locate)==3)
		{
			time_intval = time(NULL);
	}
//	if(time(NULL)-time_intval>=60)
if(QueueLength_locate(*MyQueue_locate)>1&&time(NULL)-time_intval>=60)
		{
		time_intval = time(NULL);
		printf("��ʱ��ʱ�䵽\n");
		printf("����������%d ������\n",QueueLength_locate(*MyQueue_locate));
	}
	if(QueueLength_locate(*MyQueue_locate)==0)
		sleep(3);
	else
		{
		sleep(1);
	}
	
}



if(QueueLength_locate(*MyQueue_locate)>=colldata_max_zsf)
{
	printf("�ռ���10��\n");
	
}

printf("MyQueue_locate->font = %d \n",MyQueue_locate->font);
printf("MyQueue_locate->font = %d \n",MyQueue_locate->rear);

printf("MyQueue_locate->rear-MyQueue_locate->font= %d \n",MyQueue_locate->rear-MyQueue_locate->font);
/*Ҫ������*/

 tempnum = 0;

	while(QueueLength_locate(*MyQueue_locate))
		{
		
		DeQueue_locate(MyQueue_locate, PTR, len_out_queue);
		  tempnum = PTR[16]&0x0f;
		 printf("tempnum = %d \n",tempnum);
		 colldata[colldata_size][tempnum] = PTR[17];
		 printf("colldata[%d][%d] = %02x\n",colldata_size,tempnum,colldata[colldata_size][tempnum]);
		 printf("ptr[17] = %02x \n",PTR[17]);
		 tempnum = PTR[25]&0x0f;
		 printf("tempnum = %d \n",tempnum);
		 colldata[colldata_size][tempnum] = PTR[26];
		  printf("colldata[%d][%d] = %02x\n",colldata_size,tempnum,colldata[colldata_size][tempnum]);
		  printf("ptr[26] = %02x \n",PTR[26]);
		 tempnum = PTR[34]&0x0f;
		 printf("tempnum = %d \n",tempnum);
		 colldata[colldata_size][tempnum] = PTR[35];
 		printf("colldata[%d][%d] = %02x\n",colldata_size,tempnum,colldata[colldata_size][tempnum]);
		  printf("ptr[35] = %02x \n",PTR[35]);
		 tempnum  = PTR[43]&0x0f;
		 printf("tempnum = %d \n",tempnum);
		 colldata[colldata_size][tempnum] = PTR[44];
		  printf("colldata[%d][%d] = %02x\n",colldata_size,tempnum,colldata[colldata_size][tempnum]);
		  printf("ptr[44] = %02x \n",PTR[44]);
			colldata_size++; 
		}

	

			printf("��������10��\n");
			int xxy = 0;
			for(xxy = 0;xxy<colldata_size;xxy++)
			{

			 printf("colldata[%d][1] = %02x\n",xxy,colldata[xxy][1]);
			 printf("colldata[%d][2] = %02x\n",xxy,colldata[xxy][2]);
			  printf("colldata[%d][3] = %02x\n",xxy,colldata[xxy][3]);
			   printf("colldata[%d][5] = %02x\n",xxy,colldata[xxy][5]);
			}
			 rssi2_ave = 0;
			 rssi3_ave = 0;
	

			 sum2 =0;
			 sum3 =0;
	
		 cut_temp = 0;

			for(cut_temp=0;cut_temp<colldata_size;cut_temp++)
			{
		
				sum2 += colldata[cut_temp][2];
				sum3 += colldata[cut_temp][3];
				if(colldata[cut_temp][2]!=0)
					rssi_two_num++;
				if(colldata[cut_temp][3]!=0)
					rssi_three_num++;
		
			}

		if(rssi_two_num)
			{
			rssi2_ave =sum2/rssi_two_num;
			}
		else{
			printf("û������\n");
			rssi2_ave = 0;
		}
		if(rssi_three_num){
			rssi3_ave =sum3/rssi_three_num;
			}
		else{
			printf("û������\n");
			rssi3_ave = 0;
		}

			printf("rssi_two_num = %02x\n",rssi_two_num);
			printf("rssi_three_num = %02x\n",rssi_three_num);
			printf("rssi2_ave = %02x\n",rssi2_ave);
			printf("rssi3_ave = %02x\n",rssi3_ave);

			
	
			if(rssi_two_num<colldata_max_zsf/2){
				printf("���ݲ��� ����\n");
				rssi2_ave = 0;}
			if(rssi_three_num<colldata_max_zsf/2)	{
				printf("���ݲ��� ����\n");
				rssi3_ave = 0;}
		

			//if(rssi2_ave<0xb2||rssi3_ave<0xb2){
				
			if(rssi2_ave<0xb2&&rssi3_ave<0xb2){//xxy


				if(posvalue<=2)
					{


	pos_send_to_service[19] =0x10;
	pos_send_to_service[20] =0x90;
	pos_send_to_service[21] =0x22;
	ENQueue(MyQueue, pos_send_to_service, 35);
	posvalue = 2;
				printf("��ͣ��POSITON:::222222\n");
					}
				else
					{
					posvalue = 3;
							pos_send_to_service[19] =0x10;
							pos_send_to_service[20] =0x90;
							pos_send_to_service[21] =0x23;
							ENQueue(MyQueue, pos_send_to_service, 35);
					printf("��ͣ��POSITON:::333333333    posvalue >3  ��̬��λ���\n");
				}


				}
			else{

							pos_send_to_service[19] =0x10;
							pos_send_to_service[20] =0x90;
							pos_send_to_service[21] =0x23;
							ENQueue(MyQueue, pos_send_to_service, 35);
							posvalue = 3;
				printf("��ͣ��POSITON:::333333333    ��̬��λ���\n");
			}



			colldata_size = 0;
			memset(colldata,0,300);
			rssi_four_num = 0;
			rssi_one_num = 0;
			rssi_three_num=0;
			rssi_two_num = 0;
			time_intval =0;
			SqQueue_locate_clear(MyQueue_locate);


		break;

	case 2:
		break;

	default:
			break;
	

		}
	


sleep(1);
}

}

ssize_t xread(int fd,void *ptr,size_t n)  /*�Ӵ��ڶ��̶����ȵ���Ϣ*/
{
	size_t nleft;
	ssize_t n_read;

//	printf("enter xread\n");
//	printf("xread num is %d \n",n);
	nleft=n;
	while(nleft>0)
	{
		printf("enter while(nleft >0)\n");
		if((n_read=read(fd,ptr,nleft))<0)
		{
			printf("x_read   limian =%02x \n",n_read);
			if(nleft==n){
				printf("xread is error \n");
				return(-1);}
			else
				break;
		}
		else if(n_read==0)
		{
			printf("xread n_read = = 0");
			break;		
		}
	//	printf("pass if \n");
	//	printf("nread in xread  = %d \n",n_read);
		//printf("nleft = %d \n",nleft);
		nleft-=n_read;
		ptr+=n_read;
	//	printf("nleft = %d \n",nleft);
	}
	return (n-nleft);/*����ѭ��nleft������*/
}


unsigned int checkcrc16( unsigned char *databuf, unsigned int datalen )
{
    unsigned int crcval;
    unsigned int i;

    crcval = 0;
    for ( i = 0; i < datalen; i++ )
    {
        crcval = CRCTABLE[(crcval ^= databuf[i]) & 0xFF] ^ (crcval >> 8);
    }

    return crcval;
}

/*��֤7e ��Ȼ�����������У��*/
ssize_t nread(int fd,unsigned char *ptr)
{
//	printf("enter nread\n");
	ssize_t len;/*�������ĳ���*/
	ssize_t datalen;
	ssize_t p;/*����һ��������Ŷ���������*/
	
	unsigned char data_from_uart[150];
	unsigned char uart_convet[300];

	unsigned char crc_8;
	unsigned char k;
	unsigned char t;


	/*sql*/
	char obu_id_sql[10];
	char route1_id[20];
	char route2_id[20];
	char route3_id[20];
	char route4_id[20];
	int route1_val = 0;
	int route2_val = 0;
	int route3_val = 0;
	int route4_val = 0;
	char time_sql[30]= {0};

//	char rssi_pair[9];
	p=0x0;
	while(p!=0x7e)
		{
	//	printf("while nei p = %02x \n",p);
		read(fd,&p,1);
	}
	read(fd,&p,1);
//	printf("read de zijie  p = %d \n",p);

	datalen = p -2;/*data_len*/
	from_uart_len = p;
	if((len = xread(fd,ptr,datalen))<0)  /*xread ����������� ��Ҫ���*/
		{
	printf("read error\n");
	}



	
	/*�޸�*/
	if(ptr[0] == 0x04){
		printf("����rssi ��Ϣ!\n");
		if(ptr[1]==0x01)/*��̬���ݴ������*/
		{
			ENQueue_locate(MyQueue_locate, ptr, len);
			printf("��̬̬���ݴ������ݿ� ����== %d \n",len);
		}


switch(locate_flag)
{
case 0:/*��ƽ��*/

/*�ж�obu  ����*/
if(ptr[1] ==0x00)  /*��̬*/
{
printf("��̬����\n");
colldata_size = 0;
  rssi_pair[0] = 0;/*����*/
  rssi_pair[1] = 0;
  rssi_pair[2] = 0;
 rssi_pair[3] = 0;
 rssi_pair[4] = 0;
 int tempnum = ptr[16];//��һ��·�ɱ�����һλ


pos_send_to_service[16] = ptr[6] ;/*obu��ź�3λ*/
pos_send_to_service[17] =  ptr[7];/*obu��ź�3λ*/
pos_send_to_service[18] =  ptr[8];/*obu��ź�3λ*/
 /*��ȡrssiֵ*/
 rssi_pair[tempnum] = ptr[17];//��һ��·�ɵ�rssi
 tempnum = ptr[25];//�ڶ���·�ɱ�����һλ
 rssi_pair[tempnum] = ptr[26];
 tempnum = ptr[34];
 rssi_pair[tempnum] = ptr[35];
 tempnum  = ptr[43];
 rssi_pair[tempnum] = ptr[44];
 
 int ll = 1;
 for(ll=1;ll<=4;ll++)
 	{
	printf("rssi_pair[%d]= %02x \n",ll,rssi_pair[ll]);
 }
 if ((rssi_pair[1] >= rssi_pair[2]) && (rssi_pair[1] >= rssi_pair[3]) && (rssi_pair[1] >= rssi_pair[4])&&(rssi_pair[1] >= 0xb4))
 	{
	 Ispos_two++;
	 Isleave = 0;

	Ispos_three = 0;
	 
 	}
 else if ((rssi_pair[2] >= rssi_pair[1]) && (rssi_pair[2] >= rssi_pair[3]) && (rssi_pair[2] >= rssi_pair[4])&&(rssi_pair[2] >= 0xb6))
 	{
	 Ispos_three++;
	 Isleave = 0;
		Ispos_two = 0;
		
 	}
 else if((rssi_pair[4]>=rssi_pair[3])&&(rssi_pair[4]>=rssi_pair[2])&&(rssi_pair[4]>=rssi_pair[1]))
 	{
 	Isleave++;

		Ispos_two = 0;
		Ispos_three = 0;
 	}
 else{
		Isleave = 0;
		Ispos_two = 0;
		Ispos_three = 0;
 }
if((Isleave==3)&&(OBU_NUM !=  ptr[8])&&(posvalue!=0)){/*��ֹ�ظ���ջ*/

	pos_send_to_service[19] =0x10;
	pos_send_to_service[20] =0x90;
	pos_send_to_service[21] =0x14;
	ENQueue(MyQueue, pos_send_to_service, 35);
	
	printf("bus ��ս***************��վ��ջ��ս��ƽ��************\n");
	writeFile(fop_log, NULL, 14);
	chuzhan_notice[0] = 0x7e;
	chuzhan_notice[1] = 12;
	chuzhan_notice[2] = 0x21;
	chuzhan_notice[3] = ptr[1];
	chuzhan_notice[4] = ptr[2];
	chuzhan_notice[5] = ptr[3];
	chuzhan_notice[6] = ptr[4];
	chuzhan_notice[7] = ptr[5];
	chuzhan_notice[8] = ptr[6];
	chuzhan_notice[9] = ptr[7];
	chuzhan_notice[10] = ptr[8];
	chuzhan_notice[11] = 0x00;/*check*/


	memset(chuzhan_notice_send_to_uart,0,15);
	memcpy(chuzhan_notice_send_to_uart,chuzhan_notice,12);
	departure_flag = 1;
	OBU_NUM =  ptr[8];/*��¼���һ�γ�վ��OBU*/
	posvalue = 0;

								

	
}
if (Ispos_two>=5)
{
	if (posvalue > 2)
	{
		printf("����λ����ת   ����ԭ����λ��\n");
	}
	else
	{
		printf("λ�ö���22222222222\n");
		writeFile(fop_log, NULL, 15);
		printf("����������Ͷ�λλ��22����Ϣ\n");
		pos_send_to_service[19] =0x10;
		pos_send_to_service[20] =0x90;
		pos_send_to_service[21] = 0x12;
		ENQueue(MyQueue, pos_send_to_service, 35);
		posvalue = 2;
		Isleave = 0;
		Ispos_two = 0;
		Ispos_three = 0;
	
	}

}
else if (Ispos_three >= 5)
{
	if (posvalue > 3)
	{
		printf("����λ����ת   ����ԭ����λ��\n");
	}
	else
	{
		printf("λ�ö���333333333333333\n");
		writeFile(fop_log, NULL, 16);
		printf("����������Ͷ�λλ��333333����Ϣ\n");
		pos_send_to_service[19] =0x10;
		pos_send_to_service[20] =0x90;
		pos_send_to_service[21] = 0x13;
		ENQueue(MyQueue, pos_send_to_service, 35);
		posvalue = 3;
		Isleave = 0;
		Ispos_two = 0;
		Ispos_three = 0;

	}
}




memset(rssi_pair,0,9);


}


		break;

	case 1:     /*��ɽ��*/

		printf("****posvalue = %02x \n",posvalue);
		printf("rssi****_pair[5]  = %02x \n",rssi_pair[5] );
		
		//if (rssi_pair[5] <= 0 && posvalue >= 1){
				if ( (posvalue >= 1)&&((ptr[16]&0x0f) != 0x05)&&((ptr[25]&0x0f) !=0x05)&&((ptr[43]&0x0f)!=0x05)){
			ENQueue_locate(MyQueue_locate, ptr, len);
			printf("�ռ�����\n");
			break;
		}
		/*�ж�obu  ����*/
		if(ptr[1] ==0x00)  /*��̬*/
		{
		printf("������rssi_pair[5] <= 0 && posvalue >= 1  ����\n");
		printf("posvalue = %02x \n",posvalue);
	//	printf("rssi_pair[5]  = %02x \n",rssi_pair[5] );
	printf("ptr[16]&0x0f = %02x \n",ptr[16]&0x0f);
	printf("ptr[25]&0x0f = %02x \n",ptr[25]&0x0f);
	printf("ptr[43]&0x0f = %02x \n",ptr[43]&0x0f);
		printf("��̬����\n");
			  rssi_pair[0] = 0;/*����*/
			  rssi_pair[1] = 0;
			  rssi_pair[2] = 0;
			 rssi_pair[3] = 0;
			 rssi_pair[4] = 0;/*����*/
			 rssi_pair[5] = 0;
		 int tempnum = ptr[16]&0x0f;
		
		 	/*�����±�1 2 3 5 */
		 rssi_pair[tempnum] = ptr[17];
		 tempnum = ptr[25]&0x0f;
		 rssi_pair[tempnum] = ptr[26];
		 tempnum = ptr[34]&0x0f;
		 rssi_pair[tempnum] = ptr[35];
		 tempnum  = ptr[43]&0x0f;
		 rssi_pair[tempnum] = ptr[44];
		 int lll = 1;

 pos_send_to_service[16] = ptr[6] ;
pos_send_to_service[17] =  ptr[7];
pos_send_to_service[18] =  ptr[8];
	 	for(lll=1;lll<=5;lll++)
	 	{
		printf("rssi_pair[%d]= %02x \n",lll,rssi_pair[lll]);
	 	}
		 if(rssi_pair[1]>0xb8)
		 	{
		 	if(posvalue<=1){
	writeFile(fop_log, NULL, 19);
	pos_send_to_service[19] =0x10;
	pos_send_to_service[20] =0x90;
	pos_send_to_service[21] =0x21;
	ENQueue(MyQueue, pos_send_to_service, 35);
				
		 	printf("���������\n");
			posvalue = 1;}
			if(posvalue==2)
				{
					printf("����λ�÷�ת  ����ԭ����λ�� ����Ҫ�ظ��ϴ�\n");
					writeFile(fop_log, NULL, 15);
					pos_send_to_service[19] =0x10;
					pos_send_to_service[20] =0x90;
					pos_send_to_service[21] =0x22;
					printf("�˿̳���POSTION:  222222 posvalue==2��̬��λ��� \n");
				}
			else if(posvalue==3)
				{
						printf("����λ�÷�ת  ����ԭ����λ�� ����Ҫ�ظ��ϴ�\n");
						writeFile(fop_log, NULL, 16);
						pos_send_to_service[19] =0x10;
						pos_send_to_service[20] =0x90;
						pos_send_to_service[21] =0x23;
						printf("�˿̳���POSTION: 33333333333    posvalue==3��̬��λ��� \n");
				}
		 	}
			

		 /*�������*/
			else if((rssi_pair[5]>0xBB)&&(OBU_NUM !=  ptr[8])&&(posvalue!=0))/*��ֹ�ظ���վ*/
				{
	writeFile(fop_log, NULL, 14);
	pos_send_to_service[19] =0x10;
	pos_send_to_service[20] =0x90;
	pos_send_to_service[21] =0x24;
	ENQueue(MyQueue, pos_send_to_service, 35);
				printf("����վ����վ����վ����վ����վ����վ!!!!!!!\n");
				printf("��2003���ͳ�վ��Ϣ\n");
				writeFile(fop_log, NULL, 14);
					chuzhan_notice[0] = 0x7e;
					chuzhan_notice[1] = 12;
					chuzhan_notice[2] = 0x21;
					chuzhan_notice[3] = ptr[1];
					chuzhan_notice[4] = ptr[2];
					chuzhan_notice[5] = ptr[3];
					chuzhan_notice[6] = ptr[4];
					chuzhan_notice[7] = ptr[5];
					chuzhan_notice[8] = ptr[6];
					chuzhan_notice[9] = ptr[7];
					chuzhan_notice[10] = ptr[8];
					chuzhan_notice[11] = 0x00;/*check*/

					memset(chuzhan_notice_send_to_uart,0,15);
					memcpy(chuzhan_notice_send_to_uart,chuzhan_notice,12);
					departure_flag = 1;
					OBU_NUM =  ptr[8];/*��¼���һ�γ�վ��OBU*/

				posvalue =0;
				
				}

		// else if(rssi_pair[1]<0xb4&&rssi_pair[1]>0xa0)
		 	 else if(rssi_pair[1]<0xb4)
		 	 	{
			if(posvalue<2){
				posvalue = 2;
		 	printf("�˿̳���POSTION:  2222222   ��̬��λ��� \n");
				}
			if(posvalue>2)
				{
				printf("�˿̳���POSTION: 33333333333     posvalue>2 ��̬��λ��� \n");
			}
		 	 	}

		else
			printf("Ŀǰ���޷���λ��׼ȷ���\n");
		 
		memset(rssi_pair,0,9);
		
			}


		

		break;

	case 2:/*BRT	 ��վ*/
		/*��վ�Ķ�λ*/
		/*�����������ó���λ���*/



// rssi_pair[0] = 0;/*����*/
  rssi_pair[1] = 0;/*clear*/
  rssi_pair[2] = 0;
  rssi_pair[3] = 0;
 //rssi_pair[4] = 0;
 int tempnum = ptr[16];


pos_send_to_service[16] = ptr[6] ;/*obu��ź�3λ*/
pos_send_to_service[17] =  ptr[7];/*obu��ź�3λ*/
pos_send_to_service[18] =  ptr[8];/*obu��ź�3λ*/

switch(station_flag)
{
	case 0x01:/*������������*/
		pos_send_to_service[20] =0x80;
		switch(ptr[16])
		{
		case 0x00:
			break;
		case 0x51:
			 rssi_pair[3] = ptr[17];
			break;
		case 0x54:
			 rssi_pair[2] = ptr[17];
			break;
		case 0x59:
			 rssi_pair[1] = ptr[17];
			break;
		default :
			break;

		}
		switch(ptr[25])
		{
		case 0x00:
			break;
		case 0x51:
			 rssi_pair[3] = ptr[26];
			break;
		case 0x54:
			 rssi_pair[2] = ptr[26];
			break;
		case 0x59:
			 rssi_pair[1] = ptr[26];
			break;
		default :
			break;

		}
		switch(ptr[34])
		{
		case 0x00:
			break;
		case 0x51:
			 rssi_pair[3] = ptr[35];
			break;
		case 0x54:
			 rssi_pair[2] = ptr[35];
			break;
		case 0x59:
			 rssi_pair[1] = ptr[35];
			break;
		default :
			break;

		}
		switch(ptr[43])
		{
		case 0x00:
			break;
		case 0x51:
			 rssi_pair[3] = ptr[44];
			break;
		case 0x54:
			 rssi_pair[2] = ptr[44];
			break;
		case 0x59:
			 rssi_pair[1] = ptr[44];
			break;
		default :
			break;

		}
		break;


		
	case 0x02:/*������������*/
		pos_send_to_service[20] =0x81;
		switch(ptr[16])
		{
		case 0x00:
			break;
		case 0x57:
			 rssi_pair[3] = ptr[17];
			break;
		case 0x69:
			 rssi_pair[2] = ptr[17];
			break;
		case 0x99:
			 rssi_pair[1] = ptr[17];
			break;
		default :
			break;

		}
		switch(ptr[25])
		{
		case 0x00:
			break;
		case 0x57:
			 rssi_pair[3] = ptr[26];
			break;
		case 0x69:
			 rssi_pair[2] = ptr[26];
			break;
		case 0x99:
			 rssi_pair[1] = ptr[26];
			break;
		default :
			break;

		}
		switch(ptr[34])
		{
		case 0x00:
			break;
		case 0x57:
			 rssi_pair[3] = ptr[35];
			break;
		case 0x69:
			 rssi_pair[2] = ptr[35];
			break;
		case 0x99:
			 rssi_pair[1] = ptr[35];
			break;
		default :
			break;

		}
		switch(ptr[43])
		{
		case 0x00:
			break;
		case 0x57:
			 rssi_pair[3] = ptr[44];
			break;
		case 0x69:
			 rssi_pair[2] = ptr[44];
			break;
		case 0x99:
			 rssi_pair[1] = ptr[44];
			break;
		default :
			break;

		}
	

		
		break;
	case 0x03:/*��԰����*/
		pos_send_to_service[20] =0x82;
		switch(ptr[16])
		{
		case 0x00:
			break;
		case 0x29:
			 rssi_pair[3] = ptr[17];
			break;
		case 0x42:
			 rssi_pair[2] = ptr[17];
			break;
		case 0x22:
			 rssi_pair[1] = ptr[17];
			break;
		default :
			break;

		}
		switch(ptr[25])
		{
		case 0x00:
			break;
		case 0x29:
			 rssi_pair[3] = ptr[26];
			break;
		case 0x42:
			 rssi_pair[2] = ptr[26];
			break;
		case 0x22:
			 rssi_pair[1] = ptr[26];
			break;
		default :
			break;

		}
		switch(ptr[34])
		{
		case 0x00:
			break;
		case 0x29:
			 rssi_pair[3] = ptr[35];
			break;
		case 0x42:
			 rssi_pair[2] = ptr[35];
			break;
		case 0x22:
			 rssi_pair[1] = ptr[35];
			break;
		default :
			break;

		}
		switch(ptr[43])
		{
		case 0x00:
			break;
		case 0x29:
			 rssi_pair[3] = ptr[44];
			break;
		case 0x42:
			 rssi_pair[2] = ptr[44];
			break;
		case 0x22:
			 rssi_pair[1] = ptr[44];
			break;
		default :
			break;

		}
		break;
	case 0x04:/*��԰����*/
		pos_send_to_service[20] =0x83;
		switch(ptr[16])
		{
		case 0x00:
			break;
		case 0x56:
			 rssi_pair[3] = ptr[17];
			break;
		case 0x96:
			 rssi_pair[2] = ptr[17];
			break;
		case 0x43:
			 rssi_pair[1] = ptr[17];
			break;
		default :
			break;

		}
		switch(ptr[25])
		{
		case 0x00:
			break;
		case 0x56:
			 rssi_pair[3] = ptr[26];
			break;
		case 0x96:
			 rssi_pair[2] = ptr[26];
			break;
		case 0x43:
			 rssi_pair[1] = ptr[26];
			break;
		default :
			break;

		}
		switch(ptr[34])
		{
		case 0x00:
			break;
		case 0x56:
			 rssi_pair[3] = ptr[35];
			break;
		case 0x96:
			 rssi_pair[2] = ptr[35];
			break;
		case 0x43:
			 rssi_pair[1] = ptr[35];
			break;
		default :
			break;

		}
		switch(ptr[43])
		{
		case 0x00:
			break;
		case 0x56:
			 rssi_pair[3] = ptr[44];
			break;
		case 0x96:
			 rssi_pair[2] = ptr[44];
			break;
		case 0x43:
			 rssi_pair[1] = ptr[44];
			break;
		default :
			break;

		}
		break;
	default:
		break;
}

	

 if ((rssi_pair[1] >= rssi_pair[2]) && (rssi_pair[1] >= rssi_pair[3]))
 	{
	 Ispos_one++;
	 Isleave = 0;
	Ispos_two=  0;
	Ispos_three = 0;
	 
 	}
 else if ((rssi_pair[2] >= rssi_pair[1]) && (rssi_pair[2] >= rssi_pair[3]))
 	{
	 Ispos_two++;
	 Isleave = 0;
	Ispos_one = 0;
	Ispos_three = 0;
		
 	}
 else if((rssi_pair[3]>=rssi_pair[1])&&(rssi_pair[3]>=rssi_pair[2]))/*3�����ֱ�ӳ�ջ*/
 	{
 		Isleave++;
		Ispos_one = 0;
		Ispos_two = 0;
		Ispos_three = 0;
 	}
 else{
		Isleave = 0;
		Ispos_one = 0;
		Ispos_two = 0;
		Ispos_three = 0;
 }


if((Isleave>0)&&(OBU_NUM !=  ptr[8])){/*��ֹ�ظ���ջ*/

	pos_send_to_service[19] =0x10;  /*����·��ڵ��жϾ���λ��*/
	//pos_send_to_service[20] =0x80;
	pos_send_to_service[21] =0x14;
	ENQueue(MyQueue, pos_send_to_service, 35);
	
	printf("bus ��ս***************��վ��ջ��սBRT************\n");
	writeFile(fop_log, NULL, 14); 
	chuzhan_notice[0] = 0x7e;
	chuzhan_notice[1] = 12;
	chuzhan_notice[2] = 0x21;
	chuzhan_notice[3] = ptr[1];
	chuzhan_notice[4] = ptr[2];
	chuzhan_notice[5] = ptr[3];
	chuzhan_notice[6] = ptr[4];
	chuzhan_notice[7] = ptr[5];
	chuzhan_notice[8] = ptr[6];
	chuzhan_notice[9] = ptr[7];
	chuzhan_notice[10] = ptr[8];
	chuzhan_notice[11] = 0x00;/*check*/


	memset(chuzhan_notice_send_to_uart,0,15);
	memcpy(chuzhan_notice_send_to_uart,chuzhan_notice,12);
	departure_flag = 1;
	OBU_NUM =  ptr[8];/*��¼���һ�γ�վ��OBU*/
	posvalue = 0;

								

	
}


if (Ispos_one>=1)
{
	if (posvalue > 1)
	{
		printf("����λ����ת   ����ԭ����λ��\n");
	}
	else
	{
		printf("λ�ö���11111111111\n");
		writeFile(fop_log, NULL, 19);
		printf("����������Ͷ�λλ��11����Ϣ\n");
		pos_send_to_service[19] =0x10;
		pos_send_to_service[21] = 0x11;
		ENQueue(MyQueue, pos_send_to_service, 35);
		posvalue = 1;
		Isleave = 0;
		Ispos_one = 0;
		Ispos_two = 0;
		Ispos_three = 0;
	
	}

}
else if (Ispos_two>= 1)
{
	if (posvalue > 2)
	{
		printf("����λ����ת   ����ԭ����λ��\n");
	}
	else
	{
		printf("λ�ö���22222222\n");
		writeFile(fop_log, NULL, 15);
		printf("����������Ͷ�λλ��22222����Ϣ\n");
		pos_send_to_service[19] =0x10;
		pos_send_to_service[21] = 0x12;
		ENQueue(MyQueue, pos_send_to_service, 35);
		posvalue = 2;
		Isleave = 0;
		Ispos_one=  0;
		Ispos_two = 0;
		Ispos_three = 0;

	}
}




		
		break;
	case 0x03:/*BRT��;վ*/
		// rssi_pair[0] = 0;/*����*/




  rssi_pair[1] = 0;
  rssi_pair[2] = 0;
 rssi_pair[3] = 0;

// int tempnum = ptr[16];


pos_send_to_service[16] = ptr[6] ;/*obu��ź�3λ*/
pos_send_to_service[17] =  ptr[7];/*obu��ź�3λ*/
pos_send_to_service[18] =  ptr[8];/*obu��ź�3λ*/

 switch(station_flag)
 {
	 case 0x02://ʯ��������
		 pos_send_to_service[20] =0x02;		 
			 break;
		
	 case 0x03://ʯ��������
		 pos_send_to_service[20] =0x03;		 
			 break;	 
			
	 case 0x04://�ڶ�����
		 pos_send_to_service[20] =0x04;		 
			 break;
	 case 0x05://�ڶ�����
		 pos_send_to_service[20] =0x05;		 
			 break;
	 case 0x06://ʦ���ߴ�����
		 pos_send_to_service[20] =0x06;		 
			 break;
	 case 0x07://ʦ���ߴ�����
		 pos_send_to_service[20] =0x07;		 
			 break;
	 case 0x08://�����³�����
		 pos_send_to_service[20] =0x08;		 
			 break;
	 case 0x09://�����³�����
		 pos_send_to_service[20] =0x09;		 
			 break;
	 case 0x0A://��������
		 pos_send_to_service[20] =0x10;		 
			 break;
	 case 0x0B://��������
		 pos_send_to_service[20] =0x11;		 
			 break;
	 case 0x0C://ѧԺ����
		 pos_send_to_service[20] =0x12;		 
			 break;
	 case 0x0D://ѧԺ����
		 pos_send_to_service[20] =0x13;		 
			 break;
	 case 0x0E://���´�����
		 pos_send_to_service[20] =0x14;		 
			 break;
	 case 0x0F://���´�����
		 pos_send_to_service[20] =0x15;		 
			 break;
	 case 0x10://�Ķ�����
		 pos_send_to_service[20] =0x16;		 
			 break;
	 case 0x11://�Ķ�����
		 pos_send_to_service[20] =0x17;		 
			 break;
	 case 0x12://������������
		 pos_send_to_service[20] =0x18;		 
			 break;
	 case 0x13://������������
		 pos_send_to_service[20] =0x19;		 
			 break;
	 case 0x14://��������
		 pos_send_to_service[20] =0x20;		 
			 break;
	 case 0x15://��������
		 pos_send_to_service[20] =0x21;		 
			 break;
	 case 0x16://����������
		 pos_send_to_service[20] =0x22;		 
			 break;
	 case 0x17://����������
		 pos_send_to_service[20] =0x23;		 
			 break;
	 case 0x18://�ƴ�����
		 pos_send_to_service[20] =0x24;		 
			 break;
	 case 0x19://�ƴ�����
		 pos_send_to_service[20] =0x25;		 
			 break;
	 case 0x1A://��������
		 pos_send_to_service[20] =0x26;		 
			 break;
	 case 0x1B://��������
		 pos_send_to_service[20] =0x27;		 
			 break;
	default:
			break;
 }

 /*��ȡrssiֵ*/
 rssi_pair[1] = ptr[17];

 rssi_pair[2] = ptr[26];

 rssi_pair[3] = ptr[35];


 if (((rssi_pair[2] >= rssi_pair[1]) && (rssi_pair[2] >= rssi_pair[3]))||((rssi_pair[1]!=0)&&(rssi_pair[2]==0)))
 	{
	 Ispos_two++;
	 Isleave = 0;
	Ispos_three = 0;
	posvalue=2;
	 
 	}
 else if ((rssi_pair[3] >= rssi_pair[1]) && (rssi_pair[3] >= rssi_pair[2]))
 	{
	 Ispos_three++;
	 Isleave = 0;
	Ispos_two = 0;
	posvalue=3;
 	}
 else if ((rssi_pair[1] >= rssi_pair[2]) && (rssi_pair[1] >= rssi_pair[3]))
 	{
	 Ispos_one++;
	 Isleave = 0;
	Ispos_two = 0;
	posvalue=1;	
 	}

 	
#if 0
if(((OBU_NUM !=  ptr[8])||(difftime(time(NULL),jinchuzhan_beat) >= 1200))&&(posvalue>0)){/*��ֹ�ظ���ջ*/

	jinchuzhan_beat= time(NULL);
	pos_send_to_service[19] =0x10;
	pos_send_to_service[20] =0x90;
	pos_send_to_service[21] =0x13;
	ENQueue(MyQueue, pos_send_to_service, 35);
	
	printf("bus ��ս***************��վ��ջ��ս��;վ************\n");
	writeFile(fop_log, NULL, 14);
	chuzhan_notice[0] = 0x7e;
	chuzhan_notice[1] = 12;
	chuzhan_notice[2] = 0x21;
	chuzhan_notice[3] = ptr[1];
	chuzhan_notice[4] = ptr[2];
	chuzhan_notice[5] = ptr[3];
	chuzhan_notice[6] = ptr[4];
	chuzhan_notice[7] = ptr[5];
	chuzhan_notice[8] = ptr[6];
	chuzhan_notice[9] = ptr[7];
	chuzhan_notice[10] = ptr[8];
	chuzhan_notice[11] = 0x00;/*check*/
	
	memset(chuzhan_notice_send_to_uart,0,15);
	memcpy(chuzhan_notice_send_to_uart,chuzhan_notice,12);
	departure_flag = 1;
	OBU_NUM =  ptr[8];/*��¼���һ�γ�վ��OBU*/
	posvalue = 0;
	
}

#endif
if ((posvalue==1)&&(difftime(time(NULL),pos_intval_t) >= 240))
{
		pos_intval_t = time(NULL);
		writeFile(fop_log, NULL, 19);
		printf("����������Ͷ�λλ��1����Ϣ\n");
		pos_send_to_service[19] = 0x10;
		//pos_send_to_service[20] = 0x84;
		pos_send_to_service[21] = 0x11;
		ENQueue(MyQueue, pos_send_to_service, 35);
}
else if ((posvalue==2)&&(difftime(time(NULL),pos_intval_t) >= 240))
{
		pos_intval_t = time(NULL);
		printf("λ�ö���222222222\n");
		writeFile(fop_log, NULL, 15);
		pos_send_to_service[19] = 0x10;
		//pos_send_to_service[20] = 0x84;
		pos_send_to_service[21] = 0x12;
		ENQueue(MyQueue, pos_send_to_service, 35);

	
}
else if ((posvalue==3)&&(difftime(time(NULL),pos_intval_t) >= 240))
{
	
	
		//pos_flag = 1;
		pos_intval_t = time(NULL);
		printf("λ�ö���33333333\n");
		writeFile(fop_log, NULL, 16);
		pos_send_to_service[19] = 0x10;
		//pos_send_to_service[20] = 0x84;
		pos_send_to_service[21] = 0x13;
		ENQueue(MyQueue, pos_send_to_service, 35);

	
}


if(((OBU_NUM !=  ptr[8])||(difftime(time(NULL),jinchuzhan_beat) >= 1200))&&(posvalue>0)){/*��ֹ�ظ���ջ*/

	jinchuzhan_beat= time(NULL);
	pos_send_to_service[19] =0x10;
	//pos_send_to_service[20] =0x84;
	pos_send_to_service[21] =0x14;
	ENQueue(MyQueue, pos_send_to_service, 35);
	
	printf("bus ��ս***************��վ��ջ��ս��;վ************\n");
	writeFile(fop_log, NULL, 14);
	chuzhan_notice[0] = 0x7e;
	chuzhan_notice[1] = 12;
	chuzhan_notice[2] = 0x21;
	chuzhan_notice[3] = ptr[1];
	chuzhan_notice[4] = ptr[2];
	chuzhan_notice[5] = ptr[3];
	chuzhan_notice[6] = ptr[4];
	chuzhan_notice[7] = ptr[5];
	chuzhan_notice[8] = ptr[6];
	chuzhan_notice[9] = ptr[7];
	chuzhan_notice[10] = ptr[8];
	chuzhan_notice[11] = 0x00;/*check*/
	
	memset(chuzhan_notice_send_to_uart,0,15);
	memcpy(chuzhan_notice_send_to_uart,chuzhan_notice,12);
	departure_flag = 1;
	OBU_NUM =  ptr[8];/*��¼���һ�γ�վ��OBU*/
	posvalue = 0;
	
}

		break;

	default:
			break;
	

		}

		}
	
	/*�����ӡ����*/  /*��ӡrssi��Ϣ*/
	//2015 01 12 �޸�
/*	memset(data_from_uart,0,150);
	memset(uart_convet,0,300);
	data_from_uart[0] = 0x7e;
	data_from_uart[1] = p;
	memcpy(data_from_uart+2,ptr,len);/*����У��λ*/
/*	convet(data_from_uart, uart_convet, p);/*uart_convet����������ascii*/
//	writeFile(fop_log, uart_convet, 4);
	
//	printf("datalen =  %02x \n",len);
//	crc_8 = checkcode_crc8(ptr,len-1);
//	printf("crc_8 = %02x ptr[len-1]= %02x  \n",crc_8, ptr[len-1]);
	
//	if(*(ptr+len-1)!=crc_8)
	//	{
		//	printf("jiaoyan error\n");
			//return -1;
	//	}
	return len-1;/*len-1�����������ݳ��� ��ȥУ��λ*/
}
/*
unsigned int crc16(unsigned char *buf, unsigned int len)
{
    unsigned int val = 0,i;
    for (i=0; i<len; i++) 
	{val=CRC_TAB[(val^=buf[i])&0xFF]^(val>>8);}
    return val;
}


unsigned int crc16_serial(unsigned char *buf, unsigned int len)
{
    unsigned int val = 0,i;
    val = CRC_TAB[0x7e];
    val = CRC_TAB[(unsigned char)(len+3)] ^(val>>8);
    for (i=0; i<len; i++)
	{val=CRC_TAB[(val^=buf[i])&0xFF]^(val>>8);}
    return val;
}
*/

/*
int open_uart(void)
{
	int fd;
	if((fd = open("/dev/usb/tts/5",O_RDWR | O_NOCTTY)) == -1)
	if((fd = open("/dev/ttyAMA1",O_RDWR | O_NOCTTY)) == -1)
	{
		perror("open uart");
	}
	else
	{
		printf("open_uart success\n");
	}
	
	return (fd);
}*/
int open_uart(void)
{
	int fd;
if(!DEBUG){
	printf("open usb /tts/5 \n");
	if((fd = open("/dev/usb/tts/5",O_RDWR | O_NOCTTY)) == -1)
	{
		perror("open uart");
	}
	else
	{
		printf("open_uart success\n");
	}
}
else	
{
	printf("open /dev/ttyAMA1 \n");
	if((fd = open("/dev/ttyAMA1",O_RDWR | O_NOCTTY)) == -1)
	{
		perror("open uart");
	}
	else
	{
		printf("open_uart success\n");
	}
}

	
	
	return (fd);
}

void init_uart()
{
	//printf("1\n");
	struct termios newtio,oldtio;
	//F_GETFL ȡ���ļ�������״̬��꣬�����Ϊopen�����Ĳ���flags
	if(fcntl(uart_fd, F_SETFL,0) < 0)
	{
		perror("fcntl uart ");
	}
	//printf("2\n");
	//isatty�������������desc��������ļ�������Ϊһ�ն˻��򷵻�1�����򷵻�0��
	if(isatty(STDIN_FILENO) == 0)
	{
		printf("standard input is not a terminal device\n");
	}
	//printf("3\n");
	//tcgetattr�������ڻ�ȡ���ն���صĲ���������fdΪ�ն˵��ļ������������صĽ��������termios �ṹ���У�termios�ṹͼΪ�ն˿��Ʋ���
	if(tcgetattr(uart_fd, &oldtio) != 0)
	{
		perror("tcgetattr ");
	}
	//printf("4\n");
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;//first clear the CSIZE flag and then set the CSIZE
	newtio.c_cflag |= CS8;//8 bites
	newtio.c_cflag &= ~PARENB;// no parity check;
	newtio.c_cflag &= ~CSTOPB;//one stop btie
	newtio.c_lflag &= ~ICANON;// informal mode
	//cfsetispeed���ò�����
	if(cfsetispeed(&newtio, B115200) != 0)
	{
		perror("cfsetispeed");
	}
	//printf("5\n");
	

	newtio.c_cc[VTIME] = 1;/*ָ����ȡ��һ���ַ��ĵȴ�ʱ��*/
	newtio.c_cc[VMIN] = 0;/*ָ����Ҫ��ȡ�ַ�����С����*/
	tcflush(uart_fd,TCIFLUSH);
	/*
	tcflush(int fd, int queue_selector);
	
	fd �ն�I/O�򿪵ľ��
	
	queue_selector ����tcflush�Ĳ�����ȡֵΪ�������������е�һ����
	
		TCIFLUSH  ������յ������ݣ��Ҳ����ȡ������
	
		TCOFLUSH   �����д������ݣ��Ҳ��ᷢ�����նˡ�
	
		TCIOFLUSH ����������ڷ�����I/O���ݡ�

	*/
	if(tcsetattr(uart_fd, TCSANOW, &newtio) != 0)
	{
		perror("tcsetaddr uart ");
	}	
	/*
	tcsetattr�����ն˲���
	TCSANOW���������ݴ�����Ͼ������ı����ԡ�
	TCSADRAIN���ȴ��������ݴ�������Ÿı����ԡ�
	TCSAFLUSH�������������������Ÿı����ԡ�

	*/
	//printf("success!\n");

}



/*�й���־���ܵ��Ӻ���*/
/*��ȡ��ǰϵͳ��ʱ�� */
int getTime(char *out,int fmt)
{
    if(out == NULL)
        return -1;
    time_t t;
    struct tm *tp;
   // t = time(NULL);
    time(&t);
    tp = localtime(&t);
    if(fmt == 0)
        sprintf(out, "%2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
    else if(fmt == 1)
        sprintf(out, "%2.2d-%2.2d-%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday);
    else if(fmt == 2)
        sprintf(out, "%2.2d:%2.2d:%2.2d", tp->tm_hour, tp->tm_min, tp->tm_sec);
    else if(fmt ==3 )
		   sprintf(out, "%2.2d%2.2d%2.2d_%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, tp->tm_hour);
    else if(fmt ==4 )
		   sprintf(out, "%2.2d%2.2d%2.2d_%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday-2, tp->tm_hour);
	else if(fmt ==5 )
		   sprintf(out, "%2.2d%2.2d%2.2d_%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday-1, tp->tm_hour);
	else if(fmt ==6 )
		   sprintf(out, "%2.2d%2.2d%2.2d_%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday-3, tp->tm_hour);
	else if(fmt ==7 )
		   sprintf(out, "%2.2d%2.2d", tp->tm_year+1900, tp->tm_mon);
	
    return 0;

}

FILE* openfile(const char *fileName,const char *mode)
{
    /*��ʱ��������*/
   /*׷��.log*/
//    strcat(fileName,".log");
    FILE *fp = fopen(fileName,mode);
    return fp;
}

int writeFile(FILE *fp,const char *str,int blog)
{
   // assert(fp != NULL && str != NULL);/*������۲�������ֹ����*/
  assert(fp != NULL);
    char curTime[100] = {0};
    int ret = -1;

getTime(curTime,0);
 switch(blog)
 	{
	case 1:
		ret = fprintf(fp, "[%s]:{�������ͨ��}send: %s\n", curTime, str);
		break;
	case 2:
		ret = fprintf(fp, "[%s]:{�������ͨ��}recv: %s\n", curTime, str);
		break;
	case 3:
		 ret = fprintf(fp, "[%s]:{�봮��ͨ��}send: %s\n", curTime, str);
		break;
	case 4:
		 ret = fprintf(fp, "[%s]:{�봮��ͨ��}recv: %s\n", curTime, str);
		break;
	case 5:
		ret = fprintf(fp, "[%s]:{����ʧ��}error: %s\n", curTime, str);
		break;
	case 6:
		ret = fprintf(fp, "[%s]:{�������ļ�ʧ��}error: %s\n", curTime, str);
		break;
	case 7:
		ret = fprintf(fp, "[%s]:{ARM������}heart_beat: %s\n ",curTime, str);
		break;
	case 8:
		ret = fprintf(fp, "[%s]:{ʱ������֡}send: %s\n", curTime, str);
		break;
	case 9:
		ret = fprintf(fp, "[%s]: %s\n", curTime, str);
		break;
	case 10:
		ret = fprintf(fp, "[%s]:{obu֡}recv: %s\n", curTime, str);
		break;
	case 11:
		ret = fprintf(fp, "[%s]:{Զ�����ð�}First: %s\n", curTime, str);
		break;
	case 12:
		ret = fprintf(fp, "[%s]:{Զ�����ð�}Second: %s\n", curTime, str);
		break;
	case 13:
		ret = fprintf(fp, "[%s]:{Զ�����ð�}Third: %s\n", curTime, str);
		break;
	case 14:
		ret = fprintf(fp, "[%s]:{��վ��վ}\n", curTime);
		break;
	case 15:
		ret = fprintf(fp, "[%s]:{pos = 2}\n", curTime);
		break;
	case 16:
		ret = fprintf(fp, "[%s]:{pos = 3}\n", curTime);
		break;
	case 17:
		ret = fprintf(fp, "[%s]:{���Դ��ڵĽ�վ}\n", curTime);
		break;
	case 18:
		ret = fprintf(fp, "[%s]:{���Դ��ڵĳ�վ}\n", curTime);
		break;
	case 19:
		ret = fprintf(fp, "[%s]:{pos = 1}\n", curTime);
		break;
	default:
		return -1;
		break;

 }
    if(ret >= 0)
    {
        fflush(fp);
        return 0;
    }
    else
        return -1;

}

int closeFile(FILE *fp)
{
    return fclose(fp);
}

//16����ת�ַ���
int convet(char *src,char *dest,int length)
{
    int i = 0;
    int k = length;

    for(i = 0;i<k;i++)
    {
       unsigned char temp;  /*�����unsigned �͵ķ�������*/
        temp = src[i]&0xf0;
        temp = temp >> 4;
      //  if(temp<0)  /*�������  �и���*/
      //      temp+=16;
       if((temp>9)&&(temp<16))
        {
            temp = temp+0x37;
        }
        else{
            temp = temp+0x30;
        }
        dest[2*i] =temp;/*��Ҫ��*��*/
        temp = src[i]&0x0f;
           if((temp>9)&&(temp<16))
        {
            temp = temp+0x37;
        }
        else{
            temp = temp+0x30;
        }
        dest[2*i+1] = temp;
    }
    return 2*i+1;
}

/*�������ļ���صĺ�������*/
/*
 *ȥ���ַ����Ҷ˿ո�
 */
char *strtrimr(char *pstr)
{
    int i;
    i = strlen(pstr) - 1;
    while (isspace(pstr[i]) && (i >= 0))
        pstr[i--] = '\0';
    return pstr;
}
/*
 *ȥ���ַ�����˿ո�
 */
char *strtriml(char *pstr)
{
    int i = 0,j;
    j = strlen(pstr) - 1;
    while (isspace(pstr[i]) && (i <= j))
        i++;
    if (0<i)
        strcpy(pstr, &pstr[i]);
    return pstr;
}
/*
 *ȥ���ַ������˿ո�
 */
char *strtrim(char *pstr)
{
    char *p;
    p = strtrimr(pstr);
    return strtriml(p);
}


/*
 *�������ļ���һ�ж���key��value,����itemָ��
 *line--�������ļ�������һ��
 */
int  get_item_from_line(char *line,  ITEM *item)
{
    char *p = strtrim(line);
    int len = strlen(p);
    if(len <= 0){
        return 1;//����
    }
    else if(p[0]=='#'){
        return 2;
    }else{
        char *p2 = strchr(p, '=');
        *p2++ = '\0';
        item->key = (char *)malloc(strlen(p)+1 );
        item->value = (char *)malloc(strlen(p2) + 1);
        strcpy(item->key,p);
        strcpy(item->value,p2);

        }
    return 0;//��ѯ�ɹ�
}

/*
 *��ȡvalue,�ɹ�����0��ʧ�ܷ���1
 */
int read_conf_value(const char *key, char *value,const char *file)
{
    char line[1024];
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL)
        return 1;//�ļ��򿪴���
     //   printf("���ļ��ɹ� %s \n",file);
    while (fgets(line, 1023, fp)){//char *fgets(char *buf, int bufsize, FILE *stream);
        ITEM item;
        get_item_from_line(line,&item);
        if(!strncmp(item.key,key,4)){//��ȫ��ȣ�����ֵ��=0��
        //    printf("һ�� %s  \n",key);
       //     printf("һ�� %s  \n",item.key);
            strcpy(value,item.value);
       //      printf("һ�� %s  \n",key);
            fclose(fp);
            free(item.key);
            free(item.value);
            break;
        }

    }
    return 0;//�ɹ�

}
/*�ַ���ת16����*/
unsigned long convert_atohex(char* str)
{
 unsigned long var=0;
 unsigned long t;
 int len = strlen(str);

 if (len > 8) //�8λ
  return -1;
// strupr(str);//ͳһ��д
 for (; *str; str++)
 {
  if (*str>='A' && *str <='F')
   t = *str-55;//a-f֮���ascii���Ӧ��ֵ���55��'A'Ϊ65,65-55��ΪA
  else
   t = *str-48;
  var<<=4;
  var|=t;
 }
 return var;
}


int file_to_items(const char *file,  ITEM *items,  int *num)
{
    char line[1024];
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL)
        return 1;
    int i = 0;
    while(fgets(line, 1023, fp)){
            char *p = strtrim(line);
        int len = strlen(p);
        if(len <= 0){
            continue;
        }
        else if(p[0]=='#'){
            continue;
        }else{
            char *p2 = strchr(p, '=');
            /*������Ϊֻ��keyûʲô����*/
            if(p2 == NULL)
                continue;
            *p2++ = '\0';
            items[i].key = (char *)malloc(strlen(p) + 1);
            items[i].value = (char *)malloc(strlen(p2) + 1);
            strcpy(items[i].key,p);
            strcpy(items[i].value,p2);

            i++;
        }
    }
    (*num) = i;
    fclose(fp);
    return 0;
}

int write_conf_value(const char *key,char *value,const char *file)   //strncmp
{
    ITEM items[20];// �ٶ������������20��
    int num;//�洢���ļ���ȡ����Ч��Ŀ
    file_to_items(file, items, &num);

    int i=0;
    //����Ҫ�޸ĵ���
    for(i=0;i<num;i++){
        if(!strncmp(items[i].key, key,4)){
            items[i].value = value;
            break;
        }
    }

    // ���������ļ�,Ӧ���б��ݣ�����Ĳ����Ὣ�ļ��������
    FILE *fp;
    fp = fopen(file, "w");
    if(fp == NULL)
        return 1;

    i=0;
    for(i=0;i<num;i++){
        fprintf(fp,"%s=%s\n",items[i].key, items[i].value);
        //printf("%s=%s\n",items[i].key, items[i].value);
    }
    fclose(fp);
    //�������
/*    i=0;
    for(i=0;i<num;i++){
        free(items[i].key);
        free(items[i].value);
    }*/

    return 0;

}

/*locate */

int MaxRssiID(int rssi1, int rssi2, int rssi3, int rssi4)
{
	int maxID = 1;
	int maxrssi = rssi1;
	if (maxrssi < rssi2){
		maxrssi = rssi2;
		maxID = 2;
	}
	if (maxrssi < rssi3){
		maxrssi = rssi3;
		maxID = 3;
	}
	if (maxrssi < rssi4){
		maxrssi = rssi4;
		maxID = 4;
	}
	return maxID;
}

int AverageRssi(int rssi[]) //����30������ֹԽ��
{
	int i = 0, sum = 0;
	int average;
	for (i = 0; i < 30; i++)
	{
		if ((rssi[i] == 0) && (rssi[i + 1] == 0) && (rssi[i + 2] == 0))
			break;
		sum += rssi[i];
	}
	if (i == 0)
		return 0;
	average = sum / i;
	return average;
}

int position(int rssi1[], int rssi2[], int rssi3[], int rssi4[])
{
	int average1, average2, average3, average4, maxRouterID;
	average1 = AverageRssi(rssi1);
	average2 = AverageRssi(rssi2);
	average3 = AverageRssi(rssi3);
	average4 = AverageRssi(rssi4);
	printf("AverageRssi1 = %d\nAverageRssi2 = %d\nAverageRssi3 = %d\nAverageRssi4 = %d\n", average1, average2, average3, average4);
	maxRouterID = MaxRssiID(average1, average2, average3, average4);
	if (maxRouterID == 1)
		return 1;
	else if (maxRouterID == 2)
		return 2;
	else
		return 3;
}
/*
bool is_LeaveStation(int rssi1, int rssi2, int rssi3, int rssi4)
{
	int maxRouterID = MaxRssiID(rssi1, rssi2, rssi3, rssi4);
	return maxRouterID == 4 ? true : false;
}
*/
