/*
hPipe
int main(){

fn1
fn2
fn3
close_pipe
}

fn1{
create named file 
connect
read file for NMAX_NUM times
    break
}

fn2 {
processMessages()
call 2 threads to do it
store result in shared_arr
}

fn (){
do WriteFile in while loop for NMAX_NUM times
    break
}

Producer is producing random numbers and sending through IPC(message queue Named pipe) to consumer, Consumber doing arithmatic 
operations using thread and storing it in an array with id and result and sending it back to producer. Producer checking the result using checksum

*/