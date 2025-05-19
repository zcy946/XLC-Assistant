import threading


class SingletonMeta(type):
    """单例元类，确保继承自 Singleton 的类都是单例"""
    _instances = {}  # 存储所有单例类的实例
    _lock = threading.Lock()  # 线程安全锁

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            with cls._lock:  # 加锁，确保线程安全
                if cls not in cls._instances:  # 双重检查（Double-Checked Locking）
                    cls._instances[cls] = super().__call__(*args, **kwargs)
        return cls._instances[cls]


class Singleton(metaclass=SingletonMeta):
    """单例基类，所有继承自 Singleton 的类都会自动变成单例"""
    pass
