#define _GNU_SOURCE
#include "arch.h"
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

bool archExec( char * cmd, char ** args, char ** envs ) {
   return 0 != execvpe( cmd, args, envs );
}

char * archError() { 
   return strerror( errno ); 
}

char archDirSep() { 
   return '/';
}

bool archFileStat( char * fname, Stat s ) {
   struct stat ss;
   s->exists = false;
   s->isDir = false;
   s->size = 0;
   s->modified = 0;
   if ( 0 != stat( fname, & ss ))
      return ENOENT == errno;
   s->exists = true;
   s->isDir = S_ISDIR( ss.st_mode );
   s->size = ss.st_size;
   s->modified = ss.st_mtim.tv_sec;
   return true;
}

bool archDirCreate( char * path ) {
   return 0 == mkdir( path, 0777 );
}

EnumDir archEnumStart( char * dir ) {
   return (EnumDir)opendir( dir );
}

char * archEnumNext( EnumDir ed ) {
   DIR * dp = (DIR *)ed;
   if ( ! dp ) return NULL;
   struct dirent * ep;
   if (( ep = readdir(dp) ))
      return ep->d_name;
   closedir(dp);
   return NULL;
}

bool archExecPermission( char * fname ) {
   return 0 == chmod( fname, 0777 );
}

char * archDirTemp() {
   return "/tmp";
}

