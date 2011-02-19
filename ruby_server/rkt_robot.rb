class RktRobot

	def initialize client
	  @ship = RocketShip.new client
	  puts "new rocketship"
	  @c = client
	  @pulse = Time.now.to_i
	  
	  uid = Time.now.to_i.to_s[5..9] # placeholder uid
	  $score.push [uid, @ship, 0.0, 0.0]
	end
	
	def client
	  @c
	end
	
	# Execute robot
	def run
	  pulse = Time.now.to_i - @pulse
	  return false if pulse > 120
	  @ship.run
	end
	
	# Got data from client
	def input line
	
    return if @ship.dead?
	
	  @pulse = Time.now.to_i
	
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
	  pulse = Time.now.to_i - @pulse
	  if pulse > 60
	    pulse_str = "(#{pulse})"
	  else
	    pulse_str = ""
	  end
	  return "#{@ship.item} #{@ship.name}#{pulse_str} #{@ship.x} #{@ship.y} #{@ship.angle} #{@ship.speed} #{@ship.action}"
	end
	
	def ship
    @ship
  end
  
  def record_death s
    $score.map! do |a| 
      if a[1] == s
        a = [a[0],a[1],a[2],(a[3]+1)]
      else
        a
      end
    end
  end

  def record_kill s
    $score.map! do |a| 
       if a[1] == s
        a = [a[0],a[1],(a[2]+1),a[3]]
       else
        a
       end
    end
  end
  
 	# Commands
	
	def cmd_name str
	  @ship.name = str[0..10]
	end
	
	def cmd_angle v
	  # Convert to radians
	  @ship.angle = v.to_i * Math::PI/180
	end

  def cmd_shoot
  
    # Ship target radius
    target_radius = 15
    
    # Fire length
    fire_length = 120
  
    # Trigger shoot state
    @ship.shoot = true
    
    # Loop thrue all enemy ships
    $items.each do |i|
    
      # Ignore dead ships
      next if i.ship.dead?
    
      # Get enemy dela and dist
      enemy_xd = i.ship.x - @ship.x
      enemy_yd = i.ship.y - @ship.y
      enemy_dist = Math.sqrt(enemy_xd ** 2 + enemy_yd ** 2)
      
      # Ignore my self or to distant ships
      next if enemy_dist == 0 or enemy_dist - target_radius > fire_length
      
      # Calculate angle to enemy (radians)
      enemy_rad = Math.atan2(enemy_yd, enemy_xd)
      enemy_rad = enemy_rad * Math::PI/2      if enemy_xd < 0 and enemy_yd > 0
      enemy_rad = enemy_rad * -1              if enemy_xd < 0 and enemy_yd < 0
      enemy_rad = enemy_rad * Math::PI/2 * -1 if enemy_xd > 0 and enemy_yd < 0
      enemy_rad = 2*Math::PI - enemy_rad      if enemy_xd > 0 and enemy_yd > 0

      # Calculate max and min angles
      v_rad = Math.atan(target_radius/enemy_dist)
      v1_rad = enemy_rad + v_rad
      v2_rad = enemy_rad - v_rad

      puts "#{i.ship.x} #{i.ship.y}" if $verbose > 2
      puts "#{v1_rad} #{v2_rad}" if $verbose > 2
      puts "#{enemy_rad} #{@ship.angle}" if $verbose > 2

      if v1_rad > @ship.angle and @ship.angle > v2_rad
        puts "ship #{i.ship.name} is dead"
        i.ship.dead = true
        record_kill @ship
        record_death i.ship
        begin
          i.client.puts "dead"
        rescue Errno::EPIPE
          # Connection is dead, remove robot
          $items.delete i  
        end
      end
    end
  end
  
  def cmd_boost
    @ship.boost true
  end
  
  def cmd_start
    @ship.speed_mod = 1.0
  end
  
  def cmd_stop
    @ship.speed_mod = 0.0
  end
  
  def cmd_toggle
    return cmd_start if @ship.speed == 0.0
    cmd_stop
  end

  def cmd_scan
    @ship.scan = true

    puts "My angle is #{@ship.angle}" if $verbose > 2
    puts "My cordinates is x:#{@ship.x} y:#{@ship.y}" if $verbose > 2
    
    found = false
    $items.each do |i|
   
      next if i.ship.dead?

      # Get enemy dela and dist
      xd = i.ship.x - @ship.x
      yd = i.ship.y - @ship.y
      dist = Math.sqrt(xd ** 2 + yd ** 2)
      
      # Calculate angle to enemy (radians)
      enemy_rad = Math.atan2(yd, xd)
      enemy_rad = enemy_rad * Math::PI/2      if xd < 0 and yd > 0
      enemy_rad = enemy_rad * -1              if xd < 0 and yd < 0
      enemy_rad = enemy_rad * Math::PI/2 * -1 if xd > 0 and yd < 0
      enemy_rad = 2*Math::PI - enemy_rad      if xd > 0 and yd > 0

      # Debug
      puts "Enemy in quadriant 1"             if xd < 0 and yd > 0 and $verbose > 1
      puts "Enemy in quadriant 2"             if xd < 0 and yd < 0 and $verbose > 1
      puts "Enemy in quadriant 3"             if xd > 0 and yd < 0 and $verbose > 1
      puts "Enemy in quadriant 4"             if xd > 0 and yd > 0 and $verbose > 1
      
      # convert to degres
      enemy_angle = enemy_rad * 180/Math::PI

      if $verbose > 2
        puts "Ship found at x:#{i.ship.x} y:#{i.ship.y}"
        puts "Delta x:#{xd} y:#{yd}"
        puts "The distance to the ship is #{dist}"
        puts "Angle to enemy is #{enemy_angle} (#{enemy_rad} radians)"
        puts "Shipt in 1th quadriant" if yd < 0 and xd > 0
        puts "Shipt in 2nd quadriant" if yd < 0 and xd < 0
        puts "Shipt in 3rd quadriant" if yd > 0 and xd < 0
        puts "Shipt in 4th quadriant" if yd > 0 and xd > 0
      end

      if dist < 200 and dist > 0
        @c.puts "scan #{enemy_angle.to_i} #{dist.to_i}"
        found = true
      end
    end
    @c.puts "scan 0 0" if not found
  end
  
  def cmd_verbose level
    puts "Enable verbose level #{level}"
    $verbose = level.to_i
  end
	
	alias :cmd_fire :cmd_shoot
	
end
