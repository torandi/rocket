# Rocket ruby server

require 'socket'
require 'yaml'
require 'item'
require 'ship'
require 'rkt_display'
require 'rkt_robot'
require 'thread'
require 'protocol'

Thread.abort_on_exception = true

if ARGV.length < 2
  puts "Error: <mapsize_x> <mapsize_y> [<listen_port>]"
  exit 1
end

SCREEN_SIZE = [ARGV[0].to_i, ARGV[1].to_i]
LISTEN_PORT = ARGV[2] ? ARGV[2].to_i : 4711

$items = Array.new
$score = Array.new
$verbose = 0;

# Main class
class RubyServer

  # constructor
  def initialize
    @server = nil
    @accepted_versions = [ 0.2 ]
  end

  # init server
  def open_server
    @server = TCPServer.open(LISTEN_PORT)

    loop do # Main loop

      # Opens a new thread when a client connects
      t = Thread.start(@server.accept) do |client|
        log "New connection"
        state = 0
        hash = ""
        obj = nil

        loop do
        
        	line = Protocol.read client
        	log "got #{line}"
        
        	if line.nil?
        		break
        	end
        
          # if "close" is sent, close connection
          # and break loop. 
        	if line == "close"
        		client.close
        		
        		# Remove ship
        		$items.each do |i|
        		  $items.delete i if i.client == client
        		end
        		
        		break
        	end

          # State 0 - Version
          # Used to verify that the client is using a accepted version. 
					if state == 0
		        if check_version line
		          state = 1
		          Protocol.write client, "cversion yes"
		          next
		        else
		          Protocol.write client, "cversion no"
		          client.close
		          break
		        end

          # State 1 - Auth
          # Used to send a key to the client.
					elsif state == 1
						if line == "auth"
							hash = (rand(8999)+1000).to_s
							Protocol.write client, "auth " + hash
							state = 2
							next
						else
							Protocol.write client, "invalid protocol, expected auth"
							log "invalid protocol (got: #{line}), expected auth"
							client.close
							break
						end

          # State 2 - Auth
          # Recive calculated auth hash from the client and auth.
					elsif state == 2
						auth_sum = (hash.to_i * hash[1,1].to_i ) + ( hash[3,1].to_i * 5)
						log "auth_sum[#{auth_sum}] hash[#{hash}]"
						
						if line.split[1].to_i == auth_sum
							current_time = "%10.6f" % (Time.now).to_f
							Protocol.write client, "auth ok #{current_time}"
							state = 3
							next
						else
							Protocol.write client, "auth no"
						end

          # State 3 - OK
          # Logged in and select mode.
					elsif state == 3
					
					  # Display
					  # This is a display connection
						if line == "mode display"
							log "mode display"
							obj = RktDisplay.new client
							Protocol.write client, "mode ok #{SCREEN_SIZE[0]} #{SCREEN_SIZE[1]}"
							
							# This is a display connection
							# Run display loop
							obj.run
							client.close
							log "Display closed"
							break
							
						# Robot
						# This connection is used to controll a bot.
						elsif line == "mode bot"
							log "mode robot"
							obj = RktRobot.new client
							Protocol.write client, "mode ok"
							
							log "New robot, addning to items"
							$items.push obj
							state = 4
							next
						end

					# I'm a robot
					elsif state == 4
						obj.input line
					else
						# Invalid protocol
						# Close connection and break loop
						Protocol.write client, "invalid protocol"
						client.close
						break
					end
				end # loop
      end   # thread
    end     # main loop
  end       # open_server

  private

	# Verify version
  def check_version line
    s = line.split
    if s[0] == "cversion"
      return true if @accepted_versions.include?(s[1].to_f)
    end
    false
  end
    
  # Log funktion
  # TODO: Write this down to a log file.
  def log str
    puts "RubyServer: #{str}" if $verbose > 0
  end

end

# The "run" method in the Objects in
# this array is regularly called.
t = Thread.new do 
	loop do
		$items.each do |i| 
			if not i.run
				$items.delete i
			end
		end
		sleep 0.05
	end
end

# Start everything
server = RubyServer.new
server.open_server


