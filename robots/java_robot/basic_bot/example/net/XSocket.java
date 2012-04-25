package net;

/*
 *
 * net.XSocket
 * Written by Andreas Tarandi
 *
 * Version 1.2
 */

import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;

public class XSocket extends Socket{
    private XServerSocket srv=null;
    private Socket sck = null;
    private String host=null;
    private int port=0;
    private InetSocketAddress adrs = null;
    
    boolean server=false;
    boolean closeCalled=false;
            
    private boolean showOutput=true;
    private boolean debug=false;
    private DataOutputStream ostream = null;
    private BufferedReader istream = null;
    private XStreamParent xparent = null;
    private XStreamReciver xistream = null;
    private PrintStream out = System.out; 
    private char alive_char = '\n';
    
    public boolean ignoreEmpty = true;
    public int id = 0;
    
    private String charset="UTF-8";
    private String charset_send="UTF-8";
    

    /**
     * Creates an xsocket without initializing it
     */
    public XSocket() {    
    }

    /**
     * Creates an xsocket that listens for one client
     * @param port
     */
    public XSocket(int port) {
        listen(port);
    }

    /**
     * Creates an xsocket that connects to the given server and port
     * @param host
     * @param port
     */
    public XSocket(String host, int port) {
        connect(host,port);
    }

    /**
     * Creates an xsocket that connects to the given server and port and sets
     * xparent (and inits xstreams)
     * @param host
     * @param port
     */
    public XSocket(String host, int port,XStreamParent xstream) {
        xparent=xstream;
        connect(host,port);
        initXStreamReciver();
    }
    
    public void setPrintStream(PrintStream stream) {
        out = stream;
    }
    
    public void setCharset(String _charset) {
        charset = _charset;
        charset_send = _charset;
    }
    
    public String getCharset() {
        return charset;
    }
    
    public void setCharset(String _charset, String _send_charset) {
        charset = _charset;
        charset_send = _send_charset;
    }
    
    public boolean listen(int port) {
        this.port = port;
        try {
            server = true;
            srv = new XServerSocket(port);
            srv.accept(this);
            initStreams();
            return true;
        } catch (Exception e) {
            println("XSocket: Failed to listen@port "+port+" ("+e+")");
            return false;
        }
    }
    
    public void setDelay(long delay) {
        if(xistream != null)
            xistream.delay = delay;
        else
            println("XSocket: Delay can only be set if a XStreamReciver is created.");
    }
    
     public void setIdle(long waituntil,long sleep) {
        if(xistream != null) {
            xistream.idleMode = waituntil;
            xistream.idleDelay = sleep;
        } else {
            println("XSocket: Idle-delay can only be set if a XStreamReciver is created.");
        }
    }   
    
    public boolean connect(String host, int port) {
        this.host = host;
        this.port = port;
        try {
            adrs = new InetSocketAddress(host,port);
            super.connect(adrs,port);
            initStreams();
            return true;
        } catch (Exception e) {
            println("XSocket: Failed to connect to "+host+":"+port+" ("+e+")");
            return false;
        }
    }
     void println(String line) {
        print(line+'\n');
    }
    
    void print(String data) {
        if(showOutput) {
            out.print(data);
        }
    }
    
    public void showOutput() {
        showOutput = true;
    }
    
    public void hideOutput() {
        showOutput = false;
    }
    
    public boolean isOutputOn() {
        return showOutput;
    }
    
    public void showDebug() {
        debug = true;
        showOutput();
    }
    
    public void hideDebug() {
        debug = false;
    }
    
    void initStreams() throws IOException {
            istream = new BufferedReader(new InputStreamReader(super.getInputStream()));
            ostream = new DataOutputStream(super.getOutputStream());
    }
    
    public boolean write(String str) {
        try {
            if(ostream==null) {
                initStreams();
            }
            byte[] data = str.getBytes(charset_send);
            ostream.write(data);
            ostream.flush();
            return true;
        } catch (Exception e) {
            println("XSocket: Failed to write data ("+e+")");
            return false;
        }
    }
    
    public boolean writeln(String str) {
        return write(str+'\n');
    }
    
    public boolean write(byte[] bytes) {
        try {
            if(ostream==null) {
                initStreams();
            }
            ostream.write(bytes);
            ostream.flush();
            return true;
        } catch (Exception e) {
            println("XSocket: Failed to write data ("+e+")");
            return false;
        }
    }
    
    public String readLine() {
        if(!sckClosed()) { 
            try {
                if(istream==null) {
                    initStreams();
                }
                return istream.readLine();
            } catch (Exception e) {
                println("XSocket: Failed to read data ("+e+")");
                return null;
            }
        } else {
            println("XSocket: Failed to read data (Socket Closed)");
        }
        return null;
    }
    
    public String readLineOrWait() {
        if(!sckClosed()) { 
            try {
                if(istream==null) {
                    initStreams();
                }
                String str=null;
                while (str==null || (ignoreEmpty && str.trim().equals(""))) {
                    str = istream.readLine();
                }
                 return str;
            } catch (Exception e) {
                println("XSocket: Failed to read data ("+e+")");
                return null;
            }      
        } else {
            println("XSocket: Failed to read data (Socket Closed)");
        }
        return null;
    }
    
    public boolean sckClosed() {
        if(super.isClosed()) {
            return true;
        }
        
        try {
            ostream.writeChar(alive_char);
            ostream.flush();
        } catch (IOException e) {
            try {  super.close(); } catch (IOException ex) { }
            return true;
        }
        return false;
    }
    
    
    
    public DataOutputStream getDataOutputStream() {
        try {
            return new DataOutputStream(super.getOutputStream());
        } catch (IOException ex) {
            println("XSocket: Failed to create new DataOutputStream ("+ex+")");
            return null;
        }
    }
    
    public BufferedReader getBufferedReader() {
        try {
            return new BufferedReader(new InputStreamReader(super.getInputStream(),charset));
        } catch (IOException ex) {
            println("XSocket: Failed to create new BufferedReader ("+ex+")");
            return null;
        }
    }
    
    void dataRecived(String data,ByteBuffer b) {
        if(! (ignoreEmpty && data.trim().equals(""))) {
            xparent.dataRecived(data,b,this);
        }
    }
    
    void dataSent(String data) {
        if(debug) { println(">>"+data); };
    }
    
    public void setParent(XStreamParent xparent) {
        this.xparent = xparent;
    }
    
    public void setParentInitX(XStreamParent xparent) {
        this.xparent = xparent;
        initXStreamReciver();
    }
    
    public void setId(int id) {
        this.id = id;
    }
    
    public int getId() {
        return id;
    }
    
    boolean gotParent() {
        if(xparent != null) {
            return true;
        } else {
            return false;
        }
    }
    
    void reconnect() {
        if(xistream != null) {
            xistream.interrupt();
            xistream = null;
        }
        istream = null;
        ostream = null;
        
        if(!closeCalled) {
            closeCalled = true;
            if(xparent != null) {
                if(!server) {
                    xparent.connectionClosed(this);
                    xparent.reconnect(this);
                } else {
                    xparent.connectionClosed(this);
                }
            } else {
                println("XSocket: Connection closed");
            }
        }
    }
    
    public XStreamReciver getXStreamReciver() {
        if(xparent != null) {
            if(xistream == null)
                initXStreamReciver();
            return xistream;
        } else {
            println("XSocket: Failed to create new XStreamReciver (No XStreamParent set, use setParent())");
            return null;
        }
    }
    
    public void initXStreamReciver() {
        if(xparent != null) {
            xistream = new XStreamReciver(this);
        } else {
            println("XSocket: Failed to create new XStreamReciver (No XStreamParent set, use setParent())");
        }
    }

    
    public void close() throws IOException {
        if(xistream != null) {
            xistream.interrupt();
        }
        
        super.close();
    }
}


