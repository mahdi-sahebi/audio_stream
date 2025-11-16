#include <unistd.h>
#include <memory>
#include <iostream>
#include <future>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <random>
#include <ctime>
#include <sys/wait.h>
#include <gtest/gtest.h>
#include "audio_stream/audio_stream.hpp"


using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::this_thread;


class ClientTest : public ::testing::Test
{
protected:
    audio_stream::Endpoint serverEndpoint_;
    unique_ptr<audio_stream::Client> stream_;
    string receivedFilePath_;
    string audioFilePath_;
    atomic<pid_t> serverPID_;
    future<bool> serverTask_;

    void SetUp() override
    {
        startServer();
        serverEndpoint_ = audio_stream::Endpoint("localhost", 8080);

        receivedFilePath_ = "deps/received_data.bin";
        filesystem::remove(receivedFilePath_);
        
        audioFilePath_ = "deps/glass.wav";

        stream_ = make_unique<audio_stream::Client>(4 * 1024 * 1024);
        if (nullptr == stream_) {
            GTEST_SKIP();
        }
    }

    void TearDown() override
    {
        stopServer();
        stream_ = nullptr;
    }

    void startServer()
    {
        serverPID_ = 0;

        serverTask_ = async(launch::async, [&]() {
            const auto pid = fork();
            
            if (pid < 0) {
                cout << "Process creation failed" << endl;
                return false;
            } else if (pid == 0) {
                execlp("node", "node", "deps/server.js", nullptr);
                cout << "Failed to execute server" << endl;
                exit(1);
            } else {
                serverPID_ = pid;
                this_thread::sleep_for(250ms);
                
                if (kill(serverPID_, 0) == 0) {
                    int result = system("timeout 2 bash -c 'echo > /dev/tcp/localhost/8080' 2>/dev/null");
                    if (0 != result) {
                        cout << "Server NOT listening on port 8080" << endl;
                        return false;
                    }
                    
                    return true;
                } else {
                    cout << "Server process died" << endl;
                    return false;
                }
            }
        });

        /* Let server to be executed */
        sleep_for(500ms);// TODO(MN): Wait until pid becomes valid
        
        if (0 == serverPID_) {
            FAIL();
            GTEST_SKIP();
        }
    }

    void stopServer()
    {
        /* Stop the server */
        if (serverPID_ > 0) {
            kill(serverPID_, SIGKILL);
            serverPID_ = 0;
        }
        // TODO(MN): search in processes to ensure no server ndoejs is run
        const auto serverResult = serverTask_.get();
        ASSERT_TRUE(serverResult);
    }

    bool verifyFile(const string& filePath, const vector<char>& data)
    {
        /* Let the server flush the file */
        sleep_for(20ms * data.size() / 1000);

        if (!filesystem::exists(filePath)) {
            cout << "File not exists" << endl;
            return false;
        }
        
        const auto fileSize = filesystem::file_size(filePath);
        if (fileSize != data.size()) {
            cout << "File size failed. " << data.size() << " != " << fileSize << endl; 
            return false;
        }
    
        vector<char> temp(data.size(), 0);
    
        fstream file;
        file.open(filePath, ios::in | ios::binary);
        file.read(temp.data(), fileSize);
        file.close();
    
        return (0 == memcmp(temp.data(), data.data(), data.size()));
    }
    
    vector<char> readFile(const string& filePath)
    {
        vector<char> data;
    
        if (!filesystem::exists(filePath)) {
            return data;
        }
    
        const auto fileSize = filesystem::file_size(filePath);
        data.resize(fileSize);
    
        fstream file;
        file.open(filePath, ios::in | ios::binary);
        file.read(data.data(), fileSize);
        file.close();
    
        return data;
    }
    
    vector<char> generateData(uint32_t size) {
        vector<char> data(size, 0);
    
        std::mt19937 range(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_int_distribution<int> dist(32, 126);
    
        for (uint32_t index = 0; index < size; index++) {
            data[index] = static_cast<char>(dist(range));
        }
    
        return data;
    }
};

TEST(creation, invalid)
{
    ASSERT_THROW(
        auto stream = make_unique<audio_stream::Client>(0);
    , audio_stream::Exception::BadAlloc);
}

TEST(creation, valid)
{
    ASSERT_NO_THROW(
        auto stream = make_unique<audio_stream::Client>(1024);
    );
}

TEST_F(ClientTest, connect_to_not_ready_server)
{    
    EXPECT_NO_THROW(
        const auto isConnected = stream_->connect(audio_stream::Endpoint("255.255.255.255", 9999), 100);
        ASSERT_FALSE(isConnected);

        stream_->disconnect();
        ASSERT_FALSE(stream_->isConnected());
    );
}

TEST_F(ClientTest, connect_to_ready_server)
{
    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_, 3000);
        EXPECT_TRUE(isConnected);
        EXPECT_TRUE(stream_->isConnected());

        stream_->disconnect();
        EXPECT_FALSE(stream_->isConnected());
    );
}

TEST_F(ClientTest, send_small_buffer)
{
    const string stringData = "_+ exampLe #$ tESt )(";
    vector<char> sendingData(stringData.begin(), stringData.end());

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_, 2000);
        EXPECT_TRUE(isConnected);

        string message = "$ test & example @ text ^"; 
        const audio_stream::Data data(message.data(), message.size());
        const auto sentSize = stream_->send(data);
        EXPECT_EQ(sentSize, static_cast<uint32_t>(message.size()));

        stream_->disconnect();
        EXPECT_FALSE(stream_->isConnected());

        vector<char> transferData(message.begin(), message.end());
        ASSERT_TRUE(verifyFile(receivedFilePath_, transferData));
    );
}

TEST_F(ClientTest, send_large_buffer)
{
    vector<char> sendingData = generateData(256 * 1024 + 3);

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);

        const audio_stream::Data data = sendingData;
        const auto sentSize = stream_->send(data);
        EXPECT_EQ(sentSize, static_cast<uint32_t>(sendingData.size()));

        stream_->disconnect();
        EXPECT_FALSE(stream_->isConnected());

        ASSERT_TRUE(verifyFile(receivedFilePath_, sendingData));
    );
}

TEST_F(ClientTest, send_larg_buffer_interrupted)
{
    vector<char> sampleData = generateData(2 * 1024 - 371);
    vector<char> sendingData;

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);

        for (uint32_t index = 0; index < 128; index++) {
            const audio_stream::Data data = sampleData;
            const auto sentSize = stream_->send(data);
            EXPECT_EQ(sentSize, sampleData.size());

            sendingData.insert(sendingData.end(), sampleData.begin(), sampleData.end());
            sleep_for(10ms);
        }

        stream_->disconnect();
        EXPECT_FALSE(stream_->isConnected());

        ASSERT_TRUE(verifyFile(receivedFilePath_, sendingData));
    );
}

TEST_F(ClientTest, send_small_audio)
{
    vector<char> sendingData = readFile(audioFilePath_);

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);

        const audio_stream::Data data = sendingData;
        const auto sentSize = stream_->send(data);
        EXPECT_EQ(sentSize, sendingData.size());    

        stream_->disconnect();
        EXPECT_FALSE(stream_->isConnected());

        ASSERT_TRUE(verifyFile(receivedFilePath_, sendingData));
    );
}


// TEST_F(ClientTest, send_several_audios)
// {
//     vector<char> sampleData = readFile(audioFilePath_);
//     vector<char> sendingData;

//     // ASSERT_NO_THROW(
//         const auto isConnected = stream_->connect(serverEndpoint_);
//         EXPECT_TRUE(isConnected);

//         for (uint32_t index = 0; index < 1; index++) {
//             const audio_stream::Data data = sampleData;
//             const auto sentSize = stream_->send(data);
//             EXPECT_EQ(sentSize, sampleData.size());

//             sendingData.insert(sendingData.begin(), sampleData.begin(), sampleData.end());
//             sleep_for(10ms);
//         }

//         stream_->disconnect();
//         EXPECT_FALSE(stream_->isConnected());

//         sleep_for(10000ms);// TODO(MN): Move into the verifyFile and calculate
//         ASSERT_TRUE(verifyFile(receivedFilePath_, sendingData));
//     // );
// }

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

