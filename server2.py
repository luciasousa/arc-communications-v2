import SimpleHTTPServer
import SocketServer
import sys
import cgi
import urlparse

PORT = int(sys.argv[1])

class ServerHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):

    def do_POST(self):
      content_len = int(self.headers.getheader('content-length', 0))
      post_body = self.rfile.read(content_len)
      url = "http://foo.bar/?"+post_body
      parsed = urlparse.urlparse(url)
      values = urlparse.parse_qs(parsed.query)
     # print(values)
      #print(type(values["persons"])) 
      with open("/tmp/accident_flag", "w") as outF:
          outF.write(values["persons"][0])
          outF.write('\n')
          outF.write(values["overturned"][0])
          outF.write('\n')
          outF.write(values["airbags"][0])
          outF.write('\n')
          outF.write(values["all_seatbelts"][0])

      
Handler = ServerHandler

httpd = SocketServer.TCPServer(("", PORT), Handler)

print "serving at port", PORT
httpd.serve_forever()
