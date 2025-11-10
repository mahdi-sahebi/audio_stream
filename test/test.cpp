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


using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::this_thread;


class ClientTest : public testing::Test
{
protected:
    AudioStream::Endpoint serverEndpoint_;
    AudioStream stream_;
    string receivedFilePath_;
    stomic<pid_t> serverPID_;
    future<bool> serverTask_;

    void SetUp() override
    {
        serverEndpoint_ = Endpoint("127.0.0.0", 8080);

        receivedFilePath_ = "received_data.bin";
        filesystem::remove(receivedFilePath_);
        
        stream_ = unique_ptr<AudioStream>(4 * 1024 * 1024);
        if (nullptr == stream_) {
            FAIL();
            GTEST_SKIP();
        }

        startServer();
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
            if (0 == pid) {
                cout << "Process creation failed" << endl;
                return false;
            }
    
            serverPID_ = pid;
            waitpid(serverPID_, nullptr, 0);
            return true;
        });

        /* Let server to be executed */
        sleep_for(500ms);
        
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

        const auto serverResult = serverTask_.get();
        ASSERT_EQ(serverResult, 0);
    }

    bool verifyFile(const string& filePath, const vector<char>& data)
    {
        if (!filesystem::exists(filePath)) {
            return false;
        }
        
        const auto fileSize = filesystem::size(filePath);
        if (fileSize != data.size()) {
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
            data.push_back(static_cast<char>(dist(range)));
        }
    
        return data;
    }
};


TEST(creation, invalid)
{
    ASSERT_THROW(
        auto stream = make_unique<AudioStream>(0);
    , AudioStream::Exception::BadAlloc);
}

TEST(creation, valid)
{
    ASSERT_NO_THROW(
        auto stream = make_unique<AudioStream>(1024);
    );
}

TEST_F(ClientTest, connect_to_not_ready_server)
{    
    try {
        const auto isConnected = stream_->connect(Endpoint("255.255.255.255", 9999), 100);
        ASSERT_FALSE(isConnected);

        const auto isDisconnected = stream_->disconnect();
        ASSERT_FALSE(isDisconnected);
    } catch (const exception& e) {
        cout << e.what() << endl;
        FAIL();
    } catch (...) {
        cout << "Unknown exception" << endl;
        FAIL();
    }
}

TEST_F(ClientTest, connect_to_ready_server)
{
    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);

        const auto isDisconnected = stream_->disconnect();
        EXPECTED_TRUE(isDisconnected);
    );
}

TEST_F(ClientTest, send_small_buffer)
{
    ASSERT_TRUE(filesystem::exists("server.js"));
    vector<char> sendingData = "_+ exampLe #$ tESt )(";

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);


        const string message = "$ test & example @ text ^"; 
        const Data data = message;
        const auto sentSize = stream_->send(data);
        EXPECTED_EQ(sentSize, message.size());

        const auto isDisconnected = stream_->disconnect();
        EXPECTED_TRUE(isDisconnected);
    );
}

TEST_F(ClientTest, send_large_buffer)
{
    ASSERT_TRUE(filesystem::exists("server.js"));
    vector<char> sendingData = generateData(256 * 1024);

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);


        const Data data = sendingData;
        const auto sentSize = stream_->send(data);
        EXPECTED_EQ(sentSize, sendingData.size());

        const auto isDisconnected = stream_->disconnect();
        EXPECTED_TRUE(isDisconnected);
    );
}

TEST_F(ClientTest, send_larg_buffer_interrupted)
{
    ASSERT_TRUE(filesystem::exists("server.js"));
    vector<char> sampleData = generateData(2 * 1024);
    vector<char> sendingData;

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);


        for (uint32_t index = 0; index < 128; index++) {
            const Data data = sampleData;
            const auto sentSize = stream_->send(data);
            EXPECTED_EQ(sentSize, sampleData.size());

            sendingData.append(sampleData);
            sleep_for(100ms);
        }

        const auto isDisconnected = stream_->disconnect();
        EXPECTED_TRUE(isDisconnected);
    );
}

TEST_F(ClientTest, send_small_audio)
{
    ASSERT_TRUE(filesystem::exists("server.js"));
    vector<char> sendingData = readFile("glass.wav");

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);

        const Data data = sendingData;
        const auto sentSize = stream_->send(data);
        EXPECTED_EQ(sentSize, sendingData.size());

        const auto isDisconnected = stream_->disconnect();
        EXPECTED_TRUE(isDisconnected);
    );
}

TEST_F(ClientTest, send_several_audios)
{
    ASSERT_TRUE(filesystem::exists("server.js"));
    vector<char> sampleData = readFile("glass.wav");
    vector<char> sendingData;

    ASSERT_NO_THROW(
        const auto isConnected = stream_->connect(serverEndpoint_);
        EXPECT_TRUE(isConnected);

        for (uint32_t index = 0; index < 1024; index++) {
            const Data data = sampleData;
            const auto sentSize = stream_->send(data);
            EXPECTED_EQ(sentSize, sampleData.size());

            sendingData.append(sampleData);
            sleep_for(100ms);
        }

        const auto isDisconnected = stream_->disconnect();
        EXPECTED_TRUE(isDisconnected);
    );
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

