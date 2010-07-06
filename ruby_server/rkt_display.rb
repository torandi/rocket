
class RktDisplay

	def initialize client
		@c = client
	end
	
	def run line
		i = 10
		v = 0
		loop do
			puts "send data"
		
			@c.puts "clear"
			@c.puts "ship nsg #{i} 100 #{v}"
			@c.puts "ship torandi #{600-i} 200 #{180-v} boost"
			@c.puts "update"
			sleep 0.05
			
			i = i + 1
			v = v + 1
		end
	end

end
