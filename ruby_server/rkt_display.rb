# This is a display connection
class RktDisplay

	def initialize client
		@c = client
	end
	
	# Send str to the connected client
	def send str, v=true
	  puts "RktDisplay: #{str}" if v 

    begin
  	  @c.puts str
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
        break if send obj.output, false
      end

			send "frame stop", false
			sleep 0.1
		end
	end

end
