class RocketShip < RocketItem

  def initialize name, x, y
    @item = "ship"
    @name = name
    @x = x
    @y = y
    @boost = false
    @shoot = false
    @angle = 0
  end

  def action
    r  = Array.new
    r.push "boost" if @boost
    r.push "shoot" if @shoot
    r.join ","
  end
  
  def run
    @angle = @angle % 360 if @angle > 360
    @x = @x + 1
    @y = @y + Math.tan(@angle - 90).to_i 
  end

  attr_accessor :item, :x, :y, :name, :boost, :shoot, :angle
end
