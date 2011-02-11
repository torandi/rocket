class RktRobot

	def initialize client
	  @ship = RocketShip.new client
	  puts "new rocketship"
	  @c = client
	end
	
	def client
	  @c
	end
	
	# Execute robot
	def run
	  @ship.run
	end
	
	# Got data from client
	def input line
	
    return if @ship.dead?
	
	  begin
	    if line =~ /^([a-z]+) ([a-z0-9]+)/
	      self.send("cmd_#{$1}", $2) if respond_to? "cmd_#{$1}".to_sym
	    elsif line =~ /^([a-z]+)$/
	      self.send("cmd_#{$1}") if respond_to? "cmd_#{$1}".to_sym
	    end
	  rescue Exception=>e
	    puts "Error: #{e}"
	  end
	end
	
	def output
	  return "#{@ship.item} #{@ship.name} #{@ship.x} #{@ship.y} #{@ship.angle} #{@ship.speed} #{@ship.action}"
	end
	
	def ship
    @ship
  end

	# Commands
	
	def cmd_name str
	  @ship.name = str[0..10]
	end
	
	def cmd_angle v
	  @ship.angle = v.to_i
	end

  def cmd_shoot
    @ship.shoot = true
    
    $items.each do |i|
    
      next if i.ship.dead?
    
      xd = i.ship.x - @ship.x
      yd = i.ship.y - @ship.y
      dist = Math.sqrt(xd ** 2 + yd ** 2)
      
      if dist < 129 and dist > 1
        enemy_angle = Math.atan2(yd, xd)
        relative_angle = enemy_angle - @ship.angle
        relative_angle = Math::PI - relative_angle if xd < 0
        fire_dist = Math.sin(relative_angle) * dist
        puts "#{fire_dist} #{dist} #{enemy_angle} #{relative_angle}"
        if fire_dist < 16
          puts "ship #{i.ship.name} is dead"
          i.ship.dead = true
          begin
            i.client.puts "dead"
          rescue Errno::EPIPE
            # Connection is dead, remove robot
            $items.delete i
          end
        end
      end
    end
  end
  
  def cmd_boost
    @ship.boost = true
    cmd_start
  end
  
  def cmd_start
    @ship.speed = 10
  end
  
  def cmd_stop
    @ship.speed = 0
  end

  def cmd_scan
    found = false
    $items.each do |i|
   
      next if i.ship.dead?

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
