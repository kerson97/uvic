import sys
from socket import *
import ssl


# initializes url variable
def getInput():
    global url
    global parsedurl
    url = sys.argv[1]
    parsedurl = url.split("/", 1)


def checkPassword():
    global result
    supportsPassword = False

    # initialize socket connection with given url, simple error handling if url is invalid

    try:
        s = socket(AF_INET, SOCK_STREAM)
        s.connect((parsedurl[0], 443))
        request = 'GET / HTTP/1.1\r\nHost:' + parsedurl[0] + '\r\n\r\n'
    except:
        print("Error! There was a problem with the input, enter a valid URL and try again")
        exit()

    # wrap socket, skip hostname check(seemingly necessary for sites not supporting TLS)
    ssl.SSLContext.check_hostname = False
    ss = ssl.create_default_context().wrap_socket(s, server_hostname=parsedurl[0])

    if len(parsedurl) > 1:
        inputstring = 'GET /' + parsedurl[1] + ' HTTP/1.1\r\nHost:' + parsedurl[0] + '\r\n\r\n'

    else:
        inputstring = 'GET /' + parsedurl[0] + ' HTTP/1.1\r\nHost:' + parsedurl[0] + '\r\n\r\n'

    ss.sendall(inputstring.encode('utf_8'))
    result = ss.recv(1000).decode()
    # check for password protection
    if result.__contains__("Unauthorized") or result.__contains__("401"):
        supportsPassword = True

    s.close()
    return supportsPassword


def checkHTTP2():
    global supportsHTTP2
    supportsHTTP2 = False

    # check for HTTP2 support via alpn protocols
    ssl.SSLContext.check_hostname = False
    s = ssl.create_default_context()
    s.set_alpn_protocols(['h2', 'spdy/3', 'http/1.1'])

    connection = s.wrap_socket(socket(AF_INET, SOCK_STREAM), server_hostname=url)
    connection.connect((parsedurl[0], 443))
    selectedProtocol = connection.selected_alpn_protocol()

    # if protocol used is "h2" then set supportsHTTP2 variable
    if selectedProtocol == "h2":
        supportsHTTP2 = True


def cookieHandler():
    # initialize lists for cookies
    cookies = []
    fullCookieName = []
    i = 0

    # split result into segments containing cookie information, store them in lists
    iterator = iter(result.splitlines())
    for line in iterator:
        if "Set-Cookie:" in line:
            cookies.append(line.rsplit())
            line = line.replace("Set-Cookie: ", "")
            line = line.replace(";", ",")
            fullCookieName.append("Cookie Name: " + line)
            i += 1

    # print the cookies that were found
    for elem in fullCookieName:
        print(elem)


# call 3 of the driving functions
getInput()
checkPassword()
checkHTTP2()

# print results
print("Supports HTTP2: " + str(supportsHTTP2))
print("Password Protected: " + str(checkPassword()))
print("List of Cookies: ")
cookieHandler()
