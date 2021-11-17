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
