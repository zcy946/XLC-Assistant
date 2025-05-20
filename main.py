import os
import sys
import asyncio

import qasync
from loguru import logger
from PySide6.QtWidgets import QApplication
from qasync import QEventLoop
from MainWindow import MainWindow


def initLogger() -> None:
    """初始化日志"""
    output_dir = "./logs"
    os.makedirs(output_dir, exist_ok=True)
    logger.add(
        os.path.join(output_dir, "XLC Assistant.log"),
        enqueue=True,  # 启用队列模式，防止多线程打印出错
        rotation="10 MB",  # 按10MB轮转日志
        retention=5  # 保留最近5个日志文件
    )


async def main():
    # 初始化日志
    initLogger()
    # 初始化QApplication
    app: QApplication = QApplication(sys.argv)
    # 使用 qasync 设置事件循环
    loop: QEventLoop = qasync.QEventLoop(app)
    asyncio.set_event_loop(loop)

    window = MainWindow()
    window.show()

    # 启动事件循环
    with loop:
        loop.run_forever()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("程序已由用户终止")
    # except Exception as e:
    #     logger.critical(f"错误: {e}")
    #     sys.exit(1)