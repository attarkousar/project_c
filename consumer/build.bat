if not exist .\build (
    mkdir .\build

)
gcc -c ../common_dir/myheader.c -o myheader.o -I ../common_dir/

gcc -c consumer.c -o  consumer.o -I ../common_dir/

gcc -o  ./build/consumer  myheader.o  consumer.o
