#include <iostream>
#include<fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

string composeMessage(string message){
    return message+"\n";
}

int connectToServer(){

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Error: Failed to create socket\n";
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); 
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error: Failed to connect to server\n";
        close(clientSocket);
        return 1;
    }

    const char *message = "HELLO\n";
    int bytesSent = send(clientSocket, message, strlen(message), 0);
    if (bytesSent < 0) {
        cerr << "Error: Failed to send data to server\n";
        close(clientSocket);
        return 1;
    }

    
    char buffer[1024] = {0};
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead < 0) {
        cerr << 
        close(clientSocket);
        return 1;
    }

    while(1){
        cout<<"Choose the operation"<<endl;;
        cout<<"1.READ"<<endl;
        cout<<"2.WRITE"<<endl;
        cout<<"3.COUNT"<<endl;
        cout<<"4.DELETE"<<endl;
        cout<<"5.UPLOADF"<<endl;
        cout<<"6.DOWNLOADF"<<endl;;
        cout<<"7.EDITF"<<endl;

        int input;cin>>input;
        string message="";

        if(input==1){
            message+="READ\n";
            cout<<"Enter your key"<<endl;
            string key;cin>> key;
            message+=key+"\n";
            cout<<message<<endl;
            int bytes=send(clientSocket,message.c_str(),message.size(),0);
            
        }
        else if(input==2){
            message+="WRITE\n";
            cout<<"Enter your key"<<endl;
            string key;cin>> key;
            cout<<"Enter your value"<<endl;
            string value;cin>> value;
            message+=key+"\n"+value+"\n";
            int bytes=send(clientSocket,message.c_str(),message.size(),0);
        }
        else if(input==3){
            message+="COUNT\n";
            send(clientSocket,message.c_str(),message.size(),0);
        }
        else if(input==4){
            message+="DELETE\n";
            cout<<"Enter your key"<<endl;
            string key;cin>> key;
            message+=key+"\n";
            send(clientSocket,message.c_str(),message.size(),0);
        }
        else if(input==5){
            message+="UPLOADF\n";
            cout << "Enter the path of the file to upload: "<<endl;
            string filepath;
            cin >> filepath;

            ifstream file(filepath, ios::binary);
            if (!file) cout<<"cant open file"<<endl;

            string fileContent((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            fileContent+="\n";

            string filename = composeMessage(filepath.substr(filepath.find_last_of('/') + 1));
            send(clientSocket, (message+filename+fileContent).c_str(),(message+filename+fileContent).size(), 0);

        }
        else if(input==6){
            message+="DOWNLOADF\n";
            cout<<"enter the name of the file"<<endl;
            string filename;cin>>filename;
            filename+="\n";
            send(clientSocket,filename.c_str(),filename.size(),0);
        }
        else if(input==7){

        }
        sleep(1);

        recv(clientSocket, buffer, sizeof(buffer), 0);
        cout << buffer << endl;

        // Clear buffer for next iteration
        memset(buffer, 0, sizeof(buffer));

    }
}



void startProgram(int clientSocket){
    const char *message = "HELLO\n";
    int bytesSent = send(clientSocket, message, strlen(message), 0);
    if (bytesSent < 0) {
        cerr << "Error: Failed to send data to server\n";
        close(clientSocket);
        return;
    }

    char buffer[1024] = {0};
    
}

int main() {
    int clientSocket=connectToServer();
    

}
