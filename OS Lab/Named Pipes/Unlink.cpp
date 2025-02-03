#include <unistd.h> 
#include <string.h> 
#include <fcntl.h> 
#include <iostream> 
#include<sys/wait.h>
#include <sys/stat.h> 
using namespace std; 
int main(){
    char str[256] = "hello world"; 
    int f1, f2;
    f1 = mkfifo("pipe_one" ,0666); 
    if ( f1 <0)
    cout<<"Pipe one not created"<<endl;
    else
    cout<<"Pipe one created"<<endl; 
    f2 = mkfifo("pipe_two" ,0666);
    if ( f2 <0)
    cout<<"Pipe two not created"<<endl;
    else
    cout<<"Pipe two created"<<endl;
    // removing pipes
    //remove pipe_one 
   // unlink("pipe_one"); 
    unlink ("pipe_two" ) ; // removing pipe_two 
    return 0;
}