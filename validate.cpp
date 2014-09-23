/***************************************************************************
                          validate.c  -  description
                             -------------------
    begin                : Wed Jan 29 2003
    written              : 2003-2004 by Pierre Abbat
    email                : phma@phma.hn.org
    This file is in the public domain.
 ***************************************************************************/

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "main.h"
#include "validate.h"

/* ntc, nts, ndj, ndz */
#define bad_trigraph(str) (((str)[0]|0x20)=='n' && ((str)[1]|0x30)=='t' && (((str)[2]|0x30)=='s' || ((str)[2]|0x30)=='z'))

struct piece *rafsi;
int nrafsi;

int valid_cmene(char *str)
{int i;
 char *nocomma;
 nocomma=strip_commas(strdup(str));
 for (i=0;nocomma[i+1];i++)
     switch (chartype(nocomma[i])&3)
        {case 0: /* apostrophe */
         if (!isvowel(nocomma[i+1]))
            {if (debug_option)
                printf("%s: apostrophe not followed by vowel\n",nocomma);
             free(nocomma);
             return 0;
             }
         break;
         case 1: /* vowel */
         break;
         case 2: /* consonant */
         if (isslaka(nocomma[i+1]))
            {if (debug_option)
                printf("%s: consonant followed by apostrophe\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (iscons(nocomma[i+1]) && pairtype(nocomma[i],nocomma[i+1])==' ')
            {if (debug_option)
                printf("%s: bad consonant pair\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (bad_trigraph(nocomma+i))
            {if (debug_option)
                printf("%s: bad consonant triple\n",nocomma);
             free(nocomma);
             return 0;
             }
         break;
         case 3: /* invalid */
         if (debug_option)
            printf("%s: invalid character\n",nocomma);
         free(nocomma);
         return 0;
         }
 if (!alahum_option)
    for (i=0;nocomma[i+1];i++)
        if ((!strncmp(nocomma+i,"la",2) || !strncmp(nocomma+i,"doi",3)) &&
            (i==0 || !iscons(nocomma[i-1])))
           {if (debug_option)
               printf("%s: \"la\" or \"doi\" not followed by consonant\n",nocomma);
            free(nocomma);
            return 0;
            }
 if (!iscons(nocomma[strlen(nocomma)-1]))
    {if (debug_option)
        printf("%s: does not end in consonant\n",nocomma);
     free(nocomma);
     return 0;
     }
 free(nocomma);
 return 1;
 }

int valid_cmavo(char *str)
{int i,plus,equal,colon,space,consonant;
 char *nocomma;
 nocomma=strip_commas(strdup(str));
 for (i=plus=equal=colon=space=consonant=0;nocomma[i+1];i++)
     switch (chartype(nocomma[i])&3)
        {case 0: /* apostrophe */
         if (!isvowel(nocomma[i+1]))
            {if (debug_option)
                printf("%s: apostrophe not followed by vowel\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (i==0)
            {if (debug_option)
                printf("%s: begins with apostrophe\n",nocomma);
             free(nocomma);
             return 0;
             }
         break;
         case 1: /* vowel */
         if (isvowel(nocomma[i+1]))
            {switch (pairtype(nocomma[i],nocomma[i+1]))
                {case ':': /* these can occur only in two-letter cmavo */
                 colon++;
                 break;
                 case '+': /* these can occur in any cmavo */
                 plus++;
                 break;
                 case '=': /* these cannot occur in any cmavo */
                 equal++;
                 break;
                 case ' ': /* these cannot occur in any words unless a comma intervenes */
                 space++;
                 break;
                 }
             }
         break;
         case 2: /* consonant */
         if (isslaka(nocomma[i+1]))
            {if (debug_option)
                printf("%s: consonant followed by apostrophe\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (iscons(nocomma[i+1]))
            {if (debug_option)
                printf("%s: consonant cluster\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (i>0)
            {if (debug_option)
                printf("%s: consonant not at beginning\n",nocomma);
             free(nocomma);
             return 0;
             }
         break;
         case 3: /* invalid */
         if (debug_option)
            printf("%s: invalid character\n",nocomma);
             free(nocomma);
         return 0;
         }
 if ((i>1 && colon) || equal || space)
    {if (debug_option)
        printf("%s: invalid diphthong\n",nocomma);
     free(nocomma);
     return 0;
     }
 if (!isvowel(nocomma[i]))
    {if (debug_option)
        printf("%s: does not end in vowel\n",nocomma);
     free(nocomma);
     return 0;
     }
 free(nocomma);
 return 1;
 }

int valid_brivla_basic(char *str)
/* Basic brivla validity check. A full check requires attempting to
   break the brivla into rafsi, and the decomposition is returned.
   This routine checks the following:
   * Initial and medial consonants.
   * Stress.
   * Diphthongs (only 'y' next to vowel is checked).
   * No two consecutive syllables have 'y'.*/
{int i,syll,stress2,stressother,y;
 char *nocomma,*recomma;
 nocomma=strip_commas(strdup(str));
 for (i=0;nocomma[i];i++)
     switch (chartype(nocomma[i])&3)
        {case 0: /* apostrophe */
         if (!isaeiou(nocomma[i+1]))
            {if (debug_option)
                printf("%s: apostrophe not followed by vowel\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (!i)
            {if (debug_option)
                printf("%s: begins with apostrophe\n",nocomma);
             free(nocomma);
             return 0;
             }
         break;
         case 1: /* vowel */
         if (isvowel(nocomma[i+1]))
            if ((nocomma[i]|0x20)=='y' || (nocomma[i+1]|0x20)=='y')
            {if (debug_option)
                printf("%s: 'y' next to vowel\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (isslaka(nocomma[i+1]) && nocomma[i]=='y')
            {if (debug_option)
                printf("%s: 'y' next to apostrophe\n",nocomma);
             free(nocomma);
             return 0;
             }
         break;
         case 2: /* consonant */
         if (isslaka(nocomma[i+1]))
            {if (debug_option)
                printf("%s: consonant followed by apostrophe\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (iscons(nocomma[i+1]) && pairtype(nocomma[i],nocomma[i+1])==' ')
            {if (debug_option)
                printf("%s: bad consonant pair\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (i==0 && iscons(nocomma[i+1]) && pairtype(nocomma[i],nocomma[i+1])=='+')
            {if (debug_option)
                printf("%s: bad initial consonant pair\n",nocomma);
             free(nocomma);
             return 0;
             }
         if (bad_trigraph(nocomma+i))
            {if (debug_option)
                printf("%s: bad consonant triple\n",nocomma);
             free(nocomma);
             return 0;
             }
         break;
         case 3: /* invalid */
         if (debug_option)
            printf("%s: invalid character\n",nocomma);
         free(nocomma);
         return 0;
         }
 free(nocomma);
 recomma=resyllabify(str,0);
 for (syll=1,y=stress2=stressother=0,i=strlen(recomma)-1;i>=0;i--)
     switch (chartype(recomma[i])&3)
        {case 0: /* apostrophe or comma */
         if (!y)
            syll++;
         break;
         case 1: /* vowel */
         if (y && recomma[i]=='y')
            {if (debug_option)
                printf("%s: two 'y's in a row\n",recomma); /* e.g. ratymykiu; fagyycpi has already been rejected */
             free(recomma);
             return 0;
             }
         y=recomma[i]=='y';
         if (recomma[i]=='Y')
            {if (debug_option)
                printf("%s: stressed 'Y' in brivla\n",recomma);
             free(recomma);
             return 0;
             }
         if (recomma[i]<'Z')
            if (syll==2)
               stress2=1;
            else
               stressother=1;
         break;
         case 2: /* consonant */
         break;
         case 3: /* invalid */
         if (debug_option)
            printf("%s: invalid character\n",recomma);
         free(recomma);
         return 0;
         }
 if (!stress2)
    {if (debug_option)
        printf("%s: stress not on penult\n",recomma);
     free(recomma);
     return 0;
     }
 free(recomma);
 return 1;
 }

void init_rafsi(char *word)
/* Makes a single piece consisting of the word. */
{rafsi=(struct piece *)malloc(sizeof(struct piece));
 nrafsi=1;
 rafsi[0].type=BRIVLA;
 rafsi[0].string=word;
 }

void write_rafsi()
{int i;
 for (i=0;i<nrafsi;i++)
     {switch (rafsi[i].type)
        {case KRARAFSI:
         putchar('(');
         break;
         case MIJRAFSI:
         putchar('-');
         break;
         case FAMRAFSI:
         putchar('-');
         break;
         case BRIVLA:
         putchar('(');
         break;
         }
      fputs(rafsi[i].string,stdout);
      switch (rafsi[i].type)
        {case KRARAFSI:
         putchar('-');
         break;
         case MIJRAFSI:
         putchar('-');
         break;
         case FAMRAFSI:
         ;
         case BRIVLA:
         putchar(')');
         break;
         }
      putchar(' ');
      }
 putchar('\n');
 }

void free_rafsi()
/* Frees all strings, then frees rafsi. */
{int i;
 for (i=0;i<nrafsi;i++)
     free(rafsi[i].string);
 free(rafsi);
 rafsi=NULL;
 nrafsi=0;
 }

void insert_rafsi(int n)
/* Inserts an empty rafsi before the nth rafsi. */
{rafsi=(struct piece *)realloc(rafsi,(++nrafsi)*sizeof(struct piece));
 if (!rafsi)
    {fputs("dukse sorcu calenu raflanli mi'e valfendi\n",stderr);
     exit(1);
     }
 memmove(rafsi+n+1,rafsi+n,(nrafsi-n-1)*sizeof(struct piece));
 if (rafsi[n].type==BRIVLA)
    {rafsi[n].type=KRARAFSI;
     rafsi[n+1].type=FAMRAFSI;
     }
 else if (rafsi[n].type==KRARAFSI)
    rafsi[n+1].type=MIJRAFSI;
 else if (rafsi[n].type==FAMRAFSI)
    rafsi[n].type=MIJRAFSI;
 rafsi[n].string=NULL;
 }

void split_rafsi(int n,int pos)
/* Splits rafsi n after pos characters. */
{if (debug_option)
    printf("Split {%s} ",rafsi[n].string);
 insert_rafsi(n);
 rafsi[n].string=strdup(rafsi[n+1].string);
 if (!rafsi[n].string)
    {fputs("dukse sorcu calenu raflanli mi'e valfendi\n",stderr);
     exit(1);
     }
 strcpy(rafsi[n+1].string,rafsi[n].string+pos);
 rafsi[n].string[pos]=0;
 rafsi[n].string=(char *)realloc(rafsi[n].string,strlen(rafsi[n].string)+1);
 rafsi[n+1].string=(char *)realloc(rafsi[n+1].string,strlen(rafsi[n+1].string)+1);
 if (debug_option)
    printf("into {%s} and {%s}\n",rafsi[n].string,rafsi[n+1].string);
 }

char *split_y(int n)
{int pos;
 char *ppos;
 ppos=strchr(rafsi[n].string,'y');
 if (ppos)
    {pos=ppos-rafsi[n].string;
     if (rafsi[n].string[pos+1])
        split_rafsi(n,pos+1);
     if (pos)
        {split_rafsi(n,pos);
         rafsi[n+1].type=RAFTERJOHE;
         }
     }
 return ppos;
 }

int islujvo_ypart(char *str)
/* Checks for valid part of lujvo after 'y' splitting, heeding the
   r-hyphen rule. Returns 0 if invalid, >0 if no hyphen, <0 if hyphen.
   Absolute value is number of pieces, not counting the hyphen.
   Returns 0 if the part is a long fu'ivla rafsi; that is handled elsewhere.

   If ccvvcv_rafsi_option is set, the last part can be a fu'ivla rafsi
   of the form CCVVC. */
{char *tmp,*curr,*prev,*second;
 int pieces,hyphen;
 tmp=prev=NULL;
 pieces=hyphen=0;
 curr=str;
 if (monmapti("CUVrC",str) || monmapti("CUVnr",str))
    {curr=monmapti("CUVC",str);
     hyphen=1;
     pieces++;
     }
 second=curr;
 while (tmp=is3rafsi(curr))
    {prev=curr;
     curr=tmp;
     pieces++;
     }
 tmp=monmapti("$",curr);
 if (!tmp)
    tmp=monmapti("$",isgismu(curr));
 if (!tmp)
    tmp=monmapti("$",isgismu(prev));
 if (!tmp)
    tmp=monmapti("$",is4rafsi(prev));
 if (!tmp && ccvvcv_rafsi_option)
    tmp=monmapti("ICUVC$",prev);
 /* If there is a hyphen and there are only two pieces, the morpheme
    after a hyphen must be a CVV or CVC rafsi, a four-letter-rafsi, or a gismu.
    Anything else indicates a possible fu'ivla or fu'ivla rafsi. */
 if (pieces==2 && hyphen && !monmapti("CUV",second) && !monmapti("CVC",second)
     && !isgismu(second) && !is4rafsi(second) && !(ccvvcv_rafsi_option && monmapti("ICUVC",second)))
    tmp=NULL;
 if (tmp)
    return pieces*(hyphen?-1:1);
 else
    return 0;
 }

char *split_r(int n)
/* Split the first rafsi off. If it has an r-hyphen, split that off too. */
{int pos;
 char *ppos;
 ppos=rafsi[n].string;
 if (monmapti("CUVrC..",ppos) || monmapti("CUVnr..",ppos))
    /* ".." is required to avoid breaking fu'ivla of the form CVVrCV, such as {fu'arka}. */
    {ppos=monmapti("CUV",ppos);
     while (isslaka(*ppos))
        ppos++;
     pos=ppos-rafsi[n].string;
     if (rafsi[n].string[pos+1])
        split_rafsi(n,pos+1);
     if (pos)
        split_rafsi(n,pos);
     rafsi[n+1].type=RAFTERJOHE;
     }
 else
    /* Split the 3-letter rafsi off. It must be followed by at least 3 letters,
       the first of which is a consonant, else the whole string is a long rafsi. */
    {ppos=is3rafsi(ppos);
     if (monmapti("C..",ppos))
        {pos=ppos-rafsi[n].string;
         split_rafsi(n,pos);
         }
     else
        ppos=NULL;
     }
 return ppos;
 }

char *islujvo1(char *str)
/* Checks for lujvohood, ignoring the tosmabru and r-hyphen rules
   and the last letter if it's a vowel. Used in israfsifuhivla. */
{char *tmp,*prev;
 tmp=prev=NULL;
 while (tmp=is3rafsi(str))
    {prev=str;
     str=tmp;
     }
 tmp=monmapti("$",str);
 if (!tmp)
    tmp=monmapti("V$",str);
 if (!tmp)
    tmp=monmapti("$",is4rafsi(str));
 if (!tmp)
    tmp=monmapti("V$",is4rafsi(str));
 if (!tmp)
    tmp=monmapti("$",is4rafsi(prev));
 if (!tmp)
    tmp=monmapti("V$",is4rafsi(prev));
 return tmp;
 }

char *islujvo2(char *str)
{int x;
 if (monmapti("CUVrC",str) || monmapti("CUVnr",str))
    return islujvo1(monmapti("CUVC",str));
 else
    return NULL;
 }

int israfsifuhivla(char *str)
{char *str2,*ptr;
 int ret,startpos,vowelcount;
 if (iscons(str[strlen(str)-1]))
    {str2=strdup(str);
     str2=(char *)realloc(str2,strlen(str2)+2);
     strcat(str2,"a"); /* append an arbitrary vowel if the string ends in a consonant */
     }
 else
    str2=str;
 for (startpos=1;str2[startpos] && !iscons(str2[startpos]);startpos++);
 if (pair5(str2))
    if (monmapti("CC",str2))
       if (isslinkuhi(str2) || !validinitial(str2) || monosyllabic(str2))
          ret=0;
       else
          ret=1;
    else
       if (isslinkuhi(str2+startpos) || !validinitial(str2+startpos) || monosyllabic(str2+startpos))
          ret=1;
       else
          ret=0;
 else
    ret=0;
 for (ptr=str;*ptr;ptr++)
     if (iscons(*ptr))
        vowelcount=0;
     else
        vowelcount++;
 ret&=(vowelcount<=1) && !(islujvo2(str) || islujvo1(str) || islujvo1(str+1));
 if (str2!=str)
    free(str2);
 return ret;
 }

int validrafsi(char *str)
{int valid;
 char *end,*nocomma;
 valid=0;
 nocomma=strip_commas(strdup(str));
 end=is3rafsi(nocomma);
 if (end)
    valid|=!*end;
 end=is4rafsi(nocomma);
 if (end)
    valid|=!*end;
 end=isgismu(nocomma);
 if (end)
    valid|=!*end;
 if (long_rafsi_option)
    valid|=israfsifuhivla(nocomma);
 if (ccvvcv_rafsi_option)
    valid|=!!monmapti("ICUVC$",nocomma)/*||monmapti("ICUVCV$",nocomma)*/;
 free(nocomma);
 return valid;
 }

int yvalid(int i,int first)
/* Checks whether a y-hyphen is needed between before and after.
   If the y-hyphen is present, but not needed, it is invalid.
   It is needed if any of these is true:
   * The concatenation of before and after contains an invalid consonant cluster.
   * The preceding rafsi is long.
   * The concatenation is a tosmabru, and this is the first y-hyphen.
   * The following rafsi is a fu'ivla rafsi, and the long fu'ivla rafsi option is true.
   */
{char joint[4],*before,*after;
 int tos,j;
 before=rafsi[i-1].string;
 after=rafsi[i+1].string;
 joint[0]=before[strlen(before)-1];
 if (after[0]==',') /* this happens if the -r option is specified */
    {joint[1]=after[1];
     joint[2]=after[2];
     }
 else
    {joint[1]=after[0];
     joint[2]=after[1];
     }
 joint[3]=0;
 /* bad_trigraph checks only for nytc e lo simsa. It is not necessary
    to check for ntyc because a rafsi ending in nt is long. */
 if (bad_trigraph(joint) || pairtype(joint[0],joint[1])==' ')
    return 1; /* y is required to prevent invalid consonant cluster */
 if (strlen(before)>3 && iscons(before[strlen(before)-1]))
    return 1; /* y is required after long rafsi */
 tos=0;
 if (first)
    /* Check for tosmabru. before must be CVC, and after must consist of
       at least zero CVC followed by CVCC, CVCCV, or y. All consonant
       clusters formed by adjacent rafsi must be initial. No need to
       check the consonants of before and after, since they have already
       been checked. */
    {tos=1;
     joint[2]=0;
     for (j=i+1;j<nrafsi && tos;j++)
         {if (j<nrafsi-1)
             {joint[0]=rafsi[j].string[strlen(rafsi[j].string)-1];
              joint[1]=rafsi[j+1].string[0];
              if (iscons(joint[0]) && iscons(joint[1]) && pairtype(joint[0],joint[1])==' ')
                 tos=0;
              }
          after=isgismu(rafsi[j].string);
          if (!after)
             after=is4rafsi(rafsi[j].string);
          if (!after)
             after=is3rafsi(rafsi[j].string);
          if (!after || *after)
             tos=0;
          if (monmapti("CVN",rafsi[j].string))
             tos=0;
          if (monmapti("C.V",rafsi[j].string))
             tos=0;
          if (monmapti("CVI",rafsi[j].string) || rafsi[j+1].type==RAFTERJOHE)
             break;
          }
     if (debug_option)
        printf("tosmabru test: y %srequired\n",tos?"":"not ");
     }
 if (tos)
    return 1;
 if (long_rafsi_option && israfsifuhivla(rafsi[i+1].string))
    return 1;
 return 0;
 }

int valid_brivla_full(int i)
/* Checks whether the ith token is a valid brivla. If raflanli_option
   is true, replaces the token with the rafsi. */
{int n,nrafsi_in_part,valid;
 init_rafsi(strdup(pieces[i].string));
 for (n=0;n<nrafsi;n++)
     split_y(n);
 valid=1;
 for (n=0;n<nrafsi;n++)
     {nrafsi_in_part=islujvo_ypart(rafsi[n].string);
      if (debug_option)
         printf("%s %d\n",rafsi[n].string,nrafsi_in_part);
      if (n>0 && nrafsi_in_part<0) /* r-hyphen cannot occur after y-hyphen */
         {valid=0;
          if (debug_option)
             printf("%s: r-hyphen after y-hyphen\n",pieces[i].string);
          }
      }
 for (n=0;n<nrafsi && valid;n++)
     if (islujvo_ypart(rafsi[n].string))
        split_r(n);
 for (n=0;n<nrafsi;n++)
     if (rafsi[n].type==RAFTERJOHE)
        if (!yvalid(n,n==1))
           {valid=0;
            if (debug_option)
              printf("%s: unnecessary y-hyphen\n",pieces[i].string);
            }
        else;
     else
        if (rafsi[n].type!=BRIVLA) /* Test every rafsi, because some were missed, e.g. invalid fu'ivla rafsi after 4-letter rafsi. */
           valid&=validrafsi(rafsi[n].string);
 /*write_rafsi();*/
 /*printf("valid=%d\n",valid);*/
 if (raflanli_option && valid)
    {free(pieces[i].string);
     pieces[i].string=NULL;
     for (n=1;n<nrafsi;n++)
         insert_piece(i);
     memmove(pieces+i,rafsi,n*sizeof(struct piece));
     free(rafsi);
     rafsi=NULL;
     nrafsi=0;
     }
 else
    free_rafsi();
 return valid;
 }
