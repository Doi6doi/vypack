#include "arch.h"
#include "str.h"
#include <sys/stat.h>
#include <sys/utime.h>
#include <errno.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>

#define S_ISDIR(mode) ( 0 != _S_IFDIR & (mode))

struct EnumDir {
   WIN32_FIND_DATA data;
   HANDLE handle;
};

struct EnumDir enumDir;

bool apiErr = false;

bool archFirstCurrent() { return true; }

char * archExeExt() { return ".exe"; }

bool archSetExecutable( char * fname ) {
   return true;
}

bool archSetModified( char * fname, ArchTime value ) {
   apiErr = false;
   struct _utimbuf t = { .actime = value, .modtime = value };
   return 0 == _utime( fname, & t );
}

char archDirSep() {
   return '\\';
}

char archPathSep() {
   return ';';
}

bool archFileStat( char * fname, Stat s ) {
   apiErr = false;
   struct _stat ss;
   s->exists = false;
   s->isDir = false;
   s->size = 0;
   s->modified = 0;
   if ( 0 != _stat( fname, & ss ))
      return ENOENT == errno;
   s->exists = true;
   s->isDir = 0 != (ss.st_mode & _S_IFDIR);
   s->size = ss.st_size;
   s->modified = ss.st_mtime;
   return true;
}

bool archDirCreate( char * path ) {
   apiErr = false;
   return 0 == _mkdir( path );
}


bool archExec( char * cmd, char ** args, char ** envs ) {
   apiErr = true;
   STARTUPINFO si;
   memset( &si, sizeof(si), 0 ); 
   si.cb = sizeof(si);
   PROCESS_INFORMATION pi;
   Str cmdl = strCreate( NULL, "", 0 );
//   strInsertC( cmdl, strL(cmdl), cmd, STR_LEN );
   while ( *args ) {
	  strInsertC( cmdl, strL(cmdl), " ", 1 );
	  strInsertC( cmdl, strL(cmdl), *args, STR_LEN );
	  ++ args;
   }
   Str env = strCreate( NULL, "", 0 );
   while ( *envs ) {
	  strInsertC( env, strL(env), *envs, STR_LEN );
	  strInsertC( env, strL(env), "\0", 1 );
	  ++ envs;
   }
   bool ret = false;
   if ( CreateProcessA( cmd, strC(cmdl), NULL, NULL,
      FALSE, 0, strC(env), NULL, &si, &pi )
   ) {
      if ( 0 == WaitForSingleObject( pi.hProcess, INFINITE ))
         ret = true;
      CloseHandle( pi.hProcess );
      CloseHandle( pi.hThread );
   }
   strFree( cmdl );
   strFree( env );
   return ret;
}

char * archError() { 
   static char buf[2048];
   if ( apiErr ) {
	  DWORD code = GetLastError();
	  FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, 
	     NULL, code, 0, buf, sizeof(buf), NULL );
	  return buf;
   } else {
      return strerror( errno ); 
   }
}

char * archDirTemp() {
   static char buf[MAX_PATH];
   buf[0] = 0;
   GetTempPathA( MAX_PATH, buf );
   return buf;
}

EnumDir archEnumStart( char * dir ) {
   static char buf[MAX_PATH];
   snprintf( buf, sizeof(buf), "%s\\*", dir );
   enumDir.handle = FindFirstFileA( buf, & enumDir.data );
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



