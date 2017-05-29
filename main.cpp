#include "mbed.h"
#include "lwip-interface/EthernetInterface.h"
//#include "freertos.h"
#include <string>


using std::string;
const int BROADCAST_PORT_T = 58080; //UDP�㲥���Ͷ˿�
const int BROADCAST_PORT_R = 58081; //UDP�㲥���ն˿�
EthernetInterface eth; //����һ��interface

//Thread receiver;
//Thread	led1_t;
//Thread	led2_t;
//Thread	led3_t;
Thread	can1_send_t; 
Thread	can1_receive_t; //����CAN���� �����߳�
Thread	udp_send_t;
Thread	udp_receive_t; //����UDP���� �����߳�
//Ticker can1_send;
CAN can1(PD_0,PD_1);  //����CAN1Ӳ������
DigitalOut led1(LED1); 
DigitalOut led2(LED2);
DigitalOut led3(LED3);  //����LED1 2 3Ӳ������
can_t can_1;  //����
CANMessage msg,msg_re,msg_se;  //����CAN���ձ��ģ����ͱ���
char a[8]="01234";
unsigned char rad_data[64][8]={0}; //�����״�����
//void transmit(char a[8]);


//void led1_thread() {
//    while (true) {
//        led1 = !led1;
//        Thread::wait(1000);
//    }
//}
//void led2_thread() {
//    while (true) {
//        led2 = !led2;
//        Thread::wait(1000);
//    }
//}

//void led3_thread() {
//    while (true) {
//        led3 = !led3;
//        Thread::wait(1000);
//    }
//}
void can1_send_thread()
{
	while (true) {
       can_write(&can_1,msg,0);
        Thread::wait(1000);
    }
}
void can1_receive_thread() //CAN���������߳�
{
	while (true){
		can_read(&can_1,&msg_re,0); //��ȡCAN���ݵ�msg_re
		unsigned char i;
		if((msg_re.id>=0x500)&&(msg_re.id<=0x53f)) //��ȡCANidΪ500-53f
		{
			switch(msg_re.id)
			{
				case 0x500:
				{
					for(i=0;i<8;i++)
					rad_data[0][i]=msg_re.data[i];  //��ÿ��ID��8�ֽ�����һһ��Ӧ�洢��������
				}break;
				
				case 0x501:
				{
					for(i=0;i<8;i++)
					rad_data[1][i]=msg_re.data[i];
				}break;
				
				case 0x502:
				{
					for(i=0;i<8;i++)
					rad_data[2][i]=msg_re.data[i];
				}break;
				
				case 0x503:
				{
					for(i=0;i<8;i++)
					rad_data[3][i]=msg_re.data[i];
				}break;
				
				case 0x504:
				{
					for(i=0;i<8;i++)
					rad_data[4][i]=msg_re.data[i];
				}break;
				
				case 0x505:
				{
					for(i=0;i<8;i++)
					rad_data[5][i]=msg_re.data[i];
				}break;
				
				case 0x506:
				{
					for(i=0;i<8;i++)
					rad_data[6][i]=msg_re.data[i];
				}break;
				
				case 0x507:
				{
					for(i=0;i<8;i++)
					rad_data[7][i]=msg_re.data[i];
				}break;
				
			}
		}
		printf("read=%d\n",msg_re.data[0]);
		Thread::wait(100);  //�ӳ��л��߳�
	}
}
void udp_send_thread()
{
	UDPSocket socket(&eth);  //����һ��socket
    //string out_buffer = "send very important data";
    SocketAddress udp_send_thread("192.168.2.159", BROADCAST_PORT_T); //����UDP����IP �˿� 
	while (true){
//		int ret = socket.sendto(dat, a,sizeof(a));
		int ret = socket.sendto(udp_send_thread, &rad_data,512); //����UDP���ݰ���һ֡UDP����512�ֽ�
		printf("sendto return: %d\n", ret); //�������UDP���ݰ������ݳ���
		Thread::wait(2000); //�ӳ��л��߳�
	}
}

//void transmit()
//{
//    UDPSocket socket(&eth);
//    string out_buffer = "send very important data";
//    SocketAddress transmit("192.168.1.159", BROADCAST_PORT_T);

//   while (true)
//    {
//	int ret = socket.sendto(transmit, a,sizeof(a));
//	printf("sendto return: %d\n", ret);

//	Thread::wait(5000);
//    }
//}

void udp_receive_thread()
{
    SocketAddress udp_receive_thread;
    UDPSocket socket(&eth);
    int bind = socket.bind(BROADCAST_PORT_R);
    printf("bind return: %d\n", bind);

    char buffer[256];
    while (true)
    {
	printf("\nWait for packet...\n");
	int n = socket.recvfrom(&udp_receive_thread, buffer, sizeof(buffer));
	buffer[n] = '\0';
	printf("Packet from \"%s\": %s\n", udp_receive_thread.get_ip_address(), buffer);
	msg_se.data[0]=buffer[0];
	can_write(&can_1,msg_se,0);
	Thread::wait(500);
    }
}



//void sent()
//{
//	can_write(&can_1,msg,0);
//	led2=!led2;
//}

int main()
{
	
	
	printf("main()\n");
	
	can_init(&can_1,PD_0,PD_1);
	can_frequency(&can_1,500000); //����CAN ����500KB/S
	msg.id=0x34; //����CAN ID
	msg.data[0]=0x1a; //����CANһ�ֽ� ����0X1A
	msg.len=1;	//����1�ֽ�
	msg.format=CANStandard; //��׼CAN����
	msg.type=CANData; //����֡
	
//	can1_send.attach(&sent,2);
	
//	led1_t.start(led1_thread);
//	led2_t.start(led2_thread);
//	led3_t.start(led3_thread);
	
	//eth.set_network("192.168.1.4","255.255.255.0","192.168.1.1");
    eth.connect(); //������̫������

    printf("Controller IP Address is %s\r\n", eth.get_ip_address()); //���ڴ�ӡIP��ַ
    Thread::wait(5000);	 //�ȴ�5S
//   transmitter.start(transmit);
//    receiver.start(receive);
	can1_send_t.start(can1_send_thread);	 //��ʼCAN�����߳�
	can1_receive_t.start(can1_receive_thread); //��ʼCAN�����߳�
	udp_send_t.start(udp_send_thread); //��ʼUDP���������߳�
	udp_receive_t.start(udp_receive_thread);//��ʼUDP���������߳�
	
	
	
    while(1) {
		uint8_t i;
        //printf("loop()\n");
        //if(can1.read(msg_re)) {
//		if(can_read(&can_1,&msg_re,0)){
//            printf("Message received: %x\n", msg_re.data[0]);
//			for(i=0;i<8;i++)
//			a[i]=msg_re.data[i];
//			//transmit();
//            led2 = !led2;
//        } 
	}
}
