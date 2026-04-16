#pragma once

#include "PiSubmarine/Drv8908/IDevice.h"

namespace PiSubmarine::Lamp::Drv8908
{
    struct Config
    {
        PiSubmarine::Drv8908::PwmFrequency PwmFrequency{PiSubmarine::Drv8908::PwmFrequency::Hz2000};
        PiSubmarine::Drv8908::OcpDeglitchTime OpenLoadDeglitchTime{PiSubmarine::Drv8908::OcpDeglitchTime::MicroSeconds60};
        bool EnableOpenLoadDetection{true};
        bool EnableLowCurrentOpenLoadDetection{false};
        bool KeepOutputEnabledOnOpenLoad{true};
        bool ReportOpenLoadOnFaultPin{true};
    };
}
