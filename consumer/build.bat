if not exist .\build (
    mkdir .\build

)

if not exist .\build\obj (
    mkdir .\build\obj
)

gcc -c ../common_dir/common_resource.c -o .\build\obj\common_resource.o -I ../common_dir/

gcc -c consumer.c -o  .\build\obj\consumer.o -I ../common_dir/

gcc -o  ./build/consumer  .\build\obj\common_resource.o  .\build\obj\consumer.o
