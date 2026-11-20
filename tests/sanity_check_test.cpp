// Unit tests for the input validators in sanity_check.h.
//
// sanity_check.h expects a few symbols to be visible before it is included (the
// CLI pulls them in via earlier headers). We provide them here so the validators
// can be tested in isolation, without dragging in the crypto / core-submodule
// dependency graph: <cstring> for the str/mem calls, DEFAULT_SEED, and a stubbed
// checkSumIdentity (the identity-format tests below exit before the checksum, so
// the stub is never relied upon for a result).
#include <cstring>
#include <cstdlib>
#include <string>

#include "gtest/gtest.h"

#define DEFAULT_SEED "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
static bool checkSumIdentity(const char*) { return true; }

#include "sanity_check.h"

TEST(IsValidIpAddress, AcceptsWellFormedIPv4)
{
    char ip[] = "192.168.1.1";
    EXPECT_TRUE(isValidIpAddress(ip));
    char zero[] = "0.0.0.0";
    EXPECT_TRUE(isValidIpAddress(zero));
    char max[] = "255.255.255.255";
    EXPECT_TRUE(isValidIpAddress(max));
}

TEST(IsValidIpAddress, RejectsMalformedOrOutOfRange)
{
    char over[] = "256.1.1.1";
    EXPECT_FALSE(isValidIpAddress(over));
    char neg[] = "-1.2.3.4";
    EXPECT_FALSE(isValidIpAddress(neg));
    char tooFew[] = "1.2.3";
    EXPECT_FALSE(isValidIpAddress(tooFew));
    char text[] = "not.an.ip.addr";
    EXPECT_FALSE(isValidIpAddress(text));
}

// The remaining validators abort the process with exit(1) on bad input and
// simply return on good input, so they are exercised with death tests.

TEST(SanityCheckValidAssetName, AcceptsValidNames)
{
    sanityCheckValidAssetName("QX");    // upper-case start
    sanityCheckValidAssetName("CFB123"); // letters + digits, <= 7 chars
    SUCCEED();
}

TEST(SanityCheckValidAssetName, RejectsInvalidNames)
{
    EXPECT_EXIT(sanityCheckValidAssetName("qx"), ::testing::ExitedWithCode(1), "");        // lower-case start
    EXPECT_EXIT(sanityCheckValidAssetName("TOOLONG1"), ::testing::ExitedWithCode(1), "");  // 8 chars
    EXPECT_EXIT(sanityCheckValidAssetName("1ABC"), ::testing::ExitedWithCode(1), "");      // digit start
    EXPECT_EXIT(sanityCheckValidAssetName(nullptr), ::testing::ExitedWithCode(1), "");     // null
}

TEST(SanityCheckTxType, AcceptsKnownRejectsUnknown)
{
    sanityCheckTxType(0);
    sanityCheckTxType(1);
    SUCCEED();
    EXPECT_EXIT(sanityCheckTxType(2), ::testing::ExitedWithCode(1), "");
}

TEST(CheckValidEpoch, BoundsAreEnforced)
{
    checkValidEpoch(0);
    checkValidEpoch(999);
    SUCCEED();
    EXPECT_EXIT(checkValidEpoch(-1), ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(checkValidEpoch(1000), ::testing::ExitedWithCode(1), "");
}

TEST(SanityCheckMainAuxStatus, OnlyMainOrAux)
{
    sanityCheckMainAuxStatus("MAIN");
    sanityCheckMainAuxStatus("AUX");
    SUCCEED();
    EXPECT_EXIT(sanityCheckMainAuxStatus("FOO"), ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(sanityCheckMainAuxStatus("MAIX"), ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(sanityCheckMainAuxStatus(nullptr), ::testing::ExitedWithCode(1), "");
}

TEST(SanityCheckUnitOfMeasurement, AcceptsSevenChars)
{
    sanityCheckUnitofMeasurement("1234567"); // length 7 is accepted
    SUCCEED();
}

TEST(SanityCheckUnitOfMeasurement, RejectsWrongLengthOrNull)
{
    EXPECT_EXIT(sanityCheckUnitofMeasurement("SHORT"), ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(sanityCheckUnitofMeasurement(nullptr), ::testing::ExitedWithCode(1), "");
}

TEST(SanityCheckLoggingMode, ZeroToTwoOnly)
{
    sanityCheckLoggingMode(0);
    sanityCheckLoggingMode(2);
    SUCCEED();
    EXPECT_EXIT(sanityCheckLoggingMode(3), ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(sanityCheckLoggingMode((char)-1), ::testing::ExitedWithCode(1), "");
}

TEST(SanityCheckExtraDataSize, RejectsOverLimit)
{
    sanityCheckExtraDataSize(1024);
    SUCCEED();
    EXPECT_EXIT(sanityCheckExtraDataSize(1025), ::testing::ExitedWithCode(1), "");
}
