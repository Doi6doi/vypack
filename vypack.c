#include "tools.h"
#include <stdlib.h>

#include <stdio.h>

typedef enum RunMode { rmNone, rmExternal, rmInternal } RunMode;
typedef enum ContentKind { ckNone, ckCmd, ckArg, ckEnv, ckFile, ckDir } ContentKind;
typedef enum DirMode { dmTmp, dmTmpVer } DirMode;
typedef enum CheckMode { cmNone, cmModified } CheckMode;

struct Strs {
   Strs pool;
   Str magic;
   Str vypack;
};

/// fájlban tárolt paraméterek
typedef struct Params {
   /// csatolt fájlok összmérete
   Uint dataSize;
   /// táblázat mérete
   Uint contentSize;
   /// csomag verzió
   Uint version;
   /// cél könyvtár típus
   DirMode dirMode;
   /// ellenőrzési mód
   CheckMode checkMode;
   /// futtatási mód
   RunMode runMode;
} * Params;

typedef struct Content {
   /// sor neve
   Str name;
   /// tartalom típusa
   ContentKind kind;
   /// fájl mérete
   Uint size;
   /// fájl helye
   Uint offset;
   /// ellenőrző összeg
   Uint check;
   /// helyi fájl
   Str local;
} * Content;

typedef struct Pack {
   /// teljes fájl mérete
   Uint size;
   /// fájl neve
   Str fname;
   /// rövid név (kiterjesztés nélkül)
   Str name;
   /// saját fájl folyama
   Stream stream;
   /// paraméterek
   struct Params params;
   /// táblázat
   Arr contents;
   /// parancssori argumentumok
   Arr args;
   /// környezeti változók
   Arr envs;
   /// cél könyvtár
   Str destDir;

   /// cél fájl neve
   Str outName;
   /// futtatási parancs
   Str command;
   /// cél fájl folyam
   Stream oStream;
   /// tárolási könyvtár
   Str storeDir;
} * Pack;


struct Strs strs;
struct Pack pack;

#define MAGICSIZE 8
#define PARAMSIZE sizeof(struct Params)

extern char ** environ;

Str newEmpty() {
   return strCreate( strs.pool, "", 0 );
}

Str newStr( char * data, unsigned len ) {
   return strCreate( strs.pool, data, len );
}

/// szöveges konstansok
void initStrs() {
   strs.pool = strsCreate();
   strs.magic = newStr( "vypack##", MAGICSIZE );
   strs.vypack = newStr( "%vypack%", STR_LEN );
}

/// pack objektum létrehozása
void initPack( int argc, char ** argv ) {
   pack.fname = findPath( newStr( argv[0], STR_LEN ) );
   pack.name = fileCore( pack.fname );
   pack.size = fileSize( pack.fname );
   pack.stream = fileOpen( pack.fname );
   pack.contents = NULL;
   pack.args = arrCreate( argc );
   for (int i=0; i<argc; ++i)
      arrSet( pack.args, i, newStr( argv[i], STR_LEN ));
   pack.envs = arrCreate( 0 );
   for ( char ** ei = environ; *ei; ++ei )
      arrAdd( pack.envs, newStr( *ei, STR_LEN ) );
   pack.destDir = NULL;
   pack.outName = NULL;
   pack.command = NULL;
   pack.oStream = NULL;
   pack.storeDir = NULL;
}

/// mögé van-e csomagolva valami
bool isPacked() {
   streamSeek( pack.stream, pack.size-MAGICSIZE );
   Str k = streamReadStr( pack.stream, strs.pool, MAGICSIZE );
   return strSame( strs.magic, k );
}

/// paraméterek betöltése a binárisból
void loadParams() {
   streamSeek( pack.stream, pack.size-MAGICSIZE-PARAMSIZE );
   streamRead( pack.stream, & pack.params, PARAMSIZE );
}

/// egy tartalom rekord betöltése
Content loadContent() {
   Content ret = objCreate( sizeof( struct Content ) );
   Uint l = streamReadInt( pack.stream );
   ret->name = streamReadStr( pack.stream, strs.pool, l );
   ret->kind = (ContentKind)streamReadInt( pack.stream );
   ret->size = streamReadInt( pack.stream );
   ret->offset = streamReadInt( pack.stream );
   ret->check = streamReadInt( pack.stream );
   ret->local = NULL;
   return ret;
}

/// tartalom betöltése
void loadContents() {
   Uint cz = pack.params.contentSize;
   streamSeek( pack.stream, pack.size-MAGICSIZE-PARAMSIZE-cz );
   Uint n = streamReadInt( pack.stream );
   pack.contents = arrCreate( n );
   for (int i=0; i<n; ++i) {
      arrSet( pack.contents, i, loadContent() );
   }
}

/// checksum ellenőrzés
bool checkCheck( Str s, Uint chk ) {
   switch ( pack.params.checkMode ) {
      case cmNone: return true;
      case cmModified: return chk == fileModified( s );
      default: failI( "Unknown check mode", pack.params.checkMode );
   }
   return false;
}

/// könyvtár létrehozása, ha nincs
void forceDir( Content c ) {
   Str s = fileJoin( pack.destDir, c->name );
   if ( ! fileExists( s ))
      dirCreate( s );
}

/// fájl létrehozása, ha nincs
void forceFile( Content c ) {
   Str s = fileJoin( pack.destDir, c->name );
   c->local = s;
   if ( fileExists( s ) ) {
      if ( fileSize( s ) == c->size ) {
         if ( checkCheck( s, c->check ) )
            return;
      }
      fileDelete( s );
   }
   streamSeek( pack.stream, pack.size-MAGICSIZE-PARAMSIZE
      -pack.params.contentSize-pack.params.dataSize + c->offset );
   Stream g = fileCreate( s );
   streamCopy( pack.stream, g, c->size );
   streamFree(g);
   if ( cmModified == pack.params.checkMode )
      fileSetModified( s, c->check );
}

/// egy env érték módosítása
void updateEnvItem( Str item ) {
   bool append = false;
   Int j = strFind( item, '=' );
   if ( 1 < j && '+' == strC(item)[j-1] ) {
      append = true;
      --j;
   }
   if ( 0 >= j ) failS("Invalid env item: ", strC(item) );
   Str name = strSub( item, 0, j );
   Int h = j+(append? 2:1);
   Str val = strSub( item, h, strL(item)-h );
   int n = arrN( pack.envs );
   for (int i=0; i<n; ++i) {
      Str s = (Str)arrI( pack.envs, i );
      Int k = strFind( s, '=' );
      if ( 0 >= k ) continue;
      Str ni = strSub( s, 0, k );
      if ( strSame( name, ni )) {
         if ( append ) {
            strInsert( s, strL(s), val );
         } else {
            strRemove( s, 0, strL(s));
            strInsert( s, 0, name );
            strInsertC( s, strL(s), "=", 1 );
            strInsert( s, strL(s), val );
         }
      }
   }
}

/// egy rekord ellenőrzése
void forceContent( Content c ) {
   static int argAt = 0;
   switch ( c->kind ) {
      case ckCmd: 
         if ( rmInternal == pack.params.runMode ) {
            forceFile( c );
            fileSetExecutable( c->local );
         }
         pack.command = c->name; 
      break;
      case ckArg: 
         strReplace( c->name, strs.vypack, pack.destDir );
         arrInsert( pack.args, argAt++, c->name ); 
      break;
      case ckEnv: updateEnvItem( c->name ); break;
      case ckDir: forceDir( c ); break;
      case ckFile: forceFile( c ); break;
      default: failI("Unknown content kind:", c->kind);
   }
}

/// célkönyvtár létrehozása
void forceDest() {
   Str tmp = dirTemp(strs.pool);
   switch ( pack.params.dirMode ) {
      case dmTmp:
         pack.destDir = fileJoin( tmp, pack.name );
      break;
      case dmTmpVer:
         Str s = newEmpty();
         intToStr( pack.params.version, s );
         strInsert( s, 0, pack.name );
         pack.destDir = fileJoin( tmp, s );
      break;
      default: failI("Unknown dest dir mode", pack.params.dirMode );
   }
   if ( ! fileExists( pack.destDir ))
      dirCreate( pack.destDir );
}

/// hozzáadott tartalom ellenőrzése
void forceContents() {
   loadContents();
   forceDest();
   Uint n = arrN( pack.contents );
   for ( int i=0; i<n; ++i ) {
      Content c = arrI( pack.contents, i );
      forceContent( c );
   }
}

/// végrehajtás kicsomagolás után
int execute() {
   Str cmd = pack.command;
   if ( ! cmd ) fail("Missing command");
   if ( rmInternal == pack.params.runMode )
      cmd = fileJoin( pack.destDir, cmd );
   shellExecute( cmd, pack.args, pack.envs );
   return 0;
}

/// csomagolt main
int mainPacked() {
   loadParams();
   // remove first arg
   arrRemove( pack.args, 0 );
   // apply records
   forceContents();
   return execute();
}

void usage( char * msg ) {
/** \name usage
## Usage

   `vypack <options>`

### Options

* `-o <filename>`: output executable name
* `-c <command>`: the (external) command to run
* `-a <arg>`: insert command line argument before running
* `-x <executable>`: include executable file to run
* `-d <dir>`: force directory in package and use it for further files
* `-r <path>`: include full directory (recursively) in package
* `-f <filename>`: include a file in package
* `-e <row>`: add environment variable on run
* `-v <version>`: set version

*/
   char * usg = "\n"
#include "usage.inc"
   ;
   debug( usg );
   if ( msg ) {
      debug( msg );
      exit(1);
   }
   exit(0);
}

/// argumentum behúzása, ha van
Str shiftArg() {
   if ( 0 == arrN( pack.args ) ) fail("More arguments needed");
   return arrRemove( pack.args, 0 );
}

/// egy fájl hozzáadása
void addFile( Content c, Str name ) {
   c->local = newStr( strC(name), strL(name) );
   int si = strFindLast( c->local, archDirSep() );
   if ( 0 <= si )
      strRemove( name, 0, si+1 );
   if ( NULL != pack.storeDir )
      c->name = fileJoin( pack.storeDir, name );
}

/// egy tartalom rekord hozzáadása
Content addContent( ContentKind kind, Str name ) {
   Content c = objCreate( sizeof( struct Content ));
   c->name = name;
   c->kind = kind;
   c->size = 0;
   c->offset = 0;
   c->check = 0;
   c->local = NULL;
   switch ( kind ) {
      case ckFile:
         addFile( c, name );
      break;
      case ckCmd:
         if ( rmInternal == pack.params.runMode )
            addFile( c, name );
         pack.command = c->name;
      break;
      default: ;
   }
   arrAdd( pack.contents, c );
   return c;
}

/// tárolási könyvtár beállítása
void setStoreDir( Str dir ) {
   pack.storeDir = dir;
   if ( NULL != dir )
      addContent( ckDir, dir );
}

/// rekurzív fájl hozzáadás
void addRecursive( Str dir ) {
   Arr files = fileList( dir );
   int n = arrN( files );
   for ( int i=0; i<n; ++i ) {
      Str fname = arrI( files, i );
fprintf( stderr, "addRec %s %s\n", strC(dir), strC(fname));
      if ( ! fileIsDir( fname ) ) {
         Content c = addContent( ckFile, fname );
         c->local = fileJoin( dir, fname );
      }
   }
   for ( int i=0; i<n; ++i ) {
      Str dname = arrI( files, i );
fprintf( stderr, "addRecD %s\n", strC(dname));
      if ( fileIsDir( dname )
         && ! strSameC( dname, "." )
         && ! strSameC( dname, ".." )
      ) {
         Str save = pack.storeDir;
         setStoreDir( fileJoin( save, dname ));
         Str sub = fileJoin( dir, dname );
         addRecursive( sub );
         pack.storeDir = save;
      }
   }
   arrFree( files, NULL );
}


/// egy parancssori argumentum olvasása
bool getRawParam() {
   if ( 0 == arrN( pack.args ) )
      return false;
   Str a = shiftArg();
   if ( strSameC( a, "-c" ) ) {
      if ( rmNone != pack.params.runMode ) fail("Multiple run mode");
      pack.params.runMode = rmExternal;
      addContent( ckCmd, shiftArg() );
   } else if ( strSameC( a, "-x" )) {
      if ( rmNone != pack.params.runMode ) fail("Multiple run mode");
      pack.params.runMode = rmInternal;
      addContent( ckCmd, shiftArg() );
   } else if ( strSameC( a, "-a" )) {
      addContent( ckArg, shiftArg() );
   } else if ( strSameC( a, "-o" )) {
      if ( NULL != pack.outName ) fail("Multiple output file name");
      pack.outName = shiftArg();
   } else if ( strSameC( a, "-d" )) {
      setStoreDir( shiftArg() );
   } else if ( strSameC( a, "-r" )) {
      addRecursive( shiftArg() );
   } else if ( strSameC( a, "-f" )) {
      addContent( ckFile, shiftArg() );
   } else if ( strSameC( a, "-e" )) {
      addContent( ckEnv, shiftArg() );
   } else if ( strSameC( a, "-v" )) {
      if ( pack.params.version ) fail("Multiple version");
      pack.params.dirMode = dmTmpVer;
      pack.params.version = strToInt( shiftArg() );
   } else
      failS( "Unknown argument: ", strC(a) );
   return true;
}

/// paraméterek nullázása
void initParams() {
   Params p = & pack.params;
   p->dataSize = 0;
   p->contentSize = 0;
   p->version = 0;
   p->dirMode = dmTmp;
   p->checkMode = cmModified;
   p->runMode = rmNone;
}

/// parancssori argumentumok olvasása
void getRawParams() {
   initParams();
   pack.contents = arrCreate(0);
   arrRemove( pack.args, 0 );
   if ( 0 == arrN( pack.args ) ) usage( NULL);
   while ( getRawParam() )
      ;
   if ( ! pack.outName ) usage( "Destination name missing" );
   if ( ! pack.command ) usage( "Command missing" );
}

/// saját exe mentése
void saveSelf() {
   streamSeek( pack.stream, 0 );
   streamCopy( pack.stream, pack.oStream, pack.size );
}

/// egy fájl mentése
void saveFile( Content c, Uint * offset ) {
   Size sz = fileSize( c->local );
   Stream sf = fileOpen( c->local );
   streamCopy( sf, pack.oStream, sz );
   streamFree( sf );
   c->offset = *offset;
   c->size = sz;
   *offset += sz;
   switch ( pack.params.checkMode ) {
      case cmModified: c->check = fileModified( c->local ); break;
      case cmNone: c->check = 0; break;
      default: failI( "Unknown check mode",pack.params.checkMode );
   }
}

/// fájlok mentése
void saveFiles() {
   int n = arrN( pack.contents );
   Uint offset = 0;
   for ( int i=0; i<n; ++i ) {
      Content c = (Content)arrI( pack.contents, i );
      if ( ckFile == c->kind 
         || (ckCmd == c->kind && rmInternal == pack.params.runMode)
      )
         saveFile( c, & offset );
   }
   pack.params.dataSize = offset;
}     

/// egy tartalom rekord mentése
void saveContent( Content c, Uint * offset ) {
   Uint l = strL( c->name );
   Stream o = pack.oStream;
   streamWriteInt( o, l );
   streamWrite( o, strC(c->name), l );
   streamWriteInt( o, c->kind );
   streamWriteInt( o, c->size );
   streamWriteInt( o, c->offset );
   streamWriteInt( o, c->check );
   *offset += l + 5*sizeof(Int);
}

/// táblázat mentése
void saveContents() {
   Uint n = arrN( pack.contents );
   streamWriteInt( pack.oStream, n );
   Uint offset = 4;
   for (int i=0; i<n; ++i) {
      Content c = (Content)arrI( pack.contents, i );
      saveContent( c, & offset );
   }
   pack.params.contentSize = offset;
}

int mainRaw() {
   getRawParams();
   pack.oStream = fileCreate( pack.outName );
   saveSelf();
   saveFiles();
   saveContents();
   /// paraméterek
   streamWrite( pack.oStream, & pack.params, PARAMSIZE );
   /// magic
   streamWrite( pack.oStream, strC( strs.magic ), MAGICSIZE );
   streamFree( pack.oStream );
   fileSetExecutable( pack.outName );
   return 0;
}

int main( int argc, char ** argv ) {
   initStrs();
   initPack( argc, argv );
   if ( isPacked() )
      return mainPacked();
      else return mainRaw();
   strsFree( strs.pool );
}
