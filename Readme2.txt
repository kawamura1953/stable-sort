ssortは、mps(multi partition sort) を間接ソートとして実行して、
   その中で必要になるソートとしてqs9の改造版を使用したものです。



●mpsとは
   mpsは、サンプル(m個)をソートして、区間(2m+1個)のそれぞれの先頭位置を計算して、
   その区間内の位置に各要素を移動して、最後にソートが必要な区間(m+1個)をソートします。
   間接ソートとしてのmpsの動作は、ss14g1.cの先頭部分のssort()の説明を参照。



●マージソート(glibcのqsort) と ssort の比較

マージソートでは、社員番号のようにキーが同値をもたないとき、
「比較関数の呼出し回数」が少なくなります。（伝統的qsortに比べてほんの僅かですが）
その代償として、県名のようにキーが同値をもつとき、
「比較関数の呼出し回数」と「要素の移動回数」がともにqsortより多くなります。

bench-sample.txtにあるようにssortはマージソートより高速です。

速さだけでなく、必要な作業領域にも大きな差があります。
マージソートでは概算で (8+8)*n バイト必要です。（sizeof(char*)=8,要素数=n）
ssortなら (8+2)*n バイトで済みます。（sizeof(short int)=2）



１．各ファイルの概略

ss14g1.c     : ssort関数本体。実験時は、"cc -Dssort=qsort -c ss14g1.c" で関数名をqsortへ変更。
qs_glibc.c   : glibcライブラリのqsortを比較実験用に改造したもの。（実はマージソート）
main_prog.c  : 比較回数・代入回数・処理時間などを計測して表示するプログラム。
benchmark.sh : ベンチマークテストを行うシェルスクリプト。
　　　　　　　 コンパイルオプション -DDEBUG を指定すれば、要素の移動回数を測定できる。



２．qs6 qs7 qs9 の概要

qsortにもいろいろな方式がある。
配列を２つに分割するキー値を持つ要素を分割要素（ピボット）と呼ぶ。
以下の例では、先頭の(5)が分割要素に選ばれたものとする。
各qsortは矢印(→)左の配列を矢印右のようにする。「|」は分割した位置を示している。


２．１．処理の概要

従来型  565238 → 532|568   2番目の(5)が右に移動している。「(5)不移動」の実装は異常事態を招く。（※１）
ｑｓ６  565238 → 5352|68   従来型は2つの(5)が左右に別れるが、qs6では必ず片方に集める。次に 235|5|68 とする。
ｑｓ７  565238 → 55|32|68  すべての(5)を一旦左右に集める。普通は左右両端に(5)がくる。 次に 32|55|68 とする。
ｑｓ９  565238 → 23|55|68  分割要素と同じキー値を持つ要素を中央に集める。移動できなくなった後に工夫あり。


２．２．各qsortの特徴

従来型  少し遅い。異常事態を起こす間違った実装が過去にいくつもあった。（※１）
ｑｓ６  要素の比較回数が多く、移動回数が少ない。平均するとｑｓ７と同じか少し速い。
ｑｓ７  要素の比較回数が少なく、移動回数が多い。キーが２値(男女など)の場合、要素の移動回数が激増する。
ｑｓ９  多くの場合、比較回数・移動回数が最小となる。


２．３．大域変数パラメータ

_QS_MID1  要素数n <= _QS_MID1(既定値 90) で３点処理を行う。
_QS_MID2  要素数n <= _QS_MID2(既定値200) で９点処理を行う。それ以外は２７点処理。

9点処理とは、配列から9個の要素を選び、その中央値(それに近いもの)を探して分割要素とする処理。



３．ベンチマークテストを行うプログラム　main_prog.c

main_prog.c は clock()関数を用いて実行時間を測定する。
簡易版の「ソートの正しさ検査」を行っている。正式な「ソートの正しさ検査」は別に実施している。
main_prog.c は次のパラメータを指定して実行する。

引数１　キー値の種類を指定する 0:定数 -1:昇順 -2:降順 -3:同値なし乱数 1:乱数 d>=2:乱数%d
引数２　配列の要素数
引数３　配列の要素サイズ(byte数)(4以上かつ4の倍数であること)　(0ならsizeof(char*)となる)
引数４　ソートの繰り返し回数（繰り返し毎に配列の要素のキー値は異なる）
引数５　_QS_MID1 の値 (-1のときは既定値を使用)
引数６　_QS_MID2 の値 (-1のときは既定値を使用)
引数７　_QS_MID3 の値 (-1のときは既定値を使用)　ssortでは使用しない。
引数８　比較関数の重さを調整する数値。大きいほど比較関数が重たくなる。  

main_prog.c 1回の実行で、実行結果を1行出力する。1行は13項目ある。例と意味を次に示す。

    qs9i5         d=-3         e=10000     s=8       R10000      M300:300:8:0:
　プログラム名  キー値の種類  要素の個数  大きさ   繰り返し回数　引数５～８の値

    c=130260    1302608733    a=65855     658555016    i=196115   T=19.06   191  
　平均比較回数  全比較回数  平均代入回数  全代入回数  比較＋代入  処理秒数  平均処理時間(10μ秒単位)

これをstdoutに出力する。（前半の６つと最後の２つはstderrにも出力する）
「 平均代入回数  全代入回数  比較＋代入 」の3項目は、全代入回数>0 のときだけ出力する。

