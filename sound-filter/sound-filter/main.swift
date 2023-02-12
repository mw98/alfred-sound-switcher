import Foundation
import SimplyCoreAudio

// Script filter JSON model
struct ScriptFilter: Encodable {
    let items: Array<Item>
    struct Item: Encodable {
        let title: String
        let subtitle: String
        let arg: String
        let valid: Bool
        let icon: Icon
        let variables: Variables
        struct Icon: Encodable {
            let path: String
        }
        struct Variables: Encodable {
            let type: String
        }
    }
}

// Get blocklist
let blocklistStr: String = ProcessInfo.processInfo.environment["BLOCKLIST"]!
var blocklistArr: Array<String>
if blocklistStr == "" {
    blocklistArr = []
} else {
    blocklistArr = blocklistStr.components(separatedBy: "\n")
}

// Detect appearance
let appearance: String = UserDefaults().string(forKey: "AppleInterfaceStyle") ?? "Light"
let iconDirectory: String = "assets/\(appearance)"

// Construct script filter JSON
let simplyCA = SimplyCoreAudio()
var subtitle: String
var validity: Bool
var icon: ScriptFilter.Item.Icon
var items: Array<ScriptFilter.Item> = []

for device in simplyCA.allOutputDevices {
    let deviceName: String = device.name
    let deviceTransportType: String = device.transportType!.rawValue
    if !blocklistArr.contains(deviceName.lowercased()) {
        if device.isDefaultOutputDevice {
            subtitle = "Selected · \(deviceTransportType)"
            validity = false
            icon = ScriptFilter.Item.Icon(path: "\(iconDirectory)/output_selected.png")
        } else {
            subtitle = "Not Selected · \(deviceTransportType)"
            validity = true
            icon = ScriptFilter.Item.Icon(path: "\(iconDirectory)/output.png")
        }
        items.append(ScriptFilter.Item(title: deviceName, subtitle: subtitle, arg: deviceName, valid: validity, icon: icon, variables: ScriptFilter.Item.Variables(type: "output")))
    }
}

for device in simplyCA.allInputDevices {
    let deviceName: String = device.name
    let deviceTransportType: String = device.transportType!.rawValue
    if !blocklistArr.contains(deviceName.lowercased()) {
        if device.isDefaultInputDevice {
            subtitle = "Selected · \(deviceTransportType)"
            validity = false
            icon = ScriptFilter.Item.Icon(path: "\(iconDirectory)/input_selected.png")
        } else {
            subtitle = "Not Selected · \(deviceTransportType)"
            validity = true
            icon = ScriptFilter.Item.Icon(path: "\(iconDirectory)/input.png")
        }
        items.append(ScriptFilter.Item(title: deviceName, subtitle: subtitle, arg: deviceName, valid: validity, icon: icon, variables: ScriptFilter.Item.Variables(type: "input")))
    }
}

// Encode script filter JSON
let encoder = JSONEncoder()
do {
    let result = try encoder.encode(ScriptFilter(items: items))
    print(String(data: result, encoding: .utf8)!)
} catch {
    print(error.localizedDescription)
}
