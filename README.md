<center><h1 style="font-size:3em;"><b>Mapache Server 2022</b></h1></center>

<div style='text-align: center;' align='center'>
    <img style='max-heigth: 200px;' src='misc/mapache.png'/>
</div>

<center>This project is my take on a simple, but fully functional Web Server</center>
***

## **Contents**
1. [Installation](#1-how-to-install-how-to-use-the-makefile)

	1.1. [Usage](#11-how-to-use-it)

	1.2. [Benchmarking](#12-how-to-check-an-example)

	1.3. [Server configuration](#13-how-to-edit-the-server-config-file)


## 1. How to install? How to use the Makefile?

> Just execute the ALL Makefile rule to generate the executable:

```
$ make all
```

To clean binary files & build directories, use `make all`:

```
$ make clean
```

If you have run `make all` and want to do `clean + all`:

```
$ make reset
```

And for Valgrind checking:

```
$ make val
```

**IMPORTANT DEPENDENCIES**: you need to install the `libconfuse` library for parsing configuration files. You can refer to <a href=https://github.com/libconfuse/libconfuse>its GitHub page </a> for install instructions, but basically you can do well with this command (for Debian-based systems):

```
$ sudo apt install libconfuse-dev
```

### 1.1. How to use it?

Through compiling, a server executable will be generated. You can then run it with:

```
$ ./server
```

This will init Mapache on localhost IP, and log messages will be shown in the terminal.

Nevertheless, there are also a couple of handy flags. You can check them with `-h`:

```
$ ./server -h
```

### 1.2. How to check an example?

This repo contains a `web/` folder with both the Mapache start page (for checking if it is running) and a Benchmarking page. To see the main page, just start the server and navigate to this URL in your browser:

`http://localhost:8080/home.html`

The benchmarking page contains several items that display the current capabilities of Mapache, along with a loading chronometer. It is hosted at:

`http://localhost:8080/benchmarking.html`

### 1.3 How to edit the server config file?

There is a server config file, called `server_conf.conf`, which allows you to edit:

+ The server name (`server_signature` label)

+ The root folder on which Mapache will search for your webpage files (`server_root` label)

+ The port in which to listen for clients (`listen_port` label)

+ Max number of clients to be connected at once (`max_clients` label)

Feel free to adjust these values to your needs.


<img style='max-heigth: 200px;' src='misc/emoji.png'/><i>Thanks for checking this project out!</i>