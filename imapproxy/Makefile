
$target = imapproxy
$test = mail_test

all:$target $test

$target:imapproxy.c
	clang -ggdb -o imapproxy imapproxy.c ../libxyz/libxyz.a -I../libxyz -lssl

$test:mail_test.c
	clang -ggdb -o mail_test mail_test.c -lssl

install:
	cp imapproxy  /usr/local/bin/imapproxy
	cp imapproxy.conf.example /usr/local/etc/imapproxy.conf
	chmod 755 /usr/local/bin/imapproxy

uninstall:
	rm /usr/local/bin/imapproxy
	rm /usr/local/etc/imapproxy.conf

clean:
	rm -f *.o imapproxy mail_test

