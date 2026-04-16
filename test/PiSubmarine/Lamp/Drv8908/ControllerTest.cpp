#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "PiSubmarine/Drv8908/IDevice.h"
#include "PiSubmarine/Drv8908/IPowerManager.h"
#include "PiSubmarine/Lamp/Drv8908/Controller.h"
#include "PiSubmarine/RegUtils.h"

namespace PiSubmarine::Lamp::Drv8908
{
    namespace
    {
        class IDeviceMock : public PiSubmarine::Drv8908::IDevice
        {
        public:
            MOCK_METHOD(void, SetSleeping, (bool sleepEnabled), (const, override));
            MOCK_METHOD(bool, IsSleeping, (), (const, override));
            MOCK_METHOD(bool, HasFault, (), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetStatus, (PiSubmarine::Drv8908::IcStatus& icStat), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetOpenLoadStatus, (PiSubmarine::Drv8908::OpenLoadStatus& ovp), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetOvercurrentStatus, (PiSubmarine::Drv8908::OverCurrentStatus& ovp), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetConfigCtrl, (PiSubmarine::Drv8908::ConfigCtrl& outConfigCtr), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetConfigCtrl, (const PiSubmarine::Drv8908::ConfigCtrl& inConfigCtr), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, IsHalfBridgeEnabled, (PiSubmarine::Drv8908::HalfBridge hb, bool& high, bool& low), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetHalfBridgeEnabled, (PiSubmarine::Drv8908::HalfBridge hb, bool high, bool low), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetHalfBridgeEnabled, (PiSubmarine::Drv8908::HalfBridgeBitMask hBridges, bool high, bool low), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetPwmFrequency, (PiSubmarine::Drv8908::PwmGeneratorBitMask generator, PiSubmarine::Drv8908::PwmFrequency freq), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetPwmFrequency, (PiSubmarine::Drv8908::PwmGenerator generator, PiSubmarine::Drv8908::PwmFrequency freq), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetPwmFrequency, (PiSubmarine::Drv8908::PwmGenerator generator, PiSubmarine::Drv8908::PwmFrequency& freq), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetPwmMap, (PiSubmarine::Drv8908::HalfBridge hb, PiSubmarine::Drv8908::PwmGenerator generator), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetPwmMap, (PiSubmarine::Drv8908::HalfBridgeBitMask hbMask, PiSubmarine::Drv8908::PwmGenerator generator), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetPwmMap, (PiSubmarine::Drv8908::HalfBridge hb, PiSubmarine::Drv8908::PwmGenerator& generator), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetDutyCycle, (PiSubmarine::Drv8908::PwmGenerator generator, NormalizedIntFraction<8>& value), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetDutyCycle, (PiSubmarine::Drv8908::PwmGeneratorBitMask generator, NormalizedIntFraction<8> value), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetDutyCycle, (PiSubmarine::Drv8908::PwmGenerator generator, NormalizedIntFraction<8> value), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetHalfBridgePwmModes, (PiSubmarine::Drv8908::HalfBridgeBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetHalfBridgePwmModes, (PiSubmarine::Drv8908::HalfBridgeBitMask& channels), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetHalfBridgeActiveFreeWheeling, (PiSubmarine::Drv8908::HalfBridgeBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetHalfBridgeActiveFreeWheeling, (PiSubmarine::Drv8908::HalfBridgeBitMask& channels), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetHalfBridgeFastSlewRate, (PiSubmarine::Drv8908::HalfBridgeBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetHalfBridgeFastSlewRate, (PiSubmarine::Drv8908::HalfBridgeBitMask& channels), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetEnabledPwmGenerators, (PiSubmarine::Drv8908::PwmGeneratorBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetEnabledPwmGenerators, (PiSubmarine::Drv8908::PwmGeneratorBitMask& channels), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetEnabledOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetEnabledOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask& channels), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetOpenLoadDetectControl2, (const PiSubmarine::Drv8908::OpenLoadDetectControl& value), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetOpenLoadDetectControl2, (PiSubmarine::Drv8908::OpenLoadDetectControl& value), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, SetOpenLoadDetectControl3, (PiSubmarine::Drv8908::OcpDeglitchTime deglitchTime, bool negativeCurrentOldEnabled), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetOpenLoadDetectControl3, (PiSubmarine::Drv8908::OcpDeglitchTime& deglitchTime, bool& negativeCurrentOldEnabled), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, EnableLowCurrentOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetEnabledLowCurrentOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask& channels), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, EnablePassiveOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetEnabledPassiveOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask& channels), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, EnablePassiveVmOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask channelMask), (const, override));
            MOCK_METHOD(PiSubmarine::Drv8908::IcStatus, GetEnabledPassiveVmOpenLoadDetect, (PiSubmarine::Drv8908::HalfBridgeBitMask& channels), (const, override));
        };

        class TestPowerManager : public PiSubmarine::Drv8908::IPowerManager
        {
        public:
            PiSubmarine::Drv8908::PowerLease Acquire() override
            {
                ++AcquireCount;
                return CreateLease(this, AcquireCount);
            }

            void Release(PiSubmarine::Drv8908::PowerLease& lease) override
            {
                ++ReleaseCount;
                SetLeaseManager(lease, nullptr);
                SetLeaseUserIndex(lease, -1);
            }

            int AcquireCount = 0;
            int ReleaseCount = 0;
        };

        [[nodiscard]] constexpr PiSubmarine::Drv8908::IcStatus ValidSpiStatus()
        {
            return PiSubmarine::Drv8908::IcStatus::TestBit;
        }

        void PrepareSuccessfulConfigurationDefaults(
            testing::NiceMock<IDeviceMock>& chip,
            const PiSubmarine::Drv8908::PwmGeneratorBitMask existingGenerators = static_cast<PiSubmarine::Drv8908::PwmGeneratorBitMask>(0),
            const PiSubmarine::Drv8908::HalfBridgeBitMask existingPwmModes = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0),
            const PiSubmarine::Drv8908::HalfBridgeBitMask existingActiveFreeWheeling = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0),
            const PiSubmarine::Drv8908::HalfBridgeBitMask existingFastSlewRate = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0),
            const PiSubmarine::Drv8908::HalfBridgeBitMask existingOpenLoadDetect = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0),
            const PiSubmarine::Drv8908::HalfBridgeBitMask existingLowCurrentOld = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0),
            const PiSubmarine::Drv8908::HalfBridgeBitMask existingPassiveOld = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0),
            const PiSubmarine::Drv8908::HalfBridgeBitMask existingPassiveVmOld = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0),
            const bool negativeCurrentOldEnabled = false)
        {
            using namespace PiSubmarine::RegUtils;

            ON_CALL(chip, GetConfigCtrl(testing::_))
                .WillByDefault([](PiSubmarine::Drv8908::ConfigCtrl& configCtrl)
                {
                    configCtrl = {
                        .PoldEn = false,
                        .Id = PiSubmarine::Drv8908::IcId::DRV8908,
                        .OcpRep = false,
                        .OtwRep = false,
                        .ExtOvp = false,
                        .ClrFlt = false
                    };
                    return ValidSpiStatus();
                });
            ON_CALL(chip, SetOpenLoadDetectControl2(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetOpenLoadDetectControl3(testing::_, testing::_))
                .WillByDefault([negativeCurrentOldEnabled](
                    PiSubmarine::Drv8908::OcpDeglitchTime& deglitchTime,
                    bool& isNegativeCurrentOldEnabled)
                {
                    deglitchTime = PiSubmarine::Drv8908::OcpDeglitchTime::MicroSeconds20;
                    isNegativeCurrentOldEnabled = negativeCurrentOldEnabled;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, SetOpenLoadDetectControl3(testing::_, testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetEnabledPwmGenerators(testing::_))
                .WillByDefault([existingGenerators](PiSubmarine::Drv8908::PwmGeneratorBitMask& generators)
                {
                    generators = static_cast<PiSubmarine::Drv8908::PwmGeneratorBitMask>(existingGenerators);
                    return ValidSpiStatus();
                });
            ON_CALL(chip, SetEnabledPwmGenerators(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, SetPwmFrequency(testing::A<PiSubmarine::Drv8908::PwmGenerator>(), testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetHalfBridgePwmModes(testing::_))
                .WillByDefault([existingPwmModes](PiSubmarine::Drv8908::HalfBridgeBitMask& channels)
                {
                    channels = existingPwmModes;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, SetHalfBridgePwmModes(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetHalfBridgeActiveFreeWheeling(testing::_))
                .WillByDefault([existingActiveFreeWheeling](PiSubmarine::Drv8908::HalfBridgeBitMask& channels)
                {
                    channels = existingActiveFreeWheeling;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, SetHalfBridgeActiveFreeWheeling(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetHalfBridgeFastSlewRate(testing::_))
                .WillByDefault([existingFastSlewRate](PiSubmarine::Drv8908::HalfBridgeBitMask& channels)
                {
                    channels = existingFastSlewRate;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, SetHalfBridgeFastSlewRate(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetEnabledOpenLoadDetect(testing::_))
                .WillByDefault([existingOpenLoadDetect](PiSubmarine::Drv8908::HalfBridgeBitMask& channels)
                {
                    channels = existingOpenLoadDetect;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, SetEnabledOpenLoadDetect(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetEnabledLowCurrentOpenLoadDetect(testing::_))
                .WillByDefault([existingLowCurrentOld](PiSubmarine::Drv8908::HalfBridgeBitMask& channels)
                {
                    channels = existingLowCurrentOld;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, EnableLowCurrentOpenLoadDetect(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetEnabledPassiveOpenLoadDetect(testing::_))
                .WillByDefault([existingPassiveOld](PiSubmarine::Drv8908::HalfBridgeBitMask& channels)
                {
                    channels = existingPassiveOld;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, EnablePassiveOpenLoadDetect(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, GetEnabledPassiveVmOpenLoadDetect(testing::_))
                .WillByDefault([existingPassiveVmOld](PiSubmarine::Drv8908::HalfBridgeBitMask& channels)
                {
                    channels = existingPassiveVmOld;
                    return ValidSpiStatus();
                });
            ON_CALL(chip, EnablePassiveVmOpenLoadDetect(testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, SetHalfBridgeEnabled(testing::A<PiSubmarine::Drv8908::HalfBridgeBitMask>(), testing::_, testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, SetPwmMap(testing::A<PiSubmarine::Drv8908::HalfBridgeBitMask>(), testing::A<PiSubmarine::Drv8908::PwmGenerator>()))
                .WillByDefault(testing::Return(ValidSpiStatus()));
            ON_CALL(chip, SetDutyCycle(testing::A<PiSubmarine::Drv8908::PwmGenerator>(), testing::_))
                .WillByDefault(testing::Return(ValidSpiStatus()));
        }
    }

    TEST(ControllerTest, SetIntensityPreservesNegativeCurrentOldAndConfiguresSelectedHighSides)
    {
        using namespace PiSubmarine::RegUtils;

        testing::NiceMock<IDeviceMock> chip;
        TestPowerManager powerManager;
        PrepareSuccessfulConfigurationDefaults(
            chip,
            PiSubmarine::Drv8908::PwmGeneratorBitMask::PwmGenerator1,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge1,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge1
                | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge2
                | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge4,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge4,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge1,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge4,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge2,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge2 | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge3,
            true);

        Controller controller(
            chip,
            powerManager,
            PiSubmarine::Drv8908::PwmGenerator::PwmGenerator3,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge2 | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge4,
            SwitchSide::High);

        EXPECT_CALL(chip, SetOpenLoadDetectControl3(PiSubmarine::Drv8908::OcpDeglitchTime::MicroSeconds60, true));
        EXPECT_CALL(chip, SetHalfBridgeEnabled(
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge2 | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge4,
            true,
            false));
        EXPECT_CALL(chip, SetEnabledOpenLoadDetect(
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge1
            | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge2
            | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge4));
        EXPECT_CALL(chip, EnablePassiveVmOpenLoadDetect(PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge3));
        EXPECT_CALL(chip, SetDutyCycle(PiSubmarine::Drv8908::PwmGenerator::PwmGenerator3, testing::_));

        const auto result = controller.SetIntensity(NormalizedFraction(0.5));

        EXPECT_TRUE(result.has_value());
        EXPECT_EQ(powerManager.AcquireCount, 1);
        EXPECT_EQ(powerManager.ReleaseCount, 0);
    }

    TEST(ControllerTest, GetStatusFiltersOpenLoadAndOverCurrentToConfiguredLowSides)
    {
        using namespace PiSubmarine::RegUtils;

        testing::NiceMock<IDeviceMock> chip;
        TestPowerManager powerManager;
        Config config;
        config.EnableLowCurrentOpenLoadDetection = true;
        PrepareSuccessfulConfigurationDefaults(chip);

        Controller controller(
            chip,
            powerManager,
            PiSubmarine::Drv8908::PwmGenerator::PwmGenerator2,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge1 | PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge3,
            SwitchSide::Low,
            config);

        ASSERT_TRUE(controller.SetIntensity(NormalizedFraction(0.3)).has_value());

        EXPECT_CALL(chip, GetStatus(testing::_))
            .WillOnce([](PiSubmarine::Drv8908::IcStatus& icStatus)
            {
                icStatus = PiSubmarine::Drv8908::IcStatus::OpenLoad
                    | PiSubmarine::Drv8908::IcStatus::OverCurrent
                    | PiSubmarine::Drv8908::IcStatus::OverVoltage
                    | PiSubmarine::Drv8908::IcStatus::OverTemperatureWarning;
                return ValidSpiStatus();
            });
        EXPECT_CALL(chip, GetOpenLoadStatus(testing::_))
            .WillOnce([](PiSubmarine::Drv8908::OpenLoadStatus& openLoadStatus)
            {
                using namespace PiSubmarine::RegUtils;
                openLoadStatus = PiSubmarine::Drv8908::OpenLoadStatus::HB3_LS_OLD
                    | PiSubmarine::Drv8908::OpenLoadStatus::HB4_LS_OLD;
                return ValidSpiStatus();
            });
        EXPECT_CALL(chip, GetOvercurrentStatus(testing::_))
            .WillOnce([](PiSubmarine::Drv8908::OverCurrentStatus& overCurrentStatus)
            {
                using namespace PiSubmarine::RegUtils;
                overCurrentStatus = PiSubmarine::Drv8908::OverCurrentStatus::HB1_LS_OCP
                    | PiSubmarine::Drv8908::OverCurrentStatus::HB2_LS_OCP;
                return ValidSpiStatus();
            });

        const auto statusResult = controller.GetStatus();

        ASSERT_TRUE(statusResult.has_value());
        EXPECT_TRUE(statusResult->IsActive);
        EXPECT_TRUE(statusResult->HasOpenLoadFault);
        EXPECT_TRUE(statusResult->HasOvercurrentFault);
        EXPECT_TRUE(statusResult->HasOvervoltageFault);
        EXPECT_TRUE(statusResult->HasOvertemperatureWarning);
        EXPECT_FALSE(statusResult->HasUndervoltageFault);
        EXPECT_FALSE(statusResult->HasOvertemperatureShutdownFault);
    }

    TEST(ControllerTest, SettingZeroIntensityReleasesPowerLease)
    {
        testing::NiceMock<IDeviceMock> chip;
        TestPowerManager powerManager;
        PrepareSuccessfulConfigurationDefaults(chip);

        Controller controller(
            chip,
            powerManager,
            PiSubmarine::Drv8908::PwmGenerator::PwmGenerator1,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge6,
            SwitchSide::High);

        ASSERT_TRUE(controller.SetIntensity(NormalizedFraction(0.2)).has_value());
        ASSERT_TRUE(controller.SetIntensity(NormalizedFraction(0)).has_value());
        EXPECT_EQ(powerManager.AcquireCount, 1);
        EXPECT_EQ(powerManager.ReleaseCount, 1);
    }

    TEST(ControllerTest, RejectsLowCurrentOpenLoadDetectionForHighSideLamp)
    {
        testing::NiceMock<IDeviceMock> chip;
        TestPowerManager powerManager;
        Config config;
        config.EnableLowCurrentOpenLoadDetection = true;

        Controller controller(
            chip,
            powerManager,
            PiSubmarine::Drv8908::PwmGenerator::PwmGenerator1,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge2,
            SwitchSide::High,
            config);

        const auto result = controller.SetIntensity(NormalizedFraction(0.4));

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(powerManager.AcquireCount, 0);
        EXPECT_EQ(powerManager.ReleaseCount, 0);
    }
}
