#include "audio_stream/audio_stream.hpp"

// ws://localhost:8080/
#include "libwebsockets.h"

using namespace std;

#include <iostream>

struct ClientData
{
    audio_stream::Client* client;
};

static int callback_client(
    struct lws* wsi, 
    enum lws_callback_reasons reason,
    void* userData, 
    void* in, 
    size_t len) 
{
    if (nullptr == userData) {
        return 0;
    }

    auto clientData = static_cast<ClientData*>(userData);

    switch (reason) {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        std::cout << "LWS_CALLBACK_CLIENT_ESTABLISHED\n";
        lws_callback_on_writable(wsi);
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        std::cout << "LWS_CALLBACK_CLIENT_RECEIVE: " << std::string((char*)in, len) << "\n";
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        std::cout << "LWS_CALLBACK_CLIENT_WRITEABLE Ready to write\n";
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        std::cerr << "LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n";
        break;
    case LWS_CALLBACK_CLOSED:
        std::cout << "LWS_CALLBACK_CLOSED\n";
        break;
    default:
        break;
    }

    return 0;
}


#include <iostream>

namespace audio_stream
{
    
    Client::Client(uint32_t poolSize)
    {
        if (0 == poolSize) {
            throw audio_stream::Exception::BadAlloc("Zero pool size");
        }
    }
    
    bool Client::connect(Endpoint endpoint, uint32_t timeoutMS)
    {
        const struct lws_protocols protocols[] = {
            {"example-protocol", callback_client, sizeof(ClientData), 1024},
            { nullptr, nullptr, 0, 0}
        };

        // TODO(MN): Handle if another server on port exist on othe server side
        // TODO(MN): Handle if not able to connect
        struct lws_context_creation_info info{};
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = protocols;
    
        struct lws_context* context = lws_create_context(&info);
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
            // TODO(MN): Delete context
            return false;//throw Exception::Connection("Client connection failed");
        }

        auto clientData = static_cast<ClientData*>(lws_wsi_user(wsi));
        clientData->client = this;

        // TODO(MN): condition variable until established
        while (lws_service(context, 1000) >= 0) {
            cout << "loop" << endl;
            // loop until connection closes
        }
    
        lws_context_destroy(context);

        return true;
    }
    
    void Client::disconnect()
    {

    }
    
    bool Client::isConnected()
    {
        return false;
    }
    
    uint32_t Client::send(const Data& data)
    {
        return 0;
    }
}
