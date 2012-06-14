# vim: ts=2:sts=2:sw=2:expandtab

module Rocket

  class Config

    class << self

      def port
        return 4711 if @port.nil?
        @port.to_i
      end

      def size
        return [640, 480] if @size.nil?
        @size
      end

      def size= arg
        @size = arg.split "x"
      end

      def screen_x
        size[0].to_i
      end

      def screen_y
        size[1].to_i
      end

      attr_accessor :debug, :accepted_versions
      attr_writer :port
    end

  end

end
