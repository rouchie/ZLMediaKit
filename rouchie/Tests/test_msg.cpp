#include <gtest/gtest.h>

#include "Base/RQMsg.h"

TEST(MSG, Build2) {
    RQMsg::Ptr msg = RQMsg::Build(10, 100);
    EXPECT_EQ(msg->_sender, 0);
    EXPECT_EQ(msg->_recver, 10);
    EXPECT_EQ(msg->_cmd, 100);
    EXPECT_EQ(msg->_param0, 0);
    EXPECT_DOUBLE_EQ(msg->_param1, 0);
    EXPECT_TRUE(msg->_param2.empty());
}

TEST(MSG, Build3) {
    {
        RQMsg::Ptr msg = RQMsg::Build(10, 100, 0);
        EXPECT_EQ(msg->_sender, 0);
        EXPECT_EQ(msg->_recver, 10);
        EXPECT_EQ(msg->_cmd, 100);
        EXPECT_EQ(msg->_param0, 0);
        EXPECT_DOUBLE_EQ(msg->_param1, 0);
        EXPECT_TRUE(msg->_param2.empty());
    }

    {
        RQMsg::Ptr msg = RQMsg::Build(10, 100, 0.1);
        EXPECT_EQ(msg->_sender, 0);
        EXPECT_EQ(msg->_recver, 10);
        EXPECT_EQ(msg->_cmd, 100);
        EXPECT_EQ(msg->_param0, 0);
        EXPECT_DOUBLE_EQ(msg->_param1, 0.1);
        EXPECT_TRUE(msg->_param2.empty());
    }


}
