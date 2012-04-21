class RocketShip < RocketItem

  def initialize client
    @client = client
    @item = "ship"
    @name = "n/a"
    @x = 200
    @y = 200
    @boost_ticker = 0
    @shoot = false
    @shoot_ticker = 0
    @angle = 0
    @speed = 5.0
    @speed_mod = 1.0
    @dead = false
    @dead_ticker = 0
    @scan = false
    @scan_ticker = 0
    @energy = 100;
  end

  def action
    r  = Array.new
    r.push "boost" if boost?
    r.push "shoot" if @shoot
    r.push "scan" if @scan
    r.join ","
  end
  
  def send_score uid, kill, death
    death = 1.0 if death == 0.0
    ratio = kill/death
    puts "score #{uid} #{@name} #{ratio} #{kill} #{death}" if $verbose > 3
    return "score #{uid} #{@name} #{ratio}"
  end

  def consume n
    if energy? n
      @energy = @energy - n 
    else
      @energy = 0
    end
  end
  
  def energy? n
    return true if n <= @energy
    false
  end
  
  def run
    @angle = @angle % 2*Math::PI if @angle > 2*Math::PI

    @energy = @energy + 1 if @energy < 100

    if @shoot_ticker > 5
      @shoot = false
      @shoot_ticker = 0
    end

    if @boost_ticker > 20
      boost false
      @boost_ticker = 0
    end

    if @dead_ticker > 20
      @dead = false
      @dead_ticker = 0
      @x = rand(SCREEN_SIZE[0])
      @y = rand(SCREEN_SIZE[1])

      begin
        Protocol.write @client, "alive"
      rescue Errno::EPIPE
        # Robot is dead, remove robot
        false
      end
    end

    if @scan_ticker > 2
      @scan = false
      @scan_ticker = 0
    end

    @y = (@y - speed * Math.sin(@angle)).floor
    @x = (@x + speed * Math.cos(@angle)).floor
    
    @x = @x - SCREEN_SIZE[0] if @x > SCREEN_SIZE[0]
    @y = @y - SCREEN_SIZE[1] if @y > SCREEN_SIZE[1]
    @x = SCREEN_SIZE[0] + @x if @x < 0 # x är negativ
    @y = SCREEN_SIZE[1] + @y if @y < 0
    
    @shoot_ticker+= 1 if @shoot
    @boost_ticker+= 1 if boost?
    @dead_ticker+= 1 if @dead
    @scan_ticker+= 1 if @scan
    
    true
	end

  def dead?
    @dead
  end

  def speed
		@speed*@speed_mod
  end
  
  def boost b
    if b
      @speed_mod = 1.5
    else
      @speed_mod = 1.0
    end
  end

  def boost?
    return true if @speed_mod == 1.5
    false
  end

  attr_accessor :item, :x, :y, :name, :shoot, :angle, :dead, :scan, :speed_mod, :energy
end
