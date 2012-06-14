# vim: ts=2:sts=2:sw=2:expandtab

# Robot Socket (Rocket) server
# This is the class used for communication with rocket clients.
# The communication is obfuscated to prevent (or make it harder) to cheat.

# Include some required libs
require 'digest/sha1'

module Rocket
  class Protocol

    # Auth a client
    def self.auth client
      state = 0
      loop do
        line = Protocol.read client

        case state
        # Check that the client has a supported version
        when 0
          s = line.split
          if s[0] == "cversion" and Config.accepted_versions.include?(s[1].to_f)
            Protocol.write client, "cversion yes"
          else
            Protocol.write client, "cversion no"
            return false
          end

        # Send randomized server key to the client
        when 1
          if line != "auth"
            Protocol.write client, "invalid protocol, expected auth"
            return false
          end
          @hash = (rand(8999)+1000).to_s
          Protocol.write client, "auth " + @hash

        # Recive calculated auth hash from the client and auth
        when 2
          auth_sum = (@hash.to_i * @hash[1,1].to_i ) + ( @hash[3,1].to_i * 5)
          if line.split[1].to_i != auth_sum
            Protocol.write client, "auth no"
            return false
          end
          current_time = "%10.6f" % (Time.now).to_f
          Protocol.write client, "auth ok #{current_time}"

        # Auth successful
        when 3
          return true
        end

        state = state + 1
      end
    end

    def self.read_back
      @read_back = true
      puts "!  read back" if Config.debug
    end

    # Read data from the client
    def self.read client

      if @read_back
        puts "<r #{@last_read}" if Config.debug
        @read_back = false
        return @last_read
      end    

      begin
        run = true
        payload = ""
        while run
          package = client.read(32)
          if package.nil?
            puts "!0  recived nil" if Config.debug
            return nil
          end
          frame = package.bytes.map{ |a| (a^"5"[0]).chr }.to_s
          len = frame[0] # note, vid ruby 1.9 lägg på .ord
          hash = frame[1..4]
          buffer = frame[(32-(len == 0 ? 27 : len))..-1]
          if buffer.nil?
            puts "!1  nil payload" if Config.debug
            return nil
          end
          payload = payload + buffer
          #puts "read: #{package} -> [#{frame}] '#{payload}'"
          if payload.nil?
            puts "!2  nil payload" if Config.debug
            return nil
          end
          chash = Digest::SHA1.hexdigest(payload)[0..3]
          #package.bytes.each { |x| puts x }
          if chash != hash
            puts "!3  hash missmatch package:#{hash} calc:#{chash}" if Config.debug
            return nil
          end
          
          run = false if len > 0
        end
      rescue Errno::ECONNRESET
        nil
      end

      puts "<  #{payload.chomp}" if Config.debug

      @last_read = payload.chomp
      payload.chomp
    end

    # Write data to the client
    def self.write client, str, extra={ :debug => 1 } 
      if str.length > 27
        Protocol.write client, str[0..26], { :overflow => true, :debug => extra[:debug] }
        Protocol.write client, str[27..-1], { :overflow => false, :debug => extra[:debug] }
      else
        len = extra[:overflow] ? 0 : str.length
        hash = Digest::SHA1.hexdigest(str)[0..3]
        padding = (27-(len == 0 ? 27 : len)).times.map{ rand(9) }.join
        payload = str
        frame = "#{len.chr}#{hash}#{padding}#{payload}"
        package = frame.bytes.map{ |a| (a^"5"[0]).chr }.to_s
        client.write package
        puts ">  #{str}" if Config.debug >= extra[:debug]
        #puts "wp:#{package}"
      end
    end
  end
end
