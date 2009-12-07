package ve.usb.squinlib;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * @author Daniel Izquierdo
 */
public class SparqlQuerySock {

    private final String queryServer;
    private final static Logger logger = Logger.getLogger(SparqlQuerySock.class.getCanonicalName());

    public SparqlQuerySock(final String queryServer) {
        logger.setLevel(Level.FINE);
        this.queryServer = queryServer;
    }

    public String call(final String query) throws MalformedURLException, IOException {
        logger.log(Level.INFO, "SparqlQuerySock.call(...)");

        Map<String, String> request_params = new HashMap<String, String>();
        request_params.put("query", query);

        logger.log(Level.FINE, "query server: " + queryServer);
        logger.log(Level.FINE, "query: " + query);

        String requestUrl = createQueryURLString(this.queryServer, request_params);

        logger.log(Level.FINE, "request url: " + requestUrl);

        URL url = new URL(requestUrl);

        BufferedReader in = new BufferedReader(new InputStreamReader(url.openStream()));
        StringBuffer output = new StringBuffer();
        String line;

        while ((line = in.readLine()) != null) {
            output.append(line);
        }

        return output.toString();
    }

    public static String createQueryURLString(String url, Map params) {
        return url + '?' + createQueryString(params);
    }

    public static String createQueryString(Map params) {
        StringBuffer queryString = new StringBuffer();
        boolean first = true;

        for (Object key : params.keySet()) {
            if (first) {
                first = false;
            } else {
                queryString.append('&');
            }
            try {
                queryString.append(key.toString() + '=' + URLEncoder.encode(params.get(key).toString(), "UTF-8"));
            } catch (UnsupportedEncodingException ex) {
                /* realmente no debería pasar */
                Logger.getLogger(SparqlQuerySock.class.getName()).log(Level.SEVERE, null, ex);
            }
        }

        return queryString.toString();
    }
}
