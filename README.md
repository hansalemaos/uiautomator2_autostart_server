# Runs Uiautomator2 on multiple ADB devices, and checks periodically checking each device's status

Demonstrates connecting to multiple Android devices via ADB, then running a [UIAutomator test server](https://github.com/hansalemaos/uiautomator2tocsv) on each.

This program uses a provided ADB executable path (`argv[1]`), a sleep time in milliseconds (`argv[2]`), and one or more device addresses joined by `/` (`argv[3]`) to:

1. Continuously monitor device states.
2. Attempt to connect each device using `adb connect <device>`.
3. Launch the UIAutomator instrumentation test on each connected device.

## Usage

```bash
uiautomatorstartall.exe <adb_path> <sleep_time_in_ms> <device1/device2/device3...>
```

**Example**:

```bash
uiautomatorstartall.exe adb.exe 5000 127.0.0.1:5555/127.0.0.1:5570
```

## Explanation of Key Components

- **`sleepcp(int milliseconds)`**:
  Uses `Sleep()` on Windows or `usleep()` on non-Windows systems to pause execution.

- **`strip_spaces_inplace(std::string &s)`**:
  Removes leading and trailing whitespace in a given string.

- **`get_devices(std::string&& adb_devices_output)`**:
  Splits a single string of device addresses, separated by `/`, into a vector of `DeviceInfo` objects.

- **`systemthread(DeviceInfo &stru)`**:
  Called in a background thread. Connects to the device, then starts the UIAutomator instrumentation server.

- **`main(int argc, char *argv[])`**:
  - Parses arguments for the ADB path, sleep time, and the device addresses.
  - Sets up an infinite loop, periodically checking each device's status. If the device is free (`done == true`),
    it spawns a new thread to connect to and run the server on that device.

## Compilation
```
zig c++ -std=c++2a -O3 -g0 uiautomatorstartall.cpp
```
