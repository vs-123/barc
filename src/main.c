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

   for (int i = 0; i < n; i++) {
      fputc(s[(idxs[i] + n - 1) % n], out);
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

   unsigned char *r = malloc(n);
   fread(r, 1, n, in);

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
}

int main(void)
{
   FILE *fin  = fopen("test.png", "rb");
   FILE *fenc = fopen("test.bwt", "wb");
   fbwtenc(fin, fenc);
   fclose(fin);
   fclose(fenc);

   FILE *fbwt = fopen("test.bwt", "rb");
   FILE *fdec = fopen("decoded.png", "wb");
   fbwtdec(fbwt, fdec);
   fclose(fbwt);
   fclose(fdec);

   printf("DONE\n");
   return 42;
}
