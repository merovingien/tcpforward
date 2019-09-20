#ifndef TCPFORWARD_SESSION_H
#define TCPFORWARD_SESSION_H

#include <memory>
#include <string_view>
#include <cstddef>
#include <array>

#include <asio/ts/internet.hpp>


namespace tcpforward {

const std::size_t BUFFER_SIZE = 1 << 13;  // 8KB

class Server;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(Server &fwd, asio::ip::tcp::socket &&inSock, std::string_view remoteHost, std::string_view remotePort);

    void start();
    void stop();

private:
    using Buffer = std::array<std::byte, BUFFER_SIZE>;

    const std::string_view remoteHost_, remotePort_;
    Server &forwarder_;
    bool stop_;
    asio::ip::tcp::socket inSock_, outSock_;
    asio::ip::tcp::resolver resolver_;
    Buffer inBuf_, outBuf_;

    bool running() const { return !stop_; }
    void doRead(asio::ip::tcp::socket &rSock, asio::ip::tcp::socket &wSock, Buffer &buf);
    void handleRead(asio::ip::tcp::socket &rSock, asio::ip::tcp::socket &wSock, Buffer &buf, const asio::error_code &ec, std::size_t bytesTransfered);
    void doWrite(asio::ip::tcp::socket &rSock, asio::ip::tcp::socket &wSock, Buffer &buf, std::size_t bytesToWrite);
    void handleWrite(asio::ip::tcp::socket &rSock, asio::ip::tcp::socket &wSock, Buffer &buf, const asio::error_code &ec, std::size_t bytesTransfered);
};

}

#endif
