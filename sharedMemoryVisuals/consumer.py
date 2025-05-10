import mmap
import ctypes
import time
import os
import sys
import posix_ipc

SHM_NAME = "/live_data"
SEM_NAME = "/semCtrl"
QUEUE_SIZE = 1024
TEXT_SIZE = 64

# Define structures using ctypes
class QueueEntry(ctypes.Structure):
    _fields_ = [
        ("timestamp", ctypes.c_int64),
        ("text", ctypes.c_char * TEXT_SIZE)
    ]

class SharedQueue(ctypes.Structure):
    _fields_ = [
        ("head", ctypes.c_size_t),
        ("tail", ctypes.c_size_t),
        ("count", ctypes.c_size_t),
        ("entries", QueueEntry * QUEUE_SIZE)
    ]

def main():

    # Open shared memory with retry
    shm_fd = None
    for _ in range(10):  # Retry up to 10 times
        try:
            shm_fd = os.open(f"/dev/shm{SHM_NAME}", os.O_RDWR)
            break
        except FileNotFoundError:
            print(f"Waiting for shared memory {SHM_NAME} to be created...")
            time.sleep(0.5)  # Wait 500ms before retrying
        except OSError as e:
            print(f"Failed to open shared memory {SHM_NAME}: {e}")
            time.sleep(0.5)
    if shm_fd is None:
        print(f"Failed to open shared memory {SHM_NAME} after retries.")
        sys.exit(1)
    
    # Map shared memory
    shm = mmap.mmap(shm_fd, ctypes.sizeof(SharedQueue), mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)

    # Access shared memory as a structure
    queue = SharedQueue.from_buffer(shm)

    # Open semaphore
    sem = None
    for _ in range(10):  # Retry up to 10 times
        try:
            sem = posix_ipc.Semaphore(SEM_NAME, 0)
            break
        except posix_ipc.BusyError:
            print(f"Waiting for semaphore {SEM_NAME} to be created...")
            time.sleep(0.5)
        except posix_ipc.ExistentialError:
            print(f"Semaphore {SEM_NAME} does not exist.")
            time.sleep(0.5)
    
    if sem is None:
        print(f"Failed to open semaphore {SEM_NAME} after retries.")
        shm.close()
        os.close(shm_fd)
        sys.exit(1)
        
    print(f"Consumer started. PID: {os.getpid()}")
    print(f"Shared memory {SHM_NAME} opened with size {ctypes.sizeof(SharedQueue)} bytes.")
    print(f"Semaphore {SEM_NAME} opened.")
    print(f"Queue size: {QUEUE_SIZE}, Text size: {TEXT_SIZE}")
    print(f"Head: {queue.head}, Tail: {queue.tail}, Count: {queue.count}")
    print("Waiting for data...")

    # Continuously read from shared memory
    try:
        while queue.count > 0:
            # Wait for semaphore
            sem.acquire()

            entry = queue.entries[queue.head]
            print(f"Timestamp: {entry.timestamp}, Text: {entry.text.decode('utf-8').rstrip(chr(0))}")
            # Update head and count
            queue.head = (queue.head + 1) % QUEUE_SIZE
            queue.count -= 1

            # Release semaphore
            sem.release()
            time.sleep(0.1)  # Avoid busy-waiting
    except KeyboardInterrupt:
        print("Stopping consumer...")
        shm.close()
        os.close(shm_fd)
        sem.close()
    finally:
        # Cleanup
        shm.close()
        os.close(shm_fd)
        sem.close()

if __name__ == "__main__":
    main()