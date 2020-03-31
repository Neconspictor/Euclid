from tkinter import *
from tkinter import Tk, ttk, messagebox
from tkinter.font import Font, nametofont
from time import sleep
from file_downloader import *
from common import *
import zipfile


class DownloadManager:

	def __init__(self):
		self.thread = None
		self.canceling = False
		
	
	def download(self, button, cancelor, checkboxTargetDirectoryList, cancelLabel):
	
		self.canceling = False
	
		isCanceled = cancelor.isCanceled()
		cancelor.lock()
		cancelor.toggle()
		
		errorOcurred = BooleanVar()
		errorOcurred.set(False)
		
		if (isCanceled):
			downloadDescriptors = self.__collectDescriptors(checkboxTargetDirectoryList)
			button.config(text="Cancel")
			self.thread = CallbackThread(target=self.__download, 
				args=(cancelor, downloadDescriptors, errorOcurred), 
				callback=self.__afterBusiness, 
				callback_args=(button, cancelor, cancelLabel),
				callback_error=errorOcurred)
			self.thread.start()
		else:
			button.config(state=DISABLED)
			interrupt_thread(self.thread)
			cancelLabel.grid(row=7, column=1, sticky=W)
			self.canceling = True
			print("interrupted thread")
			
			
		cancelor.release()	
		
		
	
	def __afterBusiness(self, button, cancelor, cancelLabel, errorOcurred):
		cancelor.lock()
		button.config(state=NORMAL, text="Download Files")
		cancelor.reset()
		cancelor.release()	
		cancelLabel.grid_forget()
		
		if (self.canceling):
			messagebox.showinfo(parent = button.master , title = "Notification", message = "Canceled download")
		elif (errorOcurred.get()):
			messagebox.showinfo(parent = button.master , title = "Notification", message = "Download finished with errors(!). See console output for more information")
		else:
			messagebox.showinfo(parent = button.master , title = "Notification", message = "Download finished")
			
		self.canceling = False	
		
	def __collectDescriptors(self, checkboxTargetDirectoryList):
		downloadDescriptors = []
		
		# collect download descriptors
		for checkboxDirectoryChooserPair in checkboxTargetDirectoryList:
			checkbox = checkboxDirectoryChooserPair[0]
			directoryChooser = checkboxDirectoryChooserPair[1]
			targetFolder = directoryChooser.getFolder()
			
			if (checkbox.checkboxState.get()):
				descriptor = checkbox.getDownloadDescriptor()
				descriptor.setTargetFolder(targetFolder)
				print("Option '" + descriptor.getDescription() + "' is selected!")
				downloadDescriptors.append(descriptor)
				
		return 	downloadDescriptors	
		
	def __downloadFile(self, downloadDescriptor, session, cancelor):
	
		id = downloadDescriptor.getGoogleDriveID()
		fileName = downloadDescriptor.getFileName()
		downloader = FileDownloader()
		
		print("Downloading file from Google Drive with id '" + id + "' ...")
		
		try:
			
			# retrieve filename from http header, if no filename is specified
			if (not fileName):
				fileName = FileDownloader.extractFileName(id, session)
			
			targetFolder = downloadDescriptor.getTargetFolder()
			os.makedirs(targetFolder, exist_ok=True)
			destinationFile = targetFolder + "/" + fileName;
			
			print("destinationFile = " + destinationFile)
			
			# only download the file if it doesn't exist
			if (os.path.lexists(destinationFile)):
				print("Destination file already exists. Skip downloading.")
				successful = True
			else:
				successful = downloader.download(id, session, destinationFile, cancelor)
				if (successful):
					print("Successfully downloaded file '" + fileName + "' to " + targetFolder)
					
			if (successful):
				print("Extract archive...")
				self.__extract(destinationFile, targetFolder)
				print("Successfully extracted file '" + fileName + "' to " + targetFolder)
		except IOError as e:
			print("Couldn't download file " + fileName)
			print("Reason: " + str(e))
			return False
			
		return	True
		

	def __download(self, cancelor, downloadDescriptors, errorOcurred):

		print("download started")
					
		session = requests.Session()
		#session.keep_alive = False
		session.trust_env = False	
		
		for downloadDescriptor in downloadDescriptors:
			if (cancelor.isCanceled()):
				return
			no_error = self.__downloadFile(downloadDescriptor, session, cancelor)
			if (not no_error):
				errorOcurred.set(True)
				
		return	
		
	
	def __extract(self, file, targetFolder):
		zip_ref = zipfile.ZipFile(file, 'r')
		zip_ref.extractall(targetFolder)
		zip_ref.close()
		


class LabeledCheckBox:

	def __init__(self, parent, downloadDescriptor):
		self.frame = Frame(parent)
		self.parent = parent
		self.label = Label(self.frame, text = downloadDescriptor.getDescription())
		self.label.grid(row= 0, column = 0)
		
		self.checkboxState = IntVar()
		self.checkbox = Checkbutton(self.frame, variable=self.checkboxState)
		self.checkbox.grid(row = 0, column = 1)
		
		self.downloadDescriptor = downloadDescriptor
		
	def disable(self):
		self.checkbox.config(state = DISABLED)
		self.label.config(fg="gray")
		
	def getDownloadDescriptor(self):
		return self.downloadDescriptor
		
	def getFrame(self):
		return self.frame
		
	def setDownloadDescriptor(self, downloadDescriptor):
		self.downloadDescriptor = downloadDescriptor
		self.label.config(text = downloadDescriptor.getDescription())
	
	
def createUI(root):
	mainFrame = Frame(root)
	mainFrame.pack(side=TOP, fill=BOTH, padx=10, pady=10, anchor=NW, expand=1)
	#mainFrame.grid()
	
	libDirectoryChooser = DirectoryChooser(mainFrame, "Library Directory: ", os.path.abspath("../lib"))
	libDirectoryChooser.getFrame().grid(columnspan=2, sticky=W+E)
	Grid.columnconfigure(mainFrame, 1, weight=1)

	ttk.Separator(mainFrame,orient=HORIZONTAL).grid(columnspan=2, pady=10, sticky=W+E)
	
	
	
	rootDirectoryChooser = DirectoryChooser(mainFrame, "Root Directory: ", os.path.abspath("../"))
	rootDirectoryChooser.getFrame().grid(columnspan=2, sticky=W+E)
	Grid.columnconfigure(mainFrame, 1, weight=1)

	ttk.Separator(mainFrame,orient=HORIZONTAL).grid(columnspan=2, pady=10, sticky=W+E)
	
	
	downloadDescriptor = DownloadDescriptor("1aln-VGIvA5tFn1jDJ9m598MhuHKln30h",
		"Common headers and sources",
		"libs-headers-sources.zip")
	
	checkbox1 = LabeledCheckBox(mainFrame, downloadDescriptor)
	checkbox1.checkbox.select()
	#checkbox1.disable()
	checkbox1.getFrame().grid(sticky=W)
	
	
	downloadDescriptor = DownloadDescriptor("1pCVDE-DpFrkI3bx5AEQ-tNHxO6rGSHE4", 
		"Libs for Visual Studio 2019 Win64 (x64)",
		"libs-msvc-142-x64.zip")
	checkbox2 = LabeledCheckBox(mainFrame, downloadDescriptor)
	checkbox2.checkbox.select()
	checkbox2.getFrame().grid(sticky=W)
	
	downloadDescriptor = DownloadDescriptor("1Uwr0PZa9Ykg0NSdzJBP3kKMv8KRLjmn3", 
		"Resources",
		"resources.zip")
	checkbox3 = LabeledCheckBox(mainFrame, downloadDescriptor)
	checkbox3.checkbox.select()
	checkbox3.getFrame().grid(sticky=W)
	
	checkboxTargetDirectoryList = [[checkbox1, libDirectoryChooser], [checkbox2, libDirectoryChooser], 
		[checkbox3, rootDirectoryChooser]]
	
	ttk.Separator(mainFrame,orient=HORIZONTAL).grid(columnspan=2, pady=10, sticky=W+E)
	
	
	downloadCancelor = ConcurrentCancelor()
	manager = DownloadManager()
	
	cancelLabel = Label(mainFrame, text="Canceling operation...", fg="red")
	
	downloadButton = Button(mainFrame, text="Download files",
						 command=lambda: manager.download(downloadButton, 
															downloadCancelor, 
															checkboxTargetDirectoryList, 
															cancelLabel))
	downloadButton.grid(sticky=W)
	
	cancelLabel.grid(row=7, column=1, sticky=W)
	cancelLabel.grid_forget()
	
"""	
-----------------------------------------------------
	Main program
-----------------------------------------------------	
"""
root = Tk()

default_font = nametofont("TkDefaultFont")
default_textFont = nametofont("TkTextFont")
default_font.configure(size=12, family="Segoe UI")
default_textFont.configure(size=12, family="Segoe UI")

w = 400
h = 135
ws = root.winfo_screenwidth()
wh = root.winfo_screenheight()
x = (ws / 2) - (w / 2)
y = (wh / 2) - (h / 2)
#root.geometry("%dx%d+%d+%d" % (w, h, x, y))
root.title("Extern Dependency Downloader")
root.resizable(True, False)

#root.withdraw()
#folder_selected = filedialog.askdirectory()

createUI(root)

root.mainloop()