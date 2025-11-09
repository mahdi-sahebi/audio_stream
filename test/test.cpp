#include <memory>
#include <iostream>
#include <gtest/gtest.h>


using namespace std;


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
    FAIL();
}

TEST(connection, ready_server)
{
    FAIL();
}

TEST(send_buffer, small)
{
    FAIL();
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

