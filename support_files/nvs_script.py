import os

Import("env")

def before_upload(source, target, env):
    bin_path = os.path.join(env['PROJECT_DIR'], 'support_files', 'UiFlow2_nvs.bin')
    env.Append(UPLOADERFLAGS=[0x9000, bin_path])

def after_upload(source, target, env):
    bin_path2 = os.path.join(env['PROJECT_DIR'], 'support_files', 'UiFlow1_phi.bin')
    command = (
        f'esptool.py --chip esp32 --port {env.subst("$UPLOAD_PORT")} --baud {env.subst("$UPLOAD_SPEED")} '
        f'write_flash 0xf000 {bin_path2}'
    )
    os.system(command)

env.AddPreAction("upload", before_upload)
#env.AddPostAction("upload", after_upload)