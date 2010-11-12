
class RktDisplay

	def initialize client
		@c = client
	end
	
	def send str
	  puts "RktDisplay: #{str}"
	  @c.puts str
	end
	
	def run line
		i = 10
		v = 0
		loop do

			frame_time = "%10.6f" % (Time.now).to_f
			send "frame start #{frame_time}"

      $items.each do |item|
        item.run
        send "#{item.item} #{item.name} #{item.x} #{item.y} #{item.angle} #{item.action}"
      end

			send "frame stop"
			sleep 0.01

			i = i + 1
			v = v - 1
		end
	end

end
