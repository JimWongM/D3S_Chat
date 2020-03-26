#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <net/if.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <bitset>
#include "DES.h"
using namespace std;

#define BUFFERSIZE 64
#define ServerPort 8907
char SendBuffer[BUFFERSIZE];
char ReceiveBuffer[BUFFERSIZE*8];


void Chat(int nSock,char *pRemoteName,int port);

int main(int argc,char * [])
{
 
 cout<<"Client or Server?"<<endl;
 char choice;
 cin>>choice;
 if(choice=='c'||choice=='C')
 {
  //Client
  char ServerIP[16];
  cout<<"Please input the server ip:"<<endl;
  cin>>ServerIP;

  int ClientSocket;
  ClientSocket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in ConnectAddr;
  bzero(&ConnectAddr, sizeof(ConnectAddr));
  ConnectAddr.sin_family = AF_INET;
  ConnectAddr.sin_port = htons(ServerPort);
  ConnectAddr.sin_addr.s_addr = inet_addr(ServerIP);
  int ret=connect(ClientSocket, (struct sockaddr *) &ConnectAddr, sizeof(ConnectAddr));
  if(ret==0)
  {
    cout<<"Connect Success"<<endl;
    cout<<"Begin Chat"<<endl;
    Chat(ClientSocket,ServerIP,ServerPort);
  }
   close(ClientSocket);
 }
 else
 {
  //Server
  int ListenSocket;
  ListenSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in ListenAddr;
  bzero(&ListenAddr, sizeof(ListenAddr));
  ListenAddr.sin_family = PF_INET;
  ListenAddr.sin_port = htons(ServerPort);
  ListenAddr.sin_addr.s_addr = INADDR_ANY;

  int ret2=bind(ListenSocket, (struct sockaddr *) &ListenAddr, sizeof(struct sockaddr));
  if(ret2==0)
  {
      cout<<"Bind Success"<<endl;
  }
  else
  {
  	cout<<"fail"<<endl;
  }
  listen(ListenSocket, 5);
 
  cout<<"Listening"<<endl;

  int AcceptSocket;
  struct sockaddr_in ConAddr;
  socklen_t len = sizeof(struct sockaddr);
  if ((AcceptSocket = accept(ListenSocket,(struct sockaddr*)&ConAddr,&len)) != -1)
  { 
   cout<<"Got connection from "<<inet_ntoa(ConAddr.sin_addr)<<":"<<ntohs(ConAddr.sin_port)<<endl;
   int m_port=ntohs(ConAddr.sin_port);
   Chat(AcceptSocket,inet_ntoa(ConAddr.sin_addr),m_port);
   close(ListenSocket);
   close(AcceptSocket);
  }
  else
  {
  	cout<<"fail"<<endl;
  }
 
 }
 return 0;
}


void Chat(int nSock,char *pRemoteName,int port)
{
 
 unsigned char key[8]={'B','L','A','C','K','H','A','T'};
 bitset<64> KEY;
 bitset<8> tempkey[8];
 string k;
 for(int i=0;i<8;i++)
 {
 	tempkey[i]=bitset<8>((unsigned)key[i]);
	k.append(tempkey[i].to_string());
 }
 KEY=b64(k);
 DES des(KEY);

 pid_t nPid;
 nPid = fork();
 if(nPid != 0)                    //子线程接受信息
 {
  while(1)
  {

    bzero(&ReceiveBuffer, BUFFERSIZE*8);
    int nLength = 0;

   nLength=recv(nSock,(char*)ReceiveBuffer,BUFFERSIZE*8,0);

   if(nLength !=BUFFERSIZE*8)
   {
	   cout<<"error"<<endl;
   	   break;
   }
   else
   {
    int cur=0;
    int cur2=0;
    unsigned char temp1[8];
    bitset<8> temp_miwen[8];
    bitset<64> miwen;
    bitset<64> mingwen;
    char ReceiveBuffer2[BUFFERSIZE];
    bzero(&ReceiveBuffer2,BUFFERSIZE);
    int count=0;
    while(cur<BUFFERSIZE*8)
    {    
	string b;    
    	for(int i=0;i<64;i++)
	{
		b+=ReceiveBuffer[cur];
		cur++;
	}
	miwen=b64(b);
	mingwen=des.D_DES(miwen);
    	for(int i=0;i<8;i++)
	{
		unsigned char next=0x00;
		for(int j=0;j<8;j++)
		{
			int index=64-8*i-j-1;
			int pos=8-j-1;
			if(mingwen[index])
				next|=(1<<pos);
		}
		ReceiveBuffer2[cur2]=next;
		cur2++;
	}
    }

    if(ReceiveBuffer2[0]!=0&&ReceiveBuffer2[0]!='\n')
    {
     cout<<"Receive message from "<<pRemoteName<<":"<<port<<" : "<<ReceiveBuffer2;
     if(0==memcmp("quit",ReceiveBuffer2,4))
     {
      cout<<"Quit"<<endl;
      break;
     }
    }
   }
  }
 }
 else                //父线程发送消息
 {
  while(1)
  {
   bzero(&SendBuffer, BUFFERSIZE);
   while(SendBuffer[0]==0)
   {
    if (fgets(SendBuffer, BUFFERSIZE, stdin) == NULL)                
    {
     continue;
    }
   }
   
    int cur3=0;
    int cur4=0;
    unsigned char temp2[8];
    bitset<8> temp_mingwen[8];
   
    bitset<64> miwen2;
    bitset<64> mingwen2;
    string result;
    char SendBuffer2[BUFFERSIZE*8];
    while(cur3<BUFFERSIZE)
    {
	    string c;
    	for(int i=0;i<8;i++)
	{
		temp2[i]=SendBuffer[cur3];
		cur3++;
		temp_mingwen[i]=bitset<8>((unsigned)temp2[i]);
		c.append(temp_mingwen[i].to_string());
	}
	mingwen2=b64(c);
	miwen2=des.E_DES(mingwen2);
	result.append(miwen2.to_string());
    }

	for(int i=0;i<BUFFERSIZE*8;i++)
	{
		SendBuffer2[i]=result[i];
	}
    send(nSock,SendBuffer2, BUFFERSIZE*8,0);

  // cout<<"message:"<<result<<endl;
  //cout<< "length"<<result.length()<<endl;
    if(memcmp("quit",SendBuffer,4)==0)
    {
        cout<<"Quit"<<endl;
        break;       
    }
   
  }
 }
}


