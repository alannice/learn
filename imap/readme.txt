
// readme.txt

xinetd管理自己的非标准服务

发现对于未在/etc/services中打开的端口，只要在/etc/xinetd.d/下的相应的配置文件中加上：

    type=UNLISTED

当然还要有port属性，比如我这里的

    port=1235

这两条属性就行了.

service imapxxx
{ 
    socket_type = stream 
    protocol = tcp 
    wait = no 
    user = root 
    server = /usr/local/bin/imapproxy
    server_args = -f /usr/local/etc/imapproxy/imapproxy.conf
    instances = 8000 
    nice = 10 
    
    cps = 50 30

    per_source＝5
    max_load = 2.8

    type = UNLISTED
    port = 1235

} 
