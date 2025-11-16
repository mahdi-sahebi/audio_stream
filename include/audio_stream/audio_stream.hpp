#pragma once

#include <condition_variable>
#include <mutex>
#include <future>// TODO(MN): Encapsulate
#include <deque>// TODO(MN): Encapsulate
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
        std::mutex runMutex_;
        bool isRun_;

        std::mutex apiMutex_;

        std::mutex connectionMutex_;
        std::condition_variable connectionCV_;
        bool isConnected_;
        void setConnectionStatus(bool enable);
        
        lws* websocket_;// TODO(MN): Encapsulate. smart pointer
        lws_context* context_;// TODO(MN): Encapsulate, smart pointer
        std::future<void> serviceThread_;

        std::mutex bufferMutex_;
        std::deque<char> buffer_;

        void setRunStatus(bool status);
        bool isRun();

        std::mutex sendMutex_;
        std::condition_variable sendCV_;
        void waitForSend();

        void setConnectedSocket(lws* socket);
        // bool waitForConnection(uint32_t timeoutMS);
        static int websocketEvent(// TODO(MN): Encapsulate
            struct lws* wsi, 
            enum lws_callback_reasons reason,
            void* userData, 
            void* in, 
            size_t len);
        void sendBuffer(const Data& data);
        uint32_t getBufferSize();
        void writeBuffer(const Data& data);
        std::vector<char> readBuffer();
    };
}
