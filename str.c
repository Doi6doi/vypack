#include "str.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PSZ sizeof(void *)

struct Strs {
   Str * data;
   unsigned n;
};

struct Str {
   Strs pool;
   unsigned len;
   char * data;
};

static void fail( char * msg ) {
   fprintf( stderr, "%s\n", msg );
   exit(1);
}

static void * sRealloc( void * p, unsigned sz ) {
   if ( !p && ! sz )
      return NULL;
   void * ret = realloc( p, sz );
   if ( sz && !ret )
      fail("Could not realloc");
   return ret;
}

/// hibajelzés egésszel
static void failI( char *msg, int i ) {
   static Str s = NULL;
   if ( ! s )
      s = strCreate( NULL, "", 0 );
   intToStr( i, s );
   strInsertC( s, 0, " ", 1 );
   strInsertC( s, 0, msg, strlen(msg));
   fail( strC(s));
}

static void strsAdd( Strs pool, Str s ) {
   Str * data = sRealloc( pool->data, (pool->n+1)*PSZ );
   data[ pool->n ] = s;
   ++ pool->n;
   pool->data = data;
}

static void strsRemove( Strs pool, Str s ) {
   for (int i=0; i<pool->n; ++i) {
      if ( pool->data[i] == s ) {
         memmove( pool->data+i, pool->data+i+1, (pool->n-i-1)*PSZ );
         pool->data = sRealloc( pool->data, (pool->n-1)*PSZ );
         -- pool->n;
         return;
      }
   }
}

char * strResize( Str s, unsigned len ) {
   s->data = sRealloc( s->data, len+1 );
   s->data[len] = 0;
   s->len = len;
   return s->data;
}

Str strCreate( Strs pool, char * chars, unsigned len ) {
   Str ret = sRealloc( NULL, sizeof(struct Str) );
   if ( STR_LEN == len )
      len = strlen( chars );
   ret->pool = pool;
   ret->len = 0;
   ret->data = NULL;
   strResize( ret, len );
   memcpy( ret->data, chars, len );
   if ( pool )
      strsAdd( pool, ret );
   return ret;
}

void intToStr( int i, Str s ) {
   static char buf[40];
   sprintf( buf, "%d", i );
   strRemove( s, 0, strL(s) );
   strInsertC( s, 0, buf, strlen(buf) );
}

int strToInt( Str s ) {
   int ret;
   if ( ! sscanf( strC(s), "%d", &ret ))
      return 0;
   return ret;
}

char * strC( Str s ) {
   return s->data;
}

unsigned strL( Str s ) {
   return s->len;
}

Str strCopy( Str s ) {
   return strSub( s, 0, strL(s) );
}

Str strSub( Str s, unsigned i, unsigned l ) {
   Strs pool = strS(s);
   if ( s->len < i+l )
      failI( "Invalid string index",i);
   return strCreate( pool, strC(s)+i, l );
}

bool strSame( Str a, Str b ) {
   if ( a->len != b->len ) return false;
   return 0 == memcmp( a->data, b->data, a->len );
}

bool strSameC( Str a, char * s ) {
   if ( NULL == s ) return false;
   int len = strlen(s);
   if ( a->len != len ) return false;
   return 0 == memcmp( a->data, s, len );
}

int strFind( Str s, char ch ) {
   for ( int i=0; i< s->len; ++i ) {
      if ( ch == s->data[i] )
         return i;
   }
   return -1;
}

int strFindLast( Str s, char ch ) {
   for ( int i=s->len-1; 0 <=i; --i ) {
      if ( ch == s->data[i] )
         return i;
   }
   return -1;
}

void strFree( Str s ) {
   if ( !s ) return;
   if ( s->pool )
      strsRemove( s->pool, s );
   s->data = sRealloc( s->data, 0 );
   s->len = 0;
   s = sRealloc( s, 0 );
}

/// string egy része helyett valami más
static void strReplacePart( Str s, int at, int len, Str repl ) {
   int sl = strL(s);
   int rl = strL(repl);
   int d = rl-len;
   char * sc = strC(s);
   if ( 0 < d )
      sc = strResize( s, sl+d );
   memmove( sc+at+rl, sc+at+len, sl-len-at );
   if ( 0 > d )
      sc = strResize( s, sl+d );
   memcpy( sc+at, strC(repl), rl );
}

void strInsert( Str s, unsigned at, Str part ) {
   strInsertC( s, at, strC(part), strL(part));
}

void strInsertC( Str s, unsigned at, char * part, unsigned plen ) {
   int sl = strL(s);
   strResize( s, sl+plen );
   char * sa = strC(s)+at;
   memmove( sa+plen, sa, sl-at );
   memcpy( sa, part, plen );
}

void strRemove( Str s, unsigned at, unsigned len ) {
   int sl = strL(s);
   if ( sl < at+len )
      failI( "Invalid string index", at+len);
   char * sa = strC(s)+at;
   memmove( sa, sa+len, sl-at-len );
   strResize( s, sl-len );
}

void strReplace( Str s, Str find, Str repl ) {
   int fl = strL(find);
   if ( 0 == fl ) return;
   char * sc = strC(s);
   char *fc = strC(find);
   for ( int i = strL(s)-fl; 0 <= i; --i ) {
      if ( 0 == memcmp( sc+i, fc, fl ))
         strReplacePart( s, i, fl, repl );
   }
}

Strs strS(Str s) {
   return s->pool;
}

Strs strsCreate() {
   Strs ret = sRealloc( NULL, sizeof( struct Strs ));
   ret->n = 0;
   ret->data = NULL;
   return ret;
}

void strsFree( Strs ss ) {
   for ( int i=0; i<ss->n; ++i ) {
      Str s = ss->data[i];
      s->pool = NULL;
      strFree(s);
   }
   ss->data = sRealloc( ss->data, 0 );
   ss->n = 0;
   ss = sRealloc( ss, 0 );
}

