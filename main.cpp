#include "mbed.h"
#include "lwip-interface/EthernetInterface.h"
//#include "freertos.h"
#include <string>


using std::string;
const int BROADCAST_PORT_T = 58080; //UDP广播发送端口
const int BROADCAST_PORT_R = 58081; //UDP广播接收端口
EthernetInterface eth; //定义一个interface

//Thread receiver;
//Thread	led1_t;
//Thread	led2_t;
//Thread	led3_t;
Thread	can1_send_t; 
Thread	can1_receive_t; //定义CAN发送 接收线程
Thread	udp_send_t;
Thread	udp_receive_t; //定义UDP发送 接收线程
//Ticker can1_send;
CAN can1(PD_0,PD_1);  //定义CAN1硬件引脚
DigitalOut led1(LED1); 
DigitalOut led2(LED2);
DigitalOut led3(LED3);  //定义LED1 2 3硬件引脚
can_t can_1;  //定义
CANMessage msg,msg_re,msg_se;  //定义CAN接收报文，发送报文
char a[8]="01234";
unsigned char rad_data[64][8]={0}; //定义雷达数据
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
void can1_receive_thread() //CAN接收数据线程
{
	while (true){
		can_read(&can_1,&msg_re,0); //读取CAN数据到msg_re
		unsigned char i;
		if((msg_re.id>=0x500)&&(msg_re.id<=0x53f)) //读取CANid为500-53f
		{
			switch(msg_re.id)
			{
				case 0x500:
				{
					for(i=0;i<8;i++)
					rad_data[0][i]=msg_re.data[i];  //将每个ID的8字节数据一一对应存储到数组中
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
		Thread::wait(100);  //延迟切换线程
	}
}
void udp_send_thread()
{
	UDPSocket socket(&eth);  //定义一个socket
    //string out_buffer = "send very important data";
    SocketAddress udp_send_thread("192.168.2.159", BROADCAST_PORT_T); //设置UDP发送IP 端口 
	while (true){
//		int ret = socket.sendto(dat, a,sizeof(a));
		int ret = socket.sendto(udp_send_thread, &rad_data,512); //发送UDP数据包，一帧UDP数据512字节
		printf("sendto return: %d\n", ret); //串口输出UDP数据包的数据长度
		Thread::wait(2000); //延迟切换线程
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
	can_frequency(&can_1,500000); //设置CAN 速率500KB/S
	msg.id=0x34; //设置CAN ID
	msg.data[0]=0x1a; //设置CAN一字节 数据0X1A
	msg.len=1;	//长度1字节
	msg.format=CANStandard; //标准CAN数据
	msg.type=CANData; //数据帧
	
//	can1_send.attach(&sent,2);
	
//	led1_t.start(led1_thread);
//	led2_t.start(led2_thread);
//	led3_t.start(led3_thread);
	
	//eth.set_network("192.168.1.4","255.255.255.0","192.168.1.1");
    eth.connect(); //启动以太网连接

    printf("Controller IP Address is %s\r\n", eth.get_ip_address()); //串口打印IP地址
    Thread::wait(5000);	 //等待5S
//   transmitter.start(transmit);
//    receiver.start(receive);
	can1_send_t.start(can1_send_thread);	 //开始CAN发送线程
	can1_receive_t.start(can1_receive_thread); //开始CAN接收线程
	udp_send_t.start(udp_send_thread); //开始UDP发送数据线程
	udp_receive_t.start(udp_receive_thread);//开始UDP接收数据线程
	
	
	
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
