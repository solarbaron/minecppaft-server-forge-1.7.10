/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.handler.codec.http.HttpHeaderDateFormat;
import io.netty.handler.codec.http.HttpMessage;
import io.netty.handler.codec.http.HttpMethod;
import io.netty.handler.codec.http.HttpRequest;
import io.netty.handler.codec.http.HttpResponse;
import io.netty.handler.codec.http.HttpVersion;
import java.text.ParseException;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

public abstract class HttpHeaders
implements Iterable<Map.Entry<String, String>> {
    public static final HttpHeaders EMPTY_HEADERS = new HttpHeaders(){

        @Override
        public String get(String name) {
            return null;
        }

        @Override
        public List<String> getAll(String name) {
            return Collections.emptyList();
        }

        @Override
        public List<Map.Entry<String, String>> entries() {
            return Collections.emptyList();
        }

        @Override
        public boolean contains(String name) {
            return false;
        }

        @Override
        public boolean isEmpty() {
            return true;
        }

        @Override
        public Set<String> names() {
            return Collections.emptySet();
        }

        @Override
        public HttpHeaders add(String name, Object value) {
            throw new UnsupportedOperationException("read only");
        }

        @Override
        public HttpHeaders add(String name, Iterable<?> values) {
            throw new UnsupportedOperationException("read only");
        }

        @Override
        public HttpHeaders set(String name, Object value) {
            throw new UnsupportedOperationException("read only");
        }

        @Override
        public HttpHeaders set(String name, Iterable<?> values) {
            throw new UnsupportedOperationException("read only");
        }

        @Override
        public HttpHeaders remove(String name) {
            throw new UnsupportedOperationException("read only");
        }

        @Override
        public HttpHeaders clear() {
            throw new UnsupportedOperationException("read only");
        }

        @Override
        public Iterator<Map.Entry<String, String>> iterator() {
            return this.entries().iterator();
        }
    };

    public static boolean isKeepAlive(HttpMessage message) {
        String connection = message.headers().get("Connection");
        if ("close".equalsIgnoreCase(connection)) {
            return false;
        }
        if (message.getProtocolVersion().isKeepAliveDefault()) {
            return !"close".equalsIgnoreCase(connection);
        }
        return "keep-alive".equalsIgnoreCase(connection);
    }

    public static void setKeepAlive(HttpMessage message, boolean keepAlive) {
        HttpHeaders h2 = message.headers();
        if (message.getProtocolVersion().isKeepAliveDefault()) {
            if (keepAlive) {
                h2.remove("Connection");
            } else {
                h2.set("Connection", "close");
            }
        } else if (keepAlive) {
            h2.set("Connection", "keep-alive");
        } else {
            h2.remove("Connection");
        }
    }

    public static String getHeader(HttpMessage message, String name) {
        return message.headers().get(name);
    }

    public static String getHeader(HttpMessage message, String name, String defaultValue) {
        String value = message.headers().get(name);
        if (value == null) {
            return defaultValue;
        }
        return value;
    }

    public static void setHeader(HttpMessage message, String name, Object value) {
        message.headers().set(name, value);
    }

    public static void setHeader(HttpMessage message, String name, Iterable<?> values) {
        message.headers().set(name, values);
    }

    public static void addHeader(HttpMessage message, String name, Object value) {
        message.headers().add(name, value);
    }

    public static void removeHeader(HttpMessage message, String name) {
        message.headers().remove(name);
    }

    public static void clearHeaders(HttpMessage message) {
        message.headers().clear();
    }

    public static int getIntHeader(HttpMessage message, String name) {
        String value = HttpHeaders.getHeader(message, name);
        if (value == null) {
            throw new NumberFormatException("header not found: " + name);
        }
        return Integer.parseInt(value);
    }

    public static int getIntHeader(HttpMessage message, String name, int defaultValue) {
        String value = HttpHeaders.getHeader(message, name);
        if (value == null) {
            return defaultValue;
        }
        try {
            return Integer.parseInt(value);
        }
        catch (NumberFormatException e2) {
            return defaultValue;
        }
    }

    public static void setIntHeader(HttpMessage message, String name, int value) {
        message.headers().set(name, value);
    }

    public static void setIntHeader(HttpMessage message, String name, Iterable<Integer> values) {
        message.headers().set(name, values);
    }

    public static void addIntHeader(HttpMessage message, String name, int value) {
        message.headers().add(name, value);
    }

    public static Date getDateHeader(HttpMessage message, String name) throws ParseException {
        String value = HttpHeaders.getHeader(message, name);
        if (value == null) {
            throw new ParseException("header not found: " + name, 0);
        }
        return HttpHeaderDateFormat.get().parse(value);
    }

    public static Date getDateHeader(HttpMessage message, String name, Date defaultValue) {
        String value = HttpHeaders.getHeader(message, name);
        if (value == null) {
            return defaultValue;
        }
        try {
            return HttpHeaderDateFormat.get().parse(value);
        }
        catch (ParseException e2) {
            return defaultValue;
        }
    }

    public static void setDateHeader(HttpMessage message, String name, Date value) {
        if (value != null) {
            message.headers().set(name, HttpHeaderDateFormat.get().format(value));
        } else {
            message.headers().set(name, null);
        }
    }

    public static void setDateHeader(HttpMessage message, String name, Iterable<Date> values) {
        message.headers().set(name, values);
    }

    public static void addDateHeader(HttpMessage message, String name, Date value) {
        message.headers().add(name, value);
    }

    public static long getContentLength(HttpMessage message) {
        String value = HttpHeaders.getHeader(message, "Content-Length");
        if (value != null) {
            return Long.parseLong(value);
        }
        long webSocketContentLength = HttpHeaders.getWebSocketContentLength(message);
        if (webSocketContentLength >= 0L) {
            return webSocketContentLength;
        }
        throw new NumberFormatException("header not found: Content-Length");
    }

    public static long getContentLength(HttpMessage message, long defaultValue) {
        String contentLength = message.headers().get("Content-Length");
        if (contentLength != null) {
            try {
                return Long.parseLong(contentLength);
            }
            catch (NumberFormatException e2) {
                return defaultValue;
            }
        }
        long webSocketContentLength = HttpHeaders.getWebSocketContentLength(message);
        if (webSocketContentLength >= 0L) {
            return webSocketContentLength;
        }
        return defaultValue;
    }

    private static int getWebSocketContentLength(HttpMessage message) {
        HttpResponse res;
        HttpHeaders h2 = message.headers();
        if (message instanceof HttpRequest) {
            HttpRequest req = (HttpRequest)message;
            if (HttpMethod.GET.equals(req.getMethod()) && h2.contains("Sec-WebSocket-Key1") && h2.contains("Sec-WebSocket-Key2")) {
                return 8;
            }
        } else if (message instanceof HttpResponse && (res = (HttpResponse)message).getStatus().code() == 101 && h2.contains("Sec-WebSocket-Origin") && h2.contains("Sec-WebSocket-Location")) {
            return 16;
        }
        return -1;
    }

    public static void setContentLength(HttpMessage message, long length) {
        message.headers().set("Content-Length", length);
    }

    public static String getHost(HttpMessage message) {
        return message.headers().get("Host");
    }

    public static String getHost(HttpMessage message, String defaultValue) {
        return HttpHeaders.getHeader(message, "Host", defaultValue);
    }

    public static void setHost(HttpMessage message, String value) {
        message.headers().set("Host", value);
    }

    public static Date getDate(HttpMessage message) throws ParseException {
        return HttpHeaders.getDateHeader(message, "Date");
    }

    public static Date getDate(HttpMessage message, Date defaultValue) {
        return HttpHeaders.getDateHeader(message, "Date", defaultValue);
    }

    public static void setDate(HttpMessage message, Date value) {
        if (value != null) {
            message.headers().set("Date", HttpHeaderDateFormat.get().format(value));
        } else {
            message.headers().set("Date", null);
        }
    }

    public static boolean is100ContinueExpected(HttpMessage message) {
        if (!(message instanceof HttpRequest)) {
            return false;
        }
        if (message.getProtocolVersion().compareTo(HttpVersion.HTTP_1_1) < 0) {
            return false;
        }
        String value = message.headers().get("Expect");
        if (value == null) {
            return false;
        }
        if ("100-continue".equalsIgnoreCase(value)) {
            return true;
        }
        for (String v : message.headers().getAll("Expect")) {
            if (!"100-continue".equalsIgnoreCase(v)) continue;
            return true;
        }
        return false;
    }

    public static void set100ContinueExpected(HttpMessage message) {
        HttpHeaders.set100ContinueExpected(message, true);
    }

    public static void set100ContinueExpected(HttpMessage message, boolean set) {
        if (set) {
            message.headers().set("Expect", "100-continue");
        } else {
            message.headers().remove("Expect");
        }
    }

    static void validateHeaderName(String headerName) {
        if (headerName == null) {
            throw new NullPointerException("Header names cannot be null");
        }
        for (int index = 0; index < headerName.length(); ++index) {
            char character = headerName.charAt(index);
            if (character > '\u007f') {
                throw new IllegalArgumentException("Header name cannot contain non-ASCII characters: " + headerName);
            }
            switch (character) {
                case '\t': 
                case '\n': 
                case '\u000b': 
                case '\f': 
                case '\r': 
                case ' ': 
                case ',': 
                case ':': 
                case ';': 
                case '=': {
                    throw new IllegalArgumentException("Header name cannot contain the following prohibited characters: =,;: \\t\\r\\n\\v\\f: " + headerName);
                }
            }
        }
    }

    static void validateHeaderValue(String headerValue) {
        if (headerValue == null) {
            throw new NullPointerException("Header values cannot be null");
        }
        int state = 0;
        block19: for (int index = 0; index < headerValue.length(); ++index) {
            char character = headerValue.charAt(index);
            switch (character) {
                case '\u000b': {
                    throw new IllegalArgumentException("Header value contains a prohibited character '\\v': " + headerValue);
                }
                case '\f': {
                    throw new IllegalArgumentException("Header value contains a prohibited character '\\f': " + headerValue);
                }
            }
            switch (state) {
                case 0: {
                    switch (character) {
                        case '\r': {
                            state = 1;
                            break;
                        }
                        case '\n': {
                            state = 2;
                        }
                    }
                    continue block19;
                }
                case 1: {
                    switch (character) {
                        case '\n': {
                            state = 2;
                            continue block19;
                        }
                    }
                    throw new IllegalArgumentException("Only '\\n' is allowed after '\\r': " + headerValue);
                }
                case 2: {
                    switch (character) {
                        case '\t': 
                        case ' ': {
                            state = 0;
                            continue block19;
                        }
                    }
                    throw new IllegalArgumentException("Only ' ' and '\\t' are allowed after '\\n': " + headerValue);
                }
            }
        }
        if (state != 0) {
            throw new IllegalArgumentException("Header value must not end with '\\r' or '\\n':" + headerValue);
        }
    }

    public static boolean isTransferEncodingChunked(HttpMessage message) {
        List<String> transferEncodingHeaders = message.headers().getAll("Transfer-Encoding");
        if (transferEncodingHeaders.isEmpty()) {
            return false;
        }
        for (String value : transferEncodingHeaders) {
            if (!value.equalsIgnoreCase("chunked")) continue;
            return true;
        }
        return false;
    }

    public static void removeTransferEncodingChunked(HttpMessage m2) {
        List<String> values = m2.headers().getAll("Transfer-Encoding");
        if (values.isEmpty()) {
            return;
        }
        Iterator<String> valuesIt = values.iterator();
        while (valuesIt.hasNext()) {
            String value = valuesIt.next();
            if (!value.equalsIgnoreCase("chunked")) continue;
            valuesIt.remove();
        }
        if (values.isEmpty()) {
            m2.headers().remove("Transfer-Encoding");
        } else {
            m2.headers().set("Transfer-Encoding", values);
        }
    }

    public static void setTransferEncodingChunked(HttpMessage m2) {
        HttpHeaders.addHeader(m2, "Transfer-Encoding", "chunked");
        HttpHeaders.removeHeader(m2, "Content-Length");
    }

    public static boolean isContentLengthSet(HttpMessage m2) {
        List<String> contentLength = m2.headers().getAll("Content-Length");
        return !contentLength.isEmpty();
    }

    protected HttpHeaders() {
    }

    public abstract String get(String var1);

    public abstract List<String> getAll(String var1);

    public abstract List<Map.Entry<String, String>> entries();

    public abstract boolean contains(String var1);

    public abstract boolean isEmpty();

    public abstract Set<String> names();

    public abstract HttpHeaders add(String var1, Object var2);

    public abstract HttpHeaders add(String var1, Iterable<?> var2);

    public HttpHeaders add(HttpHeaders headers) {
        if (headers == null) {
            throw new NullPointerException("headers");
        }
        for (Map.Entry e2 : headers) {
            this.add((String)e2.getKey(), e2.getValue());
        }
        return this;
    }

    public abstract HttpHeaders set(String var1, Object var2);

    public abstract HttpHeaders set(String var1, Iterable<?> var2);

    public HttpHeaders set(HttpHeaders headers) {
        if (headers == null) {
            throw new NullPointerException("headers");
        }
        this.clear();
        for (Map.Entry e2 : headers) {
            this.add((String)e2.getKey(), e2.getValue());
        }
        return this;
    }

    public abstract HttpHeaders remove(String var1);

    public abstract HttpHeaders clear();

    public static final class Values {
        public static final String APPLICATION_X_WWW_FORM_URLENCODED = "application/x-www-form-urlencoded";
        public static final String BASE64 = "base64";
        public static final String BINARY = "binary";
        public static final String BOUNDARY = "boundary";
        public static final String BYTES = "bytes";
        public static final String CHARSET = "charset";
        public static final String CHUNKED = "chunked";
        public static final String CLOSE = "close";
        public static final String COMPRESS = "compress";
        public static final String CONTINUE = "100-continue";
        public static final String DEFLATE = "deflate";
        public static final String GZIP = "gzip";
        public static final String IDENTITY = "identity";
        public static final String KEEP_ALIVE = "keep-alive";
        public static final String MAX_AGE = "max-age";
        public static final String MAX_STALE = "max-stale";
        public static final String MIN_FRESH = "min-fresh";
        public static final String MULTIPART_FORM_DATA = "multipart/form-data";
        public static final String MUST_REVALIDATE = "must-revalidate";
        public static final String NO_CACHE = "no-cache";
        public static final String NO_STORE = "no-store";
        public static final String NO_TRANSFORM = "no-transform";
        public static final String NONE = "none";
        public static final String ONLY_IF_CACHED = "only-if-cached";
        public static final String PRIVATE = "private";
        public static final String PROXY_REVALIDATE = "proxy-revalidate";
        public static final String PUBLIC = "public";
        public static final String QUOTED_PRINTABLE = "quoted-printable";
        public static final String S_MAXAGE = "s-maxage";
        public static final String TRAILERS = "trailers";
        public static final String UPGRADE = "Upgrade";
        public static final String WEBSOCKET = "WebSocket";

        private Values() {
        }
    }

    public static final class Names {
        public static final String ACCEPT = "Accept";
        public static final String ACCEPT_CHARSET = "Accept-Charset";
        public static final String ACCEPT_ENCODING = "Accept-Encoding";
        public static final String ACCEPT_LANGUAGE = "Accept-Language";
        public static final String ACCEPT_RANGES = "Accept-Ranges";
        public static final String ACCEPT_PATCH = "Accept-Patch";
        public static final String ACCESS_CONTROL_ALLOW_CREDENTIALS = "Access-Control-Allow-Credentials";
        public static final String ACCESS_CONTROL_ALLOW_HEADERS = "Access-Control-Allow-Headers";
        public static final String ACCESS_CONTROL_ALLOW_METHODS = "Access-Control-Allow-Methods";
        public static final String ACCESS_CONTROL_ALLOW_ORIGIN = "Access-Control-Allow-Origin";
        public static final String ACCESS_CONTROL_EXPOSE_HEADERS = "Access-Control-Expose-Headers";
        public static final String ACCESS_CONTROL_MAX_AGE = "Access-Control-Max-Age";
        public static final String ACCESS_CONTROL_REQUEST_HEADERS = "Access-Control-Request-Headers";
        public static final String ACCESS_CONTROL_REQUEST_METHOD = "Access-Control-Request-Method";
        public static final String AGE = "Age";
        public static final String ALLOW = "Allow";
        public static final String AUTHORIZATION = "Authorization";
        public static final String CACHE_CONTROL = "Cache-Control";
        public static final String CONNECTION = "Connection";
        public static final String CONTENT_BASE = "Content-Base";
        public static final String CONTENT_ENCODING = "Content-Encoding";
        public static final String CONTENT_LANGUAGE = "Content-Language";
        public static final String CONTENT_LENGTH = "Content-Length";
        public static final String CONTENT_LOCATION = "Content-Location";
        public static final String CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding";
        public static final String CONTENT_MD5 = "Content-MD5";
        public static final String CONTENT_RANGE = "Content-Range";
        public static final String CONTENT_TYPE = "Content-Type";
        public static final String COOKIE = "Cookie";
        public static final String DATE = "Date";
        public static final String ETAG = "ETag";
        public static final String EXPECT = "Expect";
        public static final String EXPIRES = "Expires";
        public static final String FROM = "From";
        public static final String HOST = "Host";
        public static final String IF_MATCH = "If-Match";
        public static final String IF_MODIFIED_SINCE = "If-Modified-Since";
        public static final String IF_NONE_MATCH = "If-None-Match";
        public static final String IF_RANGE = "If-Range";
        public static final String IF_UNMODIFIED_SINCE = "If-Unmodified-Since";
        public static final String LAST_MODIFIED = "Last-Modified";
        public static final String LOCATION = "Location";
        public static final String MAX_FORWARDS = "Max-Forwards";
        public static final String ORIGIN = "Origin";
        public static final String PRAGMA = "Pragma";
        public static final String PROXY_AUTHENTICATE = "Proxy-Authenticate";
        public static final String PROXY_AUTHORIZATION = "Proxy-Authorization";
        public static final String RANGE = "Range";
        public static final String REFERER = "Referer";
        public static final String RETRY_AFTER = "Retry-After";
        public static final String SEC_WEBSOCKET_KEY1 = "Sec-WebSocket-Key1";
        public static final String SEC_WEBSOCKET_KEY2 = "Sec-WebSocket-Key2";
        public static final String SEC_WEBSOCKET_LOCATION = "Sec-WebSocket-Location";
        public static final String SEC_WEBSOCKET_ORIGIN = "Sec-WebSocket-Origin";
        public static final String SEC_WEBSOCKET_PROTOCOL = "Sec-WebSocket-Protocol";
        public static final String SEC_WEBSOCKET_VERSION = "Sec-WebSocket-Version";
        public static final String SEC_WEBSOCKET_KEY = "Sec-WebSocket-Key";
        public static final String SEC_WEBSOCKET_ACCEPT = "Sec-WebSocket-Accept";
        public static final String SERVER = "Server";
        public static final String SET_COOKIE = "Set-Cookie";
        public static final String SET_COOKIE2 = "Set-Cookie2";
        public static final String TE = "TE";
        public static final String TRAILER = "Trailer";
        public static final String TRANSFER_ENCODING = "Transfer-Encoding";
        public static final String UPGRADE = "Upgrade";
        public static final String USER_AGENT = "User-Agent";
        public static final String VARY = "Vary";
        public static final String VIA = "Via";
        public static final String WARNING = "Warning";
        public static final String WEBSOCKET_LOCATION = "WebSocket-Location";
        public static final String WEBSOCKET_ORIGIN = "WebSocket-Origin";
        public static final String WEBSOCKET_PROTOCOL = "WebSocket-Protocol";
        public static final String WWW_AUTHENTICATE = "WWW-Authenticate";

        private Names() {
        }
    }
}

