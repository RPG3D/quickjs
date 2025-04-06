
#include "Vector2.h"

static const JSCFunctionListEntry Vector2_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("X", Vector2::js_GetValue, Vector2::js_SetValue, 0),
    JS_CGETSET_MAGIC_DEF("Y", Vector2::js_GetValue, Vector2::js_SetValue, 1),
    JS_CFUNC_DEF("norm", 0, Vector2::js_point_norm),
};


JSValue Vector2::js_Init(JSContext* ctx, JSModuleDef* m)
{
    JSValue proto, obj;
    JS_NewClassID(JS_GetRuntime(ctx), &ClassID);
    JS_NewClass(JS_GetRuntime(ctx), ClassID, &ClassDef);
    proto = JS_NewObject(ctx);
   
    JS_SetPropertyFunctionList(ctx, proto, Vector2_proto_funcs, countof(Vector2_proto_funcs));

    obj = JS_NewCFunction2(ctx, js_constructor, "Vector2", 2, JS_CFUNC_constructor_or_func, 0);
    if(m)
    {
        JS_SetModuleExport(ctx, m, "Vector2", obj);
    }
    
    JS_SetClassProto(ctx, ClassID, proto);
    JS_SetConstructor(ctx, obj, proto);

    return obj;
}

JSValue Vector2::js_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst *argv)
{
    double x, y;
    if (JS_ToFloat64(ctx, &x, argv[0]))
        return JS_EXCEPTION;
    if (JS_ToFloat64(ctx, &y, argv[1]))
        return JS_EXCEPTION;

        JSValue obj = JS_NewObjectClass(ctx, ClassID);
        if (JS_IsException(obj))
            return obj;  
        Vector2* p = (Vector2*)js_mallocz(ctx, sizeof(Vector2));
        p->X = x; p->Y = y;
        JS_SetOpaque(obj, p);  
        return obj;
}

void Vector2::js_finalizer(JSRuntime *rt, JSValue val)
{
    Vector2 *s = (Vector2*)JS_GetOpaque(val, ClassID);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    js_free_rt(rt, s);
}

JSValue Vector2::js_GetValue(JSContext* ctx, JSValueConst this_val, int magic)
{
    Vector2* p = (Vector2*)JS_GetOpaque2(ctx, this_val, ClassID);

    if (p)
        return JS_NewFloat64(ctx, p->X);
    else
        return JS_EXCEPTION;
}

JSValue Vector2::js_SetValue(JSContext* ctx, JSValueConst this_val, JSValueConst v, int magic)
{
    Vector2* p = (Vector2*)JS_GetOpaque2(ctx, this_val, ClassID);
    if (!p)
        return JS_EXCEPTION;

    if (JS_IsUndefined(this_val) || JS_IsNull(this_val))
        return JS_UNDEFINED;
    if (JS_IsNull(v))
        return JS_UNDEFINED;

    double value;
    if (JS_ToFloat64(ctx, &value, v))
        return JS_EXCEPTION;
    p->X = value;

    return JS_UNDEFINED;
}

JSValue Vector2::js_point_norm(JSContext *ctx, JSValue this_val,
    int argc, JSValue *argv)
{
    Vector2 *s = (Vector2*)JS_GetOpaque2(ctx, this_val, ClassID);
    if (!s)
        return JS_EXCEPTION;
    return JS_NewFloat64(ctx, sqrt((double)s->X * s->X + (double)s->Y * s->Y));
}


JSClassID Vector2::ClassID;

JSClassDef Vector2::ClassDef = {
    .class_name = "Vector2",
    .finalizer = js_finalizer,
};