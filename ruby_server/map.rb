class RocketWorld

  def initialize x, y
    @size_x = x
    @size_y = y
    @world = Array.new(x) { |i| i = Array.new(y) }
  end

  def set x, y, obj
    @world[x][y] = obj
  end
  
  def get x, y
    @world[x][y]
  end
  
  def content? x, y
    @world[x][y] != nil or not @world[x][y].empty?
  end

end
