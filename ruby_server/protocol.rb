require 'digest/sha1'

class Protocol

  def self.read client
  	begin
      package = client.read(32)
      frame = package.bytes.map{ |a| (a^5).chr }.to_s
      len = frame[0] # note, vid ruby 1.9 lägg på .ord
      hash = frame[1..4]
      payload = frame[(32-len)..-1]
      puts "read: #{package} -> [#{frame}] #{payload}"
      if payload.nil?
        puts "error: nil payload"
        return nil
      end
      chash = Digest::SHA1.digest(payload)[0..3]
      if chash != hash
        puts "error: hash missmatch package:#{hash} calc:#{chash}"
        return nil
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
      len = overflow ? 0 : str.length.chr
      hash = Digest::SHA1.digest(str)[0..3]
      padding = (27-len).times.map{ rand(9) }.join
      payload = str
      frame = "#{len}#{hash}#{padding}#{payload}"
      package = frame.bytes.map{ |a| (a^5).chr }.to_s
      client.puts package
      puts "write: #{str} [#{frame}] -> #{package}"
    end
  end

end
