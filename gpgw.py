"""
A web interface to GPG decryption
"""

from urllib.parse import parse_qs
import subprocess

# Main entry point
def application(environ,start_response):
	"""entry point"""
	try:
		p = environ['PATH_INFO']
		if p=="/":
			c,h,r = ask(environ)
		elif p=="/decr":
			c,h,r = decr(environ)
		else:
			raise ValueError("Wrong access")
	except Exception as e:
		c = "400 Bad Request"
		h = [("Content-type", "text/plain")]
		r = "{}".format(e)
	h += [('Cache-Control','max-age=0')]
	start_response(c,h)
	return [r.encode()]

def ask(environ):
	"""ask for a file name and a passphrase"""
	# Page text
	r = """
	<!DOCTYPE html>
	<html lang="en">
	<head>
	<meta charset="UTF-8">
	<title>GPGW</title>
	</head>
	<body>
	<form action="decr" method="post">
	<table>
	<tr><td>File</td><td><input type="text" name="f"></td></tr>
	<tr><td>Passphrase</td><td><input type="password" name="pp"></td></tr>
	<tr><td></td><td><input type="submit"></td></tr>
	</table>
	</form>
	</body>
	</html>
	"""
	# Return success
	c = "200 OK"
	h = [("Content-type","text/html")]
	return c,h,r

def decr(environ):
	"""decrypt a file"""
	q = parse_qs(environ['wsgi.input'].readline().decode(), \
		     keep_blank_values=True)
	try:
		f = q['f'][0]
		pp = q['pp'][0]
		cmd = ["gpg1","--batch","--passphrase-fd","0","-d",f]
		r = subprocess.check_output(cmd,input=pp,universal_newlines=True)
	except KeyError:
		raise ValueError("Missing parameter")
	except subprocess.CalledProcessError:
		raise ValueError("gpg failed")
	# Return success
	c = "200 OK"
	h = [("Content-type","text/plain")]
	return c,h,r
