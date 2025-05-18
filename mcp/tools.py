from pathlib import Path
import os
from loguru import logger


BASE_DIR = Path("E:/学习资料/作业/人工智能实训/PY/ShallowSeek/test")

def read_file(file_path: str) -> str:
    """Return file content. If not exist, return error message.
    """
    logger.info(f"call read_file({file_path})")
    try:
        with open(file_path, "r") as f:
            content = f.read()
        return content
    except Exception as e:
        return f"An error occurred: {e}"


def list_files() -> list[str]:
    logger.info(f"call  list_files()")
    file_list = []
    for item in BASE_DIR.rglob("*"):
        if item.is_file():
            file_list.append(str(item.absolute()))
    logger.info(f"   return {file_list}")
    return file_list


def rename_file(name: str, new_name: str) -> str:
    logger.info(f"call rename_file({name}, {new_name})")
    try:
        new_path = BASE_DIR / new_name
        if not str(new_path).startswith(str(BASE_DIR)):
            return "Error: new_name is outside BASE_DIR."

        os.makedirs(new_path.parent, exist_ok=True)
        os.rename(BASE_DIR / name, new_path)
        return f"File '{name}' successfully renamed to '{new_name}'."
    except Exception as e:
        return f"An error occurred: {e}"


if __name__ == "__main__":
    list_files()
