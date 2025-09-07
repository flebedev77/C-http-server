# Basic http static web server in C
Here is a basic http static web server in pure C. This project utilises the POSIX socket
API to interact with the internet. So in other words, this only supports UNIX or UNIX-like
operating systems.

# Why?
Mainly as a learning experience on how networking is done on a fairly low level.
But also, because all static web servers are SLOW and BLOATED. For many projects, I would
use the `npx http-server` to quickly host some files. Over time, I understood that JS sucks.
my beloved `npx http-server` command would secretly REDOWNLOAD the entirety of the web server's
Javascript code and then run it. Because of that, I began using `python3 -m http.server`
Which was substantially faster (by faster I mean it didn't redownload stuff everytime I ran it).
Ultimatley though, python just interprets a good old .py file located in `/usr/lib/python/http/server.py`
this is still slow.

# Features
 - No caching
 - Super lightweight and fast
 ![Image demostrating the size of a minimal response](imgs/size.png)
 - No *unused* *extra* features taking up space
