package net;

import java.nio.ByteBuffer;

public interface XStreamParent {
    public void dataRecived(String data, ByteBuffer b, XSocket sck);
    /**
     * Called when a new client connects. The client is already accepted when
     * this method is called.
     * @param client
     * @param srvr
     */
    public void newClient(XSocket client, XServerSocket srvr);
    public void reconnect(XSocket sck);
    public void connectionClosed(XSocket sck);
}

/*
    public void dataRecived(String data, XSocket sck) { }
    public void newClient(XSocket client, XServerSocket srvr) { }
    public void reconnect(XSocket sck) { }
    public void connectionClosed(XSocket sck) { }
 */
