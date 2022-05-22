# Configuring HTTPS in Mapache

> Here you will learn how to configure Mapache to use HTTPS with OpenSSL Certificates

So you want to transform your already cool Mapache distribution into an even-cooler Mapache-*HTTPS* distribution! Lucky for you, this is the right place to do so.

We will walk you through the steps needed to config the HTTPS protocol into your Mapache installation:

1. [Get an OpenSSL Key-Cert pair](#1-get-an-openssl-key-cert-pair)


***

## 1. Get an OpenSSL Key-Cert pair

The first thing you need to set up HTTPS in Mapache is to get a pair of OpenSSL key and certificate. Obviously, this repo doesn't contain any of these files, for the sake of security. Nevertheless, the Makefile includes a simple rule to generate your local `key.pem` and `cert.pem` files. Just execute the following command:

```
$ make genssl
```

You will be prompted with the OpenSSL console, which will ask you to enter some data in order to generate your personal Key & Certificate. Once you are done, you are ready to modify the server configuration file!