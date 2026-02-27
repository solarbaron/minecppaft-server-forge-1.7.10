/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import java.util.HashMap;
import java.util.Map;

public class HttpMethod
implements Comparable<HttpMethod> {
    public static final HttpMethod OPTIONS = new HttpMethod("OPTIONS");
    public static final HttpMethod GET = new HttpMethod("GET");
    public static final HttpMethod HEAD = new HttpMethod("HEAD");
    public static final HttpMethod POST = new HttpMethod("POST");
    public static final HttpMethod PUT = new HttpMethod("PUT");
    public static final HttpMethod PATCH = new HttpMethod("PATCH");
    public static final HttpMethod DELETE = new HttpMethod("DELETE");
    public static final HttpMethod TRACE = new HttpMethod("TRACE");
    public static final HttpMethod CONNECT = new HttpMethod("CONNECT");
    private static final Map<String, HttpMethod> methodMap = new HashMap<String, HttpMethod>();
    private final String name;

    public static HttpMethod valueOf(String name) {
        if (name == null) {
            throw new NullPointerException("name");
        }
        if ((name = name.trim()).isEmpty()) {
            throw new IllegalArgumentException("empty name");
        }
        HttpMethod result = methodMap.get(name);
        if (result != null) {
            return result;
        }
        return new HttpMethod(name);
    }

    public HttpMethod(String name) {
        if (name == null) {
            throw new NullPointerException("name");
        }
        if ((name = name.trim()).isEmpty()) {
            throw new IllegalArgumentException("empty name");
        }
        for (int i2 = 0; i2 < name.length(); ++i2) {
            if (!Character.isISOControl(name.charAt(i2)) && !Character.isWhitespace(name.charAt(i2))) continue;
            throw new IllegalArgumentException("invalid character in name");
        }
        this.name = name;
    }

    public String name() {
        return this.name;
    }

    public int hashCode() {
        return this.name().hashCode();
    }

    public boolean equals(Object o2) {
        if (!(o2 instanceof HttpMethod)) {
            return false;
        }
        HttpMethod that = (HttpMethod)o2;
        return this.name().equals(that.name());
    }

    public String toString() {
        return this.name();
    }

    @Override
    public int compareTo(HttpMethod o2) {
        return this.name().compareTo(o2.name());
    }

    static {
        methodMap.put(OPTIONS.toString(), OPTIONS);
        methodMap.put(GET.toString(), GET);
        methodMap.put(HEAD.toString(), HEAD);
        methodMap.put(POST.toString(), POST);
        methodMap.put(PUT.toString(), PUT);
        methodMap.put(PATCH.toString(), PATCH);
        methodMap.put(DELETE.toString(), DELETE);
        methodMap.put(TRACE.toString(), TRACE);
        methodMap.put(CONNECT.toString(), CONNECT);
    }
}

