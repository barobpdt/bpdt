import threading

class ThreadSafeSingleton:
    _instance = None
    _lock = threading.Lock()

    def __new__(cls, *args, **kwargs):
        if not cls._instance:
            with cls._lock:
                # Double-check locking: Check again inside the lock
                # to ensure no other thread created the instance
                # while this thread was waiting for the lock.
                if not cls._instance:
                    cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        # Initialize the instance if needed, but ensure it's idempotent
        # as __init__ might be called multiple times by different threads
        # even if __new__ only creates one instance.
        if not hasattr(self, '_initialized'):
            # Perform actual initialization here
            self._initialized = True
            print("Singleton instance initialized.")

# Example usage:
if __name__ == "__main__":
    def create_and_check_singleton():
        instance = ThreadSafeSingleton()
        print(f"Thread {threading.current_thread().name}: Instance ID = {id(instance)}")

    threads = []
    for i in range(5):
        thread = threading.Thread(target=create_and_check_singleton, name=f"Thread-{i+1}")
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()

    # Verify that all threads received the same instance
    instance1 = ThreadSafeSingleton()
    instance2 = ThreadSafeSingleton()
    assert instance1 is instance2
    print(f"All threads received the same singleton instance: {instance1 is instance2}")