import mmap
import numpy as np
import matplotlib.pyplot as plt
import os
import time

# Shared memory name
shm_name = "/dev/shm/limesuite_shm"

# Open shared memory with retry
shm_fd = None
while shm_fd is None:
    try:
        shm_fd = os.open(shm_name, os.O_RDONLY)
    except FileNotFoundError:
        print(f"Waiting for shared memory {shm_name} to be created...")
        time.sleep(0.5)  # Wait 500ms before retrying
    except OSError as e:
        print(f"Failed to open shared memory {shm_name}: {e}")
        time.sleep(0.5)

# Map shared memory
shm = mmap.mmap(shm_fd, 1024 * 4, mmap.MAP_SHARED, mmap.PROT_READ)  # 1024 floats

# Set up real-time plotting
plt.ion()  # Turn on interactive mode
fig, ax = plt.subplots(figsize=(10, 6))
line, = ax.plot([], [], label="Received Samples")
ax.set_title("Real-Time Received Signal Samples")
ax.set_xlabel("Sample Index")
ax.set_ylabel("Amplitude")
ax.grid(True)
ax.legend()

# Continuously read from shared memory
try:
    while True:
        plt.ion() 
        # Read samples from shared memory
        shm.seek(0)
        samples = np.frombuffer(shm.read(1024 * 4), dtype=np.float32)

        # Update plot
        line.set_xdata(np.arange(len(samples)))
        line.set_ydata(samples)
        ax.relim()
        ax.autoscale_view()
        plt.draw()
        plt.pause(0.01)  # Brief pause to allow plot update

        time.sleep(0.01)  # Avoid busy-waiting
except KeyboardInterrupt:
    print("Stopping plotter...")
finally:
    # Cleanup
    shm.close()
    os.close(shm_fd)