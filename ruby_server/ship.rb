class RocketShip < RocketItem

  def initialize
    @item = "ship"
    @name = "n/a"
    @x = 0
    @y = 0
    @boost = false
    @shoot = false
    @angle = 0
    @speed = 10
  end

  def action
    r  = Array.new
    r.push "boost" if @boost
    r.push "shoot" if @shoot
    r.join ","
  end
  
  def run
    @angle = @angle % 360 if @angle > 360

    @y = @y - (@speed * Math.sin(@angle * Math::PI/180)).to_i
    @x = @x + (@speed * Math.cos(@angle * Math::PI/180)).to_i
	end

  attr_accessor :item, :x, :y, :name, :boost, :shoot, :angle, :speed, :fire
end
