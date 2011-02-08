package robot;

import java.util.Random;

/**
 *
 * @author torandi
 */
public class Bot implements ClientInterface{
    static final String server="localhost";
    static final int port=4710;

    int id=new Random().nextInt(1000);
    
    private ServerInterface si;
    
    public Bot() {
        System.out.println("ID: "+id);
        si=new ServerInterface(server,port,this);
    }

    public void connected() {
        System.out.println("Connected to server");
    }

    public String clientName() {
        return "BOTNAME"+id;
    }

    public void ready() {
        System.out.println("Server ready");
        si.scan();
    }

    public void noScanResult() {

    }

    public void scanResult(RelativePostion rp) {

    }

    public void error(Exception e) {
        System.err.println("Server probably fucked up:");
        e.printStackTrace();
    }

    public void dead() {
        System.out.println("Snap. Du dog.");
        new Bot();
    }

}
