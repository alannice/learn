
$test = mail_test

all:$test

$test:mail_test.c
	clang -ggdb -o mail_test mail_test.c -lssl

clean:
	rm -f *.o mail_test

