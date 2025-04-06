 
#include "quickjs.h"
#include "quickjs-libc.h"
#include <string>
#include <iostream>
#include "cutils.h"

#include "Vector2.h"
 
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
 // 1. UClass Registry & JS Class Management
struct UEClassWrapper {
    JSClassID class_id;
    void* uclass;
};

static JSClassID ue_module_class_id;

// 2. UE Module Property Getter (Dynamically exports UClasses)
static JSValue ue_module_get_property(JSContext *ctx, JSValue obj, JSAtom atom,
    JSValue receiver) 
    {

    const char* name = JS_AtomToCString(ctx, atom);

    JSValue ctor = Vector2::js_Init(ctx, nullptr);

    // 6. Register and Cache
    //uclass_registry.emplace(name, UEClassWrapper{class_id, uclass});
    //JS_SetPropertyStr(ctx, obj, name, ctor);
    
    return ctor;
}

// 7. UE Module Initialization
static int js_init_ue_module(JSContext* ctx, JSModuleDef* m)
{
   
    static JSClassExoticMethods ExoticMethods =
    {
        .get_property = ue_module_get_property
    };

    // Create UE module object
    JSClassDef ue_module_class = {
        .class_name = "UEModule",
        //.exotic = &ExoticMethods,
        .exotic = &ExoticMethods
    };
    
    JS_NewClassID(JS_GetRuntime(ctx), &ue_module_class_id);
    JS_NewClass(JS_GetRuntime(ctx), ue_module_class_id, &ue_module_class);
    
    JSValue ue_obj = JS_NewObjectClass(ctx, ue_module_class_id);
    JS_SetModuleExport(ctx, m, "default", ue_obj);
    
    return 0;
}

int TestExportClassToJavaScript()
{
    JSModuleDef* m = JS_NewCModule(QJSContext, "UE", js_init_ue_module);

	JS_AddModuleExport(QJSContext, m, "default");

	char JSScript[] = 
    R"( import UE from 'UE';
        const vec = new UE.Vector2(3, 4);
        vec.X = 30;
        print(vec.X);
        print(vec.norm());
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
 
 
 