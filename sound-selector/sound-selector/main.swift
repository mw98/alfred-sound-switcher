import SimplyCoreAudio

let argv: Array<String> = CommandLine.arguments
let simplyCA = SimplyCoreAudio()
if argv[1] == "input" {
    let devices = simplyCA.allInputDevices
    for device in devices {
        if String(device.name) == argv[2] {
            device.isDefaultInputDevice = true
            break
        }
    }
} else if argv[1] == "output" {
    let devices = simplyCA.allOutputDevices
    for device in devices {
        if String(device.name) == argv[2] {
            device.isDefaultOutputDevice = true
            break
        }
    }
}
