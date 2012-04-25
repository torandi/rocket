package rocket;

/**
 *
 * @author torandi
 */
public interface ClientInterface{
    public void connected();
    public String clientName();
    public void ready();
    public void noScanResult();
    public void scanResult(RelativePostion rp);
    public void error(Exception e);
    public void dead();
}
