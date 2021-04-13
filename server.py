import SimpleHTTPServer
import SocketServer
import sys
import cgi

PORT = int(sys.argv[1])

class ServerHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):

    def do_POST(self):
      content_len = int(self.headers.getheader('content-length', 0))
      post_body = self.rfile.read(content_len)
      print post_body
      print(type(post_body))
      splitted = post_body.split("&")
      splitted = [i for i in splitted if i]      
      persons = splitted[0].split("=")[1]
      overturned = splitted[1].split("=")[1]
      airbags = splitted[2].split("=")[1]
      all_seatbelts = splitted[3].split("=")[1]
      print persons, overturned, airbags, all_seatbelts
      with open("/tmp/accident_flag", "w") as outF:
        for line in splitted:
          outF.write(line.split("=")[1])
          outF.write("\n")      
     
      
Handler = ServerHandler

httpd = SocketServer.TCPServer(("", PORT), Handler)

print "serving at port", PORT
httpd.serve_forever()
