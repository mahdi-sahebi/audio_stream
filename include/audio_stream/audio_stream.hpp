#pragma once

#include <condition_variable>
#include <mutex>
#include "libwebsockets.h"// TODO(MN): Encapsulate
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
        std::mutex apiMutex_;
        std::mutex connectionMutex_;
        std::condition_variable connectionCV_;
        bool isConnected_;
        lws* websocket_;// TODO(MN): Encapsulate. smart pointer

        void setConnectionStatus(bool enable);
        static int websocketEvent(// TODO(MN): Encapsulate
            struct lws* wsi, 
            enum lws_callback_reasons reason,
            void* userData, 
            void* in, 
            size_t len);
    };
}
