ma="make"
if [ $2 = ${ma} ]
    then 
        make clean
fi
make
for full in tests/$1*.fir
do
    echo ${full}
    pathname=$(echo "$full" | cut -f 1 -d '.')
    exec=$(echo "$pathname" | cut -f 2 -d '/')
    ./fir --target asm ${pathname}.fir
    yasm -felf32 -o ${pathname}.o ${pathname}.asm 
    ld -m elf_i386 -o ${pathname} ${pathname}.o -L$HOME/compiladores/root/usr/lib -lrts
    ./${pathname} > tests/expected/${exec}.myout


    #show my output
    #cat tests/expected/${exec}.myout 
    #compare outputs    
    diff tests/expected/${exec}.myout tests/abad_expected/out/${exec}.out

    #clean-up
    rm ${pathname}.asm ${pathname}.o ${pathname} tests/expected/${exec}.myout
done


