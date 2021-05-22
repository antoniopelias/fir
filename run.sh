#make clean
#make
file=tests/C-01-28-N-ok
echo $file
./fir --target asm $file.fir
yasm -felf32 -o $file.o $file.asm 
ld -m elf_i386 -o $file $file.o -L$HOME/compiladores/root/usr/lib -lrts
./$file
