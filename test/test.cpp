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


static bool verifyFile(const string& filePath, const vector<char>& data)
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

static vector<char> readFile(const string& filePath)
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

static vector<char> generateData(uint32_t size) {
    vector<char> data(size, 0);

    std::mt19937 range(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> dist(32, 126);

    for (uint32_t index = 0; index < size; index++) {
        data.push_back(static_cast<char>(dist(range)));
    }

    return data;
}

TEST(creation, invalid)
{
    ASSERT_THROW(
        AudioStream stream(0);
    , AudioStream::Exception::BadAlloc);
}

TEST(creation, valid)
{
    ASSERT_NO_THROW(
        AudioStream stream(1024);
    );
}

TEST(connection, not_ready_server)
{
    unique_ptr<AudioStream> stream = nullptr;

    ASSERT_NO_THROW(
        stream = make_unique<AudioStream>(1024);
    );

    ASSERT_NE(stream, nullptr);
    
    try {
        const auto isConnected = stream.connect(Endpoint("255.255.255.255", 9999), 100);
        ASSERT_FALSE(isConnected);

        const auto isDisconnected = stream.disconnect();
        ASSERT_FALSE(isDisconnected);
    } catch (const exception& e) {
        cout << e.what() << endl;
        FAIL();
    } catch (...) {
        cout << "Unknown exception" << endl;
        FAIL();
    }
}

TEST(connection, ready_server)
{
    // TODO(MN): Fixture class
    // TODO(MN): Check nodejs server file existance

    atomic<pid_t> serverPID{0};

    future<bool> serverTask = async(launch::async, [&]() {
        const auto pid = fork();
        if (0 == pid) {
            cout << "Process creation failed" << endl;
            return false;
        }

        serverPID = pid;
        waitpid(serverPID, nullptr, 0);
        return true;
    });


    ASSERT_NO_THROW(
        /* Let server to be executed */
        sleep_for(milliseconds(500);

        auto stream = unique_ptr<AudioStream>(1024);
        
        const auto isConnected = stream->connect(Endpoint("127.0.0.0", 8080));
        EXPECT_TRUE(isConnected);

        const auto isDisconnected = stream->disconnect();
        EXPECTED_TRUE(isDisconnected);

        /* Stop the server */
        if (serverPID > 0) {
            kill(serverPID, SIGKILL);
            serverPID = 0;
        }

        const auto serverResult = serverTask.get();
        ASSERT_EQ(serverResult, 0);
    );
}

TEST(send_buffer, small)
{
    // TODO(MN): Fixture class
    const auto receivedFilePath = "received_data.bin";
    filesystem::remove(receivedFilePath);
    ASSERT_TRUE(filesystem::exists("server.js"));
    atomic<pid_t> serverPID{0};
    vector<char> sendingData = "_+ exampLe #$ tESt )(";


    future<bool> serverTask = async(launch::async, [&]() {
        const auto pid = fork();
        if (0 == pid) {
            cout << "Process creation failed" << endl;
            return false;
        }

        serverPID = pid;
        waitpid(serverPID, nullptr, 0);
        return true;
    });


    ASSERT_NO_THROW(
        /* Let the server to be executed */
        sleep_for(500ms);

        auto stream = unique_ptr<AudioStream>(1024);
        
        const auto isConnected = stream->connect(Endpoint("127.0.0.0", 8080));
        EXPECT_TRUE(isConnected);


        const string message = "$ test & example @ text ^"; 
        const Data data = message;
        const auto sentSize = stream->send(data);
        EXPECTED_EQ(sentSize, message.size());

        const auto isDisconnected = stream->disconnect();
        EXPECTED_TRUE(isDisconnected);

        /* Stop the server */
        if (serverPID > 0) {
            kill(serverPID, SIGKILL);
            serverPID = 0;
        }
        const auto serverResult = serverTask.get();
        ASSERT_EQ(serverResult, 0);

        ASSERT_TRUE(verifyFile(receivedFilePath, sendingData));
    );
}

TEST(send_buffer, large)
{
    // TODO(MN): Fixture class
    const auto receivedFilePath = "received_data.bin";
    filesystem::remove(receivedFilePath);
    ASSERT_TRUE(filesystem::exists("server.js"));
    atomic<pid_t> serverPID{0};
    vector<char> sendingData = generateData(256 * 1024);


    future<bool> serverTask = async(launch::async, [&]() {
        const auto pid = fork();
        if (0 == pid) {
            cout << "Process creation failed" << endl;
            return false;
        }

        serverPID = pid;
        waitpid(serverPID, nullptr, 0);
        return true;
    });


    ASSERT_NO_THROW(
        /* Let the server to be executed */
        sleep_for(500ms);

        auto stream = unique_ptr<AudioStream>(512 * 1024);
        
        const auto isConnected = stream->connect(Endpoint("127.0.0.0", 8080));
        EXPECT_TRUE(isConnected);


        const Data data = sendingData;
        const auto sentSize = stream->send(data);
        EXPECTED_EQ(sentSize, sendingData.size());

        const auto isDisconnected = stream->disconnect();
        EXPECTED_TRUE(isDisconnected);

        /* Stop the server */
        if (serverPID > 0) {
            kill(serverPID, SIGKILL);
            serverPID = 0;
        }
        const auto serverResult = serverTask.get();
        ASSERT_EQ(serverResult, 0);

        ASSERT_TRUE(verifyFile(receivedFilePath, sendingData));
    );
}

TEST(send_buffer, large_interrupted_communication)
{    
    // TODO(MN): Fixture class
    const auto receivedFilePath = "received_data.bin";
    filesystem::remove(receivedFilePath);
    ASSERT_TRUE(filesystem::exists("server.js"));
    atomic<pid_t> serverPID{0};
    vector<char> sampleData = generateData(2 * 1024);
    vector<char> sendingData;


    future<bool> serverTask = async(launch::async, [&]() {
        const auto pid = fork();
        if (0 == pid) {
            cout << "Process creation failed" << endl;
            return false;
        }

        serverPID = pid;
        waitpid(serverPID, nullptr, 0);
        return true;
    });


    ASSERT_NO_THROW(
        /* Let the server to be executed */
        sleep_for(500ms);

        auto stream = unique_ptr<AudioStream>(512 * 1024);
        
        const auto isConnected = stream->connect(Endpoint("127.0.0.0", 8080));
        EXPECT_TRUE(isConnected);


        for (uint32_t index = 0; index < 128; index++) {
            const Data data = sampleData;
            const auto sentSize = stream->send(data);
            EXPECTED_EQ(sentSize, sampleData.size());

            sendingData.append(sampleData);
            sleep_for(100ms);
        }

        const auto isDisconnected = stream->disconnect();
        EXPECTED_TRUE(isDisconnected);

        /* Stop the server */
        if (serverPID > 0) {
            kill(serverPID, SIGKILL);
            serverPID = 0;
        }
        const auto serverResult = serverTask.get();
        ASSERT_EQ(serverResult, 0);

        ASSERT_TRUE(verifyFile(receivedFilePath, sendingData));
    );
}

TEST(send_audio, small)
{
    // TODO(MN): Fixture class
    const auto receivedFilePath = "received_data.bin";
    filesystem::remove(receivedFilePath);
    ASSERT_TRUE(filesystem::exists("server.js"));
    atomic<pid_t> serverPID{0};
    vector<char> sendingData = readFile("glass.wav");


    future<bool> serverTask = async(launch::async, [&]() {
        const auto pid = fork();
        if (0 == pid) {
            cout << "Process creation failed" << endl;
            return false;
        }

        serverPID = pid;
        waitpid(serverPID, nullptr, 0);
        return true;
    });


    ASSERT_NO_THROW(
        /* Let the server to be executed */
        sleep_for(500ms);

        auto stream = unique_ptr<AudioStream>(512 * 1024);
        
        const auto isConnected = stream->connect(Endpoint("127.0.0.0", 8080));
        EXPECT_TRUE(isConnected);


        const Data data = sendingData;
        const auto sentSize = stream->send(data);
        EXPECTED_EQ(sentSize, sendingData.size());

        const auto isDisconnected = stream->disconnect();
        EXPECTED_TRUE(isDisconnected);

        /* Stop the server */
        if (serverPID > 0) {
            kill(serverPID, SIGKILL);
            serverPID = 0;
        }
        const auto serverResult = serverTask.get();
        ASSERT_EQ(serverResult, 0);

        ASSERT_TRUE(verifyFile(receivedFilePath, sendingData));
    );
}

TEST(send_audio, small_high_load)
{
    FAIL();
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

