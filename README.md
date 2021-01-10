# TCP established connection idle-timeout tests for NAT implementations

You can use these programs to test how long time your own NAT, or your ISP's CGN, will wait before dropping an idle TCP connection.

I wrote a more thorough explanation of this issue on my blog, and blamed my ISP for killing my SSH connections: <http://anderstrier.dk/2021/01/10/my-isp-is-killing-my-idle-ssh-sessions-yours-might-be-too/>

But here's the tl;dr:

To be [RFC5382](https://tools.ietf.org/html/rfc5382) compliant, the NAT should wait at least 2 hours and 4 minutes before dropping the connection from its NAT table.
This value allows TCP keepalives to do its job, since the interval before a keepalive packet is sent "[MUST default to no less than two hours](https://tools.ietf.org/html/rfc1122)", giving the keepalives a 4 minute window to rescue the connection.

The idea is to establish some TCP connections to a server, wait some time and then send some data to test if the connections still works. This way we can discover the timeout period before the NAT will drop the connection. 

The tests will spawn 130 connections on startup, and test the first connection after 1 minute, the second after 2 minutes and so on (2 hours and 10 minutes total).

`tcp-send-test` will do exactly that.

`tcp-recv-test` will instead ask the server to test the connections (i.e be the one to try and send some send data after waiting).

`tcp-keepalive-test` works essentially the same way as tcp-send-test, but instead of sending actual data, it will use TCP-keepalives.

By default, the tests are configured to connect to a server sponsored by the non-profit organization [dotsrc.org](https://dotsrc.org/).

To compile, just run `make`. 

You should start both `tcp-recv-test` and `tcp-send-test`, and wait for about 2½ hours. Then look for when connections started failing.
