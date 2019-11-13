/***************************************************/
/*                                                 */
/*          ssort (Stable sort)   ss14             */
/*                                                 */
/* by 河村 知行 (kawamura tomoyuki)    2019.11.11  */
/*  〒745-0845 山口県周南市河東町3-2 （株）曽呂利  */
/*             t-kawa@crux.ocn.ne.jp               */
/*                                                 */
/*      本ソフトウェア はパブリックドメインです    */
/***************************************************/

// ss14 の概略  (ss14は安定な比較ソート。マージソートより高速で、作業領域はより小さい)
/*
qs9_sort( ポインタ配列 ptr, nel, indirect ) {
　この関数内では size==sizeof(char*) の定数となる。
　この関数内では if (indirect>0) {比較関数はcmp_org} else {比較関数はポインタ値の引き算}
　if (nel<=4) {専用の処理。スタックのポップアップへ移動。}
　ピボット値を決定する。
　ポインタ配列ptrを　A:ピボット値未満　B:ピボット値と同じ　C:ピボット値超　に分ける。
　if (indirect==2) {B部分をその要素(ポインタ値)で直接ソート●qs9_sort(ptrのB部分, その要素数, 0)}
　A(C)の位置をスタックに積んで、C(A)の位置を新ターゲットとして、先頭に戻る。
　スタックが空ならリターンする
}

ssort( 配列 base, 要素数 nel, 要素の大きさ size, 比較関数 cmp ) {
　比較関数cmp を 大域変数cmp_org へ保存する。
　if (nel<64) {
　　baseの各要素へのポインタよりなる配列Xを作る。間接ソート●qs9_sort(X, nel, 2)を実行。
　}else{
　　baseから適当に選んだ32組の要素を比較し、比較値==0の組数をカウントする(eqcnt)。
　　nelとeqcntと事前に用意した表を用いて、mps(multi partition sort)の分割数(仮に８(=b)とする)を決定する。
　　baseから適当に選んだ７(b-1)個の要素へのポインタ配列Yを作る。間接ソート●qs9_sort(Y, 7, 1)を実行。
　　baseのすべての要素について グループ番号 1～15(1～2*b-1) を計算する。
　　　　Y[i]と同値の要素は グループ番号 2*i とし、記憶する。
　　　　Y[i]とY[i+1]の間の要素は グループ番号 2*i+1 とし、記憶する。
　　グループ番号から次のようなポインタ配列Xをつくる。Xの要素数はnel。
　　　　[グループ番号 1 の要素へのポインタの並び]...[グループ番号 2*b-1 の要素へのポインタの並び]
　　偶数番号の並び部分は、何もすることなくこの時点で安定ソートとしてソートされている。（ただし、Xの中で）
　　奇数番号の並び部分のそれぞれで、間接ソート●qs9_sort(Xの一部, その要素数, 2)を実行。
　}
　ポインタ配列Xを使って、配列baseの要素を移動する。
}
*/

int _QS_MID1= 90;  // 要素数n  <= _QS_MID1 のときは　3つの要素から分割要素を決定する（３点処理）
int _QS_MID2=200;  // 要素数n  <= _QS_MID2 のときは　9つの要素から分割要素を決定する（９点処理）

#include <stdio.h>
//typedef long unsigned int size_t;
void exit(int);
void *malloc(size_t); void free(void*);

#ifdef DEBUG
unsigned int ass_cnt;                   /*代入回数を計測しないときは、削除可能*/
#define ASS_CNT(x)  {ass_cnt += (x);}
static void assert(int x, char *s) {
  if (x) return;
  fprintf(stderr, "++++ %s ++++\n", s); printf("++++ %s ++++ \n", s);
  exit(1);
}
#else
#define ASS_CNT(x) {}
static void assert(int x, char *s) {}
#endif


//以下、「ポインタが８バイト(sizeof(char*)==8)なら、機械語の８バイト整数あり」と仮定している。
//「ポインタが４バイトなのに、機械語の８バイト整数あり」では、十分な性能はでない。

#define F(x) (*(char**)x)
#define S(x,y)   {ASS_CNT(2) char *v=F(x); F(x)=F(y); F(y)=v;}             /*mmswap ２要素のスワップ処理*/
#define K(x,y,z) {ASS_CNT(3) char *v=F(x); F(x)=F(y); F(y)=F(z); F(z)=v;}  /*mmrot3 ３要素のローテーション*/

static inline void mmswapblock( char *a, char *b, size_t nsize ) {
 ASS_CNT((nsize/sizeof(char*))*2);
 char *e = a + nsize;
 do { char *v=F(a); F(a)=F(b); F(b)=v; b += sizeof(char*); a += sizeof(char*); } while (a < e);
}

#define  MV8(i) {((long long int*)a)[i] = ((long long int*)b)[i];} 
#define  MV4(i) {          ((int*)a)[i] =           ((int*)b)[i];} 
#define  MV1(i) {                 a [i] =                  b [i];} 

#define HIGHLOW(HIGH,LOW,MOV,WS) { \
   if (HIGH) { \
     char *e = a + HIGH; \
     do {MOV(0) MOV(1) MOV(2) MOV(3) MOV(4) MOV(5) MOV(6) MOV(7)  b += 8*WS; a += 8*WS; \
     }while (a < e); \
   } \
   switch ((LOW) & 7) { \
     case 7: MOV(6)  \
     case 6: MOV(5)  \
     case 5: MOV(4)  \
     case 4: MOV(3)  \
     case 3: MOV(2)  \
     case 2: MOV(1)  \
     case 1: MOV(0)  \
     case 0: {}      \
   } \
} 

#define INT64_OK  (sizeof(char*)==8)
#define ENINT(x)  ((char*)(x) - (char*)0)
static size_t high, low;

       void *memcpy ( char * , const char * , size_t     );
static void mmfnc8  ( char *a, const char *b, size_t siz ) {                 MV8(0)}
static void mmfnc4  ( char *a, const char *b, size_t siz ) {                 MV4(0)}
static void mmfnc8n ( char *a, const char *b, size_t siz ) {HIGHLOW(high,low,MV8,8)}
static void mmfnc4n ( char *a, const char *b, size_t siz ) {HIGHLOW(high,low,MV4,4)}
static void mmfnc1n ( char *a, const char *b, size_t siz ) {HIGHLOW(high,low,MV1,1)}
static void (*mmfnc)( char *a, const char *b, size_t siz );

static void mmprepare( void *base, size_t siz ) {
 if (INT64_OK && (ENINT(base)&(8-1))==0 && (siz&(8-1))==0) {high=(siz&(-64)); low=(siz&(64-1))/8;
                                                            if (siz == 8  ) {mmfnc=mmfnc8 ; return;}
                                                            if (siz <= 608) {mmfnc=mmfnc8n; return;}
 }else if (      (ENINT(base)&(4-1))==0 && (siz&(4-1))==0) {high=(siz&(-32)); low=(siz&(32-1))/4; 
                                                            if (siz == 4  ) {mmfnc=mmfnc4 ; return;}
                                                            if (siz <= 532) {mmfnc=mmfnc4n; return;}
 }else                                                     {high=(siz&( -8)); low=(siz&( 8-1))/1;
                                                            if (siz <= 60 ) {mmfnc=mmfnc1n; return;}
 }
 mmfnc=(void(*)())memcpy;
}
static inline void mmcopy( char *a, char *b, size_t siz ) {
 ASS_CNT(1)
 mmfnc(a, b, siz);
}
//ここまでが要素の移動に関するもの



//ここから下は、qs9によるソートプログラムです
typedef struct { char *LLss, *RRss; } stack_node;   /*L,Rを積むスタックの構造体*/
#define PUSH(llss,rrss) {top->LLss = (llss); top->RRss = (rrss); ++top;}    /*L,Rを積む*/
#define POP(llss,rrss)  {--top; llss = top->LLss; rrss = top->RRss;}        /*L,Rを戻す*/

static int (*cmp_org)( void *a, void *b );

static inline int cmp_indirect( void *a, void *b, int indirect ) {
 if (indirect) {
   return cmp_org( *(void**)a, *(void**)b );
 }else{
   return ( *(void**)a - *(void**)b );
 }
}
#define  LT(a,b)  if ((t=cmp_indirect(a,b,indirect)) <  0) 
#define  GT(a,b)  if ((t=cmp_indirect(a,b,indirect)) >  0) 
#define  else_GT  else if (t > 0)
#define  else_LT  else if (t < 0)

static inline int cmp_indirect_ifeq( void *a, void *b, int indirect ) {
 if (indirect) {
   int t = cmp_org( *(void**)a, *(void**)b );
   return ( t ? t : *(void**)a - *(void**)b );
 }else{
   return ( *(void**)a - *(void**)b );
 }
}
#define  lt(a,b)  if (cmp_indirect_ifeq(a,b,indirect) < 0) 

//#define med3(a,b,c) \
//  ((t=cmp_indirect(a,b,indirect))<=0 ? \
//     (cmp_indirect(b,c,indirect)<=0 ? b : (t==0 ? b : (cmp_indirect(a,c,indirect)<=0 ? c : a))) : \
//     (cmp_indirect(b,c,indirect)>=0 ? b :             (cmp_indirect(a,c,indirect)<=0 ? a : c) ))
// d==-3 で速くしたいなら、下の t==0 なしの方が良い
#define med3(a,b,c) (cmp_indirect(a,b,indirect)<=0 ? \
                    (cmp_indirect(b,c,indirect)<=0 ? b : (cmp_indirect(a,c,indirect)<=0 ? c : a)) : \
                    (cmp_indirect(b,c,indirect)>=0 ? b : (cmp_indirect(a,c,indirect)<=0 ? a : c)) )

#define size (sizeof(char*))
#define I(x)     {x+=size;}
#define D(x)     {x-=size;}

static void qs9_sort( void *base, size_t nel, /*size_t size, int (*cmp)(void *a, void *b),*/ int indirect )
              /* base : ソートしようとする配列へのポインタ    */
              /* nel  : 配列baseの要素数                      */
              /* size : 配列baseの要素の大きさ（バイト単位）  */  //ss14ではqs9_sortのsizeは定数となる
              /* cmp  : 要素の大小比較をする関数へのポインタ  */
              /* indirect : 2->安定な間接sort  1->安定でない間接sort  0->ポインタ配列の直接sort*/
{
 char *L = (char*)base;                  /*分割中の区間の左端の要素の先頭*/
 char *R = &((char*)base)[size*(nel-1)]; /*分割中の区間の右端の要素の先頭*/
 char *l,*f,*m,*g,*r;                    /*左、中央、右 区間を保持するためのポインタ*/
 int  t,v,w;                             /*作業用変数*/
 int  n;                                 /*分割中の区間の要素数*/
 stack_node stack[32], *top = stack;     /*現在のところでは３２で十分*/

LOOP:
 if (L>=R) {goto nxt;}
loop:
 if (L + size == R) {if (cmp_indirect_ifeq(L,R,indirect) > 0) {S(L,R);}  goto nxt;} /*要素数２*/
 
 n = (R - L + size) / size;  /*要素数*/
 
 if (n <= 4) {               //要素数３または４　安定なsortを実行
   char *p = R - size;
   lt(L,p) lt(p,R)          {        }  // 3 5 7
           else    lt(L,R)  {S(p,R)  }  // 3 7 5
                   else     {K(L,R,p)}  // 5 7 3
                                            
   else    lt(L,R)          {S(L,p)  }  // 5 3 7
           else    lt(p,R)  {K(L,p,R)}  // 7 3 5
                   else     {S(L,R)  }  // 7 5 3

   if (n == 4) {
     m = L+size;
     lt(m,p)  lt(m,L) {S(L,m)  }        /*3-2-5-7*/
              else    {        }        /*3-4-5-7 のときは何もしない*/
     else     lt(m,R) {S(m,p)  }        /*3-6-5-7*/
              else    {K(m,p,R)}        /*3-8-5-7*/
   }
   goto nxt;
 } /* n <= 4 */



 m = L + size * (n >> 1);    /*配列の中央を計算*/

 if (n <= _QS_MID1) {        //３点処理
   l=L; r=R; f=m-size; g=m+size;
   LT(l,m) LT(m,r)         {              I(l) D(r) goto _lfgr;} // 3 5 7
           else_GT LT(l,r) {S(m,r)        I(l) D(r) goto _lfgr;} // 3 7 5
                   else_GT {K(l,r,m)      I(l) D(r) goto _lfgr;} // 5 7 3
                   else    {S(m,r)             D(r) goto _5fgr;} // 5 7 5
           else            {S(g,r)        I(l) I(g) goto _lfgr;} // 3 5 5          I(l) lfg5 もあり

   else_GT LT(l,r)         {S(l,m)        I(l) D(r) goto _lfgr;} // 5 3 7
           else_GT LT(m,r) {K(l,m,r)      I(l) D(r) goto _lfgr;} // 7 3 5
                   else_GT {S(l,r)        I(l) D(r) goto _lfgr;} // 7 5 3
                   else    {K(l,f,r)      D(f) D(r) goto _lfgr;} // 7 5 5   S(l,r) D(r) 5fgr もあり
           else            {S(l,m) S(g,r) I(l) I(g) goto _lfgr;} // 5 3 5   S(l,m) I(l) lfg5 もあり

   else    LT(m,r)         {              D(r)      goto _5fgr;} // 5 5 7
           else_GT         {K(l,r,g) I(l) I(g)      goto _lfgr;} // 5 5 3   S(l,r) I(l) lfg5 もあり
           else            {S(g,r)        I(g)      goto _5fgr;} // 5 5 5               5fg5 もあり
 }



 if (n <= _QS_MID2) { //９点処理           // t=(n>>3)*size;                配列の中央に最大(最小)近傍の
   l=med3(m-size*4, m-size*3, m-size*2);   // l=med3(L    , m-t*3, m-t*2);  要素が集まりやすいときは、
   r=med3(m+size*2, m+size*3, m+size*4);   // r=med3(m+t*2, m+t*3, R    );  右側が良い。
   m=med3(m-size  ,    m    , m+size  );   // f=med3(m-t  , m    , m+t  );  左側だと 1 + 1/log(n) 倍に
   m=med3(   l    ,    m    ,    r    );   // g=med3(l    , f    , r    );  遅くなる可能性あり。しかし
                                           // if (m!=g) {S(m,g);}           これは、悲惨な遅さではない。
   l=L; r=R; f=m-size; g=m+size;
   goto _lfgr;
 }



 {char *p,*z1,*z2,*z3; int w;            //２７点処理 
 p=m-(w=size*3)*4;  f=p+size; g=f+size; z1=med3(p, f, g);
           p+=w;    f+=w;     g+=w;     z2=med3(p, f, g);
           p+=w;    f+=w;     g+=w;     z3=med3(p, f, g); l=med3(z1, z2, z3);
           p+=w;    f+=w;     g+=w;     z1=med3(p, f, g);
           p+=w;    f+=w;     g+=w;     z2=med3(p, f, g);
           p+=w;    f+=w;     g+=w;     z3=med3(p, f, g); m=med3(z1, z2, z3);
           p+=w;    f+=w;     g+=w;     z1=med3(p, f, g);
           p+=w;    f+=w;     g+=w;     z2=med3(p, f, g);
           p+=w;    f+=w;     g+=w;     z3=med3(p, f, g); r=med3(z1, z2, z3);
 m=med3(l, m, r);
 l=L; r=R; f=m-size; g=m+size;
 goto _lfgr;
 }



/*
333...555...777　　lfgr のlの位置から比較を始める。357を貯める。 l[fg]r系　 この系は l<=f g<=r を保証
L  l f m g r  R 
mは分割要素を指す。mもその要素も1回の分割終了の直前まで、変更なし。
「5」分割要素と同キーの要素を表す。「3」5より小さいキーの要素を表す。「7」5より大きいキーの要素を表す。 
「l」333の右隣の要素を指す 「r」777の左隣の要素を指す 「L」先頭要素を指す 「R」最終要素を指す
「f」555の左隣の要素を指す 「g」555の右隣の要素を指す
「.」未比較の要素の列(長さ0以上)を表す  「_」未比較の要素1つを表す

333355555...777 になったら 333355555...777 にする。333355555333...777 として第2の「3」を貯める。
   fl    g r                  f     l r               f     g  l r   
   f<l になったら                   l=g; する。             gとlの間に「3」を貯める。 [fg]lr系  r<lもあり

333...555557777 になったら 333...555557777 にする。333...777555557777 として第2の「7」を貯める。
   l f    rg  R               l r     g  R            l r  f     g   
          r<g になったら        r=f; する。             rとfの間に「7」を貯める。     lr[fg]系  r<lもあり
*/

chk:                                          // L l f  g r R
 if (l<=f) if (g<=r) {          goto _lfgr;}  // 33...55...77
           else      {r=f;      goto _lrfg;}  // 33..5577
 else      if (g<=r) {l=g;      goto _fglr;}  // 3355..77
           else      {D(l) I(r) goto fin;  }  // 333555777
            
chk_lf:
 if (l>f)  {l=g; goto _fglr;}    // 3355..77
                                                    //L l f  g r R
_lfgr:                                              //33...55...77
 GT(l,m) {_7fgr: LT(r,m) {_7fg3: S(l,r) I(l) D(r) goto chk;   }  //337..55..377
                 else_GT {_7fg7: D(r) if (g<=r)  {goto _7fgr;}   //337..55..777
                                      else  {r=f; goto _7rfg;}}  //337..55777
                 else    {                        goto _7fg5; }} //337..55..577
 else_LT {_3fgr: I(l) if (l<=f) {                 goto _lfgr; }  //333..55...77
                      else {l=g;                  goto _fglr; }} //33355...77
 else    {                                        goto _5fgr;  } //335..55...77



       //  lf  gr
_5fgr: //335.55..77    二通りある
 if ((f-l+g) < (r)) goto _5fgr_g;     // (f-l) < (r-g) の意味

_5fgr_f:
 if (l==f) {D(f) l=g;                                          goto _fglr;     }  //33555..77
 LT(f,m)   {_53gr: S(l,f) I(l) D(f)                            goto chk_lf;    }  //335.355..77
 else_GT   {_57gr: LT(r,m) {_57g3: K(l,r,f) I(l) D(f) D(r)     goto chk;     }    //335.755.377
                   else_GT {_57g7: if (g==r) {r=f-size;        goto _5rfg;  }     //335.755777
                                   else      {D(r)             goto _57gr;  }}    //335.755_777
                   else    {_57g5: if (g==r) {I(g) r=f-size;   goto _5rfg;  }     //335.755577
                                   else      {S(f,r) D(f) D(r) goto _5fgr_f;}} }  //335.755_577
 else      {_55gr: D(f)                        /*goto _5fgr;*/ goto _5fgr_f;   }  //335.555..77
 assert(0,"_5fgr_f");

_5fgr_g:
 LT(g,m) {_5f3r: S(l,g) I(l) I(g)                             goto chk;     } //335.553.77
 else_GT {_5f7r: if (g==r) {r=f;                              goto _5rfg;  }
                 else LT(r,m) {_5f73: K(l,r,g) I(l) I(g) D(r) goto chk;    }  //335.557377
                      else_GT {_5f77: D(r)                    goto _5f7r;  }
                      else    {_5f75: S(g,r) I(g) D(r)        goto _5fgr_ ;}}
 else    {_5f5r: I(g)  _5fgr_: if (g<=r) {    /*goto _5fgr;*/ goto _5fgr_g;} 
                               else      {r=f;                goto _5rfg;  }}
 assert(0,"_5fgr_g");



       //  l f  g r
_7fg5: //337..55..577    二通りある
 if ((f-l+g) < (r)) goto _7fg5_g;     // (f-l) < (r-g) の意味

_7fg5_f:
 if (l==f) {S(l,r) D(f) l=g; D(r)              goto _fglr; } //33755.577
 LT(f,m)   {_73g5: K(l,f,r) I(l) D(f) D(r)     goto chk;   } //337.355.577
 else_GT   {_77g5: S(f,r) D(f) D(r) if (g<=r) {goto _7fgr;}
                                    else {r=f; goto _7rfg;}} //337.755.577
 else      {_75g5: D(f)                        goto _7fg5; }
 assert(0,"_7fg5_f");

_7fg5_g:
 if (g==r) {_7f5:  I(g) r=f;                   goto _7rfg; }
 LT(g,m)   {_7f35: K(l,g,r) I(l) I(g) D(r)     goto chk;   }
 else_GT   {_7f75: S(g,r) I(g) D(r) if (g<=r) {goto _7fgr;}
                                    else {r=f; goto _7rfg;}} //337.557.577
 else      {_7f55: I(g)                        goto _7fg5; }
 assert(0,"_7fg5_g");



       //  l r f  g
_5rfg: //335..775577
 if (l==r) {       I(l) D(r)                                       goto fin_rlfg;}  //本本3357775577
 LT(r,m)   {_53fg: {if (r==f) S(l,r) else K(l,r,f)} I(l) D(r) D(f) goto _lrfg;   }  //      r  f  g
 else_GT   {_57fg: D(r)                                            goto _5rfg;   }
 else      {_55fg: {if (r!=f) S(r,f)} D(r) D(f)                    goto _5rfg;   }
 assert(0,"_5rfg");

_7rfg: //337..775577
 if (l==r) {       D(r)                         goto fin_rlfg;}  //33775577 3375577
 LT(r,m)   {_73fg: S(l,r) I(l) D(r)             goto _lrfg;   }  //337.3775577
 else_GT   {_77fg: D(r)                         goto _7rfg;   }  //337.7775577
 else      {_75fg: {if (r!=f) S(r,f)} D(r) D(f) goto _7rfg;   }  //337.5775577
 assert(0,"_7rfg");



_lrfg: //mの要素が移動することはない
 if (l<r)  GT(r,m) {_l7fg: D(r)                                           goto _lrfg;}
           else_LT {_l3fg: GT(l,m) {_73fG: S(l,r) I(l) D(r)               goto _lrfg;}
                           else_LT {_33fg: I(l) if (l<r) {                goto _l3fg;}
                                                else     {I(l)            goto fin_rlfg;}}
                           else    {_53fG: if (r==f) S(l,r) else K(l,r,f)
                                           I(l) D(r) D(f)                 goto _lrfg;}}
           else    {_l5fg: {if (r!=f) S(r,f)} D(r) D(f)                   goto _lrfg;}
 else
 if (l==r) LT(l,m) {_3fg:  I(l)                         goto fin_rlfg;}
           else_GT {_7fg:  D(r)                         goto fin_rlfg;}
           else    {_5fg:  I(l) D(r)                    goto fin_rlfg;}  //本本333577555777
 else              {                                    goto fin_rlfg;}  //       r f   g

fin_rlfg:
 assert(r+size == l || r+size*2 == l, "fin_rlfg");
 I(f); 
 if ((v=f-l)<=0) {l=r; r=g; goto fin;}
 if ((w=g-f)==size) S(l,f)
 else if (v>=w) mmswapblock(l,f  ,w);
 else           mmswapblock(l,g-v,v);
 l=r; r=g-v;
 goto fin;



_fglr:
 if (l<r)  LT(l,m) {_fg3r: I(l)                                           goto _fglr;}
           else_GT {_fg7r: LT(r,m) {_fg73: S(l,r) I(l) D(r)               goto _fglr;}
                           else_GT {_fg77: D(r) if (l<r) {                goto _fg7r;}
                                                else     {D(r)            goto fin_fgrl;}}
                           else    {_fg75: if (g==l) S(l,r) else K(g,r,l)
                                           I(g) I(l) D(r)                 goto _fglr;}}
           else    {_fg5r: {if (g!=l) S(g,l)} I(g) I(l)                   goto _fglr;}
 else
 if (l==r) LT(l,m) {_fg3:  I(l)       goto fin_fgrl;}  //            r
           else_GT {_fg7:  D(r)       goto fin_fgrl;}  //     f   g  l
           else    {_fg5:  I(l) D(r)  goto fin_fgrl;}  //本本33555333577
 else              {                  goto fin_fgrl;}

fin_fgrl:                                              //333355533333777   3333555333335777
 assert(r+size == l || r+size*2 == l, "fin_fgrl");     //   f   g   rl        f   g   r l
 I(f); 
 if ((v=r-g+size)<=0) {r=l; l=f-size; goto fin;}
 if ((w=g-f)==size) S(f,r)
 else if (v>=w) mmswapblock(f,r-w+size,w);
 else           mmswapblock(f,g  ,v);
 f=l; l=r-w; r=f;
 goto fin;



fin:
 //安定性を確保するため、キー値が同じ要素を位置(ポインタ)でソートする
 if (indirect == 2 && (t=(r-l)/size-1) >= 2) {qs9_sort( l+size, t, /*size, cmp,*/ 0 );}

 if (l-L < R-r) {PUSH(r,R); R = l;} /*左から先にソートする*/ //if(r,L<R,l)がないと少し遅くなる
 else           {PUSH(L,l); L = r;} /*右から先にソートする*/
 if (L<R) goto loop;                         /*要素が２個以上ある場合*/

nxt:
 if (stack == top) {return;}    /*スタックが空になったとき終了する*/
 POP(L,R);                      /*スタックに積んである値を取り出す*/
 goto LOOP;
}

#undef size    /* size は本来仮引数であるが、 qs9_sort 内では size==(sizeof(char*)) である */
//ここまでが、qs9_sortのソートプログラム()です




//ここから下は、mps(multi partition sort) のためのプログラムです
//mpsは、キーの種類が10～100(県コードなど)で有効です。最大2割ほど速くなります
  // 同値キーの個数(eqcnt)    0    1    2   3   4   5   6   7   8 
  static int bun_arr2M  []={1024,1024,1024,512,512,512,512,256,256};
  static int bun_arr1M  []={1024,1024, 512,512,512,256,256,256,256};
  static int bun_arr400K[]={1024,1024, 512,512,256,512,256,256,256};
  static int bun_arr200K[]={ 512, 512, 512,256,256,512,512,128,128};
  static int bun_arr100K[]={ 512, 512, 256,256,256,256,256,128,128};
  static int bun_arr40K []={ 256, 256, 256,256,128,256,256,128,128};
  static int bun_arr20K []={ 256, 256, 256,128,128,128,128, 64, 64};
  static int bun_arr10K []={ 256, 256, 128,128, 64, 64, 64, 64, 64};
  static int bun_arr4K  []={ 128, 128, 128, 64, 64, 64, 64, 32, 32};
  static int bun_arr2K  []={ 128, 128,  64, 64, 64, 32, 32, 32, 32};
  static int bun_arr1K  []={  64,  64,  64, 64, 32, 32, 32, 32, 32};
  static int bun_arr400 []={  -1,  64,  32, 32, 32, 32, 16, 16, 16}; // -1はmpsなしを意味する
  static int bun_arr200 []={  -1,  32,  32, 32, 16, 16, 16, 16, 16};
  static int bun_arr100 []={  -1,  16,  16, 16, 16, 16,  8,  8,  8};
  static int bun_arr64  []={  -1,   8,   8,  8,  8,  8,  8,  8,  8};
  static int *bun_arr;

#define IDX_T  int
#define GRP_T  short
#define PTR_T  void*
 
int ssort( void *base, size_t nel, size_t size,  int (*cmp)(void *a, void *b) )
{
  /*正常にソートを終了したとき、戻り値は0。そうでないときは、0以外の値。*/
  int bunkatu;     /*分割数*/
  int beki;        /*区間の個数(bunkatu*2)   1～beki-1が区間番号  0は使用しない*/
  int i;           /*iはbase中の要素の位置*/
  int g;           /*gは区間番号*/
  char  *tmp;      /*ポインタ配列ptrで配列baseを並べ替えるときに使う作業領域（下記3個と共用）*/
  GRP_T *g_grp;    /*g_grp[i]は要素iの区間番号*/
  IDX_T *g_cnt;    /*g_cnt[g]は区間gの要素数*/
  IDX_T *g_head;   /*g_head[g]は区間gの未確定要素の先頭位置*/
  PTR_T *ptr;      /*本題の配列baseの全要素をカバーするポインター配列*/

  cmp_org = cmp;

  assert(sizeof(          int) == 4, "sizeof(          int) != 4"); //cygwin64はsizeof(long int)==8
  assert(sizeof(long long int) == 8, "sizeof(long long int) != 8");
  assert(size > 0                  , "size <= 0"                 );

#define VBUN 64                       /*64は実験より*/
  int eqcnt=0;
  if (nel >= VBUN) {                  // 要素数が64以上のときのみ、mps を試みる
    // 適当な32組の要素を比較して、等しい組の数を eqcnt（同値キーの個数）とする。
    int vsize=(nel/VBUN)*size; int half=vsize*(VBUN/2); char *ip_end=base+half;
    for (char *ip=base; ip<ip_end; ip+=vsize) if (cmp(ip,ip+half)==0) eqcnt++;
  }else{
    /*非mpsの場合*/ eqcnt = -1;   bunkatu = beki = 0;  // -1はmpsなしを意味する
    /*非mpsの場合*/ goto do_malloc;
  }

  /*分割数を決定する*/
  if      (nel >=2000000) bun_arr = bun_arr2M  ;
  else if (nel >=1000000) bun_arr = bun_arr1M  ;
  else if (nel >= 400000) bun_arr = bun_arr400K;
  else if (nel >= 200000) bun_arr = bun_arr200K;
  else if (nel >= 100000) bun_arr = bun_arr100K;
  else if (nel >=  40000) bun_arr = bun_arr40K ;
  else if (nel >=  20000) bun_arr = bun_arr20K ;
  else if (nel >=  10000) bun_arr = bun_arr10K ;
  else if (nel >=   4000) bun_arr = bun_arr4K  ;
  else if (nel >=   2000) bun_arr = bun_arr2K  ;
  else if (nel >=   1000) bun_arr = bun_arr1K  ;
  else if (nel >=    400) bun_arr = bun_arr400 ;
  else if (nel >=    200) bun_arr = bun_arr200 ;
  else if (nel >=    100) bun_arr = bun_arr100 ;
  else /* (nel >=  64) */ bun_arr = bun_arr64  ;
  assert(eqcnt <= 32, "eqcnt > 32");
  assert(eqcnt >= -1, "eqcnt < -1");
  if (eqcnt > 8) eqcnt=8;
  bunkatu = bun_arr[eqcnt];

  /*非mpsの場合*/ if (bunkatu == -1) {
  /*非mpsの場合*/   eqcnt = -1;   bunkatu = beki = 0;
  /*非mpsの場合*/   goto do_malloc;
  /*非mpsの場合*/ }

  assert(bunkatu <= nel, "bunkatu > nel4");

  /*区間の個数を決定する  bekiが32の時1～31が区間番号となる  0は使用しない*/
  beki = bunkatu << 1;

#define UP(x) (((x)+(sizeof(char*)-1)) & (-sizeof(char*)))
do_malloc:
  {
   size_t SIZ_tmp    = UP( size                 );
   size_t SIZ_g_grp  = UP( nel  * sizeof(GRP_T) );
   size_t SIZ_g_cnt  = UP( beki * sizeof(IDX_T) );
   size_t SIZ_g_head = UP( beki * sizeof(IDX_T) );
   size_t SIZ_ptr    = UP( nel  * sizeof(PTR_T) );

   size_t SIZ_xxx = SIZ_g_grp + SIZ_g_cnt + SIZ_g_head; //tmp領域とg_grp,g_cnt,g_head領域を共用する
   if (SIZ_xxx < SIZ_tmp) SIZ_xxx = SIZ_tmp;            //ために、大きい方でmallocする
   tmp = malloc(SIZ_xxx + SIZ_ptr);   
   if (tmp == NULL) {
#ifdef DEBUG
     fputs(" ++ ssortの実行中にmallocに失敗した ++ ",stderr); exit(1);
#endif
     return 1;
   }

   g_grp    = (GRP_T *)( (char*)tmp   + 0         );
   g_cnt    = (IDX_T *)( (char*)g_grp + SIZ_g_grp );
   g_head   = (IDX_T *)( (char*)g_cnt + SIZ_g_cnt );
   ptr      = (PTR_T *)( (char*)tmp   + SIZ_xxx   );
  }

  /*非mpsの場合*/ if (eqcnt == -1) {
  /*非mpsの場合*/   void **tp; char *ip, *ep = &((char*)base)[size*nel];
  /*非mpsの場合*/   for (ip=base, tp=ptr; ip<ep; ip+=size, tp++) *tp=(void*)ip;
  /*非mpsの場合*/   qs9_sort( ptr, nel, /*sizeof(PTR_T), cmp,*/ 2 );
  /*非mpsの場合*/   goto move_base;
  /*非mpsの場合*/ }
  
  /*サンプルを集める*/ 
  {int delta = ( nel / bunkatu ) * size;    /*サンプルを取り出すときの要素間の距離を決定する*/
   char *tp=(char*)base+delta;
   for (int s = 1; s < bunkatu; s++, tp+=delta) ptr[s] = tp; //暫く間サンプル配列の領域としてptrを使用
  }
  
  /*サンプルをソートする*/
  qs9_sort( &ptr[1], bunkatu-1, /*sizeof(PTR_T), cmp,*/ 1 );
  
  for (g = 1; g < beki; g++) g_cnt[g] = 0; /*要素数をカウントするための初期化*/
  
  for (i = 0; i < nel; i++)   /* base中の各要素がどの区間に入るかを調べる*/
  {
    int g, n, t; char *p = (char*)base+i*size;   /*pは調べようとする要素へのポインタ*/
    
    /************************* beki＝16 の場合 *********************************/
    /*                      4　　　　　　　　　　　　（２分木の根）            */
    /*          2                       6           サンプルの添え字(g>>1の値) */
    /*    1           3           5           7　　　（２分木の葉）            */
    /* -------------------------------------------                             */
    /* 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15　区間番号(添え字)（ｇの値） */
    /***************************************************************************/
    
    for (g = beki>>1, n = g>>1; n >= 2; n >>= 1)
      if ((t = cmp( p, ptr[g>>1] )) > 0) g += n;  /*木構造を右へ進む*/
      else if (t < 0)                    g -= n;  /*木構造を左へ進む*/
      else                               goto xx; /*==時はその区間へ*/
      
    if ((t = cmp( p, ptr[g>>1] )) > 0) g++;       /*２分木の葉の右へ*/
    else if (t < 0)                    g--;       /*２分木の葉の左へ*/
    
    xx:
    g_grp[i] = g;    /*要素iの区間番号を記憶する*/
    g_cnt[g]++;      /*区間gの要素数をカウントアップする*/
  }
  
  /*各区間の先頭と末尾を決定する*/
  for (i=0, g=1; g<beki; g++) { g_head[g]=i; i+=g_cnt[g]; /*g_end[g]=i;*/ }

  /*各要素のグループ番号を調べて、その要素のあるべき位置（何番目の要素か）を決定し、
    ポインター配列ptrのその位置に当該要素の先頭番地を代入する*/
  {char *ip = base;
   for (i = 0; i < nel; i++) {ptr[g_head[g_grp[i]]++]=ip; ip+=size;} //ここよりptrは本来の意味
  }
  // ここですべての区間で g_head[g]==g_end[g] となっている  ss14ではg_end[]は省略されている

  //printf("  - bunkatu=%d\n",bunkatu); // この印刷をすれば、mps の概観がつかめるはずです。
  //for (g=1; g < beki; g++) {
  //  if (g%2) printf("%d ", g_cnt[g]); else printf("[%d] ", g_cnt[g]);
  //  if (g%20==0) puts("");
  //}puts("");

  /*奇数番号の区間を一つ一つソートする*/
  for (g=1; g < beki; g+=2) {
    int cnt = g_cnt[g];
    if (cnt >= 2) {qs9_sort( &ptr[g_head[g]-cnt], cnt, /*sizeof(PTR_T), cmp,*/ 2 );}
  }

move_base:                                          /*mpsと非mpsで 以下の部分は共用する*/ 
  mmprepare( base, size );                          //以下の処理ではmmsizeは使用しない
  void **tp;  char *ip, *kp;
  /*  Knuth vol. 3 (2nd ed.) exercise 5.2-10.  */
  // tp[i]はループの糸口(先頭)を指す。ipは退避要素の元の位置,kpは空き地を指す
  for (tp = ptr, i = 0, ip = base; i < nel; i++, ip += size)
    if ((kp = tp[i]) != ip) {
      size_t j = i;
      char *jp = ip;
      mmcopy(tmp, ip, size);
      do {
        size_t k = (kp - (char *) base) / size;
        tp[j] = jp;
        mmcopy(jp, kp, size);
        j = k;
        jp = kp;
        kp = tp[k];
      }while (kp != ip);
      tp[j] = jp;
      mmcopy(jp, tmp, size);
    }

  free( tmp );
  return 0;
}
//ここまでが、mps(multi partition sort) のプログラムです
