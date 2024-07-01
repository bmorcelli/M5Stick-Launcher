import os

Import("env")

def before_upload(source, target, env):
    bin_path = os.path.join(env['PROJECT_DIR'], 'support_files', 'UiFlow2_nvs.bin')
    env.Append(UPLOADERFLAGS=[0x9000, bin_path])

env.AddPreAction("upload", before_upload)
