#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client {
    int sock;

public:
    Client();
    ~Client();

    void write(std::string message);
};

#endif
