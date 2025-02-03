#include <unistd.h> 
#include <string.h>
#include <fcntl.h> 
#include <iostream> 
using namespace std ; 

int main(){
    char str[256] = "hello world";
    int fifo_read = open ("pipe_one" , O_RDONLY); 
    if ( fifo_read < 0){
        cout << "Error while opening file " << endl;
    }
    else {
        while(strcmp(str ,"abort")!=0){ 
            read(fifo_read , str , sizeof(str )); 
            cout<<"Text: "<<str<<endl;
        } 
        close(fifo_read);
    }
}