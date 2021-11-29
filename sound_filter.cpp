#include <iostream>
#include <fstream>
#include <memory>
#include <cstdio>
#include <array>
#include <string>
#include <cctype>
#include <algorithm>
#include <set>
#include "picojson.h"

// Run subprocess and get output
std::string exec(const char * command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe { popen(command, "r"), pclose };
    if (!pipe) { 
        std::cerr << "popen() failed!" << '\n';
        exit(1);
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Get environment variable value
std::string get_env_var(const char * varname, std::string fallback = "") {
    std::string result;
    char * val = getenv(varname);
    if (val == NULL || *val == '\0') {
        result = fallback;
    } else {
        result = val;
    }
    return result;
}

int main() {
    
    // Construct SwitchAudioSource commands
    std::string base_command { get_env_var("CMDPATH", "switchaudiosource") }; // use user-specified path if set
    const std::string get_all_devices { base_command + " -af json" };
    const std::string get_current_output { base_command + " -cf cli -t output | cut -d, -f3 | tr -d '\n'" };
    const std::string get_current_input { base_command + " -cf cli -t input | cut -d, -f3 | tr -d '\n'" };
    
    // Get current device ids
    const std::string current_output_id { exec(get_current_output.c_str()) };
    const std::string current_input_id { exec(get_current_input.c_str()) };
    const std::set<std::string> current_device_ids { current_output_id, current_input_id };
    
    // Load blocklist
    std::ifstream file;
    std::string line;
    std::set<std::string> blocklist;
    file.open("blocklist.txt");
    while (std::getline(file, line)) {
        blocklist.insert(line);
    }
    file.close();
    
    // Construct script filter JSON
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe { popen(get_all_devices.c_str(), "r"), pclose };
    if (!pipe) {
        std::cerr << "popen() failed!" << '\n';
        exit(1);
    }
    
    std::string result { "{\"items\":[" };
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string device = buffer.data();
        picojson::value v;
        std::string err = picojson::parse(v, device);
        if (! err.empty()) {
            std::cerr << err << '\n';
            exit(2);
        }
        if (! v.is<picojson::object>()) {
            std::cerr << "JSON is not an object" << '\n';
            exit(3);
        }
        const picojson::value::object &device_json { v.get<picojson::object>() };
        const std::string device_name { device_json.at("name").get<std::string>() };
        std::string device_name_lowercase;
        
        std::transform(
            device_name.begin(), 
            device_name.end(), 
            std::back_inserter(device_name_lowercase), 
            [](unsigned char c){ return std::tolower(c); }
        );
        
        if (! blocklist.contains(device_name_lowercase)) {
            const std::string device_uid { device_json.at("uid").get<std::string>() };
            const std::string device_id { device_json.at("id").get<std::string>() };
            const std::string device_type { device_json.at("type").get<std::string>() };
            result += "{\"arg\":\"";
            result += device_name;
            result += "\",\"uid\":\"";
            result += device_uid;
            result += "\",\"title\":\"";
            if (current_device_ids.contains(device_id)) {
                result += "\u25c9   ";
                result += device_name;
                result += "\",\"variables\":{\"selected\":\"true\",\"type\":\"";
            } else {
                result += "\u25cb   ";
                result += device_name;
                result += "\",\"variables\":{\"selected\":\"false\",\"type\":\"";
            }
            result += device_type;
            result += "\"},\"icon\":{\"path\":\"";
            result += device_type;
            result += ".icns\"}},";
        }
    }
    result += "]}";
    
    std::cout << result << '\n';
    return 0;
}
