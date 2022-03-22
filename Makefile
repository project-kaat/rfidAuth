build:
	gcc -c pam_rfbs.c -fPIC -fno-stack-protector
	ld -x --shared -o pam_rfbs.so pam_rfbs.o
clean:
	rm pam_rfbs.o
	rm pam_rfbs.so
install:
	cp pam_rfbs.o /lib/security/
