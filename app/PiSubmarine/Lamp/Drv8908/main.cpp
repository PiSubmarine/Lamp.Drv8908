#include <array>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <memory>
#include <poll.h>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "PiSubmarine/Drv8908/Device.h"
#include "PiSubmarine/Drv8908/PowerManager.h"
#include "PiSubmarine/Error/Api/Error.h"
#include "PiSubmarine/GPIO/Linux/Driver.h"
#include "PiSubmarine/Lamp/Drv8908/Controller.h"
#include "PiSubmarine/SPI/Linux/Driver.h"

namespace PiSubmarine::Lamp::Drv8908
{
    volatile std::sig_atomic_t g_StopRequested = 0;

    constexpr std::string_view DefaultSpiDevice = "/dev/spidev0.1";
    constexpr std::string_view DefaultGpioChip = "/dev/gpiochip0";
    constexpr std::size_t DefaultNSleepPin = 16;
    constexpr std::size_t DefaultNFaultPin = 20;
    constexpr std::uint32_t DefaultSpiSpeed = 5'000'000;
    constexpr auto DefaultTelemetryPeriod = std::chrono::milliseconds(1000);

    void HandleSignal(int)
    {
        g_StopRequested = 1;
    }

    [[nodiscard]] std::shared_ptr<spdlog::logger> CreateLogger()
    {
        auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("PiSubmarine.Lamp.Drv8908.App", std::move(sink));
        logger->set_pattern("[%n] %^[%l]%$ %v");
        return logger;
    }

    [[nodiscard]] std::string ToString(const PiSubmarine::Error::Api::Error& error)
    {
        std::string message;
        switch (error.Condition)
        {
        case PiSubmarine::Error::Api::ErrorCondition::ContractError:
            message = "Requested lamp operation violates the API contract.";
            break;

        case PiSubmarine::Error::Api::ErrorCondition::CommunicationError:
            message = "Failed to communicate with the DRV8908.";
            break;

        case PiSubmarine::Error::Api::ErrorCondition::DeviceError:
            message = "DRV8908 rejected or could not apply the requested lamp state.";
            break;

        case PiSubmarine::Error::Api::ErrorCondition::UnknownError:
            message = "Unknown lamp error.";
            break;
        }

        if (error.HasCause())
        {
            message += " Cause: " + error.Cause.message();
        }

        return message;
    }

    [[nodiscard]] bool IsValidIntensity(const double intensity)
    {
        return intensity >= 0.0 && intensity <= 1.0;
    }

    [[nodiscard]] std::string SummarizeStatus(const Lamp::Telemetry::Api::Status& status)
    {
        std::ostringstream stream;
        stream << "intensity=" << status.Intensity
               << ", faults=["
               << (status.HasOpenLoadFault ? "open-load " : "")
               << (status.HasOvercurrentFault ? "overcurrent " : "")
               << (status.HasOvertemperatureShutdownFault ? "ot-shutdown " : "")
               << (status.HasUndervoltageFault ? "undervoltage " : "")
               << (status.HasOvervoltageFault ? "overvoltage " : "");

        if (!status.HasAnyFault())
        {
            stream << "none";
        }

        stream << "], warnings=[";
        if (status.HasOvertemperatureWarning)
        {
            stream << "ot-warning";
        }
        else
        {
            stream << "none";
        }
        stream << "]";

        return stream.str();
    }

    [[nodiscard]] bool WaitForShutdownRequest(const std::chrono::milliseconds timeout)
    {
        pollfd pollDescriptor{
            .fd = STDIN_FILENO,
            .events = static_cast<short>(POLLIN | POLLHUP | POLLERR),
            .revents = 0
        };

        const auto pollResult = ::poll(&pollDescriptor, 1, static_cast<int>(timeout.count()));
        if (pollResult < 0)
        {
            if (errno == EINTR)
            {
                return g_StopRequested != 0;
            }

            throw std::system_error(errno, std::generic_category(), "poll(stdin) failed");
        }

        if (pollResult == 0)
        {
            return g_StopRequested != 0;
        }

        if ((pollDescriptor.revents & (POLLHUP | POLLERR | POLLNVAL)) != 0)
        {
            return true;
        }

        if ((pollDescriptor.revents & POLLIN) == 0)
        {
            return false;
        }

        std::array<char, 64> buffer{};
        const auto bytesRead = ::read(STDIN_FILENO, buffer.data(), buffer.size());
        if (bytesRead == 0)
        {
            return true;
        }

        if (bytesRead < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return false;
            }

            throw std::system_error(errno, std::generic_category(), "read(stdin) failed");
        }

        return false;
    }

    void ReportLampTelemetry(
        spdlog::logger& logger,
        const std::string_view lampName,
        Lamp::Drv8908::Controller& controller)
    {
        const auto intensityResult = controller.GetIntensity();
        if (!intensityResult.has_value())
        {
            logger.error("[{}] Failed to read commanded intensity: {}", lampName, ToString(intensityResult.error()));
            return;
        }

        const auto statusResult = controller.GetStatus();
        if (!statusResult.has_value())
        {
            logger.error("[{}] Failed to read telemetry: {}", lampName, ToString(statusResult.error()));
            return;
        }

        logger.info(
            "[{}] intensity={:.3f}, {}",
            lampName,
            static_cast<double>(intensityResult.value()),
            SummarizeStatus(statusResult.value()));
    }

    [[nodiscard]] bool ApplyIntensity(
        Lamp::Drv8908::Controller& controller,
        const NormalizedFraction intensity,
        const std::string_view lampName,
        spdlog::logger& logger)
    {
        const auto result = controller.SetIntensity(intensity);
        if (result.has_value())
        {
            return true;
        }

        logger.error("[{}] Failed to set intensity: {}", lampName, ToString(result.error()));
        return false;
    }
}

int main(const int argc, const char* const argv[])
{
    namespace po = boost::program_options;
    auto logger = PiSubmarine::Lamp::Drv8908::CreateLogger();

    try
    {
        std::filesystem::path spiDevicePath = PiSubmarine::Lamp::Drv8908::DefaultSpiDevice;
        std::filesystem::path gpioChipPath = PiSubmarine::Lamp::Drv8908::DefaultGpioChip;
        std::size_t nSleepPin = PiSubmarine::Lamp::Drv8908::DefaultNSleepPin;
        std::size_t nFaultPin = PiSubmarine::Lamp::Drv8908::DefaultNFaultPin;
        int telemetryPeriodMilliseconds = static_cast<int>(PiSubmarine::Lamp::Drv8908::DefaultTelemetryPeriod.count());
        double hb5IntensityValue = 0.0;
        double hb6IntensityValue = 0.0;

        po::options_description options("PiSubmarine Lamp.Drv8908 test app options");
        options.add_options()
            ("help,h", "Show help")
            ("spi-device,s", po::value<std::filesystem::path>(&spiDevicePath)->default_value(spiDevicePath),
             "SPI device path for the lamps DRV8908, e.g. /dev/spidev0.1")
            ("gpio-chip,g", po::value<std::filesystem::path>(&gpioChipPath)->default_value(gpioChipPath),
             "GPIO chip path for the lamps DRV8908 control pins, e.g. /dev/gpiochip0")
            ("n-sleep-pin", po::value<std::size_t>(&nSleepPin)->default_value(nSleepPin),
             "GPIO line number connected to DRV8908 nSLEEP")
            ("n-fault-pin", po::value<std::size_t>(&nFaultPin)->default_value(nFaultPin),
             "GPIO line number connected to DRV8908 nFAULT")
            ("telemetry-period-ms,t", po::value<int>(&telemetryPeriodMilliseconds)->default_value(telemetryPeriodMilliseconds),
             "Telemetry print period in milliseconds")
            ("hb5-intensity", po::value<double>(&hb5IntensityValue),
             "Intensity for predefined lamp on HB-5 high-side in [0.0, 1.0]")
            ("hb6-intensity", po::value<double>(&hb6IntensityValue),
             "Intensity for predefined lamp on HB-6 high-side in [0.0, 1.0]");

        po::variables_map variables;
        po::store(po::parse_command_line(argc, argv, options), variables);

        if (variables.contains("help"))
        {
            std::ostringstream output;
            output << "Control two predefined lamps connected to DRV8908.\n"
                   << "HB-5 high-side and HB-6 high-side are available.\n\n"
                   << options;
            logger->info("{}", output.str());
            return EXIT_SUCCESS;
        }

        po::notify(variables);

        const bool hb5Selected = variables.contains("hb5-intensity");
        const bool hb6Selected = variables.contains("hb6-intensity");
        if (!hb5Selected && !hb6Selected)
        {
            logger->error("Select at least one lamp by specifying --hb5-intensity and/or --hb6-intensity.");
            return EXIT_FAILURE;
        }

        if ((hb5Selected && !PiSubmarine::Lamp::Drv8908::IsValidIntensity(hb5IntensityValue))
            || (hb6Selected && !PiSubmarine::Lamp::Drv8908::IsValidIntensity(hb6IntensityValue)))
        {
            logger->error("Lamp intensities must be within [0.0, 1.0].");
            return EXIT_FAILURE;
        }

        if (telemetryPeriodMilliseconds <= 0)
        {
            logger->error("--telemetry-period-ms must be a positive integer.");
            return EXIT_FAILURE;
        }

        std::signal(SIGINT, PiSubmarine::Lamp::Drv8908::HandleSignal);
        std::signal(SIGTERM, PiSubmarine::Lamp::Drv8908::HandleSignal);

        PiSubmarine::SPI::Linux::Driver spiDriver(
            spiDevicePath.string(),
            PiSubmarine::Lamp::Drv8908::DefaultSpiSpeed,
            8,
            SPI_MODE_1,
            SPI_MODE_1);
        PiSubmarine::GPIO::Linux::Driver gpioDriver("PiSubmarine.Lamp.Drv8908.App");
        auto pinGroup = gpioDriver.CreatePinGroup(
            "LampsAndBallast",
            gpioChipPath,
            {nSleepPin, nFaultPin});
        PiSubmarine::Drv8908::Device chip(spiDriver, *pinGroup);
        PiSubmarine::Drv8908::PowerManager powerManager(chip);

        PiSubmarine::Lamp::Drv8908::Controller hb5Lamp(
            chip,
            powerManager,
            PiSubmarine::Drv8908::PwmGenerator::PwmGenerator5,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge5,
            PiSubmarine::Lamp::Drv8908::SwitchSide::High);

        PiSubmarine::Lamp::Drv8908::Controller hb6Lamp(
            chip,
            powerManager,
            PiSubmarine::Drv8908::PwmGenerator::PwmGenerator6,
            PiSubmarine::Drv8908::HalfBridgeBitMask::HalfBridge6,
            PiSubmarine::Lamp::Drv8908::SwitchSide::High);

        if (hb5Selected
            && !PiSubmarine::Lamp::Drv8908::ApplyIntensity(
                hb5Lamp,
                PiSubmarine::NormalizedFraction(hb5IntensityValue),
                "HB-5 HS",
                *logger))
        {
            return EXIT_FAILURE;
        }

        if (hb6Selected
            && !PiSubmarine::Lamp::Drv8908::ApplyIntensity(
                hb6Lamp,
                PiSubmarine::NormalizedFraction(hb6IntensityValue),
                "HB-6 HS",
                *logger))
        {
            return EXIT_FAILURE;
        }

        logger->info("Lamp test app started. Press Ctrl+C or send EOF with Ctrl+D to stop.");

        const auto telemetryPeriod = std::chrono::milliseconds(telemetryPeriodMilliseconds);
        while (PiSubmarine::Lamp::Drv8908::g_StopRequested == 0)
        {
            PiSubmarine::Lamp::Drv8908::ReportLampTelemetry(*logger, "HB-5 HS", hb5Lamp);
            PiSubmarine::Lamp::Drv8908::ReportLampTelemetry(*logger, "HB-6 HS", hb6Lamp);

            if (PiSubmarine::Lamp::Drv8908::WaitForShutdownRequest(telemetryPeriod))
            {
                break;
            }
        }

        logger->info("Shutting lamps down.");
		[[maybe_unused]] auto result = PiSubmarine::Lamp::Drv8908::ApplyIntensity(hb5Lamp, PiSubmarine::NormalizedFraction(0.0), "HB-5 HS", *logger);
        result = PiSubmarine::Lamp::Drv8908::ApplyIntensity(hb6Lamp, PiSubmarine::NormalizedFraction(0.0), "HB-6 HS", *logger);
        return EXIT_SUCCESS;
    }
    catch (const po::error& exception)
    {
        logger->error("Argument error: {}", exception.what());
        return EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        logger->error("Unhandled exception: {}", exception.what());
        return EXIT_FAILURE;
    }
}
