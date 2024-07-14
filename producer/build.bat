if not exist .\build (
    mkdir .\build
)
gcc -c ../common_dir/myheader.c -o myheader.o -I ../common_dir/

gcc -c producer.c -o producer.o -I ../common_dir/

gcc -o ./build/producer myheader.o producer.o
