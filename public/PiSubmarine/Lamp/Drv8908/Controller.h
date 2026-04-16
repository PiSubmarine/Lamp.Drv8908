#pragma once

#include <stdexcept>

#include "PiSubmarine/Drv8908/HalfBridgeBitMask.h"
#include "PiSubmarine/Drv8908/IDevice.h"
#include "PiSubmarine/Drv8908/IPowerManager.h"
#include "PiSubmarine/Drv8908/PwmGenerator.h"
#include "PiSubmarine/Lamp/Api/IController.h"
#include "PiSubmarine/Lamp/Drv8908/Config.h"
#include "PiSubmarine/Lamp/Drv8908/SwitchSide.h"
#include "PiSubmarine/Lamp/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Lamp::Drv8908
{
    class Controller : public Lamp::Api::IController, public Lamp::Telemetry::Api::IProvider
    {
    public:
        Controller(
            PiSubmarine::Drv8908::IDevice& chip,
            PiSubmarine::Drv8908::IPowerManager& powerManager,
            PiSubmarine::Drv8908::PwmGenerator pwmGenerator,
            PiSubmarine::Drv8908::HalfBridgeBitMask halfBridges,
            SwitchSide switchSide,
            Config config = {}
        );

        Error::Api::Result<void> SetIntensity(NormalizedFraction intensity) override;
        [[nodiscard]] Error::Api::Result<NormalizedFraction> GetIntensity() const override;
        [[nodiscard]] Error::Api::Result<Lamp::Telemetry::Api::Status> GetStatus() const override;

    private:
        PiSubmarine::Drv8908::IDevice& m_Chip;
        PiSubmarine::Drv8908::IPowerManager& m_PowerManager;
        PiSubmarine::Drv8908::PowerLease m_PowerLease;
        PiSubmarine::Drv8908::PwmGenerator m_PwmGenerator;
        PiSubmarine::Drv8908::HalfBridgeBitMask m_HalfBridges;
        SwitchSide m_SwitchSide;
        Config m_Config;
        NormalizedFraction m_TargetIntensity{0};

        [[nodiscard]] Error::Api::Result<void> ConfigureChip();
        [[nodiscard]] Error::Api::Result<void> ApplyDutyCycle(NormalizedFraction intensity);
        [[nodiscard]] Error::Api::Result<void> EnsurePoweredAndConfigured();
        [[nodiscard]] Error::Api::Result<void> DisableOutput();
    };
}
