#include <chrono>
#include <thread>
#include <future>
#include <iostream>// TODO(MN): Delete
#include "libwebsockets.h"
#include "audio_stream/audio_stream.hpp"


using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::this_thread;


struct ClientData
{
    audio_stream::Client* client;
};


namespace audio_stream
{
    
    Client::Client(uint32_t poolSize) :
        isRun_{false}, isConnected_{false}, websocket_{nullptr}, context_{nullptr}
    {
        if (0 == poolSize) {
            throw audio_stream::Exception::BadAlloc("Zero pool size");
        }
    }
    
    bool Client::connect(Endpoint endpoint, uint32_t timeoutMS)
    {
        // TODO(MN): double connect
        unique_lock<mutex> lock(apiMutex_);

        // TODO(MN): Reset connection flags
        setConnectionStatus(false);
        setRunStatus(true);

        serviceThread_ = async(launch::async, [&]() {
            const struct lws_protocols protocols[] = {
                {"example-protocol", Client::websocketEvent, sizeof(ClientData), 1024},
                { nullptr, nullptr, 0, 0}
            };
    
            // TODO(MN): Handle if another server on port exist on othe server side
            // TODO(MN): Handle if not able to connect
            struct lws_context_creation_info info{};
            info.port = CONTEXT_PORT_NO_LISTEN;
            info.protocols = protocols;
        
            context_ = lws_create_context(&info);
            if (nullptr == context_) {
                // TODO(MN): Stop immediately
                throw Exception::Connection("Failed to create context");
            }
            lws_cancel_service(context_);
        
            struct lws_client_connect_info conn_info = {};
            conn_info.context        = context_;
            conn_info.address        = endpoint.address.c_str();
            conn_info.port           = endpoint.port;
            conn_info.path           = "/";
            conn_info.host           = conn_info.address;
            conn_info.origin         = conn_info.address;
            conn_info.protocol       = protocols[0].name;
            conn_info.ssl_connection = 0;
        
            websocket_ = lws_client_connect_via_info(&conn_info);
            if (nullptr == websocket_) {
                lws_context_destroy(context_);
                // TODO(MN): Stop immediately
                // return false;
            }
    
            auto clientData = static_cast<ClientData*>(lws_wsi_user(websocket_));
            clientData->client = this;

            while (isRun()) {
                lws_service(context_, 100);

                // cout << "Con: " << isConnected() << endl;
                // while (isConnected()) {
                //     lws_service(context_, 1000);// TODO(MN): Decrease and check for pending  sending data from this thread

                //     if (!isConnected()) {// TODO(MN): Extra?
                //         continue;
                //     }

                //     cout << "[RCV]: " << getBufferSize() << endl;
                //     if (getBufferSize()) {
                //         auto data = readBuffer();
                //         sendBuffer(data);
                //     }
                // }
            // }
            }

            lws_cancel_service(context_);
            lws_context_destroy(context_);// TODO(MN): freeResources/allocResources
            context_ = nullptr;
        });

        unique_lock<mutex> connectionLock(connectionMutex_);
        connectionCV_.wait_for(connectionLock, milliseconds(timeoutMS), [&]() {
            return isConnected_;
        });
        connectionLock.unlock();

        return isConnected();
    }
    
    void Client::disconnect()
    {
        if (!isRun()) {
            throw Exception::Connection("Already disconnected");
        }
        
        lws_cancel_service(context_);
        setRunStatus(false);
        serviceThread_.get();
        setConnectionStatus(false);
    }
    
    void Client::setConnectionStatus(bool enable)
    {
        unique_lock<mutex> lock(connectionMutex_);
        isConnected_ = enable;
    }

    void Client::setRunStatus(bool status)
    {
        unique_lock<mutex> lock(runMutex_);
        isRun_ = status;
    }
    
    bool Client::isRun()
    {
        unique_lock<mutex> lock(runMutex_);
        return isRun_;
    }

    // bool Client::waitForConnection(uint32_t timeoutMS)
    // {
    //     while (isConnected()) {
    //         lws_service(context_, 1000);// TODO(MN): Decrease and check for pending  sending data from this thread
    //     }
    //     return false;
    // }

    bool Client::isConnected()
    {
        unique_lock<mutex> lock(connectionMutex_);
        return isConnected_;
    }
    
    uint32_t Client::send(const Data& data)
    {
        // TODO(MN): Thread-safe. check inputs and connection
        if (!isConnected()) {
            throw Exception::Connection("Not connected to server");
        }

        if (0 == data.size()) {
            return 0;
        }

        writeBuffer(data);
        return data.size();
    }

    void Client::sendBuffer(const Data& data)
    {
        struct segment_t
        {
            uint8_t lwsPre[LWS_PRE];
            uint8_t payload[1024];
        };

        segment_t segment;
        memset(&segment, 0x00, sizeof(segment));// TODO(MN): Delete it

        uint32_t totalSentSize{0};

        while (totalSentSize < data.size()) {
            const auto remainingSize = data.size() - totalSentSize;
            const auto segmentSize = std::min(remainingSize, sizeof(segment.payload));

            memcpy(segment.payload, data.data() + totalSentSize, segmentSize);
            const auto sentSize = lws_write(websocket_, segment.payload, segmentSize, LWS_WRITE_BINARY);
            
            if (sentSize > 0) {
                totalSentSize += sentSize;
            } else {
                sleep_for(1ms);
            }
        }
    }

    uint32_t Client::getBufferSize()
    {
        unique_lock<mutex> lock(bufferMutex_);
        return buffer_.size();
    }

    void Client::writeBuffer(const Data& data)
    {
        unique_lock<mutex> lock(bufferMutex_);
        buffer_.insert(buffer_.end(), data.begin(), data.end());
    }

    vector<char> Client::readBuffer()
    {
        unique_lock<mutex> lock(bufferMutex_);
        
        const auto size = buffer_.size();
        vector<char> buffer;
        
        // TODO(MN): This is not efficient at all. Define memory pool
        buffer.insert(buffer.begin(), buffer_.begin(), buffer_.end());
        buffer_.clear();

        return buffer;
    }

    void Client::setConnectedSocket(lws* socket)
    {
        websocket_ = socket;
        setConnectionStatus(true);
        connectionCV_.notify_all();
    }

    int Client::websocketEvent(
        struct lws* wsi, 
        enum lws_callback_reasons reason,
        void* userData, 
        void* in, 
        size_t len) 
    {
        auto clientData = static_cast<ClientData*>(userData);
        if ((nullptr == clientData) || (nullptr == clientData->client)) {
            return 0;
        }

        switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:// TODO(MN): Add braces for all
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            clientData->client->setConnectedSocket(wsi);
            break;
        } case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            break;
        case LWS_CALLBACK_CLOSED: {
            break;
        } default:
            break;
        }

        return 0;
    }
}
