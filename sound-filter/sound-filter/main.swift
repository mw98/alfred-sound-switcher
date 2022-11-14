import Foundation
import SimplyCoreAudio

// Script filter JSON model
struct Result: Encodable {
    var items: Array<Item>
    struct Item: Encodable {
        var title: String
        var subtitle: String
        var arg: String
        var valid: Bool
        var icon: Icon
        var variables: Variables
        struct Icon: Encodable {
            var path: String
        }
        struct Variables: Encodable {
            var type: String
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

// Construct script filter JSON
let simplyCA = SimplyCoreAudio()
var item: Result.Item
var validity: Bool
var iconPath: String
var subtitle: String

var items: Array<Result.Item> = []

for device in simplyCA.allOutputDevices {
    let deviceName: String = device.name
    if !blocklistArr.contains(deviceName.lowercased()) {
        if device.isDefaultOutputDevice {
            subtitle = "Current Output Device"
            validity = false
            iconPath = "output_selected.png"
        } else {
            subtitle = "Press ↩ to select"
            validity = true
            iconPath = "output.png"
        }
        item = Result.Item(title: deviceName, subtitle: subtitle, arg: deviceName, valid: validity, icon: Result.Item.Icon(path: iconPath), variables: Result.Item.Variables(type: "output"))
        items.append(item)
    }
}

for device in simplyCA.allInputDevices {
    let deviceName: String = device.name
    if !blocklistArr.contains(deviceName.lowercased()) {
        if device.isDefaultInputDevice {
            subtitle = "Current Input Device"
            validity = false
            iconPath = "input_selected.png"
        } else {
            subtitle = "Press ↩ to select"
            validity = true
            iconPath = "input.png"
        }
        item = Result.Item(title: deviceName, subtitle: subtitle, arg: deviceName, valid: validity, icon: Result.Item.Icon(path: iconPath), variables: Result.Item.Variables(type: "input"))
        items.append(item)
    }
}

// Encode script filter JSON
let encoder = JSONEncoder()
do {
    let result = try encoder.encode(Result(items: items))
    print(String(data: result, encoding: .utf8)!)
} catch {
    print(error.localizedDescription)
}
