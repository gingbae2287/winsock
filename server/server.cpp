#include <thread>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
// g++ -o server.exe server.cpp -lws2_32 -Wall

#define PORT 7777
#define PACKET_SIZE 1024
#define MAX_CLIENT 16

#define NAME_LEGNTH 24
#define ROOM_LEGNTH 100
#define ROOM_PER_CLIENT 8
#define ROOM_COUNT 8


SOCKET clientSock[MAX_CLIENT];
SOCKADDR_IN clientAddr[MAX_CLIENT];
bool client[MAX_CLIENT]={false};

SOCKET hListen;

char buffer[PACKET_SIZE] = {};

struct Client;
struct Room;

struct Client{
    int sockId;
    bool isConnect;
    int roomId;
    char name[NAME_LEGNTH];
    Client(){
        roomId=-1;
        isConnect=false;
    }
    bool operator==(const Client &cl1);
    void Disconnect();
    /*Client(int SockId,char Name[]="default name"){
        sockId=SockId;
        strcpy(name,Name);
        isConnect=true;
        roomId=-1;
    };*/
    ~Client(){}
};

struct Room{
    char roomName[ROOM_LEGNTH];
    //int roomId;
    bool isEmpty;
    int clientCount;
    vector<Client> clients;
    vector<Client>::iterator it;
    Room(){
        clientCount=0;
        isEmpty=true;
        clients.reserve(MAX_CLIENT);
    }
    void ClientEnter(Client &cl);
    void ClientOut(Client &cl);
    ~Room(){}
};
Room rooms[ROOM_COUNT];
Client clients[MAX_CLIENT];



void Chat(int clientIdx){
    int rResult=0;
    int sResult=0;
    char tmp[PACKET_SIZE+NAME_LEGNTH+4];

    char buff[PACKET_SIZE]={};
    while(!WSAGetLastError()){
        ZeroMemory(&buff,PACKET_SIZE);
        rResult=recv(clientSock[clientIdx],buff,PACKET_SIZE,0);
        if(rResult==SOCKET_ERROR || rResult==0){
            //printf("error %d\n",packetSize);
            break;
        }
        if(!strcmp(buff,"/q")) break;
        strcpy(tmp,clients[clientIdx].name);
        strcat(tmp,": ");
        strcat(tmp,buff);
        printf("%s\n",tmp);
        
        for(int i=0;i<MAX_CLIENT;i++){
           
            if(client[i]) {
                
                sResult= send(clientSock[i], tmp, strlen(tmp),0);
                if(sResult== SOCKET_ERROR) printf("%d clnt send error %d\n",i, WSAGetLastError());
            }
        }
        
    }
    
}

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
    int rResult=0;
    int sResult=0;
    char clntName[NAME_LEGNTH];
    
    char orderPacket;
    char buff[PACKET_SIZE];
    char roomName[ROOM_LEGNTH];

    rResult=recv(clientSock[clientIdx],clntName,PACKET_SIZE,0);
    if(rResult==SOCKET_ERROR || rResult==0){
            printf("error %d\n",rResult);
    }
    else{
        strcpy(clients[clientIdx].name,clntName);
        clients[clientIdx].isConnect=true;
        clients[clientIdx].sockId=clientIdx;
        printf("client %d's name: %s\n\n",clientIdx, clients[clientIdx].name);
    }

    while(!WSAGetLastError()){
        ZeroMemory(&orderPacket,sizeof(char));
        ZeroMemory(&buff,PACKET_SIZE);
        
        rResult=recv(clientSock[clientIdx],buff,PACKET_SIZE,0);
        if(rResult==SOCKET_ERROR || rResult==0){
            printf("tasknum recv  error \n");
            break;
        }
        printf("packet success %s\n",buff);
        orderPacket=buff[0];
        ZeroMemory(&buff,PACKET_SIZE);
        
        sResult=send(clientSock[clientIdx], &orderPacket, sizeof(char),0);
        if(sResult==SOCKET_ERROR || sResult==0){
                printf("error \n");
                break;
        }
        int order=orderPacket-48;

        if(order==1){
            //room list
            ZeroMemory(&buff,PACKET_SIZE);
            //ZeroMemory(&roomName,ROOM_LEGNTH);
            for(int i=0;i<ROOM_COUNT;i++){
                if(!rooms[i].isEmpty) {
                    /*sResult= send(clientSock[clientIdx], rooms[i].roomName, strlen(rooms[i].roomName),0);
                    if(sResult==SOCKET_ERROR || sResult==0){
                        printf("order send error %s\n",WSAGetLastError());
                    }*/
                    char idx=i+1+48;
                    strncat(buff,&idx,1);
                    strcat(buff,". ");
                    strcat(buff,rooms[i].roomName);
                    strcat(buff,"\n");
                }
            }
            strcat(buff,"=======================\n\n");
            sResult= send(clientSock[clientIdx], buff, PACKET_SIZE,0);
            if(sResult==SOCKET_ERROR || sResult==0){
                    printf("roomlist send error \n");
                    break;
            }
            printf("<<<room list>>:\n%s",buff);

        }
        else if(order==2){
            //create room
            for(int i=0;i<ROOM_COUNT;i++){
                if(rooms[i].isEmpty){
                    ZeroMemory(&roomName,100);
                    rResult=recv(clientSock[clientIdx],roomName,PACKET_SIZE,0);
                    //printf("input: %s\n",roomName);
                    if(rResult==SOCKET_ERROR || rResult==0){
                        printf("error %d\n",rResult);
                        break;
                    }
                    rooms[i].isEmpty=false;
                    strcpy(rooms[i].roomName,roomName);
                    printf(" create room. name: %s\n\n",rooms[i].roomName);
                    //sResult= send(clientSock[i], tmp, strlen(tmp),0);
                    break;
                }
            }
        }
        else if(order==3){
            //enter room
            char roomNum;
            rResult=recv(clientSock[clientIdx],&roomNum,2,0);
            if(rResult==SOCKET_ERROR || rResult==0){
                printf("error %d\n",rResult);
                break;
            }
            int tmp=roomNum-48;
            if(tmp>ROOM_COUNT || tmp<1){
                printf("worng room number\n");
                strcpy(buff,"0");
                sResult= send(clientSock[clientIdx], buff, PACKET_SIZE,0);
                if(sResult==SOCKET_ERROR || sResult==0){
                    printf("enter room error\n");
                    break;
                }
            }
            else if(rooms[tmp-1].isEmpty){
                printf("room %d was not created\n",tmp);
                strcpy(buff,"0");
                sResult= send(clientSock[clientIdx], buff, PACKET_SIZE,0);
                if(sResult==SOCKET_ERROR || sResult==0){
                    printf("enter room error\n");
                    break;
                }
            }
            else{
                printf("client %d is enter room %d\n",clientIdx,tmp);
                strcpy(buff,"1");
                sResult= send(clientSock[clientIdx], buff, PACKET_SIZE,0);
                if(sResult==SOCKET_ERROR || sResult==0){
                    printf("enter room error\n");
                    break;
                }
                clients[clientIdx].roomId=tmp-1;
                rooms[tmp-1].clients.push_back(clients[clientIdx]);
            }
        }
    }
    client[clientIdx]=false;
    clients[clientIdx].Disconnect();
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
    listen(hListen, SOMAXCONN); //SOMAXCONN 요청 가능한 최대 접속 승인수
    SOCKET sock;
    SOCKADDR_IN sockAddr;
    int iClntSize=sizeof(sockAddr);
    while(1){
        sock=accept(hListen, (SOCKADDR*)&sockAddr, &iClntSize);
        
        for(int i=0;i<MAX_CLIENT;i++){
            if(client[i]==false){
                clientSock[i]=sock;
                clientAddr[i]=sockAddr;
                

                //clientSock[i]=accept(hListen, (SOCKADDR*)&clientAddr[i], &iClntSize);
                printf("client %d connected to server(IP >> %s)\n",i,inet_ntoa(clientAddr[i].sin_addr));
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
    closesocket(hListen);

    WSACleanup();
    return 0;
}


void Room::ClientOut(Client &cl){
    it=find(clients.begin(),clients.end(),cl);
    if(it!=clients.end()){
        clients.erase(it);
        return;
    }
}

void Room::ClientEnter(Client &cl){
        clients.push_back(cl);
        return;
}
bool Client::operator==(const Client &cl1){
    return (name==cl1.name);
}
void Client::Disconnect(){
        isConnect=false;
        if(roomId!=-1){
            rooms[roomId].ClientOut(*this);
            roomId=-1;
        }
        ZeroMemory(&name,NAME_LEGNTH);

       return;
    }