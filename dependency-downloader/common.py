from tkinter import filedialog
from tkinter import *
import threading
from threading import Thread
import os

class CallbackThread(Thread):
	
	def __init__(self, target, args=(), callback=None, callback_args=None, callback_error=None):
		#super(CallbackThread, self).__init__(target, *args)
		Thread.__init__(self, target = target, args = args)
		self.callback = callback
		self.callback_args = callback_args
		self.callback_error=callback_error
		self.args = args
			
	
	def run(self):
		try:
			Thread.run(self)
		except:
			#print("Exception occurred!!!!!")
			self.callback_error.set(True)
			
		
		if (self.callback and self.callback_args and self.callback_error):
			self.callback(*self.callback_args, self.callback_error)
		elif(self.callback and self.callback_args):
			self.callback(*self.callback_args)
		elif(self.callback and self.callback_error):
			self.callback(self.callback_error)
		elif (self.callback):
			self.callback()
			#print("here!")


class Cancelor:
	
	def __init__(self, defaultValue):
		self.canceled = defaultValue
		
	def isCanceled(self):
		return self.canceled
		
	def setCanceled(self, value):
		self.canceled = value
		
	def toggle(self):
		self.canceled  = not self.canceled



class ConcurrentCancelor:
	
	def __init__(self):
		self.__lock = threading.RLock()
		self.cancelor = Cancelor(True)
		
	def toggle(self):
		self.__lock.acquire()
		self.cancelor.toggle()
		self.__lock.release()
		
	def isCanceled(self):
		return self.cancelor.isCanceled()

	def lock(self):
		self.__lock.acquire()
		
	def release(self):
		self.__lock.release()
		
	def reset(self):
		self.__lock.acquire()
		self.cancelor.setCanceled(True)
		self.__lock.release()
		
		
class DownloadDescriptor:
	
	def __init__(self, googleDriveID, description, fileName=None, fileSize=-1, targetFolder="."):
		self.googleDriveID = googleDriveID
		self.fileName = fileName
		self.fileSize = fileSize
		self.description = description
		self.targetFolder = os.path.abspath(targetFolder)
		
	def getDescription(self):
		return self.description
		
	def getFileName(self):
		return self.fileName

	"""
		If a file size is defined, this function returns the file size.
		Otherwise it will return -1 (for "no file size available")
	"""
	def getFileSize(self):
		return self.fileSize
		
	def getGoogleDriveID(self):
		return self.googleDriveID
		
	def getTargetFolder(self):
		return self.targetFolder
		
	def setTargetFolder(self, targetFolder):
		self.targetFolder = os.path.abspath(targetFolder)
		
		
class DirectoryChooser:

	def __init__(self, parent, description, defaultFolderPath):
		self.frame = Frame(parent)
		self.desc = Label(self.frame, text=description)
		self.desc.grid(row = 0, column = 0)

		
		self.textFieldVar = StringVar()
		
		self.textFieldVar.set(defaultFolderPath)
		textSize = len(self.textFieldVar.get())
		
		textSize = 50
		
		self.txtField = Entry(self.frame, textvariable = self.textFieldVar, width = textSize)
		#self.txtField.config(font=("Courier", 10))
		Grid.columnconfigure(self.frame, 1, weight=1, minsize=50)
		self.txtField.grid(row = 0, column = 1, sticky=W+E)

		myBtn = Button(self.frame, text = "Browse", command = self.__askAndSelectFolder)
		myBtn.grid(row = 0, column = 2)
		
	def getFrame(self):
		return self.frame
		
	def getFolder(self):
		return self.textFieldVar.get()
		
	def setFolder(self, folderPath):
		self.textFieldVar.set(folderPath)
		
		
		
	def __askAndSelectFolder(self):
		
		current_folder = self.getFolder()
		
		if (not os.path.exists(current_folder)):
			current_folder = os.path.abspath(".")
		
		folder_selected = filedialog.askdirectory(initialdir = current_folder)
		
		if (folder_selected):
			self.setFolder(folder_selected)		
			
			
			
			
class SigFinish(Exception):
    pass

def throw_signal_function(frame, event, arg):
    raise SigFinish()

def do_nothing_trace_function(frame, event, arg):
    # Note: each function called will actually call this function
    # so, take care, your program will run slower because of that.
    return None

def interrupt_thread(thread):
    for thread_id, frame in sys._current_frames().items():
        if thread_id == thread.ident:  # Note: Python 2.6 onwards
            set_trace_for_frame_and_parents(frame, throw_signal_function)

def set_trace_for_frame_and_parents(frame, trace_func):
    # Note: this only really works if there's a tracing function set in this
    # thread (i.e.: sys.settrace or threading.settrace must have set the
    # function before)
    while frame:
        if frame.f_trace is None:
            frame.f_trace = trace_func
        frame = frame.f_back
    del frame			