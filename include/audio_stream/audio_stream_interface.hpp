/* TODO(MN): Async send API
*/
#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <span>
#include <functional>
#include <stdexcept>


namespace AudioStream
{
    using Data = std::span<char>;

    typedef struct 
    {
        Endpoint(const std::string& address_str, uint16_t port_number) :
            address(address_str), port(port_number){}

        std::string address;
        uint16_t    port;
    }Endpoint;

    class ClientBase
    {
    public:
        virtual bool connect(Endpoint endpoint, uint32_t timeout_ms = 3000) = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() = 0;
        virtual uint32_t send(const Data& data) = 0;
    };

    class Exception
    {
    public:
        class Connection : public std::runtime_error
        {
        public:
            explicit Connection(const std::string& message) :
                std::runtime_error(message){}
        };

        class BadAlloc : public std::bad_alloc
        {
        public:
            explicit BadAlloc(const std::string& message) :
                message_(message){}
            const char* what() const noexcept override
            {
                return message_.c_str();
            }

        private:
            std::string message_;
        };

        class InvalidArgument : public std::invalid_argument
        {
        public:
            explicit InvalidArgument(const std::string& message) :
                std::invalid_argument(message){}
        }
    };
}
