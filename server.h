#ifndef TCPFORWARD_SERVER_H
#define TCPFORWARD_SERVER_H

#include <string>
#include <set>

#include <asio/ts/io_context.hpp>
#include <asio/ts/socket.hpp>

#include "session.h"

namespace tcpforward {

class Server
{
public:
    Server(asio::io_context &ioc, const asio::ip::tcp::endpoint &localEP, const std::string &remoteHost, const std::string &remotePort);

    void start();
    void stop();

    void sessionClosed(const std::shared_ptr<Session> &s);

private:
    const std::string remoteHost_, remotePort_;
    asio::io_context &ioc_;
    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket incomingSock_;
    std::set<std::shared_ptr<Session>> sessions_;

    void doAccept();
    void handleAccept(const asio::error_code &ec);
};

}

#endif
