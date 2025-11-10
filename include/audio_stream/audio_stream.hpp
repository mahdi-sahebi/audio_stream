#pragma once

#include "audio_stream_interface.hpp"


namespace AudioStream
{
    class Client : public AudioStream::ClientBase
    {
    public:
        Client(uint32_t pool_size = 2 * 1024);
        virtual ~Client() = default;

        bool connect(Endpoint endpoint, uint32_t timeout_ms = 3000) override;
        void disconnect() override;
        bool isConnected() override;
        uint32_t send(const Data& data) override;

    protected:

    private:

    };
}
