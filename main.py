import os
import sys
import asyncio

import qasync
from loguru import logger
from PySide6.QtWidgets import QApplication
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

    try:
        # 创建Qt应用
        app = QApplication(sys.argv)

        # 设置异步事件循环
        loop = qasync.QEventLoop(app)
        asyncio.set_event_loop(loop)

        window = MainWindow()
        window.show()

        # 运行事件循环
        with loop:
            loop.run_forever()

    except KeyboardInterrupt:
        logger.info("Program  terminated by user")
    # except Exception as e:
    #     logger.critical(f"Unknown  error: {e}")
    #     sys.exit(1)


if __name__ == "__main__":
    qasync.run(main())
