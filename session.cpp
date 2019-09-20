#include "session.h"

#include <iostream>
#include <cstddef>
#include <asio/ts/buffer.hpp>
#include <asio/ts/socket.hpp>

#include "server.h"

using namespace tcpforward;
using namespace asio::ip;

Session::Session(Server &fwd, asio::ip::tcp::socket &&inSock, std::string_view remoteHost, std::string_view remotePort)
    : remoteHost_{remoteHost}, remotePort_{remotePort}, forwarder_{fwd}, stop_{false}, inSock_{std::move(inSock)}, outSock_{inSock_.get_executor().context()}, resolver_{inSock_.get_executor().context()}
{
}

void Session::start()
{
    // start resolve
    resolver_.async_resolve(remoteHost_, remotePort_, [this, s=shared_from_this()](const auto &ec, tcp::resolver::results_type results) mutable {
        if (running())
        {
            if (!ec)
            {
                // connect
                asio::async_connect(outSock_, results, [this, s](const auto &ec, const asio::ip::tcp::endpoint& endpoint) mutable {
                    if (running())
                    {
                        if (!ec)
                        {
                            std::cout << "Connected to '" << endpoint << "'\n";

                            // start read from inSock
                            doRead(inSock_, outSock_, inBuf_);

                            // start read from outSock
                            doRead(outSock_, inSock_, outBuf_);
                        }
                        else
                        {
                            std::cerr << "Fail to connect to '" << endpoint << "': " << ec.message() << "\n";
                            stop();
                        }
                    }
                });
            }
            else
            {
                std::cerr << "Fail to resolve '" << remoteHost_ << ":" << remotePort_ << "': " << ec.message() << "\n";
                stop();
            }
        }
    });
}

void Session::stop()
{
    if (!stop_)
    {
        stop_ = true;

        asio::error_code ignoredError;
        resolver_.cancel();
        inSock_.close(ignoredError);
        outSock_.close(ignoredError);
        forwarder_.sessionClosed(shared_from_this());
    }
}

void Session::doRead(tcp::socket &rSock, tcp::socket &wSock, Buffer &buf)
{
    rSock.async_receive(asio::buffer(buf), [&, s=shared_from_this()](const auto &ec, auto bytesTransfered) {
        handleRead(rSock, wSock, buf, ec, bytesTransfered);
    });
}

void Session::handleRead(tcp::socket &rSock, tcp::socket &wSock, Buffer &buf, const asio::error_code &ec, std::size_t bytesTransfered)
{
    if (running())
    {
        if (!ec)
        {
            doWrite(rSock, wSock, buf, bytesTransfered);
        }
        else
        {
            std::cerr << "Read error from sock " << &rSock << ": " << ec.message() << "\n";
            stop();
        }
    }
}

void Session::doWrite(asio::ip::tcp::socket &rSock, asio::ip::tcp::socket &wSock, Buffer &buf, std::size_t bytesToWrite)
{
    asio::async_write(wSock, asio::buffer(buf), asio::transfer_exactly(bytesToWrite), [&, s=shared_from_this()](const auto &ec, auto bytesTransfered){
        handleWrite(rSock, wSock, buf, ec, bytesTransfered);
    });
}

void Session::handleWrite(asio::ip::tcp::socket &rSock, asio::ip::tcp::socket &wSock, Buffer &buf, const asio::error_code &ec, std::size_t)
{
    if (running())
    {
        if (!ec)
        {
            doRead(rSock, wSock, buf);
        }
        else
        {
            std::cerr << "Write error from sock " << &wSock << ": " << ec.message() << "\n";
            stop();
        }
    }
}
