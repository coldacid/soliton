#define NAME		"iff2c"
#define DISTRIBUTION	"(Freeware) "
#define VERSION         "1"
#define REVISION	"0"
#define DATE	        "27.01.2002"
#define AUTHOR          "by Dirk Stöcker <stoecker@epost.de>"

/* Programmheader

	Name:		iff2c
	Author:		SDI
	Distribution:	Freeware
	Description:	iff to C source converter

 1.0   05.04.02 : first Version (redone based on the included files)
*/

#include <stdio.h>

#define EndGetM32(a)  (((((unsigned char *)a)[0])<<24)|((((unsigned char *)a)[1])<<16)| \
                       ((((unsigned char *)a)[2])<< 8)|((((unsigned char *)a)[3])))
#define EndGetM16(a)  (((((unsigned char *)a)[0])<< 8)|((((unsigned char *)a)[1])))
#define MAKE_ID(a,b,c,d) ((unsigned long) (a)<<24 | (unsigned long) (b)<<16 | \
                          (unsigned long) (c)<< 8 | (unsigned long) (d))
#define ID_FORM		 MAKE_ID('F','O','R','M')
#define ID_BMHD		 MAKE_ID('B','M','H','D')
#define ID_ILBM		 MAKE_ID('I','L','B','M')
#define ID_BODY		 MAKE_ID('B','O','D','Y')
#define ID_CMAP		 MAKE_ID('C','M','A','P')
#define version "$VER: " NAME " " VERSION "." REVISION " (" DATE ") " DISTRIBUTION AUTHOR

static char buffer[4096] = version;
static char buffb[20];
static char filename[200+1];
static char filenameU[200+1];
static int convert(FILE *in, FILE *out, char *name)
{
  int iffsize, chunksize, res = 20, i,j,error = 0, bb = 0, cm = 0, bd = 0;
  char * fn = "Image", *fnU = "IMAGE";

  for(i = j = 0; name[i] && j < 200; ++i)
  {
    if(name[i] >= 'a' && name[i] <= 'z')
    {
      filenameU[j] = name[i]-'a'+'A';
      filename[j++] = name[i];
    }
    else if(name[i] >= 'A' && name[i] <= 'Z')
    {
      filename[j] = name[i];
      filenameU[j++] = name[i];
    }
    else if(name[i] == '\\' || name[i] == '/' || name[i] == ':')
      j = 0;
    else
    {
      filename[j] = '_';
      filenameU[j++] = '_';
    }
  }
  if(j >= 4 && filenameU[j-1] == 'F' && filenameU[j-2] == 'F'
  && filenameU[j-3] == 'I' && filenameU[j-4] == '_')
    j-=4;
  if(j >= 5 && filenameU[j-1] == 'M' && filenameU[j-2] == 'B'
  && filenameU[j-3] == 'L' && filenameU[j-4] == 'I' && filenameU[j-5] == '_')
    j-=4;
  if(j)
  {
    filename[j] = filenameU[j] = 0;
    fn = filename; fnU = filenameU;
  }

  if(fread(buffer, 12, 1, in) == 1)
  {
    if(EndGetM32(buffer+0) == ID_FORM && EndGetM32(buffer+8) == ID_ILBM)
    {
      iffsize = EndGetM32(buffer+4)-4;
      while(iffsize && !error)
      {
        if(fread(buffer, 8, 1, in) != 1)
          ++error;
        else
        {
          iffsize -= 8;
          chunksize = EndGetM32(buffer+4);
          switch(EndGetM32(buffer))
          {
          case ID_CMAP:
            cm = 1;
            if(chunksize > 4096)
            {
              fprintf(stderr, "CMAP chunk is too large.\n");
              ++error;
            }
            else
            {
              i = chunksize + (((chunksize & 1) && iffsize > chunksize) ? 1 : 0);
              if(fread(buffer,i,1,in) != 1)
                ++error;
              else
              {
                iffsize -= i;
                fprintf(out, "#ifdef USE_%s_COLORS\nconst ULONG %s_colors"
                "[%d] =\n{\n"/*}*/, fnU, fn, chunksize);
                for(i = 0; i < chunksize/3; ++i)
                {
                  fprintf(out,"  0x%02lx%02lx%02lx%02lx,0x%02lx%02lx%02lx%02lx,0x%02lx%02lx%02lx%02lx,\n",
                  buffer[i*3],buffer[i*3],buffer[i*3],buffer[i*3],
                  buffer[i*3+1],buffer[i*3+1],buffer[i*3+1],buffer[i*3+1],
                  buffer[i*3+2],buffer[i*3+2],buffer[i*3+2],buffer[i*3+2]);
                }
                fprintf(out, /*{*/"};\n#endif\n");
              }
            }
            break;
          case ID_BMHD:
            bb = 1;
            if(chunksize != 20)
            {
              fprintf(stderr, "BMAP chunk has wrong size.\n");
              ++error;
            }
            else
            {
              i = chunksize + (((chunksize & 1) && iffsize > chunksize) ? 1 : 0);
              if(fread(buffb,i,1,in) != 1)
                ++error;
              iffsize -= i;
            }
            break;
          case ID_BODY:
            bd = 1;
            fprintf(out, "//ifdef USE_%s_BODY\nconst UBYTE %s_body[%d] = {\n", fnU, fn, chunksize);
            j = chunksize;
            while(chunksize && !error)
            {
              int k;
              if((i = chunksize) > 4096) i = 4096;
              if(fread(buffer,i,1,in) != 1)
                ++error;
              chunksize -= i;
              for(k = 0; k < i; ++k)
                fprintf(out, "0x%02x,%s", buffer[k], (k % 15) == 14 ? "\n" : "");
            }
            iffsize -= j;
            if((j & 1) && iffsize)
            {
              iffsize--;
              if(fread(buffer,1,1,in) != 1)
                ++error;
            }
            fprintf(out, /*{*/" };\n//endif\n");
            break;
          default:
            if((chunksize & 1) && iffsize > chunksize) ++chunksize;
            iffsize -= chunksize;
            while(chunksize && !error)
            {
              if((i = chunksize) > 4096) i = 4096;
              if(fread(buffer,i,1,in) != 1)
                ++error;
              chunksize -= i;
            }
            break;
          }
          if(bb == 1 && cm == 1)
          {
            bb = cm = 2;
            fprintf(out, "\n//Width %d Height %d Depth %d - converting...\n"
            "#define %s_WIDTH      %4d\n"
            "#define %s_HEIGHT     %4d\n"
            "#define %s_DEPTH      %4d\n"
            "#define %s_COMPRESSION%4d\n"
            "#define %s_MASKING    %4d\n\n"
            "#ifdef USE_%s_HEADER\nconst struct BitMapHeader %s_header =\n"
            "{ %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d };\n#endif\n\n",
            EndGetM16(buffb), EndGetM16(buffb+2), buffb[8],
            fnU, EndGetM16(buffb), fnU, EndGetM16(buffb+2), fnU, buffb[8],
            fnU, buffb[10], fnU, buffb[9], fnU, fn,
            EndGetM16(buffb), EndGetM16(buffb+2), EndGetM16(buffb+4),
            EndGetM16(buffb+6),buffb[8],buffb[9],buffb[10],0,
            EndGetM16(buffb+12),buffb[14],buffb[15],EndGetM16(buffb+16),
            EndGetM16(buffb+18));
          }
        }
      }
      if(error)
        fprintf(stderr, "Could not read necessary data.\n");
      else if(cm && bd && bb)
        res = 0;
    }
    else
      fprintf(stderr, "Not an IFF-ILBM file.\n");
  }
  else
    fprintf(stderr, "Could not read IFF header.\n");

  return res;
}

int main(int argc, char **argv)
{
  FILE *in = 0, *out = 0;
  int res;

  if(argc > 1)
  {
    if(!(in = fopen(argv[1], "rb")))
    {
      fprintf(stderr, "Could not open file %s.\n", argv[1]);
      return 20;
    }
  }
  if(argc > 2)
  {
    if(!(out = fopen(argv[2], "wb")))
    {
      if(in) fclose(in);
      fprintf(stderr, "Could not open file %s for writing.\n", argv[2]);
      return 20;
    }
  }

  res = convert(in ? in : stdin, out ? out : stdout, argc > 1 ? argv[1] : 0);
  if(out) fclose(out);
  if(in) fclose(in);
  return res;
}
