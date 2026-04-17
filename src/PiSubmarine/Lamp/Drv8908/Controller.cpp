#include "PiSubmarine/Lamp/Drv8908/Controller.h"

#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/RegUtils.h"

namespace PiSubmarine::Lamp::Drv8908
{
    namespace
    {
        using PiSubmarine::Error::Api::ErrorCondition;
        using PiSubmarine::Error::Api::MakeError;
        using PiSubmarine::Error::Api::Result;

        [[nodiscard]] Result<void> MakeCommunicationError()
        {
            return std::unexpected(MakeError(ErrorCondition::CommunicationError));
        }

        [[nodiscard]] Result<void> MakeDeviceError()
        {
            return std::unexpected(MakeError(ErrorCondition::DeviceError));
        }

        [[nodiscard]] Result<void> MakeContractError()
        {
            return std::unexpected(MakeError(ErrorCondition::ContractError));
        }

        [[nodiscard]] constexpr bool IsZero(const NormalizedFraction value)
        {
            return static_cast<double>(value) == 0.0;
        }

        [[nodiscard]] constexpr PiSubmarine::Drv8908::OpenLoadStatus ToOpenLoadStatus(
            const PiSubmarine::Drv8908::HalfBridge halfBridge,
            const SwitchSide switchSide)
        {
            const auto bitIndex = static_cast<unsigned>(halfBridge) * 2U
                + (switchSide == SwitchSide::High ? 1U : 0U);
            return static_cast<PiSubmarine::Drv8908::OpenLoadStatus>(1U << bitIndex);
        }

        [[nodiscard]] constexpr PiSubmarine::Drv8908::OverCurrentStatus ToOverCurrentStatus(
            const PiSubmarine::Drv8908::HalfBridge halfBridge,
            const SwitchSide switchSide)
        {
            const auto bitIndex = static_cast<unsigned>(halfBridge) * 2U
                + (switchSide == SwitchSide::High ? 1U : 0U);
            return static_cast<PiSubmarine::Drv8908::OverCurrentStatus>(1U << bitIndex);
        }

        [[nodiscard]] bool HasAnySelectedOpenLoad(
            const PiSubmarine::Drv8908::OpenLoadStatus status,
            const PiSubmarine::Drv8908::HalfBridgeBitMask halfBridges,
            const SwitchSide switchSide)
        {
            using namespace PiSubmarine::RegUtils;

            for (std::uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex)
            {
                const auto halfBridgeMask = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(1U << bitIndex);
                if ((halfBridges & halfBridgeMask) == static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0))
                {
                    continue;
                }

                if (HasAnyFlag(status, ToOpenLoadStatus(static_cast<PiSubmarine::Drv8908::HalfBridge>(bitIndex), switchSide)))
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] bool HasAnySelectedOverCurrent(
            const PiSubmarine::Drv8908::OverCurrentStatus status,
            const PiSubmarine::Drv8908::HalfBridgeBitMask halfBridges,
            const SwitchSide switchSide)
        {
            using namespace PiSubmarine::RegUtils;

            for (std::uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex)
            {
                const auto halfBridgeMask = static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(1U << bitIndex);
                if ((halfBridges & halfBridgeMask) == static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0))
                {
                    continue;
                }

                if (HasAnyFlag(
                    status,
                    ToOverCurrentStatus(static_cast<PiSubmarine::Drv8908::HalfBridge>(bitIndex), switchSide)))
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] PiSubmarine::Drv8908::OpenLoadDetectControl BuildOpenLoadDetectControl2(const Config& config)
        {
            using PiSubmarine::Drv8908::OpenLoadDetectControl;
            using namespace PiSubmarine::RegUtils;

            auto value = static_cast<OpenLoadDetectControl>(0);
            if (config.KeepOutputEnabledOnOpenLoad)
            {
                value = value | OpenLoadDetectControl::OldOp;
            }
            if (config.ReportOpenLoadOnFaultPin)
            {
                value = value | OpenLoadDetectControl::OldRep;
            }
            return value;
        }
    }

    Controller::Controller(
        PiSubmarine::Drv8908::IDevice& chip,
        PiSubmarine::Drv8908::IPowerManager& powerManager,
        PiSubmarine::Drv8908::PwmGenerator pwmGenerator,
        PiSubmarine::Drv8908::HalfBridgeBitMask halfBridges,
        SwitchSide switchSide,
        Config config) :
        m_Chip(chip),
        m_PowerManager(powerManager),
        m_PwmGenerator(pwmGenerator),
        m_HalfBridges(halfBridges),
        m_SwitchSide(switchSide),
        m_Config(config)
    {
    }

    Result<void> Controller::SetIntensity(const NormalizedFraction intensity)
    {
        m_TargetIntensity = intensity;

        if (IsZero(intensity))
        {
            return DisableOutput();
        }

        if (const auto ensureResult = EnsurePoweredAndConfigured(); !ensureResult.has_value())
        {
            return ensureResult;
        }

        return ApplyDutyCycle(intensity);
    }

    Result<NormalizedFraction> Controller::GetIntensity() const
    {
        return m_TargetIntensity;
    }

    Result<void> Controller::SetPwmFrequency(const PiSubmarine::Drv8908::PwmFrequency pwmFrequency)
    {
        m_Config.PwmFrequency = pwmFrequency;

        if (!m_PowerLease.IsValid())
        {
            return {};
        }

        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetPwmFrequency(m_PwmGenerator, pwmFrequency)))
        {
            return MakeCommunicationError();
        }

        return {};
    }

    Result<PiSubmarine::Drv8908::PwmFrequency> Controller::GetPwmFrequency() const
    {
        return m_Config.PwmFrequency;
    }

    Result<Lamp::Telemetry::Api::Status> Controller::GetStatus() const
    {
        Lamp::Telemetry::Api::Status status{};
        status.IsActive = m_PowerLease.IsValid() && !IsZero(m_TargetIntensity);

        if (!m_PowerLease.IsValid())
        {
            return status;
        }

        PiSubmarine::Drv8908::IcStatus icStatus{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetStatus(icStatus)))
        {
            return std::unexpected(MakeError(ErrorCondition::CommunicationError));
        }

        using namespace PiSubmarine::RegUtils;

        status.HasOvervoltageFault = HasAnyFlag(icStatus, PiSubmarine::Drv8908::IcStatus::OverVoltage);
        status.HasUndervoltageFault = HasAnyFlag(icStatus, PiSubmarine::Drv8908::IcStatus::UnderVoltage);
        status.HasOvertemperatureShutdownFault = HasAnyFlag(
            icStatus,
            PiSubmarine::Drv8908::IcStatus::OverTemperatureShutdown);
        status.HasOvertemperatureWarning = HasAnyFlag(icStatus, PiSubmarine::Drv8908::IcStatus::OverTemperatureWarning);

        if (HasAnyFlag(icStatus, PiSubmarine::Drv8908::IcStatus::OpenLoad))
        {
            PiSubmarine::Drv8908::OpenLoadStatus openLoadStatus{};
            if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetOpenLoadStatus(openLoadStatus)))
            {
                return std::unexpected(MakeError(ErrorCondition::CommunicationError));
            }
            status.HasOpenLoadFault = HasAnySelectedOpenLoad(openLoadStatus, m_HalfBridges, m_SwitchSide);
        }

        if (HasAnyFlag(icStatus, PiSubmarine::Drv8908::IcStatus::OverCurrent))
        {
            PiSubmarine::Drv8908::OverCurrentStatus overCurrentStatus{};
            if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetOvercurrentStatus(overCurrentStatus)))
            {
                return std::unexpected(MakeError(ErrorCondition::CommunicationError));
            }
            status.HasOvercurrentFault = HasAnySelectedOverCurrent(overCurrentStatus, m_HalfBridges, m_SwitchSide);
        }

        return status;
    }

    Result<void> Controller::ConfigureChip()
    {
        using namespace PiSubmarine::RegUtils;

        PiSubmarine::Drv8908::ConfigCtrl configCtrl{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetConfigCtrl(configCtrl)))
        {
            return MakeCommunicationError();
        }

        if (configCtrl.Id != PiSubmarine::Drv8908::IcId::DRV8908)
        {
            return MakeDeviceError();
        }

        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetOpenLoadDetectControl2(BuildOpenLoadDetectControl2(m_Config))))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::OcpDeglitchTime existingDeglitchTime{};
        bool negativeCurrentOldEnabled = false;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetOpenLoadDetectControl3(existingDeglitchTime, negativeCurrentOldEnabled)))
        {
            return MakeCommunicationError();
        }

        if (!PiSubmarine::Drv8908::IsValid(
            m_Chip.SetOpenLoadDetectControl3(m_Config.OpenLoadDeglitchTime, negativeCurrentOldEnabled)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::PwmGeneratorBitMask enabledPwmGenerators{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetEnabledPwmGenerators(enabledPwmGenerators)))
        {
            return MakeCommunicationError();
        }
        enabledPwmGenerators = enabledPwmGenerators | PiSubmarine::Drv8908::ToPwmGeneratorBitMask(m_PwmGenerator);
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetEnabledPwmGenerators(enabledPwmGenerators)))
        {
            return MakeCommunicationError();
        }

        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetPwmFrequency(m_PwmGenerator, m_Config.PwmFrequency)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::HalfBridgeBitMask pwmModeHalfBridges{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetHalfBridgePwmModes(pwmModeHalfBridges)))
        {
            return MakeCommunicationError();
        }
        pwmModeHalfBridges = pwmModeHalfBridges | m_HalfBridges;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetHalfBridgePwmModes(pwmModeHalfBridges)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::HalfBridgeBitMask activeFreeWheeling{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetHalfBridgeActiveFreeWheeling(activeFreeWheeling)))
        {
            return MakeCommunicationError();
        }
        activeFreeWheeling = activeFreeWheeling & ~m_HalfBridges;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetHalfBridgeActiveFreeWheeling(activeFreeWheeling)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::HalfBridgeBitMask fastSlewRate{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetHalfBridgeFastSlewRate(fastSlewRate)))
        {
            return MakeCommunicationError();
        }
        fastSlewRate = fastSlewRate & ~m_HalfBridges;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetHalfBridgeFastSlewRate(fastSlewRate)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::HalfBridgeBitMask enabledOpenLoadDetect{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetEnabledOpenLoadDetect(enabledOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }
        enabledOpenLoadDetect = m_Config.EnableOpenLoadDetection
            ? enabledOpenLoadDetect | m_HalfBridges
            : enabledOpenLoadDetect & ~m_HalfBridges;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetEnabledOpenLoadDetect(enabledOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::HalfBridgeBitMask lowCurrentOpenLoadDetect{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetEnabledLowCurrentOpenLoadDetect(lowCurrentOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }
        lowCurrentOpenLoadDetect = m_Config.EnableLowCurrentOpenLoadDetection
            ? lowCurrentOpenLoadDetect | m_HalfBridges
            : lowCurrentOpenLoadDetect & ~m_HalfBridges;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.EnableLowCurrentOpenLoadDetect(lowCurrentOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::HalfBridgeBitMask passiveOpenLoadDetect{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetEnabledPassiveOpenLoadDetect(passiveOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }
        passiveOpenLoadDetect = passiveOpenLoadDetect & ~m_HalfBridges;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.EnablePassiveOpenLoadDetect(passiveOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }

        PiSubmarine::Drv8908::HalfBridgeBitMask passiveVmOpenLoadDetect{};
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.GetEnabledPassiveVmOpenLoadDetect(passiveVmOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }
        passiveVmOpenLoadDetect = passiveVmOpenLoadDetect & ~m_HalfBridges;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.EnablePassiveVmOpenLoadDetect(passiveVmOpenLoadDetect)))
        {
            return MakeCommunicationError();
        }

        const auto highSideEnabled = m_SwitchSide == SwitchSide::High;
        const auto lowSideEnabled = m_SwitchSide == SwitchSide::Low;
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetHalfBridgeEnabled(m_HalfBridges, highSideEnabled, lowSideEnabled)))
        {
            return MakeCommunicationError();
        }

        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetPwmMap(m_HalfBridges, m_PwmGenerator)))
        {
            return MakeCommunicationError();
        }

        return {};
    }

    Result<void> Controller::ApplyDutyCycle(const NormalizedFraction intensity)
    {
        const auto dutyCycle = NormalizedIntFraction<8>(static_cast<double>(intensity));
        if (!PiSubmarine::Drv8908::IsValid(m_Chip.SetDutyCycle(m_PwmGenerator, dutyCycle)))
        {
            return MakeCommunicationError();
        }

        return {};
    }

    Result<void> Controller::EnsurePoweredAndConfigured()
    {
        if (m_HalfBridges == static_cast<PiSubmarine::Drv8908::HalfBridgeBitMask>(0))
        {
            return MakeContractError();
        }

        if (m_Config.EnableLowCurrentOpenLoadDetection && m_SwitchSide != SwitchSide::Low)
        {
            return MakeContractError();
        }

        if (m_PowerLease.IsValid())
        {
            return {};
        }

        m_PowerLease = m_PowerManager.Acquire();
        if (!m_PowerLease.IsValid())
        {
            return MakeDeviceError();
        }

        if (const auto configureResult = ConfigureChip(); !configureResult.has_value())
        {
            m_PowerManager.Release(m_PowerLease);
            return configureResult;
        }

        return {};
    }

    Result<void> Controller::DisableOutput()
    {
        if (!m_PowerLease.IsValid())
        {
            return {};
        }

        const auto setDutyResult = ApplyDutyCycle(NormalizedFraction(0));
        m_PowerManager.Release(m_PowerLease);
        if (!setDutyResult.has_value())
        {
            return setDutyResult;
        }

        return {};
    }
}
