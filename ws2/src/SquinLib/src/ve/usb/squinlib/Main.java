package ve.usb.squinlib;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * @author Daniel Izquierdo
 */
public class Main {

    public static void main(String[] args) {
        //SparqlQuerySock sqs = new SparqlQuerySock("http://localhost:21786/SQUIN/query");
        SparqlQuerySock sqs = new SparqlQuerySock("http://ijaz.ldc.usb.ve:8080/SQUIN/query");

        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
        StringBuffer sb = new StringBuffer();
        String line;

        try {
            while ((line = in.readLine()) != null) {
                sb.append(line + '\n');
            }

            String result = sqs.call(sb.toString());

            System.out.println(result);
        } catch (Exception ex) {
            System.out.println(ex);
        }
    }
}
