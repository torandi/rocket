class RktRobot

	def initialize client
	  @ship = RocketShip.new
	  puts "new rocketship"
	  @c = client
	end
	
	# Execute robot
	def run
	  @ship.run
	end
	
	# Got data from client
	def input line
	  if line =~ /^([a-z]+) ([a-z0-9]+)/
	    self.send($1, $2) if respond_to? $1.to_sym
	  elsif line =~ /^([a-z]+)$/
	    self.send($1) if respond_to? $1.to_sym
	  end
	end
	
	def output
	  return "#{@ship.item} #{@ship.name} #{@ship.x} #{@ship.y} #{@ship.angle} #{@ship.action}"
	end
	
	def ship
    @ship
  end

	# Commands
	
	def name str
	  @ship.name = str[0..10]
	end
	
	def angle v
	  @ship.angle = v.to_i
	end

  def fire
    @ship.fire = true
  end
  
  def boosts
    @ship.boost = true
  end
  
  def start
    @ship.speed = 10
  end
  
  def stop
    @ship.speed = 0
  end

  def scan
    found = false
    $items.each do |i|
      xd = i.ship.x - @ship.x
      yd = i.ship.y - @ship.y
      dist = Math.sqrt(xd ** 2 + yd ** 2)
      if dist < 200 and dist > 0
        @c.puts "scan #{xd} #{yd}"
        found = true
      end
    end
    @c.puts "scan 0 0" if not found
  end
	
end
