import numpy as np
import matplotlib.pyplot as plt
import os
import time

# Named pipe path
fifo_name = "/tmp/limesdr_fifo"

# Ensure FIFO exists
if not os.path.exists(fifo_name):
    os.mkfifo(fifo_name)

# Set up real-time plotting
plt.ion()  # Turn on interactive mode
fig, ax = plt.subplots(figsize=(10, 6))
line, = ax.plot([], [], label="Received Samples")
ax.set_title("Real-Time Received Signal Samples")
ax.set_xlabel("Sample Index")
ax.set_ylabel("Amplitude")
ax.grid(True)
ax.legend()
plt.show()

# Open FIFO for reading
try:
    while True:
        with open(fifo_name, "rb") as fifo:
            data = fifo.read(1024 * 4)
            if len(data) == 0:
                time.sleep(0.1)
                continue
            # Convert byte data to numpy array
            samples = np.frombuffer(data, dtype=np.float32)

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
    fifo.close()
    os.remove(fifo_name)
    plt.ioff()