

# # download https://github.com/godotengine/godot/releases/download/3.2.2-stable/Godot_v3.2.2-stable_export_templates.tpz to ./bin/original_templates/

# VERSION = "3.2.2-stable"
# URL = "https://github.com/godotengine/godot/releases/download/{}/Godot_v3.2.2-stable_export_templates.tpz"
# # download

# import requests
# import os
# import shutil   
# import zipfile
# import sys
# import time
# from tqdm import tqdm

# def await_download(url: str, version: str):
#     response = requests.get(url, stream=True)
#     # print(response.content)
#     total_size_in_bytes= int(response.headers.get('content-length', 0))
#     block_size = 1024
#     progress_bar = tqdm(total=total_size_in_bytes, unit='iB', unit_scale=True)
#     # create directory if not exists
#     if not os.path.exists("./bin/original_templates"):
#         os.makedirs("./bin/original_templates")

#     with open(f"./bin/original_templates/Godot_v{version}_export_templates.tpz", 'wb') as file:
#         for data in response.iter_content(block_size):
#             progress_bar.update(len(data))
#             file.write(data)
#     progress_bar.close()
#     if total_size_in_bytes != 0 and progress_bar.n != total_size_in_bytes:
#         print("ERROR, something went wrong")

# def download_godot_templates(version: str):
#     url = URL.format(version)
#     print(f"Downloading templates for Godot {version}")
#     print(f"URL: {url}")
#     await_download(url, version)

#     print("Templates downloaded")
#     with zipfile.ZipFile(f"./bin/original_templates/Godot_v{version}_export_templates.tpz", 'r') as zip_ref:
#         zip_ref.extractall(f"./bin/original_templates/Godot_v{version}_export_templates")


# def is_template_downloaded(version: str):
#     return os.path.exists(f"./bin/original_templates/Godot_v{version}_export_templates.tpz")


# if __name__ == "__main__":
#     if is_template_downloaded(VERSION):
#         print(f"Templates for Godot {VERSION} already downloaded")
#         sys.exit(0)
#     download_godot_templates(VERSION)
#     print("Done")


