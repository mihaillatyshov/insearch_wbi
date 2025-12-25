import hashlib
import os
import shutil
import traceback
from typing import Callable

import paramiko
import pydantic
from base import ArgsBase, parse_args_new, print_to_cpp
from PIL import Image, ImageChops
import pandas as pd


class Args(ArgsBase):
    xlsx_path: str = pydantic.Field(description="Папка исходных файлов")
    xlsx_save_path: str = pydantic.Field(description="Папка для сохранения")
    # img_tmp_path: str = pydantic.Field(description="Папка для сохранения; Можно использовать ';' для нескольких папок")
    ssh_host: str = pydantic.Field(default="", description="SSH хост")
    ssh_user: str = pydantic.Field(description="SSH пользователь")
    ssh_password: str = pydantic.Field(description="SSH пароль")
    ssh_port: int = pydantic.Field(default=22, description="SSH порт")
    server_img_path: str = pydantic.Field(description="Путь на сервере для сохранения изображений")


class SshOrLocalConnection:
    def __init__(self, host: str | None, user: str, password: str, port: int = 22):
        self.host = host
        self.user = user
        self.password = password
        self.port = port
        self.ssh = None
        self.sftp = None

    def __enter__(self):
        if self.host == "" or self.host is None:
            return None, None
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


def sftp_files_content_equal(local_path: str, remote_path: str, sftp_client: paramiko.SFTPClient) -> bool:
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


def local_files_content_equal(local_path: str, destination_path: str) -> bool:
    """Сравнивает содержимое двух локальных файлов"""
    try:
        local_size = os.path.getsize(local_path)
        destination_size = os.path.getsize(destination_path)

        if local_size != destination_size:
            return False

        with open(local_path, "rb") as local_f:
            with open(destination_path, "rb") as destination_f:
                while True:
                    local_chunk = local_f.read(4096)
                    destination_chunk = destination_f.read(4096)

                    if local_chunk != destination_chunk:
                        return False

                    if not local_chunk:
                        break

        return True
    except Exception:                                                                                                   # pylint: disable=broad-exception-caught
        return False


def sftp_mkdir_p(remote_directory: str, sftp_client: paramiko.SFTPClient):
    """Создает директорию на сервере, включая все промежуточные директории (аналог mkdir -p)"""
    if remote_directory == '/':
        return
    if remote_directory == '':
        return

    try:
        sftp_client.stat(remote_directory)
    except IOError:
        # Директория не существует, создаем её
        dirname, basename = os.path.split(remote_directory.rstrip('/'))
        if dirname:
            sftp_mkdir_p(dirname, sftp_client)
        if basename:
            try:
                sftp_client.mkdir(remote_directory)
            except IOError:
                # Директория уже существует (race condition)
                pass


def sftp_upload_file(local_file, remote_file, sftp_client: paramiko.SFTPClient):
    # Создаем все необходимые директории перед загрузкой
    remote_dir = os.path.dirname(remote_file)
    if remote_dir:
        sftp_mkdir_p(remote_dir, sftp_client)

    print_to_cpp(f"[sftp] {local_file} -> {remote_file}")
    sftp_client.put(local_file, remote_file)


def local_copy_file(local_file, destination_file):
    """Копирует файл локально, создавая все необходимые директории"""
    # Создаем все необходимые директории перед копированием
    destination_dir = os.path.dirname(destination_file)
    if destination_dir:
        os.makedirs(destination_dir, exist_ok=True)

    print_to_cpp(f"[local] {local_file} -> {destination_file}")
    shutil.copyfile(local_file, destination_file)


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


def sha256_org_not_exists(
    fname: str,
    server_img_path: str,
    is_file_exists_cb: Callable[[str, str], bool],
    is_files_equal_cb: Callable[[str, str], bool],
):
    iteration = 0
    while True:
        path, pathname, ext = sha256_org(fname, iteration)

        is_file_exists = is_file_exists_cb(server_img_path, pathname)
        print_to_cpp(
            f"Проверка существования файла на сервере: {pathname} - {'найден' if is_file_exists else 'не найден'}")
        is_files_equal = is_files_equal_cb(fname, os.path.join(server_img_path, pathname))
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
    os.makedirs(args.xlsx_save_path, exist_ok=True)

    is_local = args.ssh_host == "" or args.ssh_host is None

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
    with SshOrLocalConnection(args.ssh_host, args.ssh_user, args.ssh_password,
                              args.ssh_port) as (ssh_client, sftp_client):
        print_to_cpp("Создание карты замены изображений и загрузка на сервер")

        is_file_exists_cb = (lambda remote_base_path, pathname: file_exists_on_server(
            remote_base_path, pathname, sftp_client)) if not is_local else (
                lambda remote_base_path, pathname: os.path.isfile(os.path.join(remote_base_path, pathname)))
        is_files_equal_cb = (lambda local_path, remote_path: sftp_files_content_equal(
            local_path, remote_path, sftp_client)) if not is_local else local_files_content_equal

        for img_name in imgs_set:
            if not os.path.isfile(img_name):
                raise RuntimeError(f"Файл изображения не найден: {img_name}")
            server_pathname = sha256_org_not_exists(img_name, args.server_img_path, is_file_exists_cb,
                                                    is_files_equal_cb)
            imgs_replace_map[img_name] = server_pathname
            if is_local:
                local_copy_file(img_name, os.path.join(args.server_img_path, server_pathname))
            else:
                sftp_upload_file(img_name, os.path.join(args.server_img_path, server_pathname), sftp_client)

    print_to_cpp("Создание новых xlsx файлов с обновленными ссылками на изображения")
    for filename in os.scandir(args.xlsx_path):
        if filename.is_file() and not filename.name.startswith("~$"):
            print_to_cpp(f"Прочитан файл: {filename.name}")
            df = pd.read_excel(filename.path)
            for col in ['img_pic', 'img_drw']:
                if col in df.columns:
                    df[col] = df[col].map(lambda x: imgs_replace_map.get(x, x) if pd.notna(x) else x)
            save_path = os.path.join(args.xlsx_save_path, filename.name)

            writer = pd.ExcelWriter(save_path, engine="xlsxwriter")                                                     # pylint: disable=abstract-class-instantiated
            df.to_excel(writer, sheet_name="sm", freeze_panes=(1, 0), index=False)
            worksheet = writer.sheets["sm"]
            worksheet.autofit()
            writer.close()

            print_to_cpp(f"Сохранен файл: {save_path}")


try:
    process_files(parse_args_new(Args))
except KeyboardInterrupt:
    pass
except Exception as e:                                                                                                  # pylint: disable=broad-exception-caught
    formatted_traceback = traceback.format_exc()                                                                        # pylint: disable=invalid-name
    print_to_cpp(f"An error occurred:\n{formatted_traceback}")

# pylint: disable=pointless-string-statement
r"""
python ./assets/scripts/imgs_to_server_format_and_upload.py `
    --xlsx_path      "W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\xlsx_add_info" `
    --xlsx_save_path "W:\Work\WBI\ToolinformProjects\WBI_Stock_2\data\excel\for_server_import" `
    --ssh_user "latyshov" `
    --ssh_password "Rfcf,kfyrf1515" `
    --server_img_path "W:\Work\WBI\wbi_workpiece_static\pic_tools"
"""
