#ifndef ARCHH
#define ARCHH

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct EnumDir * EnumDir;

typedef uint64_t Size;
typedef uint64_t ArchTime;

typedef struct Stat {
   bool exists;
   bool isDir;
   Size size;
   ArchTime modified;
} * Stat;

bool archFirstCurrent();

bool archFileStat( char * fname, Stat stat );

char * archExeExt();

bool archDirCreate( char * path );

bool archSetExecutable( char * fname );

bool archSetModified( char *fname, ArchTime modified );

bool archExec( char * cmd, char ** args, char ** envs );

char * archError();

char * archDirTemp();

char archDirSep();

char archPathSep();

EnumDir archEnumStart( char * dir );
char * archEnumNext( EnumDir );

#endif // ARCHH
