import org.xmlpull.v1.XmlPullParserFactory;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.nio.charset.StandardCharsets;

public class PullParserFactoryFuzzer {
    public static void fuzzerTestOneInput(byte[] data) {
        if (data == null || data.length == 0) {
            return;
        }
        int off = 0;
        int len = Math.min(100, data.length);
        String type = new String(data, off, len, StandardCharsets.UTF_8);
        off += len;
        boolean feature = off < data.length && (data[off++] & 1) != 0;
        String featureName = off < data.length
            ? new String(data, off, Math.min(30, data.length - off), StandardCharsets.UTF_8)
            : "http://xmlpull.org/v1/doc/features.html#process-namespaces";
        boolean nsAware = off < data.length && (data[off] & 1) != 0;
        try {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance(type, null);
            factory.setFeature(featureName, feature);
            factory.getFeature(featureName);
            factory.setNamespaceAware(nsAware);
            XmlPullParser xpp = factory.newPullParser();
        } catch (XmlPullParserException e) {
        }
    }
}
