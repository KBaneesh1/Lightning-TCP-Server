#include<iostream>
#include<fstream>

using namespace std;
int main(){
    
    ofstream outfile("example.txt");
    if(!outfile.is_open()){
        cerr<<"error"<<endl;
    }
    if(outfile.fail()){
        cout<<"in fail\n";
    }
    outfile << "hellowordls"<<endl;
    outfile.close();

    return 0;
}