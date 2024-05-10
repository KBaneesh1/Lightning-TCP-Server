#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <pthread.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unistd.h>
#include <fstream>
#include<dirent.h>
#include <sstream> 
#include<list>

using namespace std;

class LRUCache {
    private:
        unordered_map<string, pair<string, list<string>::iterator>> cache;
        list<string> lruList;
        size_t capacity;

    public:
        LRUCache(size_t cap) : capacity(cap) {}

        string read(string key) {
            // key here is filename
            if (cache.find(key) == cache.end()) {
                return "NULL"; // Not found
            }
            // Move accessed key to the front of the list (most recently used)
            lruList.splice(lruList.begin(), lruList, cache[key].second);
            return cache[key].first;
        }

        string write(const string& key, const string& value) {
            if (cache.find(key) != cache.end()) {
                // If key exists, update the value and move it to the front
                pthread_mutex_lock(&cacheMutex);
                cache[key].first = value;
                lruList.splice(lruList.begin(), lruList, cache[key].second);
                pthread_mutex_unlock(&cacheMutex);
                writeFile(key, value);
                return;
            }

            // If cache is full, remove the least recently used item
            if (cache.size() >= capacity) {
                string lruKey = lruList.back();
                lruList.pop_back();
                cache.erase(lruKey);
            }

            // Add the new key-value pair to the cache
            lruList.push_front(key);
            pthread_mutex_lock(&cacheMutex);
            cache[key] = {value, lruList.begin()};
            pthread_mutex_unlock(&cacheMutex);
            writeFile(key, value);
        }

        // Display the contents of the cache
        void display() {
            cout << "Cache Contents:" << endl;
            for (const auto& entry : cache) {
                cout << entry.first << ": " << entry.second.first << endl;
            }
            cout << endl;
    }
};
LRUCache db_cache(3);

int CreateSocket(int portno) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portno);

    if (bind(sfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind");
        close(sfd);
        return -1;
    }

    if (listen(sfd, 5) == -1) {
        perror("listen");
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
        perror("accept");
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

string readFile(string filename){    
    ifstream file(filename);
    // cout<<"filename in getfile "<<filename<<endl;
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
    return final;
}


void writeFile(string filename, string content) {
    FILE *file = fopen(filename.c_str(),"w");
    if (!file) {
        cout<<"error updating file\n";
        //send(clientSocket, "ERROR", 5, 0);
    } else {
        cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
        cout<<"writing vontent "<< content<<endl;
        if(content!="" and content!="\n")
        {
            fwrite(content.c_str(),sizeof(char),content.size(),file);
        }
        fclose(file);
    }
}

void* HandleClient(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[1024];
    ssize_t bytesReceived;
    // cout<<arg<<" "<<bytesReceived<<endl;;
    while(1){
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1) {
            // cout<<"nere"
            close(clientSocket);
            return NULL;
        }

        string temp = "";
        vector<string> msg;
        // cout<<"* "<<bytesReceived<<endl;
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
        // for(int i=0;i<len;i++){
        //     cout<<"mesage = "<<msg[i]<<endl;
        // }
        
        // cout<<"length = "<<len<<endl;
        while (i < len) {
            cout<<msg[i]<<endl;
            if(msg[i]=="GET_FILES"){
                string response=listTextFiles("./server/text_files/");
                // i++;
                send(clientSocket,response.c_str(),response.size(),0);
            }
            else if(msg[i]=="GET_FILE"){
                // cout<<"in get file\n";
                sleep(0.1);
                string filename="./server/text_files/";
                filename =(i+1)<len?(filename+msg[++i]):"NULL";
                string exact_file = (i+1)<len?(msg[++i]):"NULL";
                string final;
                final = db_cache.read(exact_file);
                if(final == "NULL") {
                    final = readFile(filename);
                }
                send(clientSocket, final.c_str(), final.size(), 0);
            }

            else if(msg[i]=="UPDATE_FILE"){
                string filename="./server/text_files/";
                filename =(i+1)<len?(filename+msg[++i]):"NULL";
                string content="";

                while(i+1<len and msg[i+1]!="UPDATE_FILE" and msg[i+1]!="GET_FILE" and msg[i+1]!="GET_FILES" and msg[i+1]!="\n"){
                    cout<<"* "<<msg[i+1]<<endl;
                    content+=msg[++i]+"\n";
                }
                db_cache.write(filename, content);
                // cache will call file writing function on its own
                
            }
            if (msg[i] == "END") break;

            i++;
        }
        string result = "\n";
        send(clientSocket, result.c_str(), result.size(), 0);
    }
    close(clientSocket);
}

int main(int argc, char** argv) {
    int portno;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    portno = atoi(argv[1]);
    // cout << "Server port: " << portno << endl;

    int serverSocket = CreateSocket(portno);

    while (1) {
        int clientSocket = AcceptConnection(serverSocket);
        if (clientSocket != -1) {
            pthread_t serial;
            pthread_create(&serial, NULL, HandleClient, (void*)&clientSocket);
            pthread_join(serial, NULL); // Wait for the thread to finish before accepting the next connection
        }
    }

    close(serverSocket); // Close server socket
    return 0;
}
