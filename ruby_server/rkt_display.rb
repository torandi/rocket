
class RktDisplay

	def initialize client
		@c = client
	end
	
	def run line
		i = 10
		loop do
			@c.puts "clear"
			@c.puts "ship nsg #{i} 100 45"
			@c.puts "update"
			sleep 1
			i = i + 1
		end
	end

end
