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
  	    Protocol.write @c, (@send_buffer.shift)[1] if Time.now.to_i >= (@send_buffer.first)[0]
  	  else
    	  Protocol.write @c, str
  	  end
  	  false
    rescue Exception=>e
      true
    end
    
	end
	
	# Send data to client
	def run
	  stop = false
	  last_time = 0
	
		while not stop do
			frame_time = "%10.6f" % ((Time.now).to_f + @send_buffer_delay.to_f)
			stop = send "frame start #{frame_time}"

      $items.each do |obj|
        next if obj.ship.dead?
        stop = send obj.output
      end

	    $score.each do |s|
	      if last_time < (Time.now).to_i - 5
	        stop = send s[1].send_score(s[0], s[2], s[3])
	        last_time = (Time.now).to_i
	      end
	    end

			stop = send "frame stop"
			sleep 0.05
		end
	end

end
