include("${PISUBMARINE_BUILD_SYSTEM}")

if (NOT UNIX)
    message("Skipping ${PISUBMARINE_MODULE_NAME}.App: Lamp.Drv8908 app is only available for Linux targets.")
    return()
endif()

project(${PISUBMARINE_MODULE_NAME}.App LANGUAGES CXX)

message("Adding executable: ${PISUBMARINE_MODULE_NAME}.App")
add_executable(${PISUBMARINE_MODULE_NAME}.App PiSubmarine/Lamp/Drv8908/main.cpp)
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME}.App)

target_include_directories(${PISUBMARINE_MODULE_NAME}.App PRIVATE "./")
target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE ${PISUBMARINE_MODULE_NAME})

PiSubmarineAddDependency("https://github.com/PiSubmarine/SPI.Linux.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/GPIO.Linux.git" "")

find_package(Boost CONFIG REQUIRED COMPONENTS program_options)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE
        PiSubmarine.SPI.Linux
        PiSubmarine.GPIO.Linux
        Boost::program_options
        spdlog::spdlog)
