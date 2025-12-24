import hashlib
import os
import shutil
import traceback

import paramiko
import pydantic
from base import ArgsBase, parse_args_new, print_to_cpp
from PIL import Image, ImageChops
import pandas as pd


class Args(ArgsBase):
    xlsx_path: str = pydantic.Field(description="Папка исходных файлов")
    xlsx_save_path: str = pydantic.Field(description="Папка для сохранения")
    # img_tmp_path: str = pydantic.Field(description="Папка для сохранения; Можно использовать ';' для нескольких папок")
    ssh_host: str = pydantic.Field(description="SSH хост")
    ssh_user: str = pydantic.Field(description="SSH пользователь")
    ssh_password: str = pydantic.Field(description="SSH пароль")
    ssh_port: int = pydantic.Field(default=22, description="SSH порт")
    server_img_path: str = pydantic.Field(description="Путь на сервере для сохранения изображений")


class SSHConnection:
    def __init__(self, host: str, user: str, password: str, port: int = 22):
        self.host = host
        self.user = user
        self.password = password
        self.port = port
        self.ssh = None
        self.sftp = None

    def __enter__(self):
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.ssh.connect(hostname=self.host, username=self.user, password=self.password, port=self.port)
        self.sftp = self.ssh.open_sftp()
        return self.ssh, self.sftp

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.sftp:
            self.sftp.close()
        if self.ssh:
            self.ssh.close()


def file_exists_on_server(remote_base_path: str, pathname: str, sftp_client: paramiko.SFTPClient) -> bool:
    try:
        sftp_client.stat(os.path.join(remote_base_path, pathname))
        return True
    except IOError:
        return False


def files_content_equal(local_path: str, remote_path: str, sftp_client: paramiko.SFTPClient) -> bool:
    try:
        local_size = os.path.getsize(local_path)
        remote_size = sftp_client.stat(remote_path).st_size

        if local_size != remote_size:
            return False

        with open(local_path, "rb") as local_f:
            with sftp_client.file(remote_path, "r") as remote_f:
                while True:
                    local_chunk = local_f.read(4096)
                    remote_chunk = remote_f.read(4096)

                    if local_chunk != remote_chunk:
                        return False

                    if not local_chunk:
                        break

        return True
    except Exception:                                                                                                   # pylint: disable=broad-exception-caught
        return False


def sftp_upload_file(local_file, remote_file, sftp_client: paramiko.SFTPClient):
    print_to_cpp(f"[sftp] {local_file} -> {remote_file}")
    sftp_client.put(local_file, remote_file)


def sha256_org(fname: str, iteration: int = 0):
    file_extension = os.path.splitext(fname)[1].lower()
    hash_sha = hashlib.sha256()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_sha.update(chunk + str(iteration).encode())
    hashname = hash_sha.hexdigest()

    path = hashname[:2] + "/" + hashname[2:4] + "/"
    pathname = path + hashname[4:] + file_extension

    return path, pathname, file_extension


def sha256_org_not_exists(fname: str, server_img_path: str, sftp_client: paramiko.SFTPClient):
    iteration = 0
    while True:
        path, pathname, ext = sha256_org(fname, iteration)

        is_file_exists = file_exists_on_server(server_img_path, pathname, sftp_client)
        print_to_cpp(
            f"Проверка существования файла на сервере: {pathname} - {'найден' if is_file_exists else 'не найден'}")
        is_files_equal = files_content_equal(fname, os.path.join(server_img_path, pathname), sftp_client)
        if is_file_exists and not is_files_equal:
            print_to_cpp("Файл с таким именем существует, но содержимое отличается. Повторное хеширование...")
            iteration += 1
            continue

        # os.makedirs(os.path.join(img_tmp_path, path), exist_ok=True)
        # shutil.copyfile(fname, os.path.join(img_tmp_path, pathname))
        # sftp_upload_file(fname, os.path.join(server_img_path, pathname), sftp_client)
        return pathname


# 1. Берем все картинки из папок (создаем set для уникальности)
# 2. Преобразуем картинки в нужный формат и сохраняем во временную папку.
# 3. Создаем новый xlsx файл с нужным форматом и ссылками на сервере.
# 4. Загружаем картинки на сервер.


def process_files(args: Args):
    print_to_cpp("Начало обработки изображений")
    imgs_set: set[str] = set()
    imgs_replace_map: dict[str, str] = {}
    print_to_cpp(f"Создание списка изображений из папок: {args.xlsx_path}")
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file() and not filename.name.startswith("~$"):
            print_to_cpp(f"Прочитан файл: {filename.name}")
            # print_to_cpp(f"{filename.path} {filename.name}")
            df = pd.read_excel(filename.path)
            for col in ['img_pic', 'img_drw']:
                if col in df.columns:
                    imgs_set.update(df[col].replace('', pd.NA).dropna().astype(str))

    print_to_cpp(f"Всего уникальных изображений для обработки: {len(imgs_set)}")

    print_to_cpp("Подключение к SSH серверу для загрузки изображений")
    with SSHConnection(args.ssh_host, args.ssh_user, args.ssh_password, args.ssh_port) as (ssh_client, sftp_client):
        print_to_cpp("Создание карты замены изображений и загрузка на сервер")
        for img_name in imgs_set:
            if not os.path.isfile(img_name):
                raise RuntimeError(f"Файл изображения не найден: {img_name}")
            server_pathname = sha256_org_not_exists(img_name, args.server_img_path, sftp_client)
            imgs_replace_map[img_name] = server_pathname
            sftp_upload_file(img_name, os.path.join(args.server_img_path, server_pathname), sftp_client)


try:
    process_files(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:                                                                                                  # pylint: disable=broad-exception-caught
    formatted_traceback = traceback.format_exc()                                                                        # pylint: disable=invalid-name
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")
r"""
python ./assets/scripts/imgs_to_server_format_and_upload.py `
    --xlsx_path "W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\xlsx_add_info"
    --xlsx_save_path "W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\for_server_import" `
    --ssh_host "192.168.111.115" `
    --ssh_user "tollboss" `
    --ssh_password "Big-ZAVOD-root5" `
    --ssh_port
    --server_img_path
    
    W:\Work\WBI\insearch_wbi\assets\scripts\test_imgs_to_server_format_and_upload\xlsx_input `
    
    
    W:\Work\WBI\insearch_wbi\assets\scripts\test_imgs_to_server_format_and_upload\xlsx_output `
    
    
    W:\Work\WBI\insearch_wbi\assets\scripts\test_imgs_to_server_format_and_upload\tmp_imgs `
    
"""
