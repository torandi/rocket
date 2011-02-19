package robot;

import java.util.Random;

/**
 *
 * @author torandi
 */
public class Bot implements ClientInterface {
    static final int port=4710;

	private String server;
    private RelativePostion lastScan =null;

    int id=new Random().nextInt(1000);
    
    private ServerInterface si;
    
    private Double lastAngle=null;
    private int error_ticks=0;
    private int chase_ticks=0;

    private int inactive_ticks=0;
    private double angle=0;

    public Bot(String server) {
		this.server = server;
        System.out.println("ID: "+id);
        si=new ServerInterface(server,port,this);
    }

    public void connected() {
        System.out.println("Connected to server");
    }

    public String clientName() {
        return "torandbot"+id;
    }

    public void ready() {
        System.out.println("Server ready");
        si.scan();
    }

    public void noScanResult() {
        lastScan=null;
        inactive_ticks++;
        si.scan();
        if(inactive_ticks>100) {
            angle+=Math.PI/4;
            si.rotate(angle);
				si.boost();
            inactive_ticks=0;
        }
    }

    public void scanResult(RelativePostion rp) {
        inactive_ticks=0;
        if(lastScan==null) {
            lastScan=rp;
            error_ticks=0;
            chase_ticks=0;
            si.rotate(rp.angle);
        } else if( error_ticks>10) {
            lastScan=null;
            si.scan();
            return;
        } else if(rp.diff(lastScan).norm() < 50){
            lastScan=rp;
            error_ticks=0;
				//TODO: Calculate future position of target
            si.rotate(rp.angle);
        } else {
            error_ticks++;
        }
        if(lastScan!=null) {
            if(lastScan.angle-rp.angle < Math.PI/4) {
                if(chase_ticks>5) {
                    si.boost();
                } else {
                    chase_ticks++;
                }
            }
        }
        angle=rp.angle;

        if(rp.dist<120) {
            si.fire();
        }
        si.scan();
    }

    public void error(Exception e) {
        System.err.println("Server probably fucked up:");
        e.printStackTrace();
    }

    public void dead() {
        System.out.println("Snap. Du dog.");
    }

}
