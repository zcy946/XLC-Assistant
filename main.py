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


def main():
    # 初始化日志
    initLogger()

    try:
        app = QApplication(sys.argv)

        loop = qasync.QEventLoop(app)
        asyncio.set_event_loop(loop)

        # 创建主窗口
        window = MainWindow()
        window.show()

        logger.info("Application started successfully")

        # 运行应用
        try:
            with loop:
                loop.run_forever()
        except Exception as e:
            logger.error(f"Event loop error: {e}")
            raise

    except KeyboardInterrupt:
        logger.info("Program terminated by user")
    except Exception as e:
        logger.critical(f"Unknown error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
