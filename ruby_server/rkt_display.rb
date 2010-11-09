
class RktDisplay

	def initialize client
		@c = client
	end
	
	def send str
	  @c.puts str
	  puts "RktDisplay: #{str}"
	end
	
	def run line
		i = 10
		v = 0
		loop do
		  frame_time = "%10.6f" % (Time.now).to_f
			send "frame start #{frame_time}"
			send "ship nsg #{i} 100 #{v} shoot"
			send "ship torandi #{600-i} 200 #{180-v} boost,shoot"
			send "frame stop"
			sleep 0.05
			
			i = i + 1
			v = v + 1
		end
	end

end
