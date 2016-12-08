# ngx_http_early_hints
This is an experimental nginx module that sending 103 early hints.

103 is defined by [An HTTP Status Code for Indicating Hints](https://tools.ietf.org/html/draft-kazuho-early-hints-status-code-00), and This module implements the draft-00.

# Getting started

There is the configuration below.

```nginx
location /103.html {
    add_early_header "Link" "</main.css>;rel=preload";
}
```

There is the response below.

```sh
vagrant@vagrant:~$ telnet localhost 80
Connected to localhost.
Escape character is '^]'.
GET /103.html HTTP/1.1
host:localhost
```

```
HTTP/1.1 103 Early Hints
Link: </main.css>;rel=preload

HTTP/1.1 200 OK
Server: nginx/1.11.6
Date: Wed, 07 Dec 2016 13:29:12 GMT
Content-Type: text/html
Content-Length: 22
Last-Modified: Wed, 07 Dec 2016 13:28:44 GMT
Connection: keep-alive
ETag: "58480e8c-16"
Accept-Ranges: bytes

This is main contents
```

`$early_hints` variable means sending 103 status code and Early Hints headers.
```
log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
                  'early_hints:$early_hints';
```
```
127.0.0.1 - - [08/Dec/2016:16:52:11 +0000] "GET /103.html HTTP/1.1" early_hints:1
127.0.0.1 - - [08/Dec/2016:16:52:07 +0000] "GET / HTTP/1.1" early_hints:0
```


# Builid
Get a Nginx code
```sh
vagrant@vagrant:~$ git clone https://github.com/flano-yuki/.git
vagrant@vagrant:~$ wget https://nginx.org/download/nginx-1.11.6.tar.gz
vagrant@vagrant:~$ tar zxvf ./nginx-1.11.6.tar.gz
vagrant@vagrant:~$ cd ./nginx-1.11.6/
vagrant@vagrant:~$ ./configure --add-module=/home/vagrant/ngx_http_early_hints/
vagrant@vagrant:~$ make
```

# TODO
- More strict error handling
- Support http2
- Suppport two or more add_early_header in one location directive
- Add test