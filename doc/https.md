# Configuring HTTPS in Mapache

> Here you will learn how to configure Mapache to use HTTPS with OpenSSL Certificates

So you want to transform your already cool Mapache distribution into an even-cooler Mapache-*HTTPS* distribution! Lucky for you, this is the right place to do so.

We will walk you through the steps needed to config the HTTPS protocol into your Mapache installation:

1. [Get an OpenSSL Key-Cert pair](#1-get-an-openssl-key-cert-pair)

2. [Set up the config file](#2-set-up-the-config-file)

3. [Start Mapache with HTTPS enabled](#3-start-mapache-with-https-enabled)

***

## 1. Get an OpenSSL Key-Cert pair

The first thing you need to set up HTTPS in Mapache is to get a pair of OpenSSL key and certificate. Obviously, this repo doesn't contain any of these files, for the sake of security. Just execute the following command:

```
$ openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365
```

You will be prompted with the OpenSSL console, which will ask you to enter some data in order to generate your personal Key & Certificate. It is **extremely important** that you remember the 'PASSPHRASE' that you enter here. The rest of the data is optional and leaving it blank will not matter in the generation of the Key-Certificate pair.

Once you are done, you are ready to modify the server configuration file!

# 2. Set up the config file

The next thing that you need to do is include the information for the Key & Certificate in the server's configuration file (for learning more about this file, visit the [Basic Set-up Page](./setup.md)).

These are basically the two properties that need to be filled (I'll use the filenames from the example command I showed in the previous step):

```
...
# OpenSSL-HTTPS configuration
SSL_key = key.pem
SSL_cert = cert.pem
...
```

(This example assumes that the Key & Certificate files are in the same directory as the configuration file.)

Once you are done with this, there's only one thing left to do -start the server!

# 3. Start Mapache with HTTPS enabled

The last thing you'll need to do is start the server with the appropriate flag that tells it to enable secure socket connections via TLS. This way, the HTTP part of the server will now be HTTPS!

For doing so, just add th `-s` flag along with the rest of the flags you may also want to use:

```
$ ./mapacheServer -s
```

The OpenSSL prompt will ask you to enter the 'PASSPHRASE' you previously wrote while generating the Key & Certificate files in Step 1. Just type it again.

*That's it!* Now Mapache Server will be running in your machine with HTTPS support enabled. From now on, you won't need to worry about your connection data being publicly exposed in the Internet.