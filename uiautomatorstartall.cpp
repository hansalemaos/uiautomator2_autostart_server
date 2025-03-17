/**
 * @brief Demonstrates connecting to multiple Android devices via ADB, then running a UIAutomator2 test server on each.
 *
 * This program uses a provided ADB executable path (argv[1]), a sleep time in milliseconds (argv[2]), and one or more
 * device addresses joined by '/' (argv[3]) to:
 *   1. Continuously monitor device states.
 *   2. Attempt to connect each device using "adb connect <device>".
 *   3. Launch the UIAutomator instrumentation test on each connected device.
 *
 * ### Usage:
 * @code
 *   ./main <adb_path> <sleep_time_in_ms> <device1/device2/device3...>
 * @endcode
 *
 * **Example**:
 * @code
 *   ./main adb.exe 5000 127.0.0.1:5555/127.0.0.1:5570
 * @endcode
 *
 * ### Explanation of Key Components:
 * - **sleepcp(int milliseconds)**:
 *   - Uses `Sleep()` on Windows or `usleep()` on non-Windows systems to pause execution.
 *
 * - **strip_spaces_inplace(std::string &s)**:
 *   - Removes leading and trailing whitespace in a given string.
 *
 * - **get_devices(std::string&& adb_devices_output)**:
 *   - Splits a single string of device addresses, separated by '/', into a vector of DeviceInfo objects.
 *
 * - **systemthread(DeviceInfo &stru)**:
 *   - Called in a background thread. Connects to the device, then starts the UIAutomator instrumentation server.
 *
 * - **main(int argc, char *argv[])**:
 *   - Parses arguments for the ADB path, sleep time, and the device addresses.
 *   - Sets up an infinite loop, periodically checking each device's status. If the device is free (`done == true`),
 *     it spawns a new thread to connect to and run the server on that device.
 */

#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <atomic>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

std::string adb_exe{};

void static sleepcp(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif // _WIN32
}

void constexpr static lstrip_spaces_inplace(std::string &s)
{
    if (s.size() == 0)
    {
        return;
    }
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

void constexpr static rstrip_spaces_inplace(std::string &s)
{
    if (s.size() == 0)
    {
        return;
    }
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}
void constexpr static strip_spaces_inplace(std::string &s)
{
    if (s.size() == 0)
    {
        return;
    }
    lstrip_spaces_inplace(s);
    rstrip_spaces_inplace(s);
}
struct DeviceInfo
{
    std::string name;
    std::unique_ptr<std::atomic<bool>> done;
};
void static systemthread(DeviceInfo &stru)
{
    *(stru.done) = false;
    std::string connect_cmd{adb_exe + " connect " + stru.name};
    std::string run_server{adb_exe + " -s " + stru.name +
                           " shell am instrument -w -r -e debug false -e class com.github.uiautomator.stub.Stub "
                           "com.github.uiautomator.test/androidx.test.runner.AndroidJUnitRunner"};
    system(connect_cmd.c_str());
    system(run_server.c_str());
    *(stru.done) = true;
}

std::vector<DeviceInfo> static get_devices(std::string &&adb_devices_output)
{
    std::vector<DeviceInfo> results;
    auto strs{adb_devices_output | std::views::split('/')};
    for (auto &&st : strs)
    {
        std::string s{st.begin(), st.end()};
        strip_spaces_inplace(s);
        if (s.size() > 0)
        {
            results.emplace_back(DeviceInfo{
                s,
                std::make_unique<std::atomic<bool>>(true),
            });
        }
    }
    return results;
}
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << "adbpath sleeptime ADBDEVICE/ADBDEVICE1/ADBDEVICE2 ...\n";
        std::cerr << "Example: " << argv[0]
                  << "adb.exe 5000 127.0.0.1:5555/127.0.0.1:5570/127.0.0.1:5585/127.0.0.1:5590/127.0.0.1:5595\n";
        exit(EXIT_FAILURE);
    }
    int sleep_time{::atoi(argv[2])};
    adb_exe.append(argv[1]);
    std::vector<DeviceInfo> devices{get_devices(std::string{argv[3]})};

    std::cout << "Devices:\n";
    std::vector<std::thread> allthreads;
    while (1)
    {
        for (auto &d : devices)
        {
            std::cout << d.name << '\t';
            std::cout << *d.done << '\n';
            if (*d.done)
            {
                allthreads.emplace_back(std::thread{systemthread, std::ref(d)});
            }
        }
        sleepcp(sleep_time);
    }
}
