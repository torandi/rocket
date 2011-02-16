# This is a display connection
class RktDisplay

	def initialize client
		@c = client
		@send_buffer = Array.new
		@send_buffer_delay = 0;
	end
	
	def client
	  @c
	end
	
	# Send str to the connected client
	def send str
	  puts "RktDisplay: [delay: #{@send_buffer_delay}] #{str}" if $verbose > 3

    # Add to sendbuffer if @send_buffer_delay > 0
    @send_buffer.push [Time.now.to_i + @send_buffer_delay, str] if @send_buffer_delay > 0

    # Send from sendbuffer
    begin
      if @send_buffer_delay > 0
  	    @c.puts((@send_buffer.shift)[1]) if Time.now.to_i >= (@send_buffer.first)[0]
  	  else
    	  @c.puts str
  	  end
  	  false
    rescue Exception=>e
      true
    end
    
	end
	
	# Send data to client
	def run
		loop do
			frame_time = "%10.6f" % ((Time.now).to_f + @send_buffer_delay.to_f)
			send "frame start #{frame_time}"

      $items.each do |obj|
        next if obj.ship.dead?
        break if send obj.output
      end

			send "frame stop"
			sleep 0.1
		end
	end

end
