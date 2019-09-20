#include "server.h"

#include <iostream>

using namespace tcpforward;
using namespace asio::ip;

Server::Server(asio::io_context &ioc, const asio::ip::tcp::endpoint &localEP, const std::string &remoteHost, const std::string &remotePort)
    : remoteHost_{remoteHost}, remotePort_{remotePort}, ioc_{ioc}, acceptor_{ioc_, localEP}, incomingSock_{ioc_}
{
    acceptor_.listen();

    std::cout << "Listening on " << acceptor_.local_endpoint() << "\n";
}

void Server::start()
{
    doAccept();
}

void Server::stop()
{
    if (acceptor_.is_open())
        acceptor_.close();

    for (auto &s : sessions_)
        s->stop();
}

void Server::sessionClosed(const std::shared_ptr<Session> &s)
{
    sessions_.erase(s);
}

void Server::doAccept()
{
    acceptor_.async_accept(incomingSock_, [this](const auto &ec) mutable { handleAccept(ec); });
}

void Server::handleAccept(const asio::error_code &ec)
{
    if (!ec)
    {
        std::cout << "Accepted new connection from " << incomingSock_.remote_endpoint() << "\n";

        auto sp = *sessions_.emplace(std::make_shared<Session>(*this, std::move(incomingSock_), remoteHost_, remotePort_)).first;
        sp->start();

        // accept next connection
        doAccept();
    }
}
