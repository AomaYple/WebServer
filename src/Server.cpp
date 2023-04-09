#include "Server.h"
#include "Log.h"

#include <cstring>

#include <arpa/inet.h>
#include <fcntl.h>

using std::string, std::to_string, std::vector, std::shared_ptr, std::make_shared, std::source_location;

Server::Server(unsigned short port, std::source_location sourceLocation) : self(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
        idleFileDescriptor(open("/dev/null", O_RDONLY)) {
    if (this->self == -1)
        Log::add(sourceLocation, Level::ERROR, "Server create error: " + string(strerror(errno)));

    int option {1};
    if (setsockopt(this->self, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        Log::add(sourceLocation, Level::ERROR, "Server reuseAddress error: " + string(strerror(errno)));

    if (setsockopt(this->self, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option)) == -1)
        Log::add(sourceLocation, Level::ERROR, "Server reusePort error: " + string(strerror(errno)));

    sockaddr_in address {};
    socklen_t addressLength {sizeof(address)};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1)
        Log::add(sourceLocation, Level::ERROR, "Server translate ipAddress error: " + string(strerror(errno)));

    if (::bind(this->self, reinterpret_cast<sockaddr *>(&address), addressLength) == -1)
        Log::add(sourceLocation, Level::ERROR, "Server bind error: " + string(strerror(errno)));

    if (::listen(this->self, SOMAXCONN) == -1)
        Log::add(sourceLocation, Level::ERROR, "Server listen error: " + string(strerror(errno)));
}

Server::Server(Server &&server) noexcept : self(server.self), idleFileDescriptor(server.idleFileDescriptor) {
    server.self = -1;
    server.idleFileDescriptor = -1;
}

auto Server::operator=(Server &&server) noexcept -> Server & {
    if (this != &server) {
        this->self = server.self;
        this->idleFileDescriptor = server.idleFileDescriptor;
        server.self = -1;
        server.idleFileDescriptor = -1;
    }
    return *this;
}

auto Server::accept(source_location sourceLocation) -> vector<shared_ptr<Client>> {
    vector<shared_ptr<Client>> clients;

    while (true) {
        sockaddr_in address = {};
        socklen_t addressLength {sizeof(address)};

        int fileDescriptor {accept4(this->self, reinterpret_cast<sockaddr *>(&address), &addressLength,
                                    SOCK_NONBLOCK)};

        if (fileDescriptor != -1) {
            string information(INET_ADDRSTRLEN, 0);

            if (inet_ntop(AF_INET, &address.sin_addr, information.data(), information.size()) == nullptr)
                Log::add(sourceLocation,Level::ERROR, "Client translate ipAddress error: " + string(strerror(errno)));

            information += ":" + to_string(ntohs(address.sin_port));

            Log::add(sourceLocation, Level::INFO, "new client " + information);

            clients.emplace_back(make_shared<Client>(fileDescriptor, information));
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else if (errno == EMFILE) {
                close(this->idleFileDescriptor);
                this->idleFileDescriptor = ::accept(this->self, nullptr, nullptr);
                close(this->idleFileDescriptor);
                this->idleFileDescriptor = open("/dev/null", O_RDONLY);
            } else {
                Log::add(sourceLocation, Level::ERROR, "Server accept error: " + string(strerror(errno)));
                break;
            }
        }
    }

    return clients;
}

auto Server::get() const -> int {
    return this->self;
}

Server::~Server() {
    if (this->self != -1 && close(this->self) == -1)
        Log::add(source_location::current(), Level::ERROR, "Server close error: " + string(strerror(errno)));
}