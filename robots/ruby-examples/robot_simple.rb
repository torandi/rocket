# Ruby example bot
# This is bare minimum, the bot will connect and do nothing.

require 'socket'

# Useful for debugning
Thread.abort_on_exception = true

# You need to connect to the display
hostname = 'localhost'
port = 4710

# Open connection
s = TCPSocket.open(hostname, port)

s.puts "name simplebot"

Thread.new do
  loop do
    sleep 1
    s.puts "scan"
  end
end

# Enter main loop
while line = s.gets

  if line.chomp =~ /scan (\d+) (\d+)/
    match = line.chomp.scan(/scan (\d+) (\d+)/)

    angle = match[0][0].to_i
    dist = match[0][1].to_i
    
    next if angle == 0 and dist == 0
    
    s.puts "angle #{angle}"
    
    s.puts "shoot" if dist < 120
    
  end

  puts line.chop
end

s.close
