# myCSmith

## Build

cd \<csmith-root\> && cmake . && make

## Run

`$ <csmith-root>/src/csmith`

can specify output file by -o option

## reduce
execute `./reduce.pl` will reduce codes in "problems" directory automatically based on the "base.out" file,
you can execute `reduceone.kb <target> <compiler option> <outputpath>` to reduce only one target source file.
Remeber to use absolute path.
