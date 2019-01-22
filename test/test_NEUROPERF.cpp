
#include "gtest/gtest.h"

namespace
{
	class Tamplate_Tests : public testing::Test{};

	TEST_F(Tamplate_Tests, temp)
	{
		EXPECT_TRUE(true) << "true is not true\n";
	}
}

//*******************************************************
//*******************************************************
//*******************************************************

int main(int argc, char** argv)
{

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
