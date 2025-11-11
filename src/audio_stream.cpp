#include "audio_stream/audio_stream.hpp"


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
        return false;
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
