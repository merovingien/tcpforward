#include <iostream>

#include <asio/signal_set.hpp>

#include "server.h"

void usage(const char *prog)
{
    std::cerr << "usage: " << prog << " [localIp:]localPort:remoteHost:remotePort [[localIp:]localPort:remoteHost:remotePort ...]\n";
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        usage(argv[0]);
        return 1;
    }

    asio::io_context ioc;
    std::unique_ptr<tcpforward::Server> forwarder;

    // *** manage signals ***
    asio::signal_set ss{ioc, SIGINT, SIGTERM};
#ifdef SIGBREAK
    ss.add(SIGBREAK);
#endif
    ss.async_wait([&forwarder](const auto &, int sig) {
        std::cout << "Receive signal " << sig << "\n";
        if (forwarder)
            forwarder->stop();
    });

    // *** process args
    std::istringstream sstr(argv[1]);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(sstr, item, ':'))
        elems.push_back(std::move(item));
    if (3 <= elems.size() && elems.size() <= 4)
    {
        // *** get local endpoint ***
        asio::ip::tcp::endpoint localEP;
        if (elems.size() == 3)
            elems.insert(elems.begin(), "");

        asio::ip::tcp::resolver resolver{ioc};

        try {
            // prefer ipv6
            std::error_code ec;
            auto endpoints = resolver.resolve(asio::ip::tcp::v6(), elems[0], elems[1],
                                              asio::ip::tcp::resolver::v4_mapped | asio::ip::tcp::resolver::passive, ec);
            if (ec)
            {
                // try all protocols
                endpoints = resolver.resolve(elems[0], elems[1], asio::ip::tcp::resolver::passive);
            }
            localEP = endpoints.begin()->endpoint();
        }
        catch (const std::system_error &err)
        {
            std::cerr << "Fail to resolve local endpoint: " << err.what();
            throw;
        }

        forwarder = std::make_unique<tcpforward::Server>(ioc, localEP, elems[2], elems[3]);
        forwarder->start();
    }
    else
    {
        usage(argv[0]);
        return 1;
    }

    ioc.run();

    std::cout << "Exit\n";

	return 0;
}
