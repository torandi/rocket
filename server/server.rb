#!/usr/bin/ruby
# vim: ts=2:sts=2:sw=2:expandtab

# Robot Socket (Rocket) server
# This is the main class used for communication with rocket clients.
# A client can be a display, or a forwarded robot.

# Include some required libs
require 'socket'
require 'yaml' # todo: read config from config.yml
require 'thread'

require 'config'
require 'protocol'
require 'display'
require 'ship'

DEBUG = true

Thread.abort_on_exception = true if DEBUG

module Rocket

  class Server

    class << self
      attr_accessor :ships
    end

    def initialize screen_x, screen_y, port
      @screen_size = { :x => screen_x, :y => screen_y }
      Config.accepted_versions = [ 0.2 ]
      @server = TCPServer.open port
      @ship_list = []
    end

    def listen
      loop do
        # Opens a new thread when a client connects
        thread = Thread.start(@server.accept) do |client|
          # Auth
          break if not Protocol.auth client

          puts "!  Auth phase compleated" if Config.debug
          Protocol.read_back

          # Select mode (display or rocket ship)
          case Protocol.read client
          when "mode display"
            # Send mode ok and world size back to client
            Protocol.write client, "mode ok #{@screen_size[:x]} #{@screen_size[:y]}"
            # Call the display and enter display loop
            Display.new(client).run
            # Display closed, clean up
          when /mode (ro)?bot/
            # Send mode ok to client
            Protocol.write client, "mode ok"
            # Create a new ship and adds it to @ship_list
            ship = Ship.new(client)
            Server.ships = [] if Server.ships.nil?
            Server.ships.push ship
            ship.listen
          else
            # Invalid protocol
            Protocol.write client, "invalid protocol"
          end

          # Thread compleated, clean up
          client.close
        end
      end
    end

    def run
      Thread.new do
        loop do
          if not Server.ships.nil?
            Server.ships.each do |ship|
              if ship.stale?
                Server.ships.delete ship
              else
                ship.run
              end
            end
          end
          sleep 0.05
        end
      end
    end

  end


  # Parse parameters
  ARGV.size.times do |i|
    next if not ARGV[i] =~ /^-/
    case ARGV[i]
    when "-p"
      Config.port = ARGV[i+1] if ARGV.size > i + 1
    when "-d"
      if ARGV.size > i + 1 and not ARGV[i+1] =~ /^-/
        Config.debug = ARGV[i+1].to_i
      else
        Config.debug = 1
      end
    when "-s"
      Config.size = ARGV[i+1] if ARGV.size > i + 1
    else
      puts "arguments: "
      puts "\t-p <port>\tSpecify a port, default: 4711"
      puts "\t-s <WxH>\tSpecify word dimensions, default: 640x480"
      puts "\t-d\tEnable debug mode"
    end
  end

  if Config.debug
    puts "Starting server with port #{Config.port}"
    puts "World size is set to #{Config.size.join("x")}"
  end

  server = Server.new Config.screen_x, Config.screen_y, Config.port
  server.run
  server.listen

end


