# Ruby example bot
# This is simple bot, the bot moves around.

require 'socket'

# You need to connect to the display
hostname = 'localhost'
port = 4710

# Open connection
s = TCPSocket.open(hostname, port)

s.puts "name randombot"

# Enter main loop
while true
  a = rand(360)
  puts a
  s.puts "angle #{a}"
  s.puts "boost" if rand(1) == 0
  s.puts "shoot" if rand(2) == 0
  sleep 1
end

s.close
