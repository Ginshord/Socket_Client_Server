##A client and a server written in C lang##
##The test environment is under Linux OS##
Author: HE JIN
------------------------------------------------------------------------
Client can send GET or HEAD request to any website server
the CMD input format is :  ./client -h https://...:port/...
                            -h is optional (HEAD request)
                            without -h, GET request
                            :port is optional, default is 80

                            eg. ./client -h http://www.google.com/
                            hostname: www.google.com
                            port:80
                            identifier:/
                            response: Header info abt google page

                            ./client http://www.iastate.edu/students/
                            hostname: www.iastate.edu
                            port: 80
                            identifier: students
                            response: page source abt www.iastate.edu/students/
------------------------------------------------------------------------
For server part, it just stimulate the web server, parse the Request from client.
default server port is 3509. it only has the source "testfile.html"
when clients send the GET or HEAD request to it, it will return the content or
HEAD info.
------------------------------------------------------------------------
Hint:
if you test it in local,  from client side:
eg.  ./client -h http://localhost:3509/testfile.html
     or   ./client http://localhost:3509/testfile.html

then server will return some info to client.
client will save response in the file named "response.txt"

if you put server online, from client side:
eg. ./client -h http://.../testfile.html
