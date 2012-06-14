# vim: ts=2:sts=2:sw=2:expandtab

module Rocket

  class Display

    def initialize client
      @client = client
      @send_buffer = Array.new
      @send_buffer_delay = 0;
    end

    def client
      @client
    end

    # Send str to the connected client
    def send str

      @send_buffer.push [Time.now.to_i + @send_buffer_delay, str] if @send_buffer_delay > 0

      begin
        if @send_buffer_delay > 0
          Protocol.write @client, (@send_buffer.shift)[1], :debug => 5 if Time.now.to_i >= (@send_buffer.first)[0]
        else
          Protocol.write @client, str, :debug => 5
        end
        false
      rescue Exception=>e
        puts "!D connection terminated: #{e}" if Config.debug
        true
      end
    end

    # Send data to client
   def run
      stop = false

      while not stop do
        frame_time = "%10.6f" % ((Time.now).to_f + @send_buffer_delay.to_f)
        stop = send "frame start #{frame_time}"

        if not Server.ships.nil?
          Server.ships.each do |ship|
            next if ship.dead?
            stop = send ship.output
          end
        end

#        $score.each do |s|
#          if rand(100) < 1
#          stop = send s[1].send_score(s[0], s[2], s[3])
#          end
#        end

        stop = send "frame stop"
        sleep 0.05
      end
    end


  end

  end
