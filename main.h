/***************************************************************************
                          main.h  -  description
                             -------------------
    begin                : Wed Jan 29 2003
    written              : 2003 by Pierre Abbat
    email                : phma@webjockey.net
    This file is in the public domain.
 ***************************************************************************/

struct piece
{int type;     /* what type this string is, e.g. brivla */
 char *string;
 };

extern char chartypes[], pairtable[27][28],tran[256];
#define chartype(a) (chartypes[tran[0xff&(a)]])
#define iscons(a) ((chartype(a)&3)==2)
#define isvowel(a) ((chartype(a)&3)==1)
#define isaeiou(a) ((chartype(a)&7)==1)
#define isslaka(a) ((chartype(a)&3)==0)
#define isinval(a) ((chartype(a)&3)==3)
#define pairtype(a,b) (pairtable[tran[0xff&(a)]][tran[0xff&(b)]])

char *strip_commas(char *string);
/* Strips commas from a string in place. */

char *resyllabify(char *string,int removeall);
/* Given a string possibly containing commas, resyllabify it so that the
   commas are in the canonical places: each pair of adjacent syllables
   is separated by a comma or y'y. The syllable break at a consonant cluster
   is placed before the cluster. Then all letters in the stressed syllable
   are capitalized. e.g.: {kernaUke} becomes {ke,RNAU,ke,}.
   The string is not modified in place; a copy is returned. */

char *is3rafsi(char *str);
char *is4rafsi(char *str);
char *isgismu(char *str);
char *islujvo0(char *str);
int pair5(char *str);
int validinitial(char *str);
char *monmapti(char *mon,char *str);
int monosyllabic(char *str);
char *isslinkuhi(char *str);
void insert_piece(int n);

extern int alahum_option,raflanli_option,debug_option,ccvvcv_rafsi_option,long_rafsi_option,npieces;
extern struct piece *pieces;


/* the types of pieces */
#define FLIBA     -1
#define NARTERJUHO 0
#define CMAVO      1
#define CMENE      2
#define BRIVLAVAU  3
#define BRIVLAKRA  4
#define BRIVLA     5
#define BRIVLAFAHO 6
#define KRARAFSI   7
#define MIJRAFSI   8
#define FAMRAFSI   9
#define RAFTERJOHE 10
