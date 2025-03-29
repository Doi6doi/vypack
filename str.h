#ifndef STRH
#define STRH

#include <stdbool.h>

#define STR_LEN -1

/// string típus
typedef struct Str * Str;
/// string pool típus
typedef struct Strs * Strs;

/// új string készítése
Str strCreate( Strs pool, char * chars, unsigned len );
/// string felszabadítása
void strFree( Str );
/// c string
char * strC( Str );
/// string hossza
unsigned strL( Str );
/// string pool
Strs strS( Str );
/// stringből kivágás
void strRemove( Str, unsigned at, unsigned len );
/// stringbe beillesztés
void strInsert( Str, unsigned at, Str part );
/// stringbe c string beillesztés
void strInsertC( Str, unsigned at, char * part, int len );
/// stringek összehasonlítása
bool strSame( Str, Str );
/// string összehasonlítása C stringgel
bool strSameC( Str, char * );
/// teljes másolat
Str strCopy( Str );
/// rész string
Str strSub( Str, unsigned at, unsigned len );
/// karakter keresés elölről
int strFind( Str s, char ch );
/// karakter keresés hátulról
int strFindLast( Str s, char ch );
/// string csere
void strReplace( Str s, Str find, Str repl );

/// string pool létrehozása
Strs strsCreate();
/// string pool törlése
void strsFree( Strs );

void intToStr( int i, Str s );
int strToInt( Str s );

#endif // STRH

