import requests
from tqdm import tqdm
import time
import sys
import re
import math
import os
from common import *
from google_drive_util import *
	


class FileDownloader:

	tempFileEnding = ".part"

	def __init__(self):
		return
		
	def __checkStatus(request, acceptedCodes):
		#print("status code = " + str(request.status_code))
		if (not(request.status_code in acceptedCodes)):
			print("Expected status codes are:" + str(acceptedCodes) + "\nbut got: " + str(request.status_code))
			raise IOError("Connection Error: Couldn't get resource.")
		return	
			
		
	def __getFileName(contentDisposition):
		list = re.split(';', contentDisposition)
		for x in list:
			if x.startswith("filename=\"") and x.endswith("\""):
				result = x[len("filename=\""):]
				result = result[:len(result)-1]
				return result
				
		raise IOError("Couldn't get filename from request response headers!")	
		
	
	def __getFileSizeOfPausedDownload(fileName):
		# search for exisiting file
		try:
			fileSize = os.path.getsize(fileName)
		except OSError:
			raise IOError("Couldn't retrieve file size of a resumed download file: " + fileName)
			
		return fileSize
	
	def __downloadRequest(fileName, request, fileOpeningFlags, cancelor):
		chunk_size = 1024
		
		with open(fileName, fileOpeningFlags) as f:
			for data in tqdm(request.iter_content(chunk_size), total = 0 , unit='KB', unit_scale=True):
				if (cancelor.isCanceled()):
					return False
				f.write(data)
		
		return True
		
		
	"""
		Downloads a file from a given url
		
		This function extracts the filename from the response headers and saves 
		the file in a specified target folder.
		
		To support resuming a previous started (and interrupted) download,
		as a first step, the file is downloaded to a temporary file in the target folder.
		After the download has finished, the temporary download file is renamed to the filename
		previously extracted from the response headers.
		
		If the download is successful, no exception will be thrown.
		
		If the filename could be extracted, the filename is stored by this object and can be retrieved
		by getFileName. The function getTemporaryFileName than provides the name of the temporary file.
	
		
		The following exceptions are thrown:
		IOError : If...
			- the connection to the file servers fails
			- the filename couldn't be extracted from the response headers
			- any io error occurs while writing to the destination file
			- the temporary file couldn't be renamed
	"""	
	def download(self, id, session, fileName, cancelor):		
		
		# we download the content first to a temporary file
		temporaryFile = self.getTemporaryFileName(fileName)
		
		print("Start downloading file '" + fileName + "' to temporary file '" + temporaryFile + "'")
		
		
		# resume download if the temp file already exists
		if (os.path.lexists(temporaryFile)):
			print("Resume previous started download")
			fileSize = FileDownloader.__getFileSizeOfPausedDownload(temporaryFile)
			print("Already downloaded bytes: " + str(fileSize))
			
			resume_header = {'Range': 'bytes=%d-' % fileSize}
			#r = requests.get(url, headers=resume_header, stream=True,  verify=True)
			r = get_response_for_file_from_google_drive(id, session, passed_headers=resume_header)
			
			# 206 is returned if server accepts the range
			# 416 means out of range, i.d. the download is completed
			FileDownloader.__checkStatus(r, {206, 416})
			successfulDownload = FileDownloader.__downloadRequest(temporaryFile, r, "ab", cancelor)
		else:
			#r = requests.get(url, stream = True)
			r = get_response_for_file_from_google_drive(id, session)
			FileDownloader.__checkStatus(r, {200})
			successfulDownload = FileDownloader.__downloadRequest(temporaryFile, r, "wb", cancelor)

		
		# Early exit on failure
		if (not successfulDownload): 
			print("Download was canceled")
			return False

		
		# rename the temporary download file to the destination file
		try:
			os.rename(temporaryFile, fileName)
		except OSError as e:
			#print("Couldn't rename temporary file '" + temporaryFile + "' to destintation file '" + fileName + "'")
			print("WARNING: download - Couldn't rename temporary download file to destination: " + str(e))
			#print("Aborting...")
			#raise OSError("Couldn't rename temporary file '" + temporaryFile + "' to destintation file '" + fileName + "'")
			return False

		return True
		
	"""
		Extracts the filename of a file that can be downloaded by a given url.
		
		Throws the followding exceptions:
		IOError: If...
			- the connection to the server cannot be established
			- the filename couldn't be extracted from the response headers
	"""	
	def extractFileName(id, session):
		#r = requests.get(url, stream=True, verify=True)
		#Connection: close
		r = get_response_for_file_from_google_drive(id, session, passed_headers={'Connection': 'close'})
		r.close()
		FileDownloader.__checkStatus(r, {200})
		#print(r.headers)
		return FileDownloader.__getFileName(r.headers['content-disposition'])
		
		
	"""
		Provides the file path to the destination of the tempory downloaded file.
		NOTE: This function only returns the temporary download filename 
		After the function download(self) was called
		
		NOTE: If the download and renaming of the temporary file was successful, the 
		temporary file won't exist anymore!
	"""		
	def getTemporaryFileName(self, fileName):
		return fileName + self.tempFileEnding;
		
		