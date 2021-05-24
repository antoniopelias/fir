#make clean
#make
#file=tests/E-01-59-N-ok
ma="make"

if [ $2 = ${ma} ]
    then 
        make clean
        make
fi
clear

for firname in tests/$1*.fir
do
    echo
    echo ${firname}
    name=$(echo "$firname" | cut -f 1 -d '.')
    singlename=$(echo "$name" | cut -f 2 -d '/')
    #echo ${name}
    ./fir --target asm ${name}.fir
    yasm -felf32 -o ${name}.o ${name}.asm 
    ld -m elf_i386 -o ${name} ${name}.o -L$HOME/compiladores/root/usr/lib -lrts
    ./${name} > tests/expected/${singlename}.myout
    diff -w tests/expected/${singlename}.myout tests/expected/${singlename}.out
    rm ${name}.asm ${name}.o ${name} tests/expected/${singlename}.myout
done


