import os;
import sys;
from chardet import detect

# get file encoding type
def get_encoding_type(file):
	with open(file, 'rb') as f:
		rawdata = f.read()
	return detect(rawdata)['encoding']


filePathSrc = os.path.dirname(os.path.abspath(__file__))
for root, dirs, files in os.walk(filePathSrc):
	for fn in files:
		if fn[-4:] == '.cpp' or fn[-2:] == '.h':
			
			print(os.path.join(root, fn))
			from_codec = get_encoding_type(os.path.join(root, fn))
			# add try: except block for reliability
			try: 
				with open(os.path.join(root, fn), 'r', encoding=from_codec) as f, open(os.path.join(root, "tmp"), 'w', encoding='utf-8') as e:
					text = f.read() # for small files, for big use chunks
					e.write(text)

				os.remove(os.path.join(root, fn)) # remove old encoding file
				os.rename(os.path.join(root, "tmp"), os.path.join(root, fn)) # rename new encoding
			except UnicodeDecodeError:
				print('Decode Error')
			except UnicodeEncodeError:
				print('Encode Error')
				
			
			
