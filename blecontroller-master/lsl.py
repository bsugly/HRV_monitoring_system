# Imports
from pylsl import StreamInlet, resolve_stream
import threading
from PyQt5.QtCore import QObject, pyqtSignal

class MyObject(QObject):
    messageReceived = pyqtSignal(str)

    def __init__(self):
        super().__init__()

    def sendMessage(self, message):
        self.messageReceived.emit(message)

print("# Looking for an available OpenSignals stream...")
os_stream = resolve_stream("name", "OpenSignals")
# Create an inlet to receive signal samples from the stream
inlet = StreamInlet(os_stream[0])
myObject = MyObject()
while True:
   # Receive samples
   sample, timestamp = inlet.pull_sample()
   myObject.sendMessage(sample[1])


#data = None
#data_time = None
#flag = 0
#thread = None
#arry_flag = []
#data_mutex = threading.Lock()
#data_time_mutex = threading.Lock()
#def received_data():
#    global data
#    global data_time
#    global flag
#    global arry_flag
#    global data_mutex
#    global data_time_mutex
#    # Resolve an available OpenSignals stream
#    print("# Looking for an available OpenSignals stream...")
#    os_stream = resolve_stream("name", "OpenSignals")

#    # Create an inlet to receive signal samples from the stream
#    inlet = StreamInlet(os_stream[0])

#    while True:
#        # Receive samples
#        sample, timestamp = inlet.pull_sample()
#        with data_mutex:
#            data = sample[1]
#            arry_flag.append(data)
#        with data_time_mutex:
#            data_time = timestamp
#        flag = flag+1
##        data = sample[1]
##        data_time = timestamp
##        print(timestamp, sample)
#        print("arry_falg:", len(arry_flag))
#        # return sample
#def start_data_thread():
##    global thread
##    if thread is None or not thread.is_alive():
#    thread = threading.Thread(target=received_data)
#    thread.start()

#start_data_thread()
##while True:
##        print("sample value:", data)

