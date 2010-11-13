package robot;

import java.nio.ByteBuffer;
import xjava.net.XServerSocket;
import xjava.net.XSocket;
import xjava.net.XStreamParent;

/**
 *
 * @author torandi
 */
public class ServerInterface implements XStreamParent {
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
            sck.writeln(ci.clientName());
        }
    }

    public void fire() {
        sck.writeln("f");
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

    public void boost() {
        sck.writeln("b");
    }

    public void scan() {
        scan=true;
        sck.writeln("s");
    }

    public void rotate(double angle) {
        int grad=(int)Math.toDegrees(angle);
        sck.writeln("a "+grad);
    }

    public synchronized void dataRecived(String data, ByteBuffer b, XSocket sck) {
        try {
            if(data.equals("Redo!")) {
                ci.ready();
                ready=true;
                return;
            }
            if(ready) {
                if(data.trim().equals("dead")) {
                    sck.close();
                    ci.dead();
                } else {
                    //Scanning resultat
                    if(data.trim().equals("0")) {
                       ci.noScanResult();
                    } else {
                       String[] split = data.split(" ");
                       if(split.length>1) {
                         ci.scanResult(new RelativePostion(Integer.parseInt(split[0].trim()),Integer.parseInt(split[1].trim())));
                       }
                    }
                }
            }
        } catch (Exception e) {
            ci.error(e);
        }

    }

    public void newClient(XSocket client, XServerSocket srvr) {
        
    }

    public void reconnect(XSocket sck) {
        
    }

    public void connectionClosed(XSocket sck) {
        
    }



}
