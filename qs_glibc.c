// メモリが足りないとき(179行目)、エラーで終了するように改造してあります。

#include <alloca.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
static void die(char *s) {fprintf(stderr, "++++ %s ++++\n", s); printf("++++ %s ++++ \n", s); exit(1);}

#ifdef DEBUG
unsigned int ass_cnt; /*代入回数を計測しないときは、削除可能*/
#define pppppp(x,c) {/*printf(x); fflush(stdout);*/  ass_cnt += (c);}
#else
#define pppppp(x,c)
#endif

//void *memcpy(void *, const void *, size_t);  //memcpyの宣言がないときはこの行を生かす。
void *__mempcpy(void *, const void *, size_t);  //__mempcpyの宣言がないときはこの行を生かす。
//__mempcpyの実体が存在しないときは、上の行を殺して、下の行を生かす。
//void *__mempcpy(void *dest, const void *src, size_t len) {return memcpy(dest, src, len) + len;}
typedef int (*__compar_d_fn_t) (const void *, const void *, void *);
typedef int (*__compar_fn_t) (const void *, const void *);

struct msort_param
{
  size_t s;
  size_t var;
  __compar_d_fn_t cmp;
  void *arg;
  char *t;
};
static void msort_with_tmp (const struct msort_param *p, void *b, size_t n);

static void
msort_with_tmp (const struct msort_param *p, void *b, size_t n)
{
  char *b1, *b2;
  size_t n1, n2;

  if (n <= 1)
    return;

  n1 = n / 2;
  n2 = n - n1;
  b1 = b;
  b2 = (char *) b + (n1 * p->s);

  msort_with_tmp (p, b1, n1);
  msort_with_tmp (p, b2, n2);

  char *tmp = p->t;
  const size_t s = p->s;
  __compar_d_fn_t cmp = p->cmp;
  void *arg = p->arg;
  switch (p->var)
    {
    case 0:
      while (n1 > 0 && n2 > 0)
	{
          pppppp(" *uint32_t  \n", 1)
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      *(uint32_t *) tmp = *(uint32_t *) b1;
	      b1 += sizeof (uint32_t);
	      --n1;
	    }
	  else
	    {
	      *(uint32_t *) tmp = *(uint32_t *) b2;
	      b2 += sizeof (uint32_t);
	      --n2;
	    }
	  tmp += sizeof (uint32_t);
	}
      break;
    case 1:
      while (n1 > 0 && n2 > 0)
	{
          pppppp(" *uint64_t  \n",1)
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      *(uint64_t *) tmp = *(uint64_t *) b1;
	      b1 += sizeof (uint64_t);
	      --n1;
	    }
	  else
	    {
	      *(uint64_t *) tmp = *(uint64_t *) b2;
	      b2 += sizeof (uint64_t);
	      --n2;
	    }
	  tmp += sizeof (uint64_t);
	}
      break;
    case 2:
      while (n1 > 0 && n2 > 0)
	{
	  unsigned long *tmpl = (unsigned long *) tmp;
	  unsigned long *bl;

	  tmp += s;
          pppppp(" *(unsigned long *)  \n", 1)
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      bl = (unsigned long *) b1;
	      b1 += s;
	      --n1;
	    }
	  else
	    {
	      bl = (unsigned long *) b2;
	      b2 += s;
	      --n2;
	    }
	  while (tmpl < (unsigned long *) tmp)
	    *tmpl++ = *bl++;
	}
      break;
    case 3:
      while (n1 > 0 && n2 > 0)
	{
          pppppp(" *(void **)  \n", 1)
	  if ((*cmp) (*(const void **) b1, *(const void **) b2, arg) <= 0)
	    {
	      *(void **) tmp = *(void **) b1;
	      b1 += sizeof (void *);
	      --n1;
	    }
	  else
	    {
	      *(void **) tmp = *(void **) b2;
	      b2 += sizeof (void *);
	      --n2;
	    }
	  tmp += sizeof (void *);
	}
      break;
    default:
      while (n1 > 0 && n2 > 0)
	{
          pppppp(" *__mempcpy  \n", 1)
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      tmp = (char *) __mempcpy (tmp, b1, s);
	      b1 += s;
	      --n1;
	    }
	  else
	    {
	      tmp = (char *) __mempcpy (tmp, b2, s);
	      b2 += s;
	      --n2;
	    }
	}
      break;
    }

  if (n1 > 0)
    {memcpy (tmp, b1, n1 * s);  pppppp(" *mempcpy1n1  \n", n1)}
  memcpy (b, p->t, (n - n2) * s);
  pppppp(" *mempcpy2n2  \n", n - n2)
}


void
__qsort_r (void *b, size_t n, size_t s, __compar_d_fn_t cmp, void *arg)
{
  size_t size = n * s;
  char *tmp = NULL;
  struct msort_param p;

  /* For large object sizes use indirect sorting.  */
  if (s > 32)
    size = 2 * n * sizeof (void *) + s;

    {
      tmp = malloc (size);
      if (tmp == NULL)
	{
          die("_quicksort");
	  //_quicksort (b, n, s, cmp, arg);
	  return;
	}
      p.t = tmp;
    }

  p.s = s;
  p.var = 4;
  p.cmp = cmp;
  p.arg = arg;

  if (s > 32)
    {
      /* Indirect sorting.  */
      char *ip = (char *) b;
      void **tp = (void **) (p.t + n * sizeof (void *));
      void **t = tp;
      void *tmp_storage = (void *) (tp + n);

      while ((void *) t < tmp_storage)
	{
	  *t++ = ip;
	  ip += s;
	}
      p.s = sizeof (void *);
      p.var = 3;
      msort_with_tmp (&p, p.t + n * sizeof (void *), n);

      /* tp[0] .. tp[n - 1] is now sorted, copy around entries of
	 the original array.  Knuth vol. 3 (2nd ed.) exercise 5.2-10.  */
      char *kp;
      size_t i;
      for (i = 0, ip = (char *) b; i < n; i++, ip += s)
	if ((kp = tp[i]) != ip)
	  {
	    size_t j = i;
	    char *jp = ip;
	    memcpy (tmp_storage, ip, s); pppppp(" *mempcpy1  \n", 1)

	    do
	      {
		size_t k = (kp - (char *) b) / s;
		tp[j] = jp;
		memcpy (jp, kp, s); pppppp(" *mempcpy2  \n", 1)
		j = k;
		jp = kp;
		kp = tp[k];
	      }
	    while (kp != ip);

	    tp[j] = jp;
	    memcpy (jp, tmp_storage, s); pppppp(" *mempcpy3  \n", 1)
	  }
    }
  else
    {
      if ((s & (sizeof (uint32_t) - 1)) == 0
	  && ((char *) b - (char *) 0) % __alignof__ (uint32_t) == 0)
	{
	  if (s == sizeof (uint32_t))
	    p.var = 0;
	  else if (s == sizeof (uint64_t)
		   && ((char *) b - (char *) 0) % __alignof__ (uint64_t) == 0)
	    p.var = 1;
	  else if ((s & (sizeof (unsigned long) - 1)) == 0
		   && ((char *) b - (char *) 0)
		      % __alignof__ (unsigned long) == 0)
	    p.var = 2;
	}
      msort_with_tmp (&p, b, n);
    }
  free (tmp);
}


void
qsort (void *b, size_t n, size_t s, __compar_fn_t cmp)
{
  return __qsort_r (b, n, s, (__compar_d_fn_t) cmp, NULL);
}
