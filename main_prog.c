#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void die( char *s ) {fprintf(stderr, "***** %s *****\n", s); printf("***** %s *****\n", s); exit(1);}

typedef struct { int key; int data; } el_t;
#define KEY(i)  (((el_t*)(vec+(i)*rec_siz))->key)
#define DATA(i) (((el_t*)(vec+(i)*rec_siz))->data)

unsigned int cmp_cnt,ass_cnt;
int _QS_MID1,_QS_MID2,_QS_MID3;

int strcmp0(char *s1, char *s2) { int d;
 while ((d=(*s1++ - *s2++)) == 0) if (s1[-1] == 0) return(0);
 return(d);
}

int cmp_loop,cmp_val;
int cmpfnc( xp, yp ) el_t *xp, *yp; {cmp_cnt++;
 if (cmp_loop) {int c; for (c=cmp_loop; c>0; c--) cmp_val+=strcmp0("abc","def");}
 return xp->key - yp->key;
}

int counter, arr_max, div_val, rec_siz, itarate;
char *vec, *chk;

void do_qsort(int do_qs) {int i,j,x,t,h;

 srandom( div_val + arr_max + 556  ); //乱数の初期化 //srand() rand() しかない環境では、書換えて下さい

 for (counter=0; counter<itarate; counter++) {
   /*データを用意する*/
   if (div_val == 0 ) for (i = 0; i < arr_max; i++) KEY(i)= 5;         /*一定*/
   if (div_val == -1) for (i = 0; i < arr_max; i++) KEY(i)= i+1;       /*昇順*/
   if (div_val == -2) for (i = 0; i < arr_max; i++) KEY(i)= arr_max-i; /*降順*/
   if (div_val == 1 ) for (i = 0; i < arr_max; i++) KEY(i)= random();  /*乱数*/
   if (div_val >= 2 ) for (i = 0; i < arr_max; i++) KEY(i)= random()%div_val;
   if (div_val == -3) {
     for (i = 0; i < arr_max; i++) KEY(i)= i;       /*同値キーがない乱数　入れ替えで*/
     for (i = 0; i < arr_max; i++) {x=random()%arr_max; t=KEY(i); KEY(i)=KEY(x); KEY(x)=t;}
   }
   if (div_val == -4) {
     for (i = 0; i < arr_max; i++) KEY(i)= random()%2;
     KEY(random()%arr_max)= 9;
     KEY(random()%arr_max)= 9;
   }
   if (div_val == -5) {
     for (i = 0; i < arr_max; i++) KEY(i)= random()%3;
     KEY(random()%arr_max)= 9;
     KEY(random()%arr_max)= 9;
   }

   if (rec_siz >= 8)
     for (i = 0; i < arr_max; i++) DATA(i) = i;   /*検査のための準備*/

   if (do_qs) qsort( (char*)vec, arr_max, rec_siz, cmpfnc );   /*ソートの実行*/

   /*以下でソートできたことを検査する*/
   for (i = 1; i < arr_max; i++)
     if (div_val>=0 || div_val<=(-4) ? KEY(i-1)>KEY(i) : KEY(i-1)>=KEY(i)) {
       if (do_qs==0) continue;
       puts("");
       {for (h = 0; h < arr_max && h<40; h++) printf(" %d",KEY(h)); puts(" arr_max error");}
       {for (h = 0; h <= i && h<40; h++) printf(" ."); puts("← error is here");}
       printf("  counter=%d   error i=%d  ",counter,i);
       die("not sorted  do_qsort(1)");
     }else{
       if (do_qs==0) continue;  // do_qsort(1) do_qsort(0) の時間をできるだけ合わせるための処理
     }
   if (rec_siz >= 8) {
     for (i = 0; i < arr_max; i++) chk[i] = 0;
     for (i = 0; i < arr_max; i++) chk[DATA(i)] = 123;
     for (i = 0; i < arr_max; i++) if (chk[i] != 123) die("chk err");
   }
 }
}


int main( int argc, char **argv ) {
 clock_t clk_start, clk_end, clk_end2;
 cmp_cnt=ass_cnt=0;

 if (argc != 9) die("Usage: main.exe div_val arr_max rec_siz itarate MID1 MID2 MID3 cmp");
 
 div_val = atoi(argv[1]);       /*テストデータの種類を指定する random()%div_val等*/
 arr_max = atoi(argv[2]);       /*要素の個数(要素数)*/
 rec_siz = atoi(argv[3]);       /*要素の大きさ(要素サイズ)*/
 itarate = atoi(argv[4]);       /*繰り返し回数*/
 if (atoi(argv[5])>=0) _QS_MID1 = atoi(argv[5]); // n がこれ以下で３点処理を行う  qs9 の既定値140
 if (atoi(argv[6])>=0) _QS_MID2 = atoi(argv[6]); // n がこれ以下で９点処理を行う  qs9 の既定値900
 if (atoi(argv[7])>=0) _QS_MID3 = atoi(argv[7]); // size がこれ以上=のときに間接ソートを実行
 cmp_loop  = atoi(argv[8]);     /*比較関数の重たさを調整する*/
 if (rec_siz == 0) rec_siz = sizeof(char*);
                                
 fprintf(stderr,"\n%-7s d=%d e=%d s=%d %dMB R%d ",
      argv[0]+2,div_val,   arr_max,   rec_siz,   arr_max*rec_siz/1000000 ,itarate);
 fprintf(stderr,"%c=%d:%d:%d:%d: ",(sizeof(char*)==8?'M':'m'),_QS_MID1,_QS_MID2,_QS_MID3,cmp_loop);

 printf("%-8s d=%d e=%d s=%d R%d ", argv[0]+2,div_val,   arr_max,   rec_siz ,itarate);
 printf("%c%03d:%03d:%03d:%d:",(sizeof(char*)==8?'M':'m'),_QS_MID1,_QS_MID2,_QS_MID3,cmp_loop);
 fflush(stdout);

 if (rec_siz < 4 || 100000 < rec_siz) die("本プログラムでは「要素のバイトサイズは４以上&10万以下」");
 if (rec_siz % 4) die("本プログラムでは「要素のバイトサイズは４の倍数」を仮定");
 if ((vec = (char*)malloc((arr_max+5)*rec_siz)) == NULL) die("vec NULL");
 if ((chk = (char*)malloc(arr_max)) == NULL) die("chk NULL");
 
 clk_start=clock();
 do_qsort(1);
 clk_end=clock();
 do_qsort(0);
 clk_end2=clock();

 {unsigned int cmp_av=cmp_cnt/itarate,
               ass_av=ass_cnt/itarate;
  double sum_av=(double)cmp_av+ass_av,
         etime= (double)((clk_end-clk_start)-(clk_end2-clk_end))/CLOCKS_PER_SEC;
  fprintf(stderr,"  T=%1.2f %4.0f ",etime,etime/itarate*100000);
  if (ass_av) printf(" c=%-6u %10u a=%-6u %10u i=%1.0f T=%1.2f %4.0f \n",
                   cmp_av,cmp_cnt, ass_av,ass_cnt,sum_av,etime,etime/itarate*100000);
  else        printf(" c=%-6u %10u T=%1.2f %4.0f \n",
                   cmp_av,cmp_cnt,                       etime,etime/itarate*100000);
 }

 fflush(stdout);  
 return 0;
}
