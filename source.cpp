#include <string>
#include <regex>
#include <array>
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
    if (val == NULL) {
        result = fallback;
    } else {
        result = val;
    }
    return result;
}

int main() {
    
    /* Get SwitchAudioSource path from env var */
    string cmd = get_env_var("CMDPATH", "/usr/local/bin/SwitchAudioSource");
    string tmp = cmd + " -af json";
    const char * json_cmd = tmp.c_str();
    cmd.append(" -cf cli -t ");
    string tmp1 = cmd + "input";
    const char * current_input_cmd = tmp1.c_str();
    string tmp2 = cmd + "output";
    const char * current_output_cmd = tmp2.c_str();
    
    /* Get currently selected device ids */
    string raw_current_output = exec(current_input_cmd);
    string raw_current_input = exec(current_output_cmd);
    smatch current_output_id;
    smatch current_input_id;
    regex rgx("[0-9]{2}");
    if (!regex_search(raw_current_output, current_output_id, rgx)) {
        cerr << "current output id not found" << '\n';
        exit(1);
    }
    if (!regex_search(raw_current_input, current_input_id, rgx)) {
        cerr << "current input id not found" << '\n';
        exit(1);
    }
    set<string> current_devices = {current_output_id[0], current_input_id[0]};
    
    /* Parse SwitchAudioSource JSON */
    string json = regex_replace(exec(json_cmd), regex("\n"), ",");
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
    picojson::object result;
    picojson::array items;
    const picojson::value::array& devices = v.get<picojson::array>();
    for (picojson::value::array::const_reverse_iterator i = devices.rbegin(); i != devices.rend(); ++i) {
        
        const picojson::value::object& device = (*i).get<picojson::object>();
        string name = device.at("name").get<string>();
        string nozoom = get_env_var("NOZOOM");        
        if (nozoom == "true" && name == "ZoomAudioDevice") {
            continue;
        }
        string id = device.at("id").get<string>();
        string type = device.at("type").get<string>();
        picojson::object item;
        picojson::object variables;
        picojson::object icon;
        
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
