tcpforward
==========

TCP Port forwarder using standalone C++ asio

Dependencies
------------

* asio-1.12 or up (http://think-async.com/Asio/)
* A recent C++ compiler (example: gcc-8)

Usage
-----

./tcpforward [localIp:]localPort:remoteHost:remotePort [[localIp:]localPort:remoteHost:remotePort ...]
