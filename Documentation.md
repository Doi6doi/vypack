# Vypack documentation

Vypack is a simple C program which allows you to pack an interpreter 
(like Python, Node, PHP, or any other) together with your program and
optional resource files. You can also add the needed external libraries 
or interpreter modules. It creates a single binary of your operating 
system which can be transferred to other systems and run without
being installed.

## How does it work

The packed files are appended to the end of the small vypack binary with
a content table and some extra information. When executed, the program
copies the added files to the system temporary directory and runs from there.

## Command line usage

You can call vypack (or vypack.exe in Windows) to create binary packs.

```
Usage: vypack <options>

vypack is a program which can pack an interpreter (e.g php)
with an application and data files. It results in a single
binary which can be used on other system without installing.

Options:
   -o <filename>: output executable name
   -c <command>: the (external) command to run
   -a <arg>: insert command line argument before running
   -x <executable>: include executable file to run
   -d <dir>: force directory in package and use it for further files
   -r <path>: include full directory (recursively) in package
   -f <filename>: include a file in package
   -e <row>: add environment variable on run
   -v <version>: set version
```

## Examples

To write "Hello world" on Windows:

```
vpack.exe -o hello.exe -c echo -a "Hello, world"
```

To run phpinfo:

**info.php**
```
<php 
phpinfo();
```

```
vpack.exe -o phpinfo.exe -x php.exe -a %vypack%\info.php -f info.php
```



