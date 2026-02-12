#include <stdio.h>

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void
rlewrite (uint8_t c, int32_t count, FILE *out)
{
   while (count > 0)
      {
         if (count == 1)
            {
               fputc (c, out);
               count = 0;
            }
         else
            {
               fputc (c, out);
               fputc (c, out);
               count -= 2;
               if (count > 255)
                  {
                     fputc (255, out);
                     count -= 255;
                  }
               else
                  {
                     fputc (count, out);
                     count = 0;
                  }
            }
      }
}

static const char *bwtptr;
static int32_t bwtlen;

int
bwtcmp (const void *a, const void *b)
{
   int32_t idx1, idx2, i;

   idx1 = *(const int32_t *)a;
   idx2 = *(const int32_t *)b;

   for (i = 0; i < bwtlen; i++)
      {
         uint8_t c1 = bwtptr[(idx1 + i) % bwtlen];
         uint8_t c2 = bwtptr[(idx2 + i) % bwtlen];
         if (c1 != c2)
            {
               return c1 - c2;
            }
      }
   return 0;
}

long
fbwtenc (FILE *in, FILE *out)
{
   long startpos;
   int32_t n, primidx, rlecnt, i, rank;
   char *s;
   int32_t *idxs;
   uint8_t dict[256];
   uint8_t lastc, c, mtfval;

   startpos = ftell (out);
   fseek (in, 0, SEEK_END);
   n = ftell (in);
   rewind (in);

   if (n == 0)
      {
         return 0;
      }

   s = malloc (n);
   fread (s, 1, n, in);

   idxs = malloc (n * sizeof (int32_t));
   for (i = 0; i < n; i++)
      {
         idxs[i] = i;
      }

   bwtptr = s;
   bwtlen = n;
   qsort (idxs, n, sizeof (int32_t), bwtcmp);

   primidx = 0;
   for (i = 0; i < n; i++)
      {
         if (idxs[i] == 0)
            {
               primidx = i;
               break;
            }
      }

   fwrite (&primidx, sizeof (int32_t), 1, out);

   for (i = 0; i < 256; i++)
      {
         dict[i] = i;
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

         mtfval = rank;
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
   return ftell (out) - startpos;
}

void
fbwtdec (FILE *in, FILE *out, int32_t datalen)
{
   int32_t primidx, lastc, haslast, c, extra, k, rank, sum, crnt;
   long nread;
   size_t cap, n, i;
   uint8_t *r, dict[256], curc;
   int32_t arrcnt[256] = { 0 }, first[256], curcnt[256] = { 0 };
   int32_t *T;

   if (datalen <= 0)
      {
         return;
      }

   fread (&primidx, sizeof (int32_t), 1, in);
   nread = sizeof (int32_t);

   cap     = 1024;
   n       = 0;
   r       = malloc (cap);
   lastc   = -1;
   haslast = 0;

   while (nread < datalen)
      {
         c = fgetc (in);
         nread++;
         if (n >= cap)
            {
               cap *= 2;
               r = realloc (r, cap);
            }
         r[n++] = c;

         if (haslast && c == lastc)
            {
               extra = fgetc (in);
               nread++;
               for (k = 0; k < extra; k++)
                  {
                     if (n >= cap)
                        {
                           cap *= 2;
                           r = realloc (r, cap);
                        }
                     r[n++] = c;
                  }
               haslast = 0;
               lastc   = -1;
            }
         else
            {
               lastc   = c;
               haslast = 1;
            }
      }

   for (i = 0; i < 256; i++)
      {
         dict[i] = i;
      }

   for (i = 0; i < n; i++)
      {
         rank = r[i];
         curc = dict[rank];
         r[i] = curc;
         if (rank > 0)
            {
               memmove (&dict[1], &dict[0], rank);
               dict[0] = curc;
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

   T = malloc (n * sizeof (int32_t));
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

void
arcfile (const char *fname, FILE *outarc)
{
   FILE *in;
   int32_t namelen, placeholder, cmpsz;
   long sizepos, endpos;

   in = fopen (fname, "rb");
   if (!in)
      {
         fprintf (stderr, "[BARC ERROR] %s NOT OPEN\n", fname);
         return;
      }

   namelen = strlen (fname);
   fwrite (&namelen, sizeof (int32_t), 1, outarc);
   fwrite (fname, 1, namelen, outarc);

   sizepos     = ftell (outarc);
   placeholder = 0;
   fwrite (&placeholder, sizeof (int32_t), 1, outarc);

   printf ("[BARC COMPRESS] %s\n", fname);
   cmpsz = fbwtenc (in, outarc);

   endpos = ftell (outarc);
   fseek (outarc, sizepos, SEEK_SET);
   fwrite (&cmpsz, sizeof (int32_t), 1, outarc);
   fseek (outarc, endpos, SEEK_SET);

   fclose (in);
}

void
exarc (FILE *inarc)
{
   int32_t namelen, cmpsz;
   char *fname;
   char outname[256];
   FILE *out;

   while (fread (&namelen, sizeof (int32_t), 1, inarc) == 1)
      {
         if (namelen == -1)
            {
               break;
            }

         fname = malloc (namelen + 1);
         fread (fname, 1, namelen, inarc);
         fname[namelen] = '\0';

         fread (&cmpsz, sizeof (int32_t), 1, inarc);
         snprintf (outname, sizeof (outname), "BARC_EXT_%s", fname);

         out = fopen (outname, "wb");
         if (out)
            {
               printf ("[BARC EXTRACT] %s -> %s\n", fname, outname);
               fbwtdec (inarc, out, cmpsz);
               fclose (out);
            }
         free (fname);
      }
}

int
strcmpci (char const *str1, char const *str2)
{
   for (;;)
      {
         int cmp = tolower (*str1) - tolower (*str2);
         if (!*str1 || !*str2 || cmp != 0)
            {
               return cmp;
            }
         str1++, str2++;
      }
}

void
print_info (void)
{
#define pinfo(aspect, detail) printf ("   * %-17s %s\n", aspect, detail)
   printf ("[INFO]\n");
   printf ("   BARC -- A DEAD-SIMPLE, BWT-BASED FILE ARCHIVER AND COMPRESSOR.\n");
   printf ("\n");
   pinfo ("[AUTHOR]", "vs-123 @ https://github.com/vs-123");
   pinfo ("[REPOSITORY]", "https://github.com/vs-123/barc");
   pinfo ("[LICENSE]", "GNU AFFERO GENERAL PUBLIC LICENSE VERSION 3.0 OR LATER");
#undef pinfo
}

void
print_usage (const char *program_name)
{
#define popt(opt, desc) printf ("   %-45s %s\n", opt, desc)

   printf ("=== BARC ===\n");
   printf ("[USAGE] %s [OPTION]\n\n", program_name);
   printf ("[OPTIONS]\n");
   popt ("-c, --compress <ARCHIVE> <FILE> [<FILE>]...", "CREATE ARCHIVE <ARCHIVE> FROM FILE(S)");
   popt ("-x, --extract <ARCHIVE>", "EXTRACT FILE(S) FROM <ARCHIVE>");
   popt ("-h, -?, --help", "PRINT THIS HELP MESSAGE AND EXIT");
   popt ("-i, --info", "VIEW INFORMATION ABOUT THIS PROGRAM");
   printf ("\n");
   printf ("[EXAMPLE]\n");
   printf ("   %s -c archive.barc file1.txt file2.txt\n", program_name);
   printf ("   %s -x archive.barc\n", program_name);
   printf ("\n");
   printf ("[NOTE] FLAGS ARE CASE-INSENSITIVE\n");

#undef popt
}

typedef enum
{
   PROGRAM_MODE_COMPRESS,
   PROGRAM_MODE_EXTRACT,
   PROGRAM_MODE_NIL,
} program_mode_t;

int
main (int argc, const char **argv)
{
   program_mode_t program_mode = PROGRAM_MODE_NIL;
   const char *program_name    = argv[0];

   if (argc < 2)
      {
         print_usage (program_name);
         return 1;
      }

   const char *arg = argv[1];
   if (strcmpci (arg, "--info") == 0 || strcmpci (arg, "-i") == 0)
      {
         print_info ();
         return 0;
      }
   else if (strcmpci (arg, "--help") == 0 || strcmpci (arg, "-h") == 0 || strcmpci (arg, "-?") == 0)
      {
         print_usage (program_name);
         return 0;
      }
   else if (strcmpci (arg, "-c") == 0 || strcmpci (arg, "--compress") == 0)
      {
         program_mode = PROGRAM_MODE_COMPRESS;
      }
   else if (strcmpci (arg, "-x") == 0 || strcmpci (arg, "--extract") == 0)
      {
         program_mode = PROGRAM_MODE_EXTRACT;
      }
   else if (arg[0] == '-')
      {
         fprintf (stderr, "[BARC ERROR] INVALID FLAG, USE --HELP\n");
         return 1;
      }
   else
      {
         fprintf (stderr, "[BARC ERROR] UNEXPECTED ARGUMENT, USE --HELP\n");
         return 1;
      }

   switch (program_mode)
      {
      case PROGRAM_MODE_COMPRESS:
         {
            if (argc < 4)
               {
                  fprintf (stderr,
                           "[BARC ERROR] COMPRESS REQUIRES <ARCHIVE> AND AT LEAST ONE <FILE>\n");
                  return 1;
               }

            FILE *arc = fopen (argv[2], "wb");
            if (!arc)
               {
                  fprintf (stderr, "[BARC ERROR] COULD NOT CREATE ARCHIVE %s\n", argv[2]);
                  return 1;
               }

            for (int i = 3; i < argc; i++)
               {
                  arcfile (argv[i], arc);
               }

            int32_t eofmark = -1;
            fwrite (&eofmark, sizeof (int32_t), 1, arc);
            fclose (arc);
            printf ("[BARC] ARCHIVED\n");
         }
         break;

      case PROGRAM_MODE_EXTRACT:
         {
            if (argc != 3)
               {
                  fprintf (stderr, "[BARC ERROR] EXTRACT REQUIRES <ARCHIVE>\n");
                  return 1;
               }

            FILE *arc = fopen (argv[2], "rb");
            if (!arc)
               {
                  fprintf (stderr, "[BARC ERROR] COULD NOT OPEN ARCHIVE %s\n", argv[2]);
                  return 1;
               }

            exarc (arc);
            fclose (arc);
            printf ("[BARC] EXTRACTED\n");
         }
         break;

      case PROGRAM_MODE_NIL:
         {
            assert (0 && "[BARC] SECRET ENDING UNLOCKED");
         }
         break;

      default:
         {
            assert (0 && "[BARC] UNHANDLED PROGRAM MODE");
         }
         break;
      }

   return 0;
}
