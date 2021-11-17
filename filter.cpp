#include <string>
#include <regex>
#include <array>
#include <vector>
#include <set>
#include <cstdio>
#include <memory>
#include <iostream>
#include "picojson.h"

using namespace std;

/* Get output from subprocess */
string exec(const char * cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) { 
        cerr << "popen() failed!" << '\n';
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

/* Get env var */
string get_env_var(const char * varname, string fallback = "") {
    string result;
    char * val = getenv(varname);
    if (val == NULL || *val == '\0') {
        result = fallback;
    } else {
        result = val;
    }
    return result;
}

/* Update PATH env var */
int prepend_to_PATH(string path) {
    string current_PATH = get_env_var("PATH");
    string new_PATH = path+current_PATH;
    return setenv("PATH", new_PATH.c_str(), 1);
}

/* Regex search for substring */
vector<string> regex_match_substr(string input_str, regex pattern) {
    vector<string> results;
    for (
        sregex_iterator i = sregex_iterator(input_str.begin(), input_str.end(), pattern);
        i != sregex_iterator();
        ++i
    ) {
        smatch match = *i;
        string match_str = match.str();
        results.push_back(match_str);
    }
    if (results.empty()) {
        cerr << "no regex match; input: " << input_str << '\n';
        exit(1);
    }
    return results;
}

int main() {
    
    /* Construct SwitchAudioSource commands */
    prepend_to_PATH("/opt/homebrew/bin:/usr/local/bin:"); // update PATH
    string cmd = get_env_var("CMDPATH", "switchaudiosource"); // use user-specified path if set
    string get_json = cmd + " -af json";
    cmd.append(" -cf cli -t ");
    string get_current_input = cmd + "input";
    string get_current_output = cmd + "output";
    
    /* Get currently selected device ids */
    string raw_current_output = exec(get_current_input.c_str());
    string raw_current_input = exec(get_current_output.c_str());
    regex two_digits("[0-9]{2}");
    string current_output_id = regex_match_substr(raw_current_output, two_digits)[0];
    string current_input_id = regex_match_substr(raw_current_input, two_digits)[0];
    set<string> current_devices = {current_output_id, current_input_id};
    
    /* Parse SwitchAudioSource JSON */
    string json = regex_replace(exec(get_json.c_str()), regex("\n"), ",");
    json.pop_back();
    json.insert(0, "[");
    json.append("]");
    picojson::value v;
    string err = picojson::parse(v, json);
    if (!err.empty()) {
        cerr << err << '\n';
        exit(2);
    }
    if (!v.is<picojson::array>()) {
        cerr << "JSON is not an array" << '\n';
        exit(3);
    }
    
    /* Construct script filter JSON */
    string nozoom = get_env_var("NOZOOM"); 
    string airpods_battery = get_env_var("AIRPODS_BATTERY");
    picojson::object result;
    picojson::array items;
    const picojson::value::array& devices = v.get<picojson::array>();
    for (picojson::value::array::const_reverse_iterator i = devices.rbegin(); i != devices.rend(); ++i) {
        
        const picojson::value::object& device = (*i).get<picojson::object>();
        string name = device.at("name").get<string>();       
        if (nozoom == "true" && name == "ZoomAudioDevice") {
            continue;
        }
        string id = device.at("id").get<string>();
        string type = device.at("type").get<string>();
        picojson::object item;
        picojson::object variables;
        picojson::object icon;
        
        if (airpods_battery == "true") {
            string subtitle;
            if (name.find("AirPods") != string::npos) {
                if (subtitle.empty()) {
                    string raw_battery_data = exec("defaults read /Library/Preferences/com.apple.Bluetooth | grep 'BatteryPercentLeft\\|BatteryPercentRight\\|BatteryPercentCase'");
                    regex one_to_three_digits("[0-9]{1,3}");
                    vector<string> battery_data = regex_match_substr(raw_battery_data, one_to_three_digits);
                    subtitle = "         L: "+battery_data[1]+"%  R: "+battery_data[2]+"%  Case: "+battery_data[0]+'%';
                }
                item["subtitle"] = picojson::value(subtitle);
            }
        }
                
        if (current_devices.contains(id)) {
            item["title"] = picojson::value("\u25c9   "+name);
            variables["selected"] = picojson::value("true");
        } else {
            item["title"] = picojson::value("\u25cb   "+name);
            variables["selected"] = picojson::value("false");
        }
        
        variables["type"] = picojson::value(type);
        icon["path"] = picojson::value(type+".icns");
        item["arg"] = picojson::value(name);
        item["icon"] = picojson::value(icon);
        item["variables"] = picojson::value(variables);
        
        items.push_back(picojson::value(item));
    }
    
    result["items"] = picojson::value(items);
    cout << picojson::value(result) << '\n';
    
    return 0;
}
