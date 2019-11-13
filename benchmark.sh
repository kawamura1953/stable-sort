#!/bin/sh

cc  -O               -w                 -c qs_glibc.c   # -DDEBUG で代入回数を計測
cc  -O  -o qs_glibc  -w   main_prog.c      qs_glibc.o

cc  -O               -w  -Dssort=qsort  -c ss14g1.c     # -DDEBUG で代入回数を計測
cc  -O  -o ss14g1    -w   main_prog.c      ss14g1.o

#cc -O  -o qsort     -w   main_prog.c   # この行で、自システム用のプログラムを生成

prin=benchmark

echo '----------------- benchmark.txt begin -------------------- '    >>$prin.txt
 echo ' '                                                             >>$prin.txt
 echo 'キー種別:乱数　要素数:1万個　要素サイズ:8,20,400byte '         >>$prin.txt
 time ./qs_glibc    -3    10000     0       5000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1      -3    10000     0       5000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc    -3    10000    20       4000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1      -3    10000    20       4000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc    -3    10000   400       2000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1      -3    10000   400       2000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 echo 'キー種別:100種　要素数:1万個　要素サイズ:8,20,400byte '        >>$prin.txt
 time ./qs_glibc   100    10000     0       6000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1     100    10000     0       6000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc   100    10000    20       5000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1     100    10000    20       5000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc   100    10000   400       2000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1     100    10000   400       2000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 echo 'キー種別:2種　要素数:1万個　要素サイズ:8,20,400byte '          >>$prin.txt
 time ./qs_glibc     2    10000     0       8000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1       2    10000     0       8000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc     2    10000    20       8000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1       2    10000    20       8000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc     2    10000   400       3000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1       2    10000   400       3000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 echo ' '                                                             >>$prin.txt
echo 'キー種別:10種　要素数:1万,10万,100万個　要素サイズ:1000byte '   >>$prin.txt
 time ./qs_glibc    10    10000  1000       2000   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1      10    10000  1000       2000   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc    10   100000  1000        100   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1      10   100000  1000        100   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
 time ./qs_glibc    10  1000000  1000         10   -1  -1  -1  0      >>$prin.txt
 time ./ss14g1      10  1000000  1000         10   -1  -1  -1  0      >>$prin.txt
 echo ' '                                                             >>$prin.txt
echo '=================  benchmark.txt end  ==================== '    >>$prin.txt
echo '各行の最後の数値がソート１回あたりの処理時間(10μ秒単位)です '  >>$prin.txt
