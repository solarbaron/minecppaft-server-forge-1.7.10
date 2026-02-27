/*
 * Decompiled with CFR 0.152.
 */
package com.google.common.reflect;

import com.google.common.annotations.Beta;
import com.google.common.base.Joiner;
import com.google.common.base.Preconditions;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Maps;
import com.google.common.reflect.TypeVisitor;
import com.google.common.reflect.Types;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import javax.annotation.Nullable;

@Beta
public final class TypeResolver {
    private final TypeTable typeTable;

    public TypeResolver() {
        this.typeTable = new TypeTable();
    }

    private TypeResolver(TypeTable typeTable) {
        this.typeTable = typeTable;
    }

    static TypeResolver accordingTo(Type type) {
        return new TypeResolver().where(TypeMappingIntrospector.getTypeMappings(type));
    }

    public TypeResolver where(Type formal, Type actual) {
        HashMap<TypeVariable<?>, Type> mappings = Maps.newHashMap();
        TypeResolver.populateTypeMappings(mappings, Preconditions.checkNotNull(formal), Preconditions.checkNotNull(actual));
        return this.where(mappings);
    }

    TypeResolver where(Map<? extends TypeVariable<?>, ? extends Type> mappings) {
        return new TypeResolver(this.typeTable.where(mappings));
    }

    private static void populateTypeMappings(final Map<TypeVariable<?>, Type> mappings, Type from, final Type to2) {
        if (from.equals(to2)) {
            return;
        }
        new TypeVisitor(){

            @Override
            void visitTypeVariable(TypeVariable<?> typeVariable) {
                mappings.put(typeVariable, to2);
            }

            @Override
            void visitWildcardType(WildcardType fromWildcardType) {
                int i2;
                WildcardType toWildcardType = (WildcardType)TypeResolver.expectArgument(WildcardType.class, to2);
                Type[] fromUpperBounds = fromWildcardType.getUpperBounds();
                Type[] toUpperBounds = toWildcardType.getUpperBounds();
                Type[] fromLowerBounds = fromWildcardType.getLowerBounds();
                Type[] toLowerBounds = toWildcardType.getLowerBounds();
                Preconditions.checkArgument(fromUpperBounds.length == toUpperBounds.length && fromLowerBounds.length == toLowerBounds.length, "Incompatible type: %s vs. %s", fromWildcardType, to2);
                for (i2 = 0; i2 < fromUpperBounds.length; ++i2) {
                    TypeResolver.populateTypeMappings(mappings, fromUpperBounds[i2], toUpperBounds[i2]);
                }
                for (i2 = 0; i2 < fromLowerBounds.length; ++i2) {
                    TypeResolver.populateTypeMappings(mappings, fromLowerBounds[i2], toLowerBounds[i2]);
                }
            }

            @Override
            void visitParameterizedType(ParameterizedType fromParameterizedType) {
                ParameterizedType toParameterizedType = (ParameterizedType)TypeResolver.expectArgument(ParameterizedType.class, to2);
                Preconditions.checkArgument(fromParameterizedType.getRawType().equals(toParameterizedType.getRawType()), "Inconsistent raw type: %s vs. %s", fromParameterizedType, to2);
                Type[] fromArgs = fromParameterizedType.getActualTypeArguments();
                Type[] toArgs = toParameterizedType.getActualTypeArguments();
                Preconditions.checkArgument(fromArgs.length == toArgs.length, "%s not compatible with %s", fromParameterizedType, toParameterizedType);
                for (int i2 = 0; i2 < fromArgs.length; ++i2) {
                    TypeResolver.populateTypeMappings(mappings, fromArgs[i2], toArgs[i2]);
                }
            }

            @Override
            void visitGenericArrayType(GenericArrayType fromArrayType) {
                Type componentType = Types.getComponentType(to2);
                Preconditions.checkArgument(componentType != null, "%s is not an array type.", to2);
                TypeResolver.populateTypeMappings(mappings, fromArrayType.getGenericComponentType(), componentType);
            }

            @Override
            void visitClass(Class<?> fromClass) {
                throw new IllegalArgumentException("No type mapping from " + fromClass);
            }
        }.visit(from);
    }

    public Type resolveType(Type type) {
        Preconditions.checkNotNull(type);
        if (type instanceof TypeVariable) {
            return this.typeTable.resolve((TypeVariable)type);
        }
        if (type instanceof ParameterizedType) {
            return this.resolveParameterizedType((ParameterizedType)type);
        }
        if (type instanceof GenericArrayType) {
            return this.resolveGenericArrayType((GenericArrayType)type);
        }
        if (type instanceof WildcardType) {
            WildcardType wildcardType = (WildcardType)type;
            return new Types.WildcardTypeImpl(this.resolveTypes(wildcardType.getLowerBounds()), this.resolveTypes(wildcardType.getUpperBounds()));
        }
        return type;
    }

    private Type[] resolveTypes(Type[] types) {
        Type[] result = new Type[types.length];
        for (int i2 = 0; i2 < types.length; ++i2) {
            result[i2] = this.resolveType(types[i2]);
        }
        return result;
    }

    private Type resolveGenericArrayType(GenericArrayType type) {
        Type componentType = this.resolveType(type.getGenericComponentType());
        return Types.newArrayType(componentType);
    }

    private ParameterizedType resolveParameterizedType(ParameterizedType type) {
        Type owner = type.getOwnerType();
        Type resolvedOwner = owner == null ? null : this.resolveType(owner);
        Type resolvedRawType = this.resolveType(type.getRawType());
        Type[] vars = type.getActualTypeArguments();
        Type[] resolvedArgs = new Type[vars.length];
        for (int i2 = 0; i2 < vars.length; ++i2) {
            resolvedArgs[i2] = this.resolveType(vars[i2]);
        }
        return Types.newParameterizedTypeWithOwner(resolvedOwner, (Class)resolvedRawType, resolvedArgs);
    }

    private static <T> T expectArgument(Class<T> type, Object arg2) {
        try {
            return type.cast(arg2);
        }
        catch (ClassCastException e2) {
            throw new IllegalArgumentException(arg2 + " is not a " + type.getSimpleName());
        }
    }

    private static final class WildcardCapturer {
        private final AtomicInteger id = new AtomicInteger();

        private WildcardCapturer() {
        }

        Type capture(Type type) {
            Preconditions.checkNotNull(type);
            if (type instanceof Class) {
                return type;
            }
            if (type instanceof TypeVariable) {
                return type;
            }
            if (type instanceof GenericArrayType) {
                GenericArrayType arrayType = (GenericArrayType)type;
                return Types.newArrayType(this.capture(arrayType.getGenericComponentType()));
            }
            if (type instanceof ParameterizedType) {
                ParameterizedType parameterizedType = (ParameterizedType)type;
                return Types.newParameterizedTypeWithOwner(this.captureNullable(parameterizedType.getOwnerType()), (Class)parameterizedType.getRawType(), this.capture(parameterizedType.getActualTypeArguments()));
            }
            if (type instanceof WildcardType) {
                WildcardType wildcardType = (WildcardType)type;
                Type[] lowerBounds = wildcardType.getLowerBounds();
                if (lowerBounds.length == 0) {
                    Object[] upperBounds = wildcardType.getUpperBounds();
                    String name = "capture#" + this.id.incrementAndGet() + "-of ? extends " + Joiner.on('&').join(upperBounds);
                    return Types.newTypeVariable(WildcardCapturer.class, name, wildcardType.getUpperBounds());
                }
                return type;
            }
            throw new AssertionError((Object)"must have been one of the known types");
        }

        private Type captureNullable(@Nullable Type type) {
            if (type == null) {
                return null;
            }
            return this.capture(type);
        }

        private Type[] capture(Type[] types) {
            Type[] result = new Type[types.length];
            for (int i2 = 0; i2 < types.length; ++i2) {
                result[i2] = this.capture(types[i2]);
            }
            return result;
        }
    }

    private static final class TypeMappingIntrospector
    extends TypeVisitor {
        private static final WildcardCapturer wildcardCapturer = new WildcardCapturer();
        private final Map<TypeVariable<?>, Type> mappings = Maps.newHashMap();

        private TypeMappingIntrospector() {
        }

        static ImmutableMap<TypeVariable<?>, Type> getTypeMappings(Type contextType) {
            TypeMappingIntrospector introspector = new TypeMappingIntrospector();
            introspector.visit(wildcardCapturer.capture(contextType));
            return ImmutableMap.copyOf(introspector.mappings);
        }

        @Override
        void visitClass(Class<?> clazz) {
            this.visit(clazz.getGenericSuperclass());
            this.visit(clazz.getGenericInterfaces());
        }

        @Override
        void visitParameterizedType(ParameterizedType parameterizedType) {
            Type[] typeArgs;
            Class rawClass = (Class)parameterizedType.getRawType();
            TypeVariable<Class<T>>[] vars = rawClass.getTypeParameters();
            Preconditions.checkState(vars.length == (typeArgs = parameterizedType.getActualTypeArguments()).length);
            for (int i2 = 0; i2 < vars.length; ++i2) {
                this.map(vars[i2], typeArgs[i2]);
            }
            this.visit(rawClass);
            this.visit(parameterizedType.getOwnerType());
        }

        @Override
        void visitTypeVariable(TypeVariable<?> t2) {
            this.visit(t2.getBounds());
        }

        @Override
        void visitWildcardType(WildcardType t2) {
            this.visit(t2.getUpperBounds());
        }

        private void map(TypeVariable<?> var, Type arg2) {
            if (this.mappings.containsKey(var)) {
                return;
            }
            Type t2 = arg2;
            while (t2 != null) {
                if (var.equals(t2)) {
                    Type x2 = arg2;
                    while (x2 != null) {
                        x2 = this.mappings.remove(x2);
                    }
                    return;
                }
                t2 = this.mappings.get(t2);
            }
            this.mappings.put(var, arg2);
        }
    }

    private static class TypeTable {
        private final ImmutableMap<TypeVariable<?>, Type> map;

        TypeTable() {
            this.map = ImmutableMap.of();
        }

        private TypeTable(ImmutableMap<TypeVariable<?>, Type> map) {
            this.map = map;
        }

        final TypeTable where(Map<? extends TypeVariable<?>, ? extends Type> mappings) {
            ImmutableMap.Builder<TypeVariable<?>, Type> builder = ImmutableMap.builder();
            builder.putAll(this.map);
            for (Map.Entry<TypeVariable<?>, Type> mapping : mappings.entrySet()) {
                Type type;
                TypeVariable<?> variable = mapping.getKey();
                Preconditions.checkArgument(!variable.equals(type = mapping.getValue()), "Type variable %s bound to itself", variable);
                builder.put(variable, type);
            }
            return new TypeTable(builder.build());
        }

        final Type resolve(final TypeVariable<?> var) {
            final TypeTable unguarded = this;
            TypeTable guarded = new TypeTable(){

                @Override
                public Type resolveInternal(TypeVariable<?> intermediateVar, TypeTable forDependent) {
                    if (intermediateVar.getGenericDeclaration().equals(var.getGenericDeclaration())) {
                        return intermediateVar;
                    }
                    return unguarded.resolveInternal(intermediateVar, forDependent);
                }
            };
            return this.resolveInternal(var, guarded);
        }

        Type resolveInternal(TypeVariable<?> var, TypeTable forDependants) {
            Type type = this.map.get(var);
            if (type == null) {
                Type[] bounds = var.getBounds();
                if (bounds.length == 0) {
                    return var;
                }
                return Types.newTypeVariable(var.getGenericDeclaration(), var.getName(), new TypeResolver(forDependants).resolveTypes(bounds));
            }
            return new TypeResolver(forDependants).resolveType(type);
        }
    }
}

