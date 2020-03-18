# dll-hijacking
DLL Proxying

Used in my blog post: https://b.ou.is/articles/2020-03/context-menu-persistance

### Creation Script:
```ps
PS C:\Users\rek7\Documents\dll-hijacking> python3 .\parse.py --help
usage: parse.py [-h] -d DLL [-f HEADER_FILE] [-b DUMP_BIN]

Proxy DLL Creator

optional arguments:
  -h, --help      show this help message and exit
  -d DLL          Path to DLL
  -f HEADER_FILE  Path to created definitions Header File
  -b DUMP_BIN     Path to Dumpbin Binary
```