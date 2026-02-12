#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
rlewrite (unsigned char c, int count, FILE *out)
{
   while (count > 255)
      {
         fputc (255, out);
         fputc (c, out);
         count -= 255;
      }

   fputc (count, out);
   fputc (c, out);
}

static const char *bwtptr;
static int bwtlen;

int
bwtcmp (const void *a, const void *b)
{
   int idx1, idx2, i;
   unsigned char c1, c2;

   idx1 = *(const int *)a;
   idx2 = *(const int *)b;

   for (i = 0; i < bwtlen; i++)
      {
         c1 = bwtptr[(idx1 + i) % bwtlen];
         c2 = bwtptr[(idx2 + i) % bwtlen];
         if (c1 != c2)
            {
               return c1 - c2;
            }
      }

   return 0;
}

void
fbwtenc (FILE *in, FILE *out)
{
   int n, primidx, rlecnt, i, rank;
   char *s;
   int *idxs;
   unsigned char dict[256];
   unsigned char lastc, c, mtfval;

   fseek (in, 0, SEEK_END);
   n = (int)ftell (in);
   rewind (in);

   s = malloc (n);
   fread (s, 1, n, in);

   idxs = malloc (n * sizeof (int));
   for (i = 0; i < n; i++)
      {
         idxs[i] = i;
      }

   bwtptr = s;
   bwtlen = n;
   qsort (idxs, n, sizeof (int), bwtcmp);

   primidx = 0;
   for (i = 0; i < n; i++)
      {
         if (idxs[i] == 0)
            {
               primidx = i;
               break;
            }
      }

   fwrite (&primidx, sizeof (int), 1, out);

   for (i = 0; i < 256; i++)
      {
         dict[i] = (unsigned char)i;
      }

   rlecnt = 0;
   lastc  = 0;

   for (i = 0; i < n; i++)
      {
         c    = s[(idxs[i] + n - 1) % n];
         rank = 0;
         while (dict[rank] != c)
            {
               rank++;
            }

         if (rank > 0)
            {
               memmove (&dict[1], &dict[0], rank);
               dict[0] = c;
            }

         mtfval = (unsigned char)rank;
         if (i == 0)
            {
               lastc  = mtfval;
               rlecnt = 1;
            }
         else if (mtfval == lastc)
            {
               rlecnt++;
            }
         else
            {
               rlewrite (lastc, rlecnt, out);
               lastc  = mtfval;
               rlecnt = 1;
            }
      }
   rlewrite (lastc, rlecnt, out);

   free (s);
   free (idxs);
}

void
fbwtdec (FILE *in, FILE *out)
{
   int primidx, count, c, rank, sum, crnt;
   size_t cap, n, i, k;
   unsigned char *r;
   unsigned char dict[256];
   int arrcnt[256] = { 0 };
   int first[256];
   int curcnt[256] = { 0 };
   int *T;

   if (fread (&primidx, sizeof (int), 1, in) != 1)
      {
         return;
      }

   cap = 1024;
   n   = 0;
   r   = malloc (cap);

   while ((count = fgetc (in)) != EOF)
      {
         c = fgetc (in);
         for (k = 0; k < (size_t)count; k++)
            {
               if (n >= cap)
                  {
                     cap *= 2;
                     r = realloc (r, cap);
                  }
               r[n++] = (unsigned char)c;
            }
      }

   for (i = 0; i < 256; i++)
      {
         dict[i] = (unsigned char)i;
      }

   for (i = 0; i < n; i++)
      {
         rank = r[i];
         c    = dict[rank];
         r[i] = c;
         if (rank > 0)
            {
               memmove (&dict[1], &dict[0], rank);
               dict[0] = c;
            }
      }

   for (i = 0; i < n; i++)
      {
         arrcnt[r[i]]++;
      }

   sum = 0;
   for (i = 0; i < 256; i++)
      {
         first[i] = sum;
         sum += arrcnt[i];
      }

   T = malloc (n * sizeof (int));
   for (i = 0; i < n; i++)
      {
         T[first[r[i]] + curcnt[r[i]]] = i;
         curcnt[r[i]]++;
      }

   crnt = T[primidx];
   for (i = 0; i < n; i++)
      {
         fputc (r[crnt], out);
         crnt = T[crnt];
      }

   free (T);
   free (r);
}

int
main (void)
{
   FILE *fin, *fenc, *fbwt, *fdec;

   fin = fopen ("test.txt", "rb");
   if (!fin)
      {
         return 1;
      }

   fenc = fopen ("test.barc", "wb");
   fbwtenc (fin, fenc);
   fclose (fin);
   fclose (fenc);

   fbwt = fopen ("test.barc", "rb");
   fdec = fopen ("decoded.txt", "wb");
   fbwtdec (fbwt, fdec);
   fclose (fbwt);
   fclose (fdec);

   printf ("SUCCESS\n");
   return 0;
}
