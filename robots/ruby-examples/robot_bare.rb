# Ruby example bot
# This is bare minimum, the bot will connect and do nothing.

require 'socket'

# You need to connect to the display
hostname = 'localhost'
port = 4710

# Open connection
s = TCPSocket.open(hostname, port)

# Enter main loop
while line = s.gets
  puts line.chop
end

s.close
