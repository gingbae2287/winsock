#include <thread>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
// g++ -o server.exe server.cpp -lws2_32 -Wall

#define PORT 7777
#define PACKET_SIZE 1024
#define MAX_CLIENT 16

SOCKET clientSock[MAX_CLIENT];
SOCKADDR_IN clientAddr[MAX_CLIENT];
bool client[MAX_CLIENT]={false};

SOCKET hListen;

char buffer[PACKET_SIZE] = {};


bool rooms[8];
/*
void proc_recvs(){
	

	while(!WSAGetLastError()){
		ZeroMemory(&buffer,PACKET_SIZE);
		recv(Client, buffer, PACKET_SIZE,0);
        if(!isChat) break;
        if(!strcmp(buffer,"/q")) {
            printf("other exited\n\n");
            break;
        }
		printf("recv: %s\n", buffer);
	}
}
void Chat(){
    isChat=true;
    
    cout<<"<<Chat mode>>\n\n";

    thread proc2(proc_recvs);

    char cMsg[PACKET_SIZE]={};
    while(1){
        printf(">>");
        cin.getline(cMsg,PACKET_SIZE);
        
        send(hClient, cMsg, strlen(cMsg),0);
        if(!strcmp(cMsg,"/q")) {
            //cout<<"exit"<<endl;
            break;
        }
        
    }
    isChat=false;
    proc2.join();
    cout<<"exit"<<endl;
    
}
*/
/*
void ReversShell(){
    
    cout<<"<<ReversShell>>\n\n";
    //===reverse shell====
    string str;
    char buf[PACKET_SIZE],rev[10000];
    char titleset[100];
    sprintf(titleset, "client connected: %s", inet_ntoa(tClntAddr.sin_addr));
    SetConsoleTitle(titleset);
    while(1){
        ZeroMemory(buf,PACKET_SIZE);
        ZeroMemory(rev,10000);
        cout << "Shell>>";
        getline(cin,str);
        
        for(int i=0;i<str.size();i++) buf[i] = str[i];        //string -> char[]로 변환
        //종료
        if(!strcmp(buf,"/q")) 
        {
            send(hClient,buf,sizeof(buf),0);
            break;
        }
        send(hClient,buf,sizeof(buf),0);
        recv(hClient,rev,10000,0);
        cout << rev << endl;
    }
    //==============
}
*/

void ClientPacket(int clientIdx){
    int packetSize=0;
    int sResult=0;
    while(!WSAGetLastError()){
        char buff[PACKET_SIZE]={};
        packetSize=recv(clientSock[clientIdx],buff,PACKET_SIZE,0);
        if(packetSize==SOCKET_ERROR || packetSize==0){
            //printf("error %d\n",packetSize);
            break;
        }
        if(!strcmp(buff,"/q")) break;
        printf("%d client packet: %s\n", clientIdx, buff);
        
        for(int i=0;i<MAX_CLIENT;i++){
           
            if(client[i]) {
                
                sResult= send(clientSock[i], buff, strlen(buff),0);
                if(sResult== SOCKET_ERROR) printf("%d clnt send error %d\n",i, WSAGetLastError());
            }
        }
        
    }
    client[clientIdx]=false;
    printf("client %d is disconnected\n",clientIdx);
    closesocket(clientSock[clientIdx]);
}
int main() {
    WSADATA wsaData;
    int iResult;
    char name[100]={};

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        cout <<"WSAStartup failed: "<< iResult<<endl;
        return 1;
    }

    
    hListen=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //PF_INET : ipv4    sock_strean: 연결지향형(tcp)

    SOCKADDR_IN tListenAddr={};
    tListenAddr.sin_family=AF_INET;
    tListenAddr.sin_port=htons(PORT); //h to n s(short) : 빅엔디안으로 데이터를 변환
    tListenAddr.sin_addr.s_addr=htonl(INADDR_ANY);  //h to n l : h->host, to, n->network l->long
    //서버addr 서버를 구현하면 서버 ip, INADDR_ANY: 현재 컴퓨터 ip (서버없이 구현중)

    bind(hListen , (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));

    while(1){
        listen(hListen, SOMAXCONN); //SOMAXCONN 요청 가능한 최대 접속 승인수
        
        for(int i=0;i<MAX_CLIENT;i++){
            if(client[i]==false){
                
                int iClntSize=sizeof(clientAddr[i]);
                clientSock[i]=accept(hListen, (SOCKADDR*)&clientAddr[i], &iClntSize);
                cout << "client connected to server(IP >> " << inet_ntoa(clientAddr[i].sin_addr) << ")\n" << endl;
                client[i]=true;
                thread client(ClientPacket,i);
                client.detach();
                break;
            }
        }
    }
    //thread ClientConnect(ClientConnect);
    /*
    while(1){
        char idx[2]={};
        recv(hClient,idx,sizeof(idx),0);
        cout<<idx<<endl;
        if(!strcmp(idx,"1")) Chat();
        else if(!strcmp(idx,"2")) ReversShell();
        else if (!strcmp(idx,"0")) {
            cout<<"quit\n\n";
            break;
        }
        else{
            cout<< "worng connection\n\n";
        }
    }
    */

    

    
    /*
    
    */

    //closesocket(hClient);
    //ClientConnect.join();
    closesocket(hListen);

    WSACleanup();
    return 0;
}