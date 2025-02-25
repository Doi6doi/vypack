#include "arch.h"
#include <sys/stat.h>
#include <sys/utime.h>
#include <errno.h>
#include <string.h>
#include <windows.h>

#define S_ISDIR(mode) ( 0 != _S_IFDIR & (mode))

struct EnumDir {
   WIN32_FIND_DATA data;
   HANDLE handle;
};

struct EnumDir enumDir;

bool archFirstCurrent() { return true; }

char * archExeExt() { return ".exe"; }

bool archSetExecutable( char * fname ) {
   return true;
}

bool archSetModified( char * fname, unsigned value ) {
   struct _utimbuf t = { .actime = value, .modtime = value };
   return 0 == _utime( fname, & t );
}

char archDirSep() {
   return '\\';
}

bool archFileStat( char * fname, Stat s ) {
   struct _stat ss;
   s->exists = false;
   s->isDir = false;
   s->size = 0;
   s->modified = 0;
   if ( 0 != _stat( fname, & ss ))
      return ENOENT == errno;
   s->exists = true;
   s->isDir = S_ISDIR( ss.st_mode );
   s->size = ss.st_size;
   s->modified = ss.st_mtime;
   return true;
}

bool archDirCreate( char * path ) {
   return 0 == _mkdir( path );
}

bool archExec( char * cmd, char ** args, char ** envs ) {
   return 0 != _execvpe( cmd, args, envs );
}

char * archError() { 
   return strerror( errno ); 
}

char * archDirTemp() {
   static char buf[MAX_PATH];
   buf[0] = 0;
   GetTempPathA( MAX_PATH, buf );
   return buf;
}

EnumDir archEnumStart( char * dir ) {
   enumDir.handle = FindFirstFileA( dir, & enumDir.data );
   if ( INVALID_HANDLE_VALUE == enumDir.handle )
	  return NULL;
   return & enumDir;
}

char * archEnumNext( EnumDir ed ) {
   static char buf[MAX_PATH];
   if ( INVALID_HANDLE_VALUE == enumDir.handle )
	  return NULL;
   memcpy( buf, enumDir.data.cFileName, MAX_PATH );
   if ( ! FindNextFile( enumDir.handle, & enumDir.data )) {
	   FindClose( enumDir.handle );
       enumDir.handle = INVALID_HANDLE_VALUE;
   }
   return buf;
}



