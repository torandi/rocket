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
        }
    }

    public void fire() {
        sck.writeln("shoot");
    }

    public void stop() {
        sck.writeln("stop");
    }

    public void start() {
        sck.writeln("start");
    }

    public void boost() {
        sck.writeln("boost");
    }

    public void scan() {
        scan=true;
        sck.writeln("scan");
    }

    public void send_name() {
        sck.writeln("name "+ci.clientName());
    }

    public void rotate(double angle) {
        int grad=(int)Math.toDegrees(angle);
        sck.writeln("angle "+grad);
    }

    public synchronized void dataRecived(String data, ByteBuffer b, XSocket sck) {
        try {
            if(data.equals("ready")) {
                //Send client name
                send_name();
                ci.ready();
                ready=true;
                return;
            }
            if(ready) {
                if(data.trim().equals("dead")) {
                    ci.dead();
                } else if(data.trim().startsWith("scan")){
                    data=data.trim().substring(5);
                    //Scanning resultat
                    if(data.trim().equals("0 0")) {
                       ci.noScanResult();
                    } else {
                       String[] split = data.split(" ");
                       if(split.length>1) {
                         ci.scanResult(new RelativePostion(Integer.parseInt(split[0].trim()),Integer.parseInt(split[1].trim())));
                       }
                    }
                } else {
                    System.out.println(data);
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
