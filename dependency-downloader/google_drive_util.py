import requests

def get_response_for_file_from_google_drive(id, session, passed_headers=None):
	URL = "https://docs.google.com/uc?export=download"
	
	response = session.get(URL, params = { 'id' : id }, stream = True, headers = passed_headers)
	token = get_confirm_token(response)

	if token:
		params = { 'id' : id, 'confirm' : token }
		response = session.get(URL, params = params, stream = True, headers = passed_headers)
		#return response
		
	return response

def get_confirm_token(response):
    for key, value in response.cookies.items():
        if key.startswith('download_warning'):
            return value

    return None