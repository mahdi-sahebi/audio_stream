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
        isConnected_{false}
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

        const struct lws_protocols protocols[] = {
            {"example-protocol", Client::websocketEvent, sizeof(ClientData), 1024},
            { nullptr, nullptr, 0, 0}
        };

        // TODO(MN): Handle if another server on port exist on othe server side
        // TODO(MN): Handle if not able to connect
        struct lws_context_creation_info info{};
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = protocols;
    
        auto context = lws_create_context(&info);
        if (nullptr == context) {
            throw Exception::Connection("Failed to create context");
        }
    
        struct lws_client_connect_info conn_info = {};
        conn_info.context        = context;
        conn_info.address        = endpoint.address.c_str();
        conn_info.port           = endpoint.port;
        conn_info.path           = "/";
        conn_info.host           = conn_info.address;
        conn_info.origin         = conn_info.address;
        conn_info.protocol       = protocols[0].name;
        conn_info.ssl_connection = 0;
    
        auto wsi = lws_client_connect_via_info(&conn_info);
        if (nullptr == wsi) {
            lws_context_destroy(context);
            return false;
        }

        auto clientData = static_cast<ClientData*>(lws_wsi_user(wsi));
        clientData->client = this;

        future<void> serviceThread = async(launch::async, [&]() {
            while (!isConnected()) {
                lws_service(context, 1000);
            }
        });

        unique_lock<mutex> connectionLock(connectionMutex_);
        connectionCV_.wait_for(connectionLock, milliseconds(timeoutMS), [&]() {
            return isConnected_;
        });

        // TODO(MN): What about timedout and service thread is run?
        serviceThread.get();
        lws_context_destroy(context);

        return isConnected_;
    }
    
    void Client::disconnect()
    {
        if (isConnected()) {
            // throw runtime_error("Not implemented");
        }
    }
    
    void Client::setConnectionStatus(bool enable)
    {
        unique_lock<mutex> lock(connectionMutex_);
        isConnected_ = enable;
        connectionCV_.notify_all();
    }

    bool Client::isConnected()
    {
        unique_lock<mutex> lock(connectionMutex_);
        return isConnected_;
    }
    
    uint32_t Client::send(const Data& data)
    {
        return 0;
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
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            // std::cout << "LWS_CALLBACK_CLIENT_RECEIVE: " << std::string((char*)in, len) << "\n";
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            clientData->client->setConnectionStatus(true);
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            break;
        case LWS_CALLBACK_CLOSED:
            break;
        default:
            break;
        }

        return 0;
    }
}
