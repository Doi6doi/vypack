#ifndef TOOLSH
#define TOOLSH

#include "arch.h"
#include "str.h"

typedef int Int;
typedef unsigned Uint;
typedef size_t Size;

typedef void * Obj;
typedef struct Stream * Stream;
typedef struct Arr * Arr;

typedef void (*Destructor)( Obj );

/// program meghalás
void fail( char * msg );
/// program meghalás egész változóval
void failI( char * msg, Int param );
/// program meghalás egy szöveges paraméterrel
void failS( char *msg, char * param );
/// program meghalás két szöveges paraméterrel
void failSS( char *msg, char * p1, char * p2 );
/// debug üzenet
void debug( char * msg );
/// debug üzenet egésszel
void debugI( char * msg, int i );
/// debug üzenet stringgel
void debugS( char * msg, char * s );
/// debug üzenet két stringgel
void debugSS( char * msg, char * p1, char * p2 );

/// objektum létrehozása
Obj objCreate( Size size );
/// objektum felszabadítása
void objFree( void * );

/// tömb létrehozása
Arr arrCreate( Uint n );
/// tömb felszámolása
void arrFree( Arr, Destructor );
/// tömbelem beállítása
void arrSet( Arr, Uint i, Obj );
/// új tömbelem hozzáadása
void arrAdd( Arr, Obj );
/// tömbelem beszúrása
void arrInsert( Arr, Uint, Obj );
/// tömbelem törlése
Obj arrRemove( Arr, Uint );
/// tömb mérete
Uint arrN( Arr );
/// egy tömbelem
Obj arrI( Arr, Uint i );
 
/// stream mérete
Uint streamSize( Stream );
/// stream bezárás
void streamFree( Stream );
/// mozgás streamben
void streamSeek( Stream, Size pos );
/// olvasás streamből
void streamRead( Stream, void *, Size n );
/// írás streambe
void streamWrite( Stream, void *, Size n );
/// egész olvasás streamből
Int streamReadInt( Stream );
/// egész írás streambe
void streamWriteInt( Stream, Int );
/// stream másolás
void streamCopy( Stream src, Stream dst, Size len ); 


/// létezik-e a fájl
bool fileExists( Str );
/// fájlnév mag (útvonal és kiterjesztés nélkül)
Str fileCore( Str );
/// a fájl egy könyvtár
bool fileIsDir( Str );
/// fájl mérete
Size fileSize( Str );
/// módosítási bélyeg
Uint fileModified( Str );
/// fájl megnyitása olvasásra
Stream fileOpen( Str );
/// új fájl létrehozása
Stream fileCreate( Str );
/// fájlnév kapcsolás
Str fileJoin( Str, Str );
/// fájl törlése
void fileDelete( Str );
/// könyvtár tartalma
Arr fileList( Str dir );
/// fájl futtatási jog
void fileSetExecutable( Str );
/// fájl módosítási dátum
void fileSetModified( Str, Uint );
  
/// rendszer temp könyvtár
Str dirTemp( Strs );
/// könyvtár létrehozása
void dirCreate( Str );

/// futtatás
void shellExecute( Str command, Arr args, Arr envs );

#endif // TOOLSH
