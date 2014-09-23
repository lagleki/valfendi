/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Sat Nov 30 19:22:52 EST 2002
    written              : 2002-2004 by Pierre Abbat
    email                : phma@phma.hn.org
    This file is in the public domain.
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include "main.h"
#include "validate.h"

int alahum_option,
/* With this off, {la} etc. may be followed by y'y and considered to
   break off; the remaining part, beginning with y'y, is invalid. With
   this on, {la} etc. must be followed by a consonant to break off. */
    ccvvcv_rafsi_option,
/* With this on, words of the form CCVVCV have rafsi according to the
   experimental proposal in the Book. */
    long_rafsi_option,
/* With this on, a wide variety of fu'ivla have rafsi. This and
   ccvvcv_rafsi_option are mutually exclusive. */
    stress_option,
/* With this off, stress must be indicated by capitalization for brivla
   to be lexed. With this on, a brivla with no stress indicated will be
   considered to end at the next pause or space, unless that is preceded by
   y'y. */
    resyllabify_option,
/* With this on, brivla are output fully syllabified. With it off, they
   are output as they are input. */
    raflanli_option,
/* With this on, a token representing a brivla is replaced with tokens
   representing its rafsi and hyphens. */
    debug_option,unknown_option;

char options[]="aclsdrf";
char chartypes[]=
/* 'abcdefghijklmnopqrstuvwxyz */
  "01222122?12222612?62212?252";
/* Bit 0 is set if vowel, 1 if consonant, 2 if hyphen-letter in lujvo. */
char tran[256]; /* used to translate characters to indices in chartypes and pairtable */
char pairtable[27][28]=
    /* 'abcdefghijklmnopqrstuvwxyz */
/*'*/{"                           ",
/*a*/ "         +           +     ",
/*b*/ "    +  +  + =++   =   +   +",
/*c*/ "      =    ==== = = =      ",
/*d*/ "  +    +  = +++   =   +   =",
/*e*/ "         +                 ",
/*f*/ "   +       +=++ + =++   +  ",
/*g*/ "  + +     + =++   =   +   +",
/*h*/ "                           ",
/*i*/ " :   :   :     :     :   = ",
/*j*/ "  = =  =    +=+   +   =    ",
/*k*/ "   +  +     =++ + =++      ",
/*l*/ "  +++ ++  ++ ++ + +++ + + +",
/*m*/ "  +++ ++  ++= + + =++ + +  ",
/*n*/ "  +++ ++  ++++  + +++ + + +",
/*o*/ "         +                 ",
/*p*/ "   +  +    +=++   =++   +  ",
/*q*/ "                           ",
/*r*/ "  +++ ++  +++++ +  ++ + + +",
/*s*/ "      =    ==== = = =   +  ",
/*t*/ "   =  +    ++++ + ==    +  ",
/*u*/ " :   :   :     :     :   = ",
/*v*/ "  + +  +  + =++   =       +",
/*w*/ "                           ",
/*x*/ "      +     =++   =++      ",
/*y*/ "                           ",
/*z*/ "  = =  =    +=+   +   =    "};
/* Consonants: = if a brivla can begin with these two letters, else + if a cmene can.
   Vowels: + if the diphthong can appear in lujvo, else : if it can appear in fu'ivla,
   else = if it can appear in cmene. */
/* chartype, iscons, isvowel, isslaka, pairtype defined in main.h */

struct piece *pieces;
int npieces;

/* the types of pieces (FLIBA etc.) are in main.h */

void filltran()
{int i;
 memset(tran,0x17,256); /* all non-Lojban letters are mapped to 'w' */
 for (i='a';i<='z';i++)
     {tran[i]=i-'a'+1;
      tran[i+'A'-'a']=i-'a'+1;
      }
 tran['\'']=tran[',']=0;
 }

int validinitial(char *str)
/* Returns nonzero if a brivla can begin with this. Returns 1 if
   valid initial consonant cluster or single consonant not followed by 'y',
   -1 if vowel, 0 otherwise. Vowels are distinguished from consonants
   so that when looking for secondary stress /LEkraTAIgo/ is broken
   into {lekra tai go} but /gernLIkaOne/ is not broken. */
{int i;
 while (*str==',') str++;
 switch (chartype(*str)&3)
    {case 0:;   /* apostrophe */
     case 3:    /* invalid character */
     return 0;
     case 1:    /* vowel */
     return -1;
     case 2:    /* consonant */
     for (i=0;iscons(str[i+1]);i++)
         {if (pairtype(str[i],str[i+1])!='=')
             return 0;
          }
     if (chartype(str[i+1])&4) /* 'y' or 'Y' */
        return 0;
     else
        return 1;
     }
 }

int cluster(char *str)
/* Returns true if str begins with CC or Cy. */
{int t1;
 t1=iscons(*str);
 if (t1) str++;
 while (*str==',') str++;
 return t1 && (iscons(*str) || *str=='y');
 }

char * readline()
/* Reads one line of arbitrary length. */
{char *line;
 int length;
 int ch;
 line=(char*)malloc(1);
 line[0]=0;
 length=0;
 do {ch=getchar();
     if (ch>31)
        {line[length]=ch;
         line=(char *)realloc(line,(++length)+1);
         line[length]=0;
         if (!line) /* realloc failed */
            {fputs("dukse ke seltcidu lijvelsku mi'e valfendi\n",stderr);
             exit(1);
             }
         }
     }
 while (ch!='\n' && ch>=0);
 return line;
 }

void init_pieces(char *line)
/* Makes a single piece consisting of the line. */
{pieces=(struct piece*)malloc(sizeof(struct piece));
 npieces=1;
 pieces[0].type=NARTERJUHO;
 pieces[0].string=line;
 }

void write_pieces()
{int i;
 for (i=0;i<npieces;i++)
     {switch (pieces[i].type)
        {case NARTERJUHO:
         putchar('?');
         break;
         case FLIBA:
         putchar('>');
         break;
         case CMAVO:
         putchar('-');
         break;
         case BRIVLA:
         case KRARAFSI:
         putchar('(');
         break;
         }
      fputs(pieces[i].string,stdout);
      switch (pieces[i].type)
        {case CMENE:
         putchar('.');
         break;
         case FLIBA:
         putchar('<');
         break;
         case BRIVLA:
         case FAMRAFSI:
         putchar(')');
         break;
         }
      if (pieces[i].type==KRARAFSI || pieces[i].type==MIJRAFSI || pieces[i].type==RAFTERJOHE)
         putchar('-');
      else
         putchar(' ');
      }
 putchar('\n');
 }

void free_pieces()
/* Frees all strings, then frees pieces. */
{int i;
 for (i=0;i<npieces;i++)
     free(pieces[i].string);
 free(pieces);
 pieces=NULL;
 npieces=0;
 }

void insert_piece(int n)
/* Inserts an empty piece before the nth piece. */
{pieces=(struct piece*)realloc(pieces,(++npieces)*sizeof(struct piece));
 if (!pieces)
    {fputs("dukse sorcu calenu fendi mi'e valfendi\n",stderr);
     exit(1);
     }
 memmove(pieces+n+1,pieces+n,(npieces-n-1)*sizeof(struct piece));
 pieces[n].type=NARTERJUHO;
 pieces[n].string=NULL;
 }

void split_piece(int n,int pos)
/* Splits piece n after pos characters. Any periods or commas at the break are removed. */
{int posl,posr;
 if (debug_option)
    printf("Split {%s} ",pieces[n].string);
 insert_piece(n);
 pieces[n].string=strdup(pieces[n+1].string);
 if (!pieces[n].string)
    {fputs("dukse sorcu calenu fendi mi'e valfendi\n",stderr);
     exit(1);
     }
 posl=posr=pos;
 while (pieces[n].string[posr]=='.' || pieces[n].string[posr]==',')
       posr++;
 while (posl && (pieces[n].string[posl-1]=='.' || pieces[n].string[posl-1]==','))
       posl--;
 strcpy(pieces[n+1].string,pieces[n].string+posr);
 pieces[n].string[posl]=0;
 pieces[n].string=(char *)realloc(pieces[n].string,strlen(pieces[n].string)+1);
 pieces[n+1].string=(char *)realloc(pieces[n+1].string,strlen(pieces[n+1].string)+1);
 if (debug_option)
    printf("into {%s} and {%s}\n",pieces[n].string,pieces[n+1].string);
 }

void stripends(char *string)
/* Strip leading and trailing periods and commas. */
{int i;
 i=strlen(string);
 while (i && (string[i-1]=='.' || string[i-1]==','))
    i--;
 string[i]=0;
 i=0;
 while (string[i]=='.' || string[i]==',')
    i++;
 memmove(string,string+i,strlen(string)+1-i);
 }

void spacepause(char *string)
/* Replace all spaces with pauses, unless preceded with commas. This is needed to lex written Lojban. */
{int i,ch;
 for (i=ch=0;string[i];i++)
     {if (string[i] == ' ')
         string[i]=(ch==',')?',':'.';
      }
 }

void split_pauses()
{int i;
 char *pos;
 i=0;
 while (i<npieces)
    {stripends(pieces[i].string);
     pos=strchr(pieces[i].string,'.');
     if (pos)
        split_piece(i,pos-pieces[i].string);
     else
        i++;
     }
 }

char *strip_commas(char *string)
/* Strips commas from a string in place. */
{char *src,*dst;
 src=dst=string;
 while (*dst=*src)
    if (*src++!=',')
       dst++;
 return string;
 }

char *resyllabify(char *string,int removeall)
/* Given a string possibly containing commas, resyllabify it so that the
   commas are in the canonical places: each pair of adjacent syllables
   is separated by a comma or y'y. The syllable break at a consonant cluster
   is placed before the cluster. Then all letters in the stressed syllable
   are capitalized. e.g.: {kernaUke} becomes {ke,RNAU,ke,}.
   The string is not modified in place; a copy is returned.
   If removeall is true, all commas are removed, else commas between vowels are retained.
   */
{char *nocomma,*recomma;
 int i,j,stress;
 nocomma=(char *)malloc(strlen(string)+1);
 recomma=(char *)malloc(strlen(string)*2+1);
 for (i=j=0;string[i];)
     if (string[i]==',' && (removeall || !((chartype(string[i+1])&1) && i && (chartype(string[i-1])&1))))
        i++;
     else
        nocomma[j++]=string[i++];
 nocomma[j]=0;
 for (i=j=0;nocomma[i];)
     {if (i && (chartype(nocomma[i-1])&3)==1 && (chartype(nocomma[i])&3)==2) /* VC -> V,C */
         recomma[j++]=',';
      if (i && (chartype(nocomma[i-1])&3)==1 && (chartype(nocomma[i])&3)==1 &&
          (pairtype(nocomma[i-1],nocomma[i])<=' ' || /* VV -> V,V if not a valid diphthong */
           (j>1 && (chartype(recomma[j-2])&3)==1)))                        /* VVV -> VV,V */
         recomma[j++]=',';
      recomma[j++]=nocomma[i++];
      }
 recomma[j]=0;
 for (i=0;recomma[i];i++)
     if (iscons(recomma[i]) && recomma[i]>='A' && recomma[i]<='Z')
        recomma[i]+='a'-'A';
 if (stress_option)
    {for (stress=i=0;recomma[i];i++)
         if (recomma[i]>='A' && recomma[i]<='Z' && isvowel(recomma[i]))
            stress=1;
     if (!stress)
        for (i=strlen(recomma)-1;i>=0 && stress<2;i--)
            {if (isslaka(recomma[i]))
                stress++;
             if (recomma[i]=='y')
                stress--; /* this gives weird results with "yy" or "ya", but words containing those aren't brivla */
             if (stress==1 && isvowel(recomma[i]))
                recomma[i]+='A'-'a';
             }
     }
 for (i=1;recomma[i];i++)
     if (recomma[i-1]>='A' && recomma[i-1]<='Z' && recomma[i]>='a' && recomma[i]<='z')
        recomma[i]+='A'-'a';
 for (i=strlen(recomma);i;i--)
     if (recomma[i]>='A' && recomma[i]<='Z' && recomma[i-1]>='a' && recomma[i-1]<='z')
        recomma[i-1]+='A'-'a';
 free(nocomma);
 return recomma;
 }

int monosyllabic(char *str)
{char *syl;
 int mono;
 syl=resyllabify(str,1);
 mono=!strchr(syl,',') && !strchr(syl,'\'');
 free(syl);
 return mono;
 }

char *decapitalize(char *string)
/* Decapitalize string in place. Don't use tolower in case
   Robin runs this in Turkish locale - it'd change dotless
   capital I to dotless lowercase i, which isn't Lojban. */
{char *a;
 a=string;
 while (*string)
    {if (*string>='A' && *string<='Z')
        *string+='a'-'A';
     string++;
     }
 return a;
 }

char *comma_corresponds(char *pos,char *a,char *b)
/* Returns the character in b that corresponds to pos in a,
   assuming that they are the same except for commas. */
{if (!pos)
 return pos;
 while (a<pos)
    {a++;
     b++;
     while (*a==',') a++;
     while (*b==',') b++;
     }
 return b;
 }

#define icomma_corresponds(pos,a,b) (comma_corresponds((pos)+(a),a,b)-b)

#define ends_in_cmene(string) ((chartype((string)[strlen(string)-1])&3)==2)
#define ends_in_y(string) ((chartype((string)[strlen(string)-1]))=='5')

char *tolcri_cmegadri(char *cmevau)
/* Returns a pointer to the first character after the last occurrence
   of {la}, {lai}, {la'i}, or {doi} that can be broken off. */
{char *a,*b,*nocomma;
 nocomma=decapitalize(strip_commas(strdup(cmevau)));
 for (a=nocomma+strlen(nocomma)-1,b=NULL;a>=nocomma && b==NULL;a--)
     if (chartype(*a)&2)
        {if (a>=nocomma+2 && !strncmp(a-2,"la",2))
            b=a-2;
         if (a>=nocomma+3 && !strncmp(a-3,"lai",3))
            b=a-3;
         if (a>=nocomma+4 && !strncmp(a-4,"la'i",4))
            b=a-4;
         if (a>=nocomma+3 && !strncmp(a-3,"doi",3))
            b=a-3;
         if (b && b>nocomma && (chartype(b[-1])&2)) /* cmegadri is preceded by consonant; ignore it */
            b=NULL;
         }
 a=comma_corresponds(a+1,nocomma,cmevau);
 free(nocomma);
 return b?a:NULL;
 }

void break_cmene(int i)
/* Assumes that the ith piece ends in a cmene. Breaks off the cmene and cmegadri. */
{char *cmene,*cmegadri;
 int cmene_pos,cmegadri_pos;
 cmene=cmegadri=tolcri_cmegadri(pieces[i].string);
 if (cmene)
    while ((chartype(*--cmene)&3)!=2);
 cmene_pos=cmene-pieces[i].string;
 cmegadri_pos=cmegadri-pieces[i].string;
 if (cmegadri)
    {split_piece(i,cmegadri_pos);
     pieces[i+1].type=CMENE;
     if ((chartype(pieces[i+1].string[0])&2)==0) /* doesn't begin with consonant */
        pieces[i+1].type=FLIBA /* this test is not needed with alahum */;
     pieces[i].type=CMAVO;
     }
 else
    pieces[i].type=CMENE;
 if (cmene && cmene_pos)
    split_piece(i,cmene_pos); /* new piece is marked NARTERJUHO */
 }

char *tolcri_lervla(char *lervau)
/* Assume that the ith piece ends in 'y'. If it consists only of vowels,
   commas, and apostrophes, it's a cmavo; else scan backward for a consonant.
   If the consonant is preceded by a consonant, it's an error; else break it
   off as a cmavo. */
{char *start,*nocomma;
 nocomma=decapitalize(strip_commas(strdup(lervau)));
 for (start=nocomma+strlen(nocomma)-1;start>nocomma;start--)
     if ((chartype(*start)&3)==2)
        break;
 start=comma_corresponds(start,nocomma,lervau);
 free(nocomma);
 return start;
 }

void break_lervla(int i)
{int startpos;
 char *start,*prev;
 start=tolcri_lervla(pieces[i].string);
 startpos=start-pieces[i].string;
 pieces[i].type=CMAVO;
 prev=start-1;
 while (prev>=pieces[i].string && *prev==',')
    prev--;
 if (prev>=pieces[i].string)
    if ((chartype(*prev)&3)==2) /* two consonants in a row */
       pieces[i].type=FLIBA;
    else
       split_piece(i,startpos);
 }

char *all_cmavo(char *string)
/* If string is one cmavo, return position of beginning of string.
   If string is more than one cmavo, return beginning of some middle cmavo.
   If string is not all cmavo, return NULL. */
{int conscount,i,prevchar;
 char *splitpoint;
 splitpoint=string;
 prevchar=1;
 for (i=conscount=0;string[i];i++)
     {if ((chartype(prevchar)&3)==2 && (chartype(string[i])&3)==2)
         return NULL;
      if ((chartype(prevchar)&3)<=1 && (chartype(string[i])&3)==2)
         {conscount++;
          if (!(conscount&(conscount-1)))
             splitpoint=string+i;
          }
      if (string[i]!=',' && (prevchar==1 || string[i]!='y')) /* ignore commas, skip 'y' because e.g. micycau is a lujvo */
         prevchar=string[i];
      }
 return splitpoint;
 }

void break_cmavo(int i,char *splitpoint)
{if (splitpoint==pieces[i].string)
    pieces[i].type=CMAVO;
 else
    split_piece(i,splitpoint-pieces[i].string);
 }

int lerfu_ji_brivla(char *string)
/* Scan backward until you find either yC or C*V. Returns the position
   of the y or C. If you find yC first,
   break between y and C. If you find C*V, the piece contains a brivla. */
{int i;
 char daha,dahare;
 for (daha=dahare=0,i=strlen(string)-1;i>=0;i--)
     {if (((chartype(string[i])&3)==2 && (chartype(dahare)&3)==1) ||
         ((chartype(string[i])&7)==5 && (chartype(daha)&3)==2))
         break;
      if (!isslaka(string[i]))
         {dahare=daha;
          daha=string[i];
          }
      }
 return i;
 }

char *monmapti(char *mon,char *str)
/* Matches str to a pattern mon of letters as follows:
   a-z: matches the same letter, upper or lower case.
   C:   matches any consonant.
   I:   matches any consonant followed by another consonant making an initial pair.
   N:   matches any consonant followed by another consonant making a non-initial pair.
   V:   matches any vowel other than 'y'.
   U:   matches the first vowel of a diphthong allowed in lujvo or a vowel followed by apostrophe.
   .:	matches any character.
   $:   matches the end of the string.
   Commas are ignored in str; apostrophes are ignored except when matched by U.
   Returns the first unmatched character of str if successful, else NULL. */
{char *next;
 if (!str) return str;
 for (next=str;*mon;str++,mon++)
     {while (isslaka(*str)) str++;
      if (*str) next=str+1;
      while (isslaka(*next)) next++;
      switch (*mon)
         {case 'C':
          if (!iscons(*str))
             return NULL;
          break;
          case 'I':
          if (!iscons(*str) || pairtype(*str,*next)!='=')
             return NULL;
          break;
          case 'N':
          if (!iscons(*str) || pairtype(*str,*next)!='+')
             return NULL;
          break;
          case 'V':
          if (!isvowel(*str) || *str=='y')
             return NULL;
          break;
          case 'U':
          if (!isvowel(*str) || (pairtype(*str,*next)!='+' && str[1]!='\''))
             return NULL;
          break;
          case '$':
          if (*str)
             return NULL;
          break;
          case '.':
          if (!*str)
             return NULL;
          break;
          default:
          if ((*str|0x20)!=*mon)
             return NULL;
          break;
          }
      }
 return str;
 }

char *is3rafsi(char *str)
{char *ret;
 ret=monmapti("ICV",str);
 if (!ret)
    ret=monmapti("CVC",str);
 if (!ret)
    ret=monmapti("CUV",str);
 return ret;
 }

char *is4rafsi(char *str)
{char *ret;
 ret=monmapti("CVCC",str);
 if (!ret)
    ret=monmapti("ICVC",str);
 return ret;
 }

char *isgismu(char *str)
{char *ret;
 ret=monmapti("CVCCV",str);
 if (!ret)
    ret=monmapti("ICVCV",str);
 return ret;
 }

int pair5(char *str)
/* Returns true if there is a consonant pair in the first five letters,
   not counting apostrophes, commas, and 'y'. */
{int i;
 char last;
 last='a';
 i=0;
 while (i<4 && *str && (!iscons(last) || !iscons(*str)))
    {/*printf("%d %c %c\n",i,last,*str);*/
     if (iscons(*str) || isvowel(*str) && *str!='y')
        {last=*str;
         ++i;
         }
     ++str;
     }
 return iscons(*str) && iscons(last);
 }

char *islujvo0(char *str)
/* Checks for lujvohood, ignoring the tosmabru and r-hyphen rules
   and everything after the first 'y'. Used in isslinkuhi. */
{char *tmp,*prev;
 tmp=prev=NULL;
 while (tmp=is3rafsi(str))
    {prev=str;
     str=tmp;
     }
 tmp=monmapti("$",str);
 if (!tmp)
    tmp=monmapti("y",str);
 if (!tmp)
    tmp=monmapti("$",isgismu(str));
 if (!tmp)
    tmp=monmapti("y",is4rafsi(str));
 if (!tmp)
    tmp=monmapti("$",isgismu(prev));
 if (!tmp)
    tmp=monmapti("y",is4rafsi(prev));
 return tmp;
 }

char *isslinkuhi(char *str)
/* A slinku'i, as far as word breaking is concerned, is anything that matches
   the regex
   ^C[raf3]*([gim]?$|[raf4]?y)
   but does not match the regex
   ^[raf3]*([gim]?$|[raf4]?y)
   where
   C matches any consonant
   [raf3] matches any 3-letter rafsi
   [raf4] matches any 4-letter rafsi
   [gim] matches any gismu.
   Anything after the first 'y' is ignored. It has no effect on where to break the
   word, only on whether the word is valid. */
{char *tmp;
 if (debug_option)
    fputs(str,stdout);
 tmp=islujvo0(str);
 if (!(str=monmapti("C",str)))
    {if (debug_option)
        puts(" is not a slinku'i");
     return str;
     }
 if (tmp)
    {if (debug_option)
        puts(" is not a slinku'i");
     return NULL;
     }
 tmp=islujvo0(str);
 if (debug_option)
    if (tmp)
       puts(" is a slinku'i");
    else
       puts(" is not a slinku'i");
 return tmp;
 }

int tolcri_brivlafaho(char *str)
/* str must be fully syllabified. Finds the end of the brivla in str.
   If it finds an error, returns 0. If there is nothing after the brivla,
   returns strlen(str). */
{int i,syll,y,end,cc;
 for (i=y=end=cc=0,syll=-1;!end && str[i];i++)
     {if (isslaka(str[i]))
         {if (!y)
             syll--;
          y=0;
          }
      if (isslaka(str[i+1]) && iscons(str[i+2]) && (iscons(str[i+3]) || (str[i+3]=='y' && isslaka(str[i+4]) && iscons(str[i+5]))))
         cc=1;
      if (iscons(str[i]) && iscons(str[i+1])) /* no need to test for CyC at beginning of word since that's invalid */
         cc=1;
      if (str[i]=='y') /* stressed Y cannot be part of a brivla so ignore it */
         y=1;
      if (cc && isvowel(str[i]) && str[i]<'Y')
         syll=2;
      if (syll==0 && isslaka(str[i]) && validinitial(str+i)>0)
         end=1;
      }
 if (end || syll==1)
    return i;
 else
    return 0;
 }

void break_brivlafaho(int i,char *commas)
{int endpos;
 endpos=icomma_corresponds(tolcri_brivlafaho(commas),commas,pieces[i].string);
 if (endpos)
    if (pieces[i].string[endpos])
       {split_piece(i,endpos);
        pieces[i].type=pieces[i+1].type;
        pieces[i+1].type=NARTERJUHO;
        }
    else;
 else
    pieces[i].type=FLIBA;
 }

void break_brivlakra(int i)
{int startpos,cmavo,j;
 for (startpos=1;!iscons(pieces[i].string[startpos]);startpos++);
 if (pair5(pieces[i].string))
    if (monmapti("CC",pieces[i].string))
       if (isslinkuhi(pieces[i].string) || !validinitial(pieces[i].string) || monosyllabic(pieces[i].string))
          cmavo=-1;
       else
          cmavo=0;
    else
       if (isslinkuhi(pieces[i].string+startpos) || !validinitial(pieces[i].string+startpos) || monosyllabic(pieces[i].string+startpos))
          cmavo=0;
       else
          cmavo=1;
 else
    cmavo=1;
 /*if (cmavo==0)
    for (j=0;j<startpos;j++)
        if (pieces[i].string[j]=='Y')
           cmavo=1;*/
 if (cmavo>0)
    {split_piece(i,startpos);
     }
 else if (!cmavo)
    pieces[i].type=BRIVLA;
 else
    {if (debug_option)
        printf("%s: no brivla beginning found\n",pieces[i].string);
     pieces[i].type=FLIBA;
     }
 }

void break_brivla(int i)
{char *commas;
 commas=resyllabify(pieces[i].string,0);
 if (resyllabify_option)
    {free(pieces[i].string);
     pieces[i].string=commas;
     }
 break_brivlafaho(i,commas);
 break_brivlakra(i);
 /*if (pair5(pieces[i].string))
    pieces[i].type=FLIBA;*/
 if (!resyllabify_option)
    free(commas);
 }

void break_lerfu_or_brivla(int i)
{int pos;
 pos=lerfu_ji_brivla(pieces[i].string);
 if (chartype(pieces[i].string[pos])=='5')
    split_piece(i,pos+1);
 else
    break_brivla(i);
 }

void process_line()
{int i;
 char *splitpoint;
 /* 1.  Convert all spaces to pauses unless preceded by comma. This is needed
    for handling written Lojban where stresses are not indicated. */
 spacepause(pieces[0].string);
 /* 2.  Break at all pauses (cannot pause in the middle of a word). */
 split_pauses();
 for (i=0;i<npieces;i+=!!pieces[i].type)
     {if (!pieces[i].type)
         if (ends_in_cmene(pieces[i].string))
            break_cmene(i);
         else if (ends_in_y(pieces[i].string))
            break_lervla(i);
         else if (splitpoint=all_cmavo(pieces[i].string))
            break_cmavo(i,splitpoint);
         else
            break_lerfu_or_brivla(i);
      if (pieces[i].type==CMENE && !valid_cmene(pieces[i].string))
         pieces[i].type=FLIBA;
      if (pieces[i].type==CMAVO && !valid_cmavo(pieces[i].string))
         pieces[i].type=FLIBA;
      if (pieces[i].type==BRIVLA && !valid_brivla_basic(pieces[i].string))
         pieces[i].type=FLIBA;
      if (pieces[i].type==BRIVLA && !valid_brivla_full(i))
         pieces[i].type=FLIBA;
      }
 }

void test()
{int i;
 char *splitpoint;
 /* 1.  Convert all spaces to pauses unless preceded by comma. This is needed
    for handling written Lojban where stresses are not indicated. */
 spacepause(pieces[0].string);
 /* 2.  Break at all pauses (cannot pause in the middle of a word). */
 split_pauses();
 for (i=0;i<npieces;i++)
     {if (israfsifuhivla(pieces[i].string))
         pieces[i].type=MIJRAFSI;
      else
         pieces[i].type=FLIBA;
      }
 }

void usage()
{puts("valfendi [options]\nLexes Lojban text.");
 puts("-a	cmevla can contain {la'u}, {doie}, etc.");
 puts("-c	fu'ivla of form CCVVCV have rafsi");
 puts("-l	a wide variety of fu'ivla have rafsi");
 puts("-s	stress is assumed in brivla if not indicated");
 puts("-r	brivla are output with syllables and stress indicated");
 puts("-f	lujvo are output as sequences of rafsi");
 puts("-d	outputs debugging information");
 }

int main(int argc, char *argv[])
{char *line;
 int i;
 i=1;
 alahum_option=stress_option=resyllabify_option=ccvvcv_rafsi_option=raflanli_option=unknown_option=0;
 while (i)
    switch (getopt(argc,argv,options))
       {case -1:
        i=0;
        break;
        case 'a':
        alahum_option=1;
        break;
        case 'c':
        ccvvcv_rafsi_option=1;
        break;
        case 'l':
        long_rafsi_option=1;
        break;
        case 's':
        stress_option=1;
        break;
        case 'd':
        debug_option=1;
        break;
        case 'r':
        resyllabify_option=1;
        break;
        case 'f': /* 'r' and 'l' are both taken */
        raflanli_option=1;
        break;
        case '?':
        unknown_option=1;
        break;
        }
 filltran();
 if (unknown_option)
    {usage();
     exit(1);
     }
 if (ccvvcv_rafsi_option && long_rafsi_option)
    {fputs("-c (only CCVVCV are rafsi fu'ivla) and -l (all type-3's and many others\nare rafsi fu'ivla) are incompatible.\n",stderr);
     exit(1);
     }
 do {line=readline();
     if (line[0]=='#')
        {puts(line);
         free(line);
         }
     else if (!feof(stdin))
        {init_pieces(line);
         process_line();
         write_pieces();
         free_pieces();
         }
     }
 while (!feof(stdin));
 return 0;
 }
