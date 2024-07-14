if not exist .\build (
    mkdir .\build
)

if not exist .\build\obj (
    mkdir .\build\obj
)

gcc -c ../common_dir/common_resource.c -o .\build\obj\common_resource.o -I ../common_dir/

gcc -c producer.c -o .\build\obj\producer.o -I ../common_dir/

gcc -o ./build/producer .\build\obj\common_resource.o .\build\obj\producer.o
