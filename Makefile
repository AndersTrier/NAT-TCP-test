all: tcp-send-test tcp-recv-test tcp-keepalive-test

tcp-send-test:

tcp-recv-test:

tcp-keepalive-test:

.PHONY: clean

clean:
	rm -f tcp-send-test tcp-recv-test tcp-keepalive-test
