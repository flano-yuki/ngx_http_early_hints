# ngx_http_early_hints
This is an experimental nginx module that sending 103 early hints before sending content(NGX_HTTP_ACCESS_PHASE).

103 is defined by [An HTTP Status Code for Indicating Hints](https://tools.ietf.org/html/draft-kazuho-early-hints-status-code-00), and This module implements the draft-00.

# Getting started

There is the configuration below.

```nginx
location /103.html {
    add_early_header "Link" "</main.css>;rel=preload";
    add_early_header "Link" "</main.js>;rel=preload";
    add_early_header "hoge" "hogehoge";
}
```

There is the response below.

```sh
vagrant@vagrant:~$ curl --dump-header - localhost/103.html
HTTP/1.1 103 Early Hints
Link: </main.css>;rel=preload
Link: </main.js>;rel=preload
hoge: hogehoge

HTTP/1.1 200 OK
Server: nginx/1.11.6
Date: Sun, 11 Dec 2016 10:27:20 GMT
Content-Type: text/html
Content-Length: 22
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
vagrant@vagrant:~$ git clone https://github.com/flano-yuki/ngx_http_early_hints.git
vagrant@vagrant:~$ wget https://nginx.org/download/nginx-1.11.6.tar.gz
vagrant@vagrant:~$ tar zxvf ./nginx-1.11.6.tar.gz
vagrant@vagrant:~$ cd ./nginx-1.11.6/
vagrant@vagrant:~$ ./configure --add-module=/home/vagrant/ngx_http_early_hints/
vagrant@vagrant:~$ make
```

# TODO
- More strict error handling
- Support http2
- Add test
