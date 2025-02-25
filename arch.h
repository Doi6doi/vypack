#ifndef ARCHH
#define ARCHH

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct EnumDir * EnumDir;

typedef struct Stat {
   bool exists;
   bool isDir;
   size_t size;
   unsigned modified;
} * Stat;

bool archFirstCurrent();

bool archFileStat( char * fname, Stat stat );

char * archExeExt();

bool archDirCreate( char * path );

bool archSetExecutable( char * fname );

bool archSetModified( char *fname, unsigned modified );

bool archExec( char * cmd, char ** args, char ** envs );

char * archError();

char * archDirTemp();

char archDirSep();

EnumDir archEnumStart( char * dir );
char * archEnumNext( EnumDir );

#endif // ARCHH
