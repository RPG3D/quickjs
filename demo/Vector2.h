 

#pragma once

#include "quickjs.h"
#include "quickjs-libc.h"
#include <string>
#include <iostream>
#include "cutils.h"


struct Vector2
{
public:
    int X = 10;
    int Y = 20;
public:
    //js glue

    static JSValue js_Init(JSContext* ctx, JSModuleDef* m);
    static JSValue js_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst *argv);

    static void js_finalizer(JSRuntime *rt, JSValue val);

    static JSValue js_GetValue(JSContext* ctx, JSValueConst this_val, int magic);

    static JSValue js_SetValue(JSContext* ctx, JSValueConst this_val, JSValueConst v, int magic);

    static JSValue js_GetProperty(JSContext *ctx, JSValue this_val, JSAtom atom,
        JSValue receiver);

    static JSValue js_point_norm(JSContext *ctx, JSValue this_val,
        int argc, JSValue *argv);

    static JSClassID ClassID;
    static JSClassDef ClassDef;

    inline static const JSCFunctionListEntry ProtoFuncs[] = {
        JS_CGETSET_MAGIC_DEF("X", js_GetValue,  js_SetValue, 0),
        JS_CGETSET_MAGIC_DEF("Y", js_GetValue,  js_SetValue, 1),
        JS_CFUNC_DEF("norm", 0, js_point_norm),
    };
    
};