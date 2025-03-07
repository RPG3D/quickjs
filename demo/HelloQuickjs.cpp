 
#include "quickjs.h"
#include "quickjs-libc.h"
#include <string>
#include <iostream>
#include "cutils.h"
 
JSRuntime* QJSRuntime = nullptr;
JSContext* QJSContext = nullptr;
 
int InitQuickJSEnv()
{
	QJSRuntime = JS_NewRuntime();
    QJSContext = JS_NewContext(QJSRuntime);
	js_std_init_handlers(QJSRuntime);
	JS_SetModuleLoaderFunc(QJSRuntime, nullptr, js_module_loader, nullptr);
	js_std_add_helpers(QJSContext, -1, nullptr);
    js_init_module_std(QJSContext, "std");
    js_init_module_std(QJSContext, "os");
    
    return 0;
}
 
struct Vector2
{
public:
    int X = 0;
    int Y = 0;
public:
    //js glue

    static void js_Init(JSContext* ctx, JSModuleDef* m)
    {
        JSValue proto, obj;
        JS_NewClassID(JS_GetRuntime(ctx), &ClassID);
        JS_NewClass(JS_GetRuntime(ctx), ClassID, &ClassDef);
        proto = JS_NewObject(ctx);
        JS_SetPropertyFunctionList(ctx, proto, ProtoFuncs, countof(ProtoFuncs));
        JS_SetClassProto(ctx, ClassID, proto);
    
        obj = JS_NewCFunction2(ctx, js_constructor, "Vector2", 2, JS_CFUNC_constructor_or_func, 0);
        JS_SetModuleExport(ctx, m, "Vector2", obj);
    }

    static JSValue js_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst *argv)
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

    static void js_finalizer(JSRuntime *rt, JSValue val)
    {
        Vector2 *s = (Vector2*)JS_GetOpaque(val, ClassID);
        /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
        js_free_rt(rt, s);
    }

    static JSValue js_GetValue(JSContext* ctx, JSValueConst this_val, int magic)
    {
        Vector2* p = (Vector2*)JS_GetOpaque2(ctx, this_val, ClassID);

        if (p)
            return JS_NewFloat64(ctx, p->X);
        else
            return JS_EXCEPTION;
    }

    static JSValue js_SetValue(JSContext* ctx, JSValueConst this_val, JSValueConst v, int magic)
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

    static JSClassID ClassID;
    static JSClassDef ClassDef;

    inline static const JSCFunctionListEntry ProtoFuncs[] = {
        JS_CGETSET_MAGIC_DEF("X", js_GetValue,  js_SetValue, 0)
    };
    
};

JSClassID Vector2::ClassID;
JSClassDef Vector2::ClassDef = {
    "Vector2",
    js_finalizer,
};


void InitClasses(JSContext* ctx, JSModuleDef* m)
{
	Vector2::js_Init(ctx, m);
}

void InitModuleClasses(JSContext* ctx, JSModuleDef* m)
{
	JS_AddModuleExport(ctx, m, "Vector2");
}

static int InitModule(JSContext* ctx, JSModuleDef* m)
{
	InitClasses(ctx, m);

    return 0;
}

int TestExportClassToJavaScript()
{
	JSModuleDef* m = nullptr;

	m = JS_NewCModule(QJSContext, "NativeModule", InitModule);
	
	if (!m)
		return -1;
	
    InitModuleClasses(QJSContext, m);

	char JSScript[] = 
    R"( import * as NativeModule from 'NativeModule';
        var tmp = new NativeModule.Vector2(123, 45);
        console.log(tmp.X);
        tmp.X = 1;
        console.log(tmp.X);
        )";
 
    JSValue RetValue = JS_Eval(QJSContext, JSScript, strlen(JSScript), "", JS_EVAL_TYPE_MODULE);
 
    if (JS_IsException(RetValue))
    {
        js_std_dump_error(QJSContext);
    }

    JS_FreeValue(QJSContext, RetValue);
 
    return 0;
}
 
int main(int argc,  char* argv[])
{
    InitQuickJSEnv();
 
    TestExportClassToJavaScript();

    return 0;
}
 
 
 