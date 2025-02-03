# include <unistd.h>
# include <stdlib.h>
# include <iostream>
# include <sys/wait.h>
# include <sys/stat.h>
using namespace std ;
int main ( ) {
int f1 , f2 ;
// c r e a t e a p i p e wi t h name " pi p e _ o n e " and s e t p e r m i s s i o n s t o 0666
f1 = mkfifo( " pipe_one " , 0666 ) ;
// c h e c k i f m k f i f o c a l l was s u c c e s s f u l
if ( f1 <0 )
cout << " Pipe one no t c r e a t ed "<<endl ;
else
cout << " Pipe one c r e a t ed "<<endl ;
f2 = mkfifo ( " pipe_two " , 0666 ) ;
if ( f2 <0 )
cout << " Pipe two no t c r e a t ed "<<endl ;
else
cout << " Pipe two c r e a t ed "<<endl ;
return 0 ;
}