# Rocket ruby server

LISTEN_PORT = 4711

require 'socket'
require 'map'
require 'rkt_display'
require 'rkt_robot'

# Main class
class RubyServer

  # constructor
  def initialize
    @server = nil
    @accepted_versions = [ 0.1 ]
  end

  # init server
  def open_server
    @server = TCPServer.open(LISTEN_PORT)

    loop do # Main loop

      # Opens a new thread when a client connects
      Thread.start(@server.accept) do |client|
        log "New connection"
        state = 0
        hash = ""
        obj = nil

        loop do
        
        	log "read state #{state}"
        	line = read(client)
        	log "got #{line}"
        
          # if "close" is sent, close connection
          # and break loop. 
        	if line == "close"
        		client.close
        		break
        	end

          # State 0 - Version
          # Used to verify that the client is using a accepted version. 
					if state == 0
		        if check_version line
		          state = 1
		          client.puts "cversion yes"
		          next
		        else
		          client.puts "cversion no"
		          client.close
		          break
		        end

          # State 1 - Auth
          # Used to send a key to the client.
					elsif state == 1
						if line == "auth"
							hash = (rand(8999)+1000).to_s
							client.puts "auth " + hash
							state = 2
							next
						else
							client.puts "invalid protocol, expected auth"
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
							client.puts "auth ok #{current_time}"
							state = 3
							next
						else
							client.puts "auth no"
						end

          # State 3 - OK
          # Logged in and select mode.
					elsif state == 3
					
					  # Display
					  # This is a display connection
						if line == "mode display"
							log "mode display"
							obj = RktDisplay.new client
							client.puts "mode ok"
							
						# Robot
						# This connection is used to controll a bot.
						elsif line == "mode bot"
							log "mode robot"
							obj = RktRobot.new client
							client.puts "mode ok"
							
						# Invalid protocol
						# Close connection and break loop
						else
							client.puts "invalid protocol, expected mode <str>"
							client.close
							break
						end
						
						# Start mode
						# Execute a mode in a new thread
						# TODO: Tråden fortsätter och skicka data även när anslutingen är stängd
						Thread.new { obj.run line }
						break # break loop
						
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
  
  # Read a line från a client
  def read client
	 	client.gets.chomp
  end
  
  # Log funktion
  # TODO: Write this down to a log file.
  def log str
    puts "RubyServer: #{str}"
  end

end

# Start everything
$world = RocketWorld.new 640, 480
server = RubyServer.new
server.open_server
