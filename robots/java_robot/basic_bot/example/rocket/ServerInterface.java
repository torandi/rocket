package rocket;

import java.nio.ByteBuffer;
import net.XServerSocket;
import net.XSocket;
import net.XStreamParent;

/**
 *
 * @author torandi
 */
public class ServerInterface implements XStreamParent {

	/**
	 * PUBLIC METHODS
	 **/

	public void fire() {
		 sck.writeln("fire");
	}

	public void stop() {
		 if(moving)
		 sck.writeln("stop");
	}

	public void start() {
		 if(moving) {
			  sck.writeln("stop");
		 }
	}

	public void shield() {
		sck.writeln("shield");
	}

	public void boost() {
		 sck.writeln("boost");
	}

	public void scan() {
		 scan=true;
		 sck.writeln("scan");
	}

	public void color(String color) {
		sck.writeln("color "+color);
	}

	public void rotate(double angle) {
		 int grad=(int)Math.toDegrees(angle);
		 sck.writeln("angle "+grad);
	}

	/**
	 * End public methods
	 **/

    private XSocket sck;
    private ClientInterface ci;
    private String server;
    private int port;

    private boolean scan=false;

    private boolean moving=true;

    private boolean ready=false;

    public ServerInterface(String server,int port,ClientInterface clientInterface) {
        this.server=server;
        this.port=port;
        ci=clientInterface;

        initialize();
    }
    
    private void initialize() {
        sck=new XSocket();
       
        if(sck.connect(server, port)) {
            ci.connected();
            sck.setParent(this);
            sck.initXStreamReciver();
            //Send client name
            sck.writeln("name "+ci.clientName());
        }
    }


    public synchronized void dataRecived(String data, ByteBuffer b, XSocket sck) {
        try {
            if(data.equals("Redo!")) {
                ci.ready();
                ready=true;
                return;
            }
            if(ready) {
					 String cmd[] = data.trim().split(" ");
                if(cmd[0].equals("dead")) {
						 ci.dead();
                } else if(cmd[0].equals("scan")) {
                    //Scanning resultat
                    if(cmd[1].equals("0") && cmd[1].equals("0")) {
                       ci.noScanResult();
                    } else {
                       if(cmd.length>2) {
                         ci.scanResult(new RelativePostion(Integer.parseInt(cmd[1]),Integer.parseInt(cmd[2])));
                       }
                    }
                } else {
							System.out.println("Unknown cmd recived: "+data.trim());
					 }
            }
        } catch (Exception e) {
            ci.error(e);
        }

    }

    public void newClient(XSocket client, XServerSocket srvr) { }

    public void reconnect(XSocket sck) {
        
    }

    public void connectionClosed(XSocket sck) {
        
    }



}
