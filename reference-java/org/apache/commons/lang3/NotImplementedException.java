/*
 * Decompiled with CFR 0.152.
 */
package org.apache.commons.lang3;

public class NotImplementedException
extends UnsupportedOperationException {
    private static final long serialVersionUID = 20131021L;
    private String code;

    public NotImplementedException(String message) {
        super(message);
    }

    public NotImplementedException(Throwable cause) {
        super(cause);
    }

    public NotImplementedException(String message, Throwable cause) {
        super(message, cause);
    }

    public NotImplementedException(String message, String code) {
        super(message);
        this.code = code;
    }

    public NotImplementedException(Throwable cause, String code) {
        super(cause);
        this.code = code;
    }

    public NotImplementedException(String message, Throwable cause, String code) {
        super(message, cause);
        this.code = code;
    }

    public String getCode() {
        return this.code;
    }
}

