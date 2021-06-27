/*
 rekotoppm - convert RKP and REKO cardsets to PPM graphics format
 Copyright (C) 2004 by Dirk Stöcker <doc@dstoecker.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Version history
Version Date       Author   Changes
------- ---------- -------- --------------------------------------------------
  1.0   28.03.2004 sdi      first release
  1.1   14.04.2015 sdi      add info option
*/

#define VERSION "1.1 (14.04.2015)"

/* This program is not optimized at all. All the conversion use the
worst algorithm possible, but the source-code is much easier this way. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Mode {REKO_REKODT39, REKO_MREKO, REKO_NORMAL};

#define FULLHEIGHT              4  /* Number of cards vertically */
#define NORMWIDTH               14
#define FULLWIDTH               17
#define REKO_I                  55 /* Cards in REKO I cardset */
#define REKO_II                 59 /* Cards in REKO II cardset */
#define REKO_III                68 /* Cards in REKO III cardset */

#define BORDERCOL               0xF0
#define BACKCOL                 0x00

#define EndGetM32(a)  (((((unsigned char *)a)[0])<<24)| \
                       ((((unsigned char *)a)[1])<<16)| \
                       ((((unsigned char *)a)[2])<< 8)| \
                       ((((unsigned char *)a)[3])))
#define EndGetM16(a)  (((((unsigned char *)a)[0])<< 8)| \
                       ((((unsigned char *)a)[1])))

#define EndGetI32(a)  (((((unsigned char *)a)[3])<<24)| \
                       ((((unsigned char *)a)[2])<<16)| \
                       ((((unsigned char *)a)[1])<< 8)| \
                       ((((unsigned char *)a)[0])))
#define EndGetI16(a)  (((((unsigned char *)a)[1])<< 8)| \
                       ((((unsigned char *)a)[0])))

static const unsigned char Mapping[REKO_III][2] = {
  {13,2}, {13,1}, {13,0},
  { 0,0}, { 0,1}, { 0,2}, { 0,3},
  { 1,0}, { 1,1}, { 1,2}, { 1,3},
  { 2,0}, { 2,1}, { 2,2}, { 2,3},
  { 3,0}, { 3,1}, { 3,2}, { 3,3},
  { 4,0}, { 4,1}, { 4,2}, { 4,3},
  { 5,0}, { 5,1}, { 5,2}, { 5,3},
  { 6,0}, { 6,1}, { 6,2}, { 6,3},
  { 7,0}, { 7,1}, { 7,2}, { 7,3},
  { 8,0}, { 8,1}, { 8,2}, { 8,3},
  { 9,0}, { 9,1}, { 9,2}, { 9,3},
  {10,0}, {10,1}, {10,2}, {10,3},
  {11,0}, {11,1}, {11,2}, {11,3},
  {12,0}, {12,1}, {12,2}, {12,3},
  {13,3}, {14,3}, {15,3}, {16,3},
  {14,0}, {15,0}, {16,0},
  {14,1}, {15,1}, {16,1},
  {14,2}, {15,2}, {16,2}
};

/* Calculate number of cards horizontally */
static int GetFullWidth(int cards)
{
  int HCnt;

  if(cards <= REKO_I) /* REKO-I cardset */
    HCnt=NORMWIDTH;
  else if(cards <= REKO_III) /* REKO-II or REKO-III cardset */
    HCnt=FULLWIDTH;
  else /* Unknown cardset */
    HCnt=cards/FULLHEIGHT+(cards%FULLHEIGHT>0);
  return HCnt;
}

#define SetVal(buffer,width,height,fullwidth,x,y,val,r,g,b) \
{int setval = (((x)*(width))+((val)%(width)))*3+\
(((y)*(height))+((val)/(width)))*(fullwidth)*3; \
buffer[setval++] = (r); buffer[setval++] = (g); buffer[setval] = (b);}

static void GetXY(int num, int *x, int *y, enum Mode mode)
{
  if(num < REKO_III)
  {
    *x = Mapping[num][0];
    *y = Mapping[num][1];
    if(num < 3)
    {
      if(mode == REKO_MREKO)
      {
        switch(num)
        {
        case 0: *y = 1; break;
        case 1: *y = 2; break;
        case 2: *y = 3; break;
        };
      }
      else if(mode == REKO_REKODT39)
      {
        *y = 1;
        switch(num)
        {
        case 0: *x = 13; break;
        case 1: *x = 14; break;
        case 2: *x = 15; break;
        }
      }
    }
    else if(num >= REKO_I && mode != REKO_NORMAL)
    {
      if(num < REKO_II) /* stack cards */
        *y = 0;
      else if(mode == REKO_REKODT39 && num < REKO_II+2)
      {
        *x = 13; *y = num-REKO_II+2;
      }
      else
        (*y)++;
    }
  }
  else
  {
    *x = num/FULLHEIGHT;
    *y = num%FULLHEIGHT;
  }
}

static void MakeBackCard(unsigned char *buffer, int width, int height,
int fullwidth, enum Mode mode)
{
  int j;
  int x,y;
  GetXY(1,&x,&y,mode);

  for(j = 0; j < width*height; ++j) /* clear background */
    SetVal(buffer,width,height,fullwidth,x,y,j,BACKCOL,BACKCOL,BACKCOL);

  for(j = 1; j < width-1; ++j) /* upper,bottom border */
  {
    SetVal(buffer,width,height,fullwidth,x,y,j,BORDERCOL,BORDERCOL,BORDERCOL);
    SetVal(buffer,width,height,fullwidth,x,y,j+width*(height-1),
    BORDERCOL,BORDERCOL,BORDERCOL);
  }

  for(j = width; j < width*(height-1); j+=width) /* left,right border */
  {
    SetVal(buffer,width,height,fullwidth,x,y,j,BORDERCOL,BORDERCOL,BORDERCOL);
    SetVal(buffer,width,height,fullwidth,x,y,j+width-1,
    BORDERCOL,BORDERCOL,BORDERCOL);
  }
}

static int SaveField(FILE *out, unsigned char *buffer, int width,
int height, int depth)
{
  fprintf(out,"P6\n%d\n%d\n%d\n",width,height,depth);
  fflush(out);
  if(fwrite(buffer, width*height*3, 1, out) != 1)
    return 30;
  return 0;
}

static int GetPCREKO(FILE *in, FILE *out, enum Mode mode, int back, int info)
{
  int res = 20;
  unsigned char header[21];

  if(fread(header, 21, 1, in) == 1)
  {
    if(!strncmp("CREKO", header, 5))
    {
      int bodysize, cardsize, width, height, depth, cards,fullwidth;
      bodysize = EndGetI32(header+7);
      cardsize = EndGetI32(header+11);
      width    = EndGetI16(header+15);
      height   = EndGetI16(header+17);
      depth    = header[19];
      cards    = header[20];
      fullwidth = GetFullWidth(cards+2)*width;
      if(info)
      {
        printf("CardSize: %d\n"
               "Height:   %d\n"
               "Width:    %d\n"
               "Depth:    %d\n"
               "Cards:    %d\n"
               "FullSize: %d\n", cardsize, height, width, depth, cards, cards*(4+cardsize)+22);
      }
      if(((header[5] == 'D' && header[6] == ' ' && bodysize == 681492
      && cardsize == 11440 && depth == 8)
      || (!header[5] && !header[6] && bodysize == 1304388 && cardsize == 22880))
      && width == 88 && height == 130 && cards == 57)
      {
        unsigned char *buffer;
        if((buffer = (unsigned char *)malloc(fullwidth*height*4*3)))
        {
          unsigned char *tmp;
          memset(buffer, 0, fullwidth*height*4*3);

          switch(depth)
          {
          case 16:
            if((tmp = (unsigned char *) malloc(cardsize+4)))
            {
              int x, y;
              int i, card = 0;

              res = 0;
              while(card < cards)
              {
                if(fread(tmp, cardsize+4, 1, in) != 1)
                {
                  res = 10; break;
                }
                else
                {
                  GetXY(card+2, &x, &y, mode);
                  for(i = 4; i < cardsize; i += 2)
                  {
                    SetVal(buffer,width,height,fullwidth,x,y,(i-4)/2,
                    (tmp[i+1]<<1)&0xF8,                /* red */
                    (tmp[i+1]<<6 | tmp[i]>>2)&0xF8,    /* green */
                    (tmp[i]<<3)&0xF8);                 /* blue */
                  }
                  ++card;
                }
              }
              free(tmp);
            }
            break;
          case 8:
            if((tmp = (unsigned char *) malloc(cardsize+516)))
            {
              int x, y;
              int i, j, card = 0;

              res = 0;
              while(card < cards)
              {
                if(fread(tmp, cardsize+516, 1, in) != 1)
                {
                  res = 10; break;
                }
                else
                {
                  GetXY(card+2, &x, &y, mode);
                  for(i = 0; i < cardsize; ++i)
                  {
                    j = 4+2*tmp[516+i];
                    SetVal(buffer,width,height,fullwidth,x,y,i,
                    (tmp[j+1]<<1)&0xF8,                /* red */
                    (tmp[j+1]<<6 | tmp[j]>>2)&0xF8,    /* green */
                    (tmp[j]<<3)&0xF8);                 /* blue */
                  }
                  ++card;
                }
              }
              free(tmp);
            }
            break;
          }
          if(back)
            MakeBackCard(buffer,width,height,fullwidth,mode);
          if(!res && !info)
            res = SaveField(out,buffer,fullwidth,height*4,255);
          free(buffer);
        }
      }
    }
  }
  return res;
}

static int GetREKO(FILE *in, FILE *out, enum Mode mode, int info)
{
  int res = 20;
  unsigned char header[21];

  if(fread(header, 21, 1, in) == 1)
  {
    if(!strncmp("EKO", header, 3))
    {
      int cardsize, width, height, depth, cards,fullwidth,modeid;
      cardsize = EndGetM32(header+7);
      height   = EndGetM16(header+11);
      width    = EndGetM16(header+13);
      modeid   = EndGetM32(header+15);
      depth    = header[19];
      cards    = header[20];
      if(info)
      {
        printf("CardSize: %d\n"
               "Height:   %d\n"
               "Width:    %d\n"
               "ModeId:   0x%x%s\n"
               "Depth:    %d\n"
               "Cards:    %d\n", cardsize, height, width, modeid,
               (modeid & 0x800) ? " HAM" : "", depth, cards);
      }
      if(mode == REKO_REKODT39)
      {
        if(cards > REKO_II)
          cards = REKO_II;
      }
      fullwidth = GetFullWidth(cards)*width;
      if(info)
      {
        printf("FullSize: %d\n", cards*cardsize+22
        +(modeid & 0x800 ? (1<<(depth-2))*3 : (1<<depth)*3));
      }
      if(width == 88 && height == 130)
      {
        unsigned char *buffer;
        if((buffer = (unsigned char *)malloc(fullwidth*height*4*3)))
        {
          unsigned char *tmp, *pal;
          memset(buffer, 0, fullwidth*height*4*3);

          if(modeid & 0x800) /* HAM mode */
          {
            if((tmp = (unsigned char *) malloc((1<<(depth-2))*3+cardsize)))
            {
              pal = tmp+cardsize;
              if((fread(pal,(1<<(depth-2))*3,1,in) == 1))
              {
                int x, y;
                int i,j,v,h,l,r=0,g=0,b=0,card = 0;

                res = 0;
                while(card < cards)
                {
                  if(fread(tmp, cardsize, 1, in) != 1)
                  {
                    res = 10; break;
                  }
                  else
                  {
                    GetXY(card, &x, &y, mode);
                    for(i = 0; i < width*height; ++i)
                    {
                      if(!(i%width))
                      {
                        r = pal[0];
                        g = pal[1];
                        b = pal[2];
                      }
                      v = h = 0;
                      l = ((i/width)*width*depth+(i%width))/8;
                      for(j = 0; j < depth-2; ++j)
                      {
                        v |= (((tmp[l])>>(7-i%8))&1)<<j;
                        l += width/8;
                      }
                      for(j = 0; j < 2; ++j)
                      {
                        h |= (((tmp[l])>>(7-i%8))&1)<<j;
                        l += width/8;
                      }
                      if(!h) v *= 3;
                      else   v <<= (10-depth);

                      switch(h)
                      {
                      case 0: r = pal[v++]; g = pal[v++]; b = pal[v]; break;
                      case 1: b = v; break;
                      case 2: r = v; break;
                      case 3: g = v; break;
                      }
                      SetVal(buffer,width,height,fullwidth,x,y,i,r,g,b);
                    }
                    ++card;
                  }
                }
              }
              free(tmp);
            }
          }
          else
          {
            if((tmp = (unsigned char *) malloc((1<<depth)*3+cardsize)))
            {
              pal = tmp+cardsize;
              if((fread(pal,(1<<depth)*3,1,in) == 1))
              {
                int x, y;
                int i,j,b,l,card = 0;

                res = 0;
                while(card < cards)
                {
                  if(fread(tmp, cardsize, 1, in) != 1)
                  {
                    res = 10; break;
                  }
                  else
                  {
                    GetXY(card, &x, &y, mode);
                    for(i = 0; i < width*height; ++i)
                    {
                      b = 0;
                      l = ((i/width)*width*depth+(i%width))/8;
                      for(j = 0; j < depth; ++j)
                      {
                        b |= (((tmp[l])>>(7-i%8))&1)<<j;
                        /* build the index value */
                        l += width/8;
                      }
                      b *= 3;
                      SetVal(buffer,width,height,fullwidth,x,y,i,
                      pal[b],    /* red */
                      pal[b+1],  /* green */
                      pal[b+2]); /* blue */
                    }
                    ++card;
                  }
                }
              }
              free(tmp);
            }
          }
          if(!res && !info)
            res = SaveField(out,buffer,fullwidth,height*4,255);
          free(buffer);
        }
      }
    }
  }
  return res;
}

int main(int argc, char **argv)
{
  enum Mode mode = REKO_NORMAL;
  int backcard = 0, info = 0, i;

  while(--argc)
  {
    ++argv;
    if(!strcmp("-m", *argv) || !strcmp("--mreko", *argv))
      mode = REKO_MREKO;
    else if(!strcmp("-d", *argv) || !strcmp("--rekodt", *argv))
      mode = REKO_REKODT39;
    else if(!strcmp("-b", *argv) || !strcmp("--back", *argv))
      backcard = 1;
    else if(!strcmp("-i", *argv) || !strcmp("--info", *argv))
      info = 1;
    else
    {
      if(strcmp("-h", *argv) && strcmp("--help", *argv))
        fprintf(stderr, "Unknown option '%s'\n", *argv);
      fprintf(stderr, "rekotoppm Version %s - Options:\n"
      "-m or --mreko  create card order of Amiga datatype mreko\n"
      "-d or --rekodt create card order of Amiga V39 datatype\n"
      "-b or --back   create back card for RKP\n"
      "-i or --info   Output cardset information\n"
      "-h or --help   this help text\n", VERSION);
      return 20;
    }
  }
  switch(fgetc(stdin))
  {
  case 'P': i = GetPCREKO(stdin, stdout, mode, backcard, info); break;
  case 'R': i = GetREKO(stdin, stdout, mode, info); break;
  default: i = 20; break;
  }
  if(i)
    fprintf(stderr, "error converting file");
  return i;
}
