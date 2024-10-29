#include "tools.h"
#include "arch.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define REALLOC( p, sz ) realloc( p, sz )
#define BUFSIZE 512
#define MIN(x,y) ((x)<(y)?(x):(y))

struct Arr {
   Uint n;
   Obj * data;
};


Str fileCore( Str s ) {
   Int k = strFindLast( s, '.' );
   if (0>k) k = strL( s );
   Int i = strFindLast( s, '/' );
   if (0>i) i = 0;
   Int j = strFindLast( s, '\\' ); 
   if (0>j) j = 0;
   if ( i < j ) i = j;
   if (0<i) ++i;
   if (k<i) k = strL(s);
   return strSub( s, i, k-i );   
}
   
Stat fileStat( Str s ) {
   static struct Stat ret;
   if ( ! archFileStat( strC(s), &ret ))
      failSS( "Could not stat file", strC(s), archError() );
   return & ret;
}
   
Size fileSize( Str s ) {
   return fileStat(s)->size;
}

Uint fileModified( Str s ) {
   return fileStat(s)->modified;
}

bool fileIsDir( Str s ) {
   return fileStat(s)->isDir;
}

bool fileExists( Str s ) {
   return fileStat(s)->exists;
}


Stream fileOpen( Str fname ) {
   FILE * ret = fopen( strC(fname), "rb" );
   if ( ! ret ) failS("Could not open file", strC(fname));
   return (Stream)ret;
}

Str fileJoin( Str a, Str b ) {
   int la = strL(a);
   Str ret = strSub( a, 0, la );
   char sep = archDirSep();
   if ( la && strC(a)[la-1] != sep )
      strInsertC( ret, la, &sep, 1 );
   strInsert( ret, strL(ret), b );
   return ret;
}

void fileDelete( Str fname ) {
   if ( 0 != remove( strC(fname) )) 
      failS("Could not remove file", strC(fname));
}

Stream fileCreate( Str fname ) {
   FILE * ret = fopen(strC(fname), "wb");
   if ( ! ret ) failS("Could not create file", strC(fname));
   return (Stream)ret;
}

Arr fileList( Str dir ) {
   Arr ret = arrCreate(0);
   char * pc;
   EnumDir ed = archEnumStart( strC(dir) );
   Strs pool = strS(dir);
   if ( ! ed ) failSS( "Could not open directory", strC(dir), archError());
   while (( pc = archEnumNext( ed ))) {
      Str s = strCreate( pool, pc, STR_LEN );
      if ( strSameC( s, "." ) && !strSameC( s, ".." ))
         strFree(s);
      else
         arrAdd( ret, s );
   }
   return ret;
} 

void fileSetExecutable( Str fname ) {
   if ( ! archSetExecutable( strC(fname) ))
      failSS("Cannot set exec permission", strC(fname), archError() );
}

void fileSetModified( Str fname, Uint value ) {
   if ( ! archSetModified( strC(fname), value ))
      failSS("Cannot set modification time", strC(fname), archError() );
}

void dirCreate( Str s ) {
   if ( ! archDirCreate( strC(s) ) ) 
      failSS("Could not create directory", strC(s), archError() );
}

Str dirTemp( Strs pool ) {
   static Str ret = NULL;
   if ( ! ret )
      ret = strCreate( pool, archDirTemp(), STR_LEN );
   return ret;
}
 
Arr arrCreate( Uint n ) {
   Arr ret = objCreate( n );
   ret->n = n;
   ret->data = REALLOC( NULL, n*sizeof(Obj));
   if ( ! ret->data )
      fail( "Could not allocate array");
   return ret;
}

void arrFree( Arr arr, Destructor destr ) {
   int n = arrN(arr);
   for ( int i=0; i<n; ++i) {
      Obj o = arrI( arr, i );
      if (o && destr) destr(o);
   }
   arr->data = REALLOC( arr->data, 0 );
   arr->n = 0;
   objFree( arr );
}

void arrSet( Arr arr, Uint i, Obj o ) {
   if ( arr->n <= i ) failI("Index out of bounds",i);
   arr->data[i] = o;
}

void arrAdd( Arr arr, Obj o ) {
   arrInsert( arr, arr->n, o );
}

Obj arrRemove( Arr arr, Uint i ) {
   if ( arr->n <= i ) failI("Invalid array index for remove",i);
   Obj ret = arr->data[i];
   memmove( arr->data+i, arr->data+i+1, (arr->n-i-1)*sizeof(Obj) );
   --arr->n;
   return ret;
}

void arrInsert( Arr arr, Uint i, Obj o ) {
   if ( arr->n < i ) failI("Invalid array index for insert",i);
   Obj * data = REALLOC( arr->data, (arr->n+1)*sizeof(Obj));
   if ( ! data ) failI("Could not grow array",i);
   arr->data = data;
   if ( i < arr->n )
      memmove( data+i+1, data+i, (arr->n-i-1)*sizeof(Obj) );
   data[ arr->n ] = o;
   ++ arr->n;
}


Uint arrN( Arr arr ) {
   return arr->n;
}

Obj arrI( Arr arr, Uint i ) {
   if ( arr->n <= i ) failI("Invalid array index", i);
   return arr->data[i];
}

void streamSeek( Stream s, Size i ) {
   if ( 0 != fseek( (FILE *)s, i, SEEK_SET ))
      failI("Could not seek",i);
}


void streamRead( Stream s, void * dst, Size n ) {
   if ( ! fread( dst, n, 1, (FILE *)s ))
      fail("Could not read");
}

void streamWrite( Stream s, void * src, Size n ) {
   if ( ! fwrite( src, n, 1, (FILE *)s ))
      fail("Could not write");
}
      
Int streamReadInt( Stream s ) {
   Int ret;
   streamRead( s, &ret, sizeof(ret));
   return ret;
}

void streamWriteInt( Stream s, Int i ) {
   streamWrite( s, &i, sizeof(i) );
}

void streamCopy( Stream src, Stream dst, Size n ) {
   static char buf[BUFSIZE];
   while ( 0 < n ) {
      int m = MIN(BUFSIZE,n);
      streamRead( src, buf, m );
      streamWrite( dst, buf, m );
      n -= m;
   }
}

void streamFree( Stream s ) {
   fclose( (FILE *)s );
}
            
Obj objCreate( Size size ) {
   Obj ret = REALLOC( NULL, size );
   if ( ! ret ) fail("Could not allocate object");
   return ret;
}

void objFree( Obj obj ) {
   obj = REALLOC( obj, 0 );
}

void fail( char * msg ) {
   debug( msg );
   exit(1);
}

void failI( char * msg, Int i ) {
   debugI( msg, i );
   exit(1);
}

void failS( char * msg, char * s ) {
   debugS( msg, s );
   exit(1);
}

void failSS( char * msg, char * p1, char * p2 ) {
   debugSS( msg, p1, p2 );
   exit(1);
}

void debug( char * msg ) {
   fprintf( stderr, "%s\n", msg );
   fflush( stderr );
}

void debugI( char * msg, int i ) {
   fprintf( stderr, "%s %d\n", msg, i );
   fflush( stderr );
}

void debugS( char * msg, char * s ) {
   fprintf( stderr, "%s %s\n", msg, s );
   fflush( stderr );
}

void debugSS( char * msg, char * p1, char * p2 ) {
   fprintf( stderr, "%s %s %s\n", msg, p1, p2 );
   fflush( stderr );
}

void shellExecute( Str cmd, Arr args, Arr envs ) {
   int na = args ? arrN( args ) : 0;
   char * ags[ na+2 ];
   ags[0] = strC( cmd );
   for (int i=0; i<na; ++i)
      ags[i+1] = strC( (Str)arrI( args, i ) );
   ags[na+1] = NULL;
   int ne = envs ? arrN( envs ) : 0;
   char * evs[ ne+1 ];
   for ( int i=0; i<ne; ++i)
      evs[i] = strC( (Str)arrI( envs, i ) );
   evs[ne] = NULL;
   if ( ! archExec( strC(cmd), ags, evs ) )
      failSS( "Could not execute", strC(cmd), archError() );
}



