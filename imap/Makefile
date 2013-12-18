
$target = imapproxy
$test = mail_test

all:$target $test

$target:imapproxy.c
	clang -ggdb -o imapproxy imapproxy.c ../libxyz/libxyz.a -I../libxyz -lssl

$test:mail_test.c
	clang -ggdb -o mail_test mail_test.c -lssl

clean:
	rm -f *.o imapproxy mail_test

