#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char * argv[]) {
        
    std::string query;
    if (argc == 2) {
        query = argv[1];
    } else {
        query = "";
    }
    
    std::ifstream blocklist;
    std::string entry;
    
    std::string result { "{\"items\":[{\"title\":\"Add '" };
    result.append(query);
    result.append("' to blocklist\",\"arg\":\"");
    result.append(query);
    result.append("\"},");
    
    blocklist.open("blocklist.txt");
    while (std::getline(blocklist, entry)) {
        if (entry.find(query) != std::string::npos) {
            result.append("{\"title\":\"Remove '");
            result.append(entry);
            result.append("' from blocklist\",\"arg\":\"");
            result.append(entry);
            result.append("\"},");
        }
    }
    blocklist.close();
    
    result.append("]}");
    std::cout << result << '\n';
    return 0;
}