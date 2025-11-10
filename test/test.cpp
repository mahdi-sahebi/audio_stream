#include <unistd.h>
#include <memory>
#include <iostream>
#include <future>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
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
    file.open(filePath, ios::in);
    file.read(temp.data(), fileSize);
    file.close();

    return (0 == memcmp(temp.data(), data.data(), data.size()));
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
    FAIL();
}

TEST(send_buffer, large_interrupted_communication)
{
    FAIL();
}

TEST(send_audio, small)
{
    FAIL();
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

