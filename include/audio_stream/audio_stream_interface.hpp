/* TODO(MN): Async send API
*/
#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <span>
#include <functional>
#include <stdexcept>


namespace audio_stream
{
    using Data = std::span<char>;

    struct Endpoint 
    {
        Endpoint() : Endpoint("", 0){}
        Endpoint(const std::string& addressStr, uint16_t portNumber) :
            address(addressStr), port(portNumber){}

        std::string address;
        uint16_t    port;
    };

    class BaseClient
    {
    public:
        virtual bool connect(Endpoint endpoint, uint32_t timeoutMS = 3000) = 0;
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
        };
    };
}
