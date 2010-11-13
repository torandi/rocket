package robot;

/**
 *
 * @author torandi
 */
public class Main{

    public static void main(String args[]) {
		if(args.length<1) {
			System.err.println("Misisng first argument: server");
		}
		String server=args[0];
        new Bot(server);
    }


}
