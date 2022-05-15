# Setting up the server

> This page teaches you to properly set up the Mapache Web Server

If you have read the documentation on the main page, you should know enough basic concepts about Mapache in order to set up your own webpage. I will cut straight into matters, because who reads biblical-sized documentations, anyways?

There are basically three steps you need to follow in order to serve your webpage in Mapache:

1. [Set up the root directory](#1-set-up-the-root-directory)

2. [Adjust the config file](#2-adjust-the-config-file)

3. [Init the server daemon](#3-init-the-server-daemon)

I made Mapache with simplicity in mind, so designed the set-up process to pretty straight forward. You'll hopefully be ready to go with as few tweaks as possible!

***

## 1. Set up the root directory

The first step will be to get your HTML, CSS and other files that form up your webpage in a dedicated directory. If you have a notion or two about web programming, you'll probably have done this beforehand. Once you know where your files will be, you are ready to jump into step 2.

<center><i>I told you this would be simple ;)</i></center>

## 2. Adjust the config file

With your files ready in their own directory, you have to adjust the server config file. This file must be located in the same directory as the server executable, and if you don't execute it with the `-C` flag, for specifying its name (more on this later), it must be named `mapache.conf`.

This config file has the following key properties:

+ **`server_root`**: This is the path to the directory where your web files have been stored (we did this in the previous step). It can be an absolute path, or relative to the server executable's location.

+ **`max_clients`**: This number will be the maximum number of clients which will be allowed to connect simultaneously to the web server.

+ **`server_ip`**: This is the IP address of the web server. If no IP value is provided, the server will be binded to `localhost`. The same applies if you enter a value that doesn't fit in the [IPv4 standard](https://en.wikipedia.org/wiki/IPv4).

+ **`listen_port`**: This will be the port number on which the web server will listen.

+ **`server_signature`**: This will be the name of the server (by default it's *Mapache*).

Once you have configured all the key-value propierties, you'll be ready to go!

**IMPORTANT**: If you changed the name of the config file, be sure to provide it with the `-C` flag, just like this:

```
$ ./server -C <CONFIG_FILE_NAME>
```

## 3. Init the server daemon

You are ready to start the server! Just execute it with the daemon flag (and any other flags you may need, like `-C`):

```
$ ./server -d -C ...
```

To kill it you can use this command:

```
$ kill -9 $(lsof -t -i:<PORT>)
```

Where PORT refers to the port on which the server is listening (you must've specified it in the config file in the previous step).

Alternatively, if you want to debug the server, you can specify the log file with the `-f` flag, or just run it without the daemon (`-d`) flag at all. In the latter way, the server's log messages will be prompted in the console (stdin).

***

<center><i>Hopefully you followed this steps correctly, and you can view your webpage correctly served!</i></center>
