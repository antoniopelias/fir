ma="make"
if [ $2 = ${ma} ]
    then 
        make clean
fi
make
clear
for firname in tests/$1*.fir
do
    echo ${firname}
    name=$(echo "$firname" | cut -f 1 -d '.')
    singlename=$(echo "$name" | cut -f 2 -d '/')
    ./fir --target asm ${name}.fir
    yasm -felf32 -o ${name}.o ${name}.asm 
    ld -m elf_i386 -o ${name} ${name}.o -L$HOME/compiladores/root/usr/lib -lrts
    ./${name} > tests/expected/${singlename}.myout
    tr -d '\n' < tests/expected/${singlename}.myout > tests/expected/${singlename}.nonlout
    echo >> tests/expected/${singlename}.nonlout
    cat tests/expected/${singlename}.myout
    diff tests/expected/${singlename}.nonlout tests/expected/${singlename}.out
    rm tests/expected/${singlename}.myout
    rm ${name}.asm ${name}.o ${name} tests/expected/${singlename}.nonlout
    echo
done


