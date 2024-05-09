#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unistd.h>
#include <fstream>
#include<dirent.h>
#include <sstream> 



//FINAL
using namespace std;

pthread_mutex_t dbMutex;
pthread_mutex_t qMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

const int threadCount = 4;
queue<int> clients;
vector<int> editClients;

class Database {
    private:
    unordered_map<string, string> db;

    public:
    string write(string key, string value) {
        pthread_mutex_lock(&dbMutex);
        db[key] = value;
        pthread_mutex_unlock(&dbMutex);
        return "FIN";
    }

    string read(string key) {
        unordered_map<string, string>::iterator iter = db.find(key);
        if (iter == db.end()) return "NULL";
        return iter->second;
    }

    int count() {
        return db.size();
    }

    string remove(string key) {
        // cout<<key<<endl;
        pthread_mutex_lock(&dbMutex);
        if(db.find(key) != db.end()) {
        // cout<<db[key]<<endl;
        bool done = db.erase(key);
        pthread_mutex_unlock(&dbMutex);
        return "FIN";
      }
   
        pthread_mutex_unlock(&dbMutex);
        return "NULL";
    
    }
};

Database db;

int CreateSocket(int portno) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    int iSetOption = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,sizeof(iSetOption));

    if (sfd == -1) {
        // perror("socket");
        return -1;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portno);

    if (bind(sfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        // perror("bind");
        close(sfd);
        return -1;
    }

    if (listen(sfd, 5) == -1) {
        // perror("listen");
        close(sfd);
        return -1;
    }

    return sfd;
}

int AcceptConnection(int serverSocket) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
    if (clientSocket == -1) {
        // perror("accept");
        return -1;
    }
    return clientSocket;
}

string listTextFiles(const string& directoryPath) {
    string fileList;

    // Open directory
    DIR* dir = opendir(directoryPath.c_str());
    if (dir == nullptr) {
        cerr << "Error: Unable to open directory " << directoryPath << endl;
        return fileList;
    }

    // Read directory entries
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string filename = entry->d_name;
        if (filename != "." && filename != "..") {
            if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".txt") {
                fileList += filename + "\n";
            }
        }
    }

    // cout<<fileList<<endl;;
    // Close directory
    closedir(dir);

    return fileList;
}


void* HandleClient(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[1024];
    ssize_t bytesReceived;
    // cout<<arg<<" "<<bytesReceived<<endl;;
    while(1){
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1) {
            perror("recv");
            close(clientSocket);
            return NULL;
        }

        string temp = "";
        vector<string> msg;
        cout<<"* "<<bytesReceived<<endl;
        for (int i = 0; i < bytesReceived; i++) {
            // cout<<temp<<endl;
            if (buffer[i] != '\n') temp += buffer[i];
            else {
                msg.push_back(temp);
                temp = "";
            }
        }

        int i = 0;
        int len = msg.size();
        for(int i=0;i<len;i++){
            cout<<"mesage = "<<msg[i]<<endl;
        }
        
        cout<<"length = "<<len<<endl;
        while (i < len) {
            if(msg[i]=="GET_FILES"){
                string response=listTextFiles("./server/text_files/");
                // i++;
                send(clientSocket,response.c_str(),response.size(),0);
            }
            else if(msg[i]=="GET_FILE"){
                cout<<"in get file\n";
                sleep(0.1);
                string filename="./server/text_files/";
                filename =(i+1)<len?(filename+msg[++i]):"NULL";
                
                ifstream file(filename);
                cout<<"filename in getfile "<<filename<<endl;
                if(!file.is_open()){
                    cerr<<"error opening in get file"<<endl;
                }
                string fileContents;
                string line;
                //while(getline(file,line)){
                //    fileContents += line;
                //}
                stringstream buffer;
                buffer << file.rdbuf();
                fileContents = buffer.str();
                file.close();
                string final;
                final = "RESPONSE\n"+fileContents+"\nEND";
                //cout<<"buffer = "<<file<<endl;
                //fileContents = buffer.str();
                cout<<"sent "<<fileContents<<endl;

                send(clientSocket, final.c_str(), final.size(), 0);
            }
            else if(msg[i]=="UPDATE_FILE"){
                string filename="./server/text_files/";
                filename =(i+1)<len?(filename+msg[++i]):"NULL";
                FILE *file = fopen(filename.c_str(),"w");
                
                 cout<<"filename in update"<<filename<<endl;
                if (!file) {
                    cout<<"error updating file\n";
                    //send(clientSocket, "ERROR", 5, 0);
                } else {
                    string content="";
                    while(i+1<len)
                        content += msg[++i]+"\n";
                    //while(i+1<len)
                     //   content += (msg[++i]+"\n");
                    cout<<"sontent = "<<content<<endl;
                    if(content!="")
                    {
                        fwrite(content.c_str(),sizeof(char),content.size(),file);
                    }
                    fclose(file);
                    cout<<"Successfully written\n";
                    //send(clientSocket, "SUCCESS", 7, 0);
                }
            }
            if (msg[i] == "CLOSE_CLIENT"){
                close(clientSocket);
                return NULL;
            }
            i++;
        }
        string result = "\n";
        send(clientSocket, result.c_str(), result.size(), 0);
    }
    close(clientSocket);
    return NULL;
}
void print(queue<int> clients){
    cout<<"in print queue\n";
    while(!clients.empty()){
        cout<<clients.front()<<" ";
        clients.pop();
    }
    cout<<endl;
}


void* HandleQueue(void* arg) {
    pthread_t threadId = pthread_self();
    cout<<"in thread = "<<threadId<<endl;
    while (1) {
        cout<<"in this thread loop = "<<threadId<<endl;
        pthread_mutex_lock(&qMutex);
        if (clients.empty()) {
            pthread_cond_wait(&cv, &qMutex);
        }
        // Pop client socket from the queue
        int clientSocket = clients.front();
        //cout<<"client socker = "<<clientSocket<<" handled by "<<threadId<<endl;
        clients.pop();
        //print(clients);
        pthread_mutex_unlock(&qMutex);
        // Process client
        HandleClient(&clientSocket);
    }
}



void* AcceptConnections(void* arg) {
    int serverSocket = *((int*)arg);
    while (1) {
        int clientSocket = AcceptConnection(serverSocket);
        pthread_mutex_lock(&qMutex);
        clients.push(clientSocket);
        //print(clients);
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&qMutex);
    }   
}

int main(int argc, char** argv) {
    int portno;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    portno = atoi(argv[1]);

    pthread_t threadPool[threadCount];

    for (int i = 0; i < threadCount; i++) {
        cout<<"thread no "<<i<<endl;
        pthread_create(&threadPool[i], NULL, HandleQueue, NULL);
    }

    int serverSocket = CreateSocket(portno);

    pthread_t acceptThread;
    pthread_create(&acceptThread, NULL, AcceptConnections, (void*)&serverSocket);

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threadPool[i], NULL);
    }

    pthread_join(acceptThread, NULL);

    close(serverSocket);

    return 0;
}