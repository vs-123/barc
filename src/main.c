#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STX 0x02
#define ETX 0x03

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

void fbwtenc(FILE *in, FILE *out, char chrstart, char chrend)
{
   fseek(in, 0, SEEK_END);
   long inplen = ftell(in);
   rewind(in);

   int outlen = (int)inplen + 2;
   char *s    = malloc(outlen + 1);

   s[0] = chrstart;
   fread(s + 1, 1, inplen, in);
   s[outlen - 1] = chrend;
   s[outlen]     = '\0';

   int *indices = malloc(outlen * sizeof(int));
   for (int i = 0; i < outlen; i++) {
      indices[i] = i;
   }

   bwtptr = s;
   bwtlen = outlen;
   qsort(indices, outlen, sizeof(int), bwtcmp);

   for (int i = 0; i < outlen; i++) {
      int idxlastchr = (indices[i] + outlen - 1) % outlen;
      fputc(s[idxlastchr], out);
   }

   free(s);
   free(indices);
}

void fbwtdec(FILE *in, FILE *out, char chrstart, char chrend)
{
   fseek(in, 0, SEEK_END);
   int n = (int)ftell(in);
   rewind(in);

   if (n <= 2) {
      return;
   }

   char *r = malloc(n + 1);
   fread(r, 1, n, in);
   r[n] = '\0';

   int count[256] = { 0 }, first[256], crntcount[256] = { 0 };
   for (int i = 0; i < n; i++) {
      count[(unsigned char)r[i]]++;
   }

   int sum = 0;
   for (int i = 0; i < 256; i++) {
      first[i] = sum;
      sum += count[i];
   }

   int *T = malloc(n * sizeof(int));
   for (int i = 0; i < n; i++) {
      unsigned char c                = (unsigned char)r[i];
      T[first[c] + crntcount[c]] = i;
      crntcount[c]++;
   }

   int curr = -1;
   for (int i = 0; i < n; i++) {
      if (r[i] == chrstart) {
         curr = i;
         break;
      }
   }

   for (int i = 0; i < n; i++) {
      if (r[curr] != chrstart && r[curr] != chrend) {
         fputc(r[curr], out);
      }
      curr = T[curr];
   }

   free(T);
   free(r);
}

int main(void)
{
   FILE *fin  = fopen("test.txt", "rb");
   FILE *fenc = fopen("test.bwt", "wb");
   fbwtenc(fin, fenc, STX, ETX);
   fclose(fin);
   fclose(fenc);

   FILE *fbwt = fopen("test.bwt", "rb");
   FILE *fdec = fopen("decoded.txt", "wb");
   fbwtdec(fbwt, fdec, STX, ETX);
   fclose(fbwt);
   fclose(fdec);

   printf("DONE\n");
   return 0;
}
