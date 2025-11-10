#pragma once

#include "audio_stream_interface.hpp"


namespace audio_stream
{
    class Client : public audio_stream::BaseClient
    {
    public:
        explicit Client(uint32_t poolSize = 2 * 1024);
        virtual ~Client() = default;

        bool connect(Endpoint endpoint, uint32_t timeoutMS = 3000) override;
        void disconnect() override;
        bool isConnected() override;
        uint32_t send(const Data& data) override;

    protected:

    private:

    };
}
