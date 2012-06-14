# vim: ts=2:sts=2:sw=2:expandtab

# TODO: score

module Rocket

  class Ship

    def initialize client
      puts "! New rocket ship born with UID:#{@uid}"
      @client = client
      @pulse = Time.now.to_i
      @uid = Time.now.to_i.to_s[5..9] # placeholder uid
      @name = "n/a"
      @x = 0
      @y = 0
      @boost = 0
      @shoot = 0
      @angle = 0
      @speed = 5
      @speed_mod = 1.0
      @dead = 0
      @scan = 0
      @energy = 0
      @shield = false
      @color = "ffffff"
      @explosion = 0
    end

    # This function is executed once a tick
    def run
      puts "!S run UID:#{@uid}" if Config.debug >= 5
      @angle = @angle % 2*Math::PI if @angle > 2*Math::PI

      # Energy
      @energy = @energy + 1 if @energy < 100
      @energy = @energy - 1 if @shield

      if @dead == 1
        # @dead will be set to zero (alive) in this tick
        @x = rand(Config.screen_x)
        @y = rand(Config.screen_y)
        begin
          Protocol.write @client, "alive"
        rescue Errno::EPIPE
          # Client has closed the connection, remove robot
          @pulse = 0 # stale? will be true and the ship will be removed
        end
      end

      # Decay states
      @explosion = @explosion - 1 if @explosion > 0
      @boost = @boost - 1 if @boost > 0
      @shoot = @shoot - 1 if @shoot > 0
      @deadi = @dead - 1 if @dead > 0
      @scan = @scan - 1 if @scan > 0

      # Move the ship
      @y = (@y - speed * Math.sin(@angle)).floor
      @x = (@x + speed * Math.cos(@angle)).floor
      @x = @x - Config.screen_x if @x > Config.screen_x
      @y = @y - Config.screen_y if @y > Config.screen_y
      @x = Config.screen_x + @x if @x < 0 # x är negativ
      @y = Config.screen_y + @y if @y < 0

    end

    # Get the ships speed
    def speed
      @speed * @speed_mod
    end

    # Listen for input from the client
    def listen
      while line = Protocol.read(@client) do
        break if dead?

        @pulse = Time.now.to_i

        begin
          if line =~ /^([a-z]+) ([a-z0-9a-z]+)/i
            self.send("cmd_#{$1}", $2) if respond_to? "cmd_#{$1}".to_sym
          elsif line =~ /^([a-z]+)$/
            self.send("cmd_#{$1}") if respond_to? "cmd_#{$1}".to_sym
          end
        rescue Exception=>e
          puts "! Error: #{e}"
        end
      end
    end

    def action
      r  = Array.new
      r.push "boost" if boost?
      r.push "shoot" if @shoot > 0
      r.push "scan" if @scan > 0
      r.push "shield" if @shield
      r.push "explosion" if @explosion > 0
      r.join ","
    end

    # Output sent to the client
    def output
      pulse = Time.now.to_i - @pulse
      extra = ""
      extra = "(#{pulse})" if pulse > 60

      "ship #{@name}#{extra} #{@x} #{@y} #{@angle} #{@speed} #{@energy} #{@color} #{action}"
    end

    # Is this ship stale/idle?
    def stale?
      (Time.now.to_i - @pulse) > 120
    end

    # Consume energy
    def consume n
      if energy? n
        @energy = @energy - n
      else
        @energy = 0
      end
    end

    # Have we this amount of energy?
    def energy? n
      return true if n <= @energy
      false
    end 

    # Consume energy if we have the needed energy
    def consume_if? n
      if energy? n
        consume n
        return true
      end
      false
    end

    # Are we dead?
    def dead?
      return true if @dead > 0
      false
    end

    # Have we the boost active?
    def boost?
      return true if @boost > 0
      false
    end

    # Have we the shield active?
    def shield?
      @shield
    end

    # Set the color
    def color= str
      @color = str.downcase
    end

    ##
    # Commands from the bot
    ##

    def cmd_name str
      @name = str[0..10]
    end

    def cmd_angle v
      if consume_if? 5
        @angle = v.to_i * Math::PI/180
      end
    end

    def cmd_shoot
      return if not consume_if? 5
      
      # Ship target radius
      target_radius = 15

      # Fire length
      fire_length = 120

      # Trigger shoot state
      @shoot = 20

      # Loop thru all enemy ships
      Server.ships.each do |i|

        # Ignore dead ships
        next if i.dead?

        # Get enemy dela and dist
        enemy_xd = i.x - @x
        enemy_yd = i.y - @y
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

        if v1_rad > @angle and @angle > v2_rad
          # This is a hit

          # Have the ship the shield enabled?
          if shield? and consume_if? 40
            cmd_energy
          else
            puts "ship #{i.name} is dead" if Config.debug
            i.dead = 120
            i.explosion = 5

            begin
              Protocol.write i.client, "dead"
            rescue Errno::EPIPE
              # Connection is dead, remove robot
              Server.ships.delete i
            end
          end
        end
      end
    end

    def cmd_boost
      return if not consume_if? 20
      @boost = 20
    end

    def cmd_start
      @speed_mod = 1.0
    end

    def cmd_stop
      @speed_mod = 0.0
    end

    def cmd_toggle
      return cmd_start if @speed == 0.0
      cmd_stop
    end

    def cmd_scan

      return if not consume_if? 10

      @scan = 10

      found = false
      Server.ships.each do |i|

        next if i.dead?

        # Get enemy dela and dist
        xd = i.x - @x
        yd = i.y - @y
        dist = Math.sqrt(xd ** 2 + yd ** 2)

        # Calculate angle to enemy (radians)
        enemy_rad = Math.atan2(yd, xd)
        enemy_rad = enemy_rad * Math::PI/2      if xd < 0 and yd > 0
        enemy_rad = enemy_rad * -1              if xd < 0 and yd < 0
        enemy_rad = enemy_rad * Math::PI/2 * -1 if xd > 0 and yd < 0
        enemy_rad = 2*Math::PI - enemy_rad      if xd > 0 and yd > 0

        # convert to degres
        enemy_angle = enemy_rad * 180/Math::PI

        if dist < 400 and dist > 0
          Protocol.write @client, "scan #{enemy_angle.to_i} #{dist.to_i}"
        found = true
        end
      end
      Protocol.write @client, "scan 0 0" if not found
    end

    def cmd_energy
      Protocol.write @client, "energy #{@energy}"
    end

    # Deprecated
    def cmd_shield
      @shield = ! @shield
    end

    def cmd_shieldoff
      @shield = false
    end

    def cmd_shieldon
      @shield = true
    end

    def cmd_color color
      @color = color
    end

    alias :cmd_fire :cmd_shoot

    attr_reader :x, :y, :name, :client
    attr_writer :dead, :explosion

  end

end
