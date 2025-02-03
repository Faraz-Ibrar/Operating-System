#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <iostream> 
#include<sys/stat.h>

using namespace std ; 

int main(){
string str = "hello world";
int fifo_write ;
int f1 = mkfifo("pipe_one" ,0666);
if (f1 < 0) {
    cout << "Error while creating pipe " << endl;
}
// open "pipe_one" with WRITE only mode // and return its file descriptor 
fifo_write = open ("pipe_one" ,O_WRONLY ); 
// check if open call was successful
if ( fifo_write < 0){
     cout<<"Error opening file ";
}
else {
    while(str.compare ("abort") !=0){ 
        cout<<"Enter text: "<<endl;
        cin>>str ;
        write(fifo_write , &str , sizeof(str )); 
        cout<<"∗"<<str <<"∗"<<endl ;
}
close(fifo_write);
}
}