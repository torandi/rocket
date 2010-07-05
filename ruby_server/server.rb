# Rocket ruby server

require 'socket'
require 'map'

class RubyServer

  def initialize
    @server = nil
    @accepted_versions = [
      0.1
    ]
    @authed_clients = []
    @display_sockets = []
    @bot_sockets = []
  end

  def open_server
    @server = TCPServer.open(4711)

    loop do

      Thread.start(@server.accept) do |client|
        log "New connection"

        loop do

          if not @authed_clients.include?(client.object_id)
            if check_version client
              @authed_clients.push(client.object_id)
              client.puts "cversion yes"
            else
              client.puts "cversion no"
              client.close
            end
          end

          # Get line from client
          line = client.gets.chop
          log "Data: '#{line}'"

          # Lägger klienten i @display_sockets
          # om den inte redan ligger där.
          if line == "mode display" and not @display_sockets.include?(client.object_id)
            @display_sockets.push(client.object_id)
            client.puts "mode ok"
            log "Set display"
          end

          # Lägger klienten i @bot_sockets
          # om den inte redan ligger där.
          if line == "mode bot" and not @bot_sockets.include?(client.object_id)
            @bot_sockets.push(client.object_id)
            client.puts "mode ok"
            log "Set bot"
          end

          # Anroppar rätt metod beroende som det
          # är en robot eller display som ansluter.
          if @bot_sockets.include?(client.object_id)
            bot client, line
          elsif @display_sockets.include?(client.object_id)
            display client, line
          else
            log "error, no mode given"
            client.puts "error, no mode given"
            client.close
          end
        end
      end
    end
  end

  def display client, line

    if line == "close"
      client.close # todo: kolla så den verkligen stänger tråden
    end

  end

  def bot client, line

    if line == "close"
      client.close # todo: kolla så den verkligen stänger tråden
    end

  end

  private

  def check_version client
    s = client.gets.chop.split
    if s[0] == "cversion"
      return true if @accepted_versions.include?(s[1].to_f)
    end
    false
  end
  
  def log str
    puts str
  end

end

server = RubyServer.new
server.open_server

world = RocketWorld.new 100, 100
