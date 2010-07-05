
class RocketWorld

  def initialize x, y
    @x = x
    @y = y
    @world = Array.new(x) { |i| i = Array.new(y) }
  end

end
