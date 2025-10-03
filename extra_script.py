Import("env")

def after_upload(source, target, env):
    print("Upload completed!")
    print("Device: ESP32 DevKitC V4")
    print("Firmware: WebBT")
    print("Features: WebUI + Bluetooth + Audio + FileSystem")

env.AddPostAction("upload", after_upload)
