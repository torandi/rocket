class RocketShip < RocketItem

  def initialize client
    @client = client
    @item = "ship"
    @name = "n/a"
    @x = 200
    @y = 200
    @boost = false
    @boost_ticker = 0
    @shoot = false
    @shoot_ticker = 0
    @angle = 0
    @speed = 10
    @dead = false
    @dead_ticker = 0
  end

  def action
    r  = Array.new
    r.push "boost" if @boost
    r.push "shoot" if @shoot
    r.join ","
  end
  
  def run
    @angle = @angle % 360 if @angle > 360

    if @shoot_ticker > 5
      @shoot = false
      @shoot_ticker = 0
    end

    if @boost_ticker > 10
      @boost = false
      @boost_ticker = 0
    end

    if @dead_ticker > 20
      @dead = false
      @dead_ticker = 0
      @x = 200
      @y = 200

      begin
        @client.puts "alive"
      rescue Errno::EPIPE
        # Robot is dead, remove robot
        false
      end
    end

    @y = @y - (speed * Math.sin(@angle * Math::PI/180)).to_i
    @x = @x + (speed * Math.cos(@angle * Math::PI/180)).to_i
    
    @x = 0 if @x > SCREEN_SIZE[0]
    @y = 0 if @y > SCREEN_SIZE[1]
    @x = SCREEN_SIZE[0] if @x < 0
    @y = SCREEN_SIZE[0] if @y < 0
    
    @shoot_ticker+= 1 if @shoot
    @boost_ticker+= 1 if @boost
    @dead_ticker+= 1 if @dead
    
    true
	end

  def dead?
    @dead
  end

  def speed
		@speed*speed_mod 
  end

  def speed_mod
    if @boost
      speed_mod = 1.5
    else
      speed_mod = 1.0
    end
  end

  attr_accessor :item, :x, :y, :name, :boost, :shoot, :angle, :speed, :dead
end
