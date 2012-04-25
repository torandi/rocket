package net;

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.charset.*;

public class XStreamReciver implements Runnable {
    public  Thread thread= new Thread(this);
    private boolean run = true;
    private BufferedInputStream stream=null;
    private XSocket sck = null;
    private StreamHndl lastHndl=null;
    private long lastInData = 0;
    public long idleMode = 5000;
    public long delay = 100;
    public long idleDelay = 1000;
    CharsetDecoder decoder = null;
    
    public XStreamReciver(XSocket sck){
        if(!sck.gotParent()) {
            sck.println("XStreamReciver: XSocket has no parent!");
            return;
        }
        this.sck = sck;
        decoder = Charset.forName(sck.getCharset()).newDecoder();
        decoder.onUnmappableCharacter(CodingErrorAction.IGNORE);
        decoder.onMalformedInput(CodingErrorAction.IGNORE);
        if(sck.isConnected()) {
                    try {
                        stream = new BufferedInputStream(sck.getInputStream());
                    } catch (IOException ex) {
                        sck.println("XStreamReciver: Error in init() ("+ex+")");
                    }
        }
        thread.start();
    }
    
    public void interrupt() {
        run = false;
    }
    
    public void run() {
        String[] data;
        while(!Thread.interrupted() && run) {
            if(sck.isConnected()) {
                if(stream == null && !sck.isClosed()) {
                    try {
                        stream = new BufferedInputStream(sck.getInputStream());
                    } catch (IOException ex) {
                        sck.println("XStreamReciver: Error in run() (init) ("+ex+")");
                    }
                }
                try {
                    if(!sck.isClosed()) {
                        byte[] b = new byte[1024];
                        
                        if(stream.read(b) != -1) {
                            ByteBuffer bb = ByteBuffer.wrap(b);
                            ByteBuffer b2 = bb.duplicate();
                            data = decoder.decode(bb).toString().split("\n");
                            
                            for(int i=0;i<data.length;i++) {
                                if(!data[i].trim().equals("")) {
                                    lastHndl = new StreamHndl(sck,data[i],b2);
                                    lastInData = System.currentTimeMillis();
                                }
                            }
                            
                        }
                    }
                } catch (SocketException e) {
                    if(run) {
                        sck.println("XStreamReciver: Error in run() ("+e+")");
                        sck.reconnect();
                    }
                    thread.interrupt();
                    run = false;
                } catch (Exception e) {
                    e.printStackTrace();
                    if(run) {
                        sck.println("XStreamReciver: Error in run() ("+e+")");
                    }
                }
            }
            try {
                if(lastInData+idleMode < System.currentTimeMillis()) {
                    Thread.sleep(idleDelay);
                }   else {
                    Thread.sleep(delay);
                }
            } catch (InterruptedException e) {
                
            }
        }
        try {
            stream.close();
        } catch (Exception e) { }
    }
}

class StreamHndl implements Runnable {
    public Thread thread = new Thread(this);
    private XSocket sck=null;
    private String data=null;
    private ByteBuffer buff=null;
    
    public StreamHndl(XSocket sck,String data,ByteBuffer b) {
        this.sck = sck;
        this.data = data;
        buff = b;
        thread.start();
    }
    
    public void run() {
        sck.dataRecived(data,buff);
    }
}
