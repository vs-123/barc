#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *bwtptr;
static int bwtlen;

int bwtcmp(const void *a, const void *b)
{
   int idx1 = *(const int *)a;
   int idx2 = *(const int *)b;
   for (int i = 0; i < bwtlen; i++) {
      unsigned char c1 = (unsigned char)bwtptr[(idx1 + i) % bwtlen];
      unsigned char c2 = (unsigned char)bwtptr[(idx2 + i) % bwtlen];

      if (c1 != c2) {
         return c1 - c2;
      }
   }
   return 0;
}

void fbwtenc(FILE *in, FILE *out)
{
   fseek(in, 0, SEEK_END);
   int n = (int)ftell(in);
   rewind(in);

   char *s = malloc(n);
   fread(s, 1, n, in);

   int *idxs = malloc(n * sizeof(int));
   for (int i = 0; i < n; i++) {
      idxs[i] = i;
   }

   bwtptr = s;
   bwtlen = n;
   qsort(idxs, n, sizeof(int), bwtcmp);

   int idx_primary = 0;
   for (int i = 0; i < n; i++) {
      if (idxs[i] == 0) {
         idx_primary = i;
         break;
      }
   }

   fwrite(&idx_primary, sizeof(int), 1, out);

   unsigned char dict[256];
   for (int i = 0; i < 256; i++) {
      dict[i] = (unsigned char)i;
   }

   for (int i = 0; i < n; i++) {
      unsigned char c = (unsigned char)s[(idxs[i] + n - 1) % n];
      int rank        = 0;
      while (dict[rank] != c) {
         rank++;
      }

      fputc(rank, out); 

      if (rank > 0) {
         memmove(&dict[1], &dict[0], rank);
         dict[0] = c;
      }
   }

   free(s);
   free(idxs);
}

void fbwtdec(FILE *in, FILE *out)
{
   int idx_primary;
   if (fread(&idx_primary, sizeof(int), 1, in) != 1) {
      return;
   }

   fseek(in, 0, SEEK_END);
   int n = (int)ftell(in) - sizeof(int);
   fseek(in, sizeof(int), SEEK_SET);

   unsigned char *mtfin = malloc(n);
   fread(mtfin, 1, n, in);

   unsigned char *r = malloc(n); 
   unsigned char dict[256];
   for (int i = 0; i < 256; i++) {
      dict[i] = (unsigned char)i;
   }

   for (int i = 0; i < n; i++) {
      int rank        = mtfin[i];
      unsigned char c = dict[rank];
      r[i]            = c;

      if (rank > 0) {
         memmove(&dict[1], &dict[0], rank);
         dict[0] = c;
      }
   }

   int count[256] = { 0 }, first[256], crntcount[256] = { 0 };
   for (int i = 0; i < n; i++) {
      count[r[i]]++;
   }

   int sum = 0;
   for (int i = 0; i < 256; i++) {
      first[i] = sum;
      sum += count[i];
   }

   int *T = malloc(n * sizeof(int));
   for (int i = 0; i < n; i++) {
      T[first[r[i]] + crntcount[r[i]]] = i;
      crntcount[r[i]]++;
   }

   int crnt = T[idx_primary];
   for (int i = 0; i < n; i++) {
      fputc(r[crnt], out);
      crnt = T[crnt];
   }

   free(T);
   free(r);
   free(mtfin);
}

int main(void)
{
   FILE *fin = fopen("test.png", "rb");
   if (!fin) {
      return 1;
   }
   FILE *fenc = fopen("test.barc", "wb");
   fbwtenc(fin, fenc);
   fclose(fin);
   fclose(fenc);

   FILE *fbwt = fopen("test.barc", "rb");
   FILE *fdec = fopen("decoded.png", "wb");
   fbwtdec(fbwt, fdec);
   fclose(fbwt);
   fclose(fdec);

   printf("SUCCESS\n");
   return 0;
}
