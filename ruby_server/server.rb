# Rocket ruby server

require 'socket'
require 'map'
require 'rkt_display'
require 'rkt_robot'

class RubyServer

  def initialize
    @server = nil
    @accepted_versions = [
      0.1
    ]
    @authed_clients = []
    @display_sockets = []
    @bot_sockets = []
  end

  def open_server
    @server = TCPServer.open(4711)

    loop do

      Thread.start(@server.accept) do |client|
        log "New connection"
        state = 0
        hash = ""
        obj = nil

        loop do
        
        	puts "read state[#{state}]"
        	line = read(client)
        	puts "got[#{line}]"
        
        	if line == "close"
        		client.close
        		break
        	end

					if state == 0

						# Kollar att klienten ansluter med en accepterad version
		        if check_version line
		          state = 1
		          client.puts "cversion yes"
		          next
		        else
		          client.puts "cversion no"
		          client.close
		          break
		        end

					elsif state == 1
					
						# auth
						if line == "auth"
							hash = (rand(8999)+1000).to_s
							client.puts "auth " + hash
							state = 2
							next
						else
							client.puts "invalid protocol, expected auth"
							puts "invalid protocol (got: #{line}), expected auth"
							client.close
							break
						end

					elsif state == 2
					
						auth_sum = (hash.to_i * hash[1,1].to_i ) + ( hash[3,1].to_i * 5)
						puts "auth_sum[#{auth_sum}] hash[#{hash}]"
						
						if true || line.split[1].to_i == auth_sum
							client.puts "auth ok"
							state = 3
							next
						else
							client.puts "auth no"
						end
					
					elsif state == 3
					
						if line == "mode display"
							puts "mode display"
							obj = RktDisplay.new client
							client.puts "mode ok"
						elsif line == "mode bot"
							puts "mode robot"
							obj = RktRobot.new client
							client.puts "mode ok"							
						else
							client.puts "invalid protocol, expected mode <str>"
							client.close
							break
						end
						
						# TODO: Tråden fortsätter och skicka data även när anslutingen är stängd
						Thread.new { obj.run line }
						break # break loop
						
					end
					
				end
      end
    end
  end

  private

  def check_version line
    s = line.split
    if s[0] == "cversion"
      return true if @accepted_versions.include?(s[1].to_f)
    end
    false
  end
  
  def read client
	 	client.gets.chomp
  end
  
  def log str
    puts str
  end

end

server = RubyServer.new
server.open_server
