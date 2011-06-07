require 'digest/sha1'

class Protocol

  def self.read client
  	begin
  	  run = true
  	  payload = ""
  	  while run
        package = client.read(32)
        if package.nil?
          puts "recived nil"
          return nil
        end
        frame = package.bytes.map{ |a| (a^"5"[0]).chr }.to_s
        len = frame[0] # note, vid ruby 1.9 lägg på .ord
        hash = frame[1..4]
        payload = payload + frame[(32-(len == 0 ? 27 : len))..-1]
        #puts "read: #{package} -> [#{frame}] '#{payload}'"
        if payload.nil?
          puts "error: nil payload"
          return nil
        end
        chash = Digest::SHA1.hexdigest(payload)[0..3]
        #package.bytes.each { |x| puts x }
        if chash != hash
          puts "error: hash missmatch package:#{hash} calc:#{chash}"
          return nil
        end
        
        run = false if len > 0
      end
	 	rescue Errno::ECONNRESET
	 		nil
	 	end
	 	payload.chomp
  end

  def self.write client, str, overflow=false
    if str.length > 27
      Protocol.write client, str[0..26], true
      Protocol.write client, str[27..-1]
    else
      len = overflow ? 0 : str.length
      hash = Digest::SHA1.hexdigest(str)[0..3]
      padding = (27-(len == 0 ? 27 : len)).times.map{ rand(9) }.join
      payload = str
      frame = "#{len.chr}#{hash}#{padding}#{payload}"
      package = frame.bytes.map{ |a| (a^"5"[0]).chr }.to_s
      client.write package
      #puts "ws: #{str}"
      #puts "wp:#{package}"
    end
  end

end
