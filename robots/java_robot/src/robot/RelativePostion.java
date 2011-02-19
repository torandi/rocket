package robot;

/**
 *
 * @author torandi
 */
public class RelativePostion {
    public int dist;
    public double angle;
    public int x,y;

    public RelativePostion(double angle, int dist, int x, int y) {
        this.angle=angle;
        this.dist=dist;
        this.x = x;
        this.y = y;
    }

    public RelativePostion(int angle, int dist) {
		  this.angle=Math.toRadians(angle);
		  x=(int)Math.round(Math.cos(angle)*dist);
		  y=(int)Math.round(Math.sin(angle)*dist);
        dist=(int)Math.round(Math.sqrt(Math.pow(x,2)+Math.pow(y,2)));
    }

    public RelativePostion diff(RelativePostion rp) {
        return new RelativePostion(angle-rp.angle, dist-rp.dist,x-rp.x,y-rp.y);
    }

    public double norm () {
        return Math.sqrt(Math.pow(x,2)+Math.pow(y,2));
    }
}
