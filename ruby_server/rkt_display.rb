# This is a display connection
class RktDisplay

	def initialize client
		@c = client
		@send_buffer = Array.new
		@send_buffer_delay = 3;
	end
	
	def client
	  @c
	end
	
	# Send str to the connected client
	def send str, v=true
	  puts "RktDisplay: [delay: #{@send_buffer_delay}] #{str}" if v 

    # Add to sendbuffer
    @send_buffer.push [Time.now.to_i + @send_buffer_delay, str]

    # Send from sendbuffer
    begin
  	  @c.puts((@send_buffer.shift)[1]) if Time.now.to_i > (@send_buffer.first)[0]
  	  false
    rescue Exception=>e
      true
    end
    
	end
	
	# Send data to client
	def run
		loop do
			frame_time = "%10.6f" % (Time.now).to_f
			send "frame start #{frame_time}", false

      $items.each do |obj|
        next if obj.ship.dead?
        break if send obj.output, false
      end

			send "frame stop", false
			sleep 0.1
		end
	end

end
