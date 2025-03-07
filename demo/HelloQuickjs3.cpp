 
#include "quickjs.h"
#include "quickjs-libc.h"
#include <string>
#include <iostream>
#include "cutils.h"
#include <unordered_map>
 
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
static std::unordered_map<std::string, UEClassWrapper> uclass_registry;

// 2. UE Module Property Getter (Dynamically exports UClasses)
static JSValue ue_module_get_property(JSContext *ctx, JSValue obj, JSAtom atom,
    JSValue receiver) 
    {

    const char* name = JS_AtomToCString(ctx, atom);

    // Check if class already exists
    auto it = uclass_registry.find(name);
    if (it != uclass_registry.end()) {
        return JS_NewObjectClass(ctx, it->second.class_id);
    }

    // Find UClass in Unreal's reflection system
    void* uclass = nullptr;
    if (!uclass) {
        return JS_ThrowReferenceError(ctx, "UClass '%s' not found", name);
    }

    // 3. Create JS Class for UClass
    JSClassID class_id;
    JS_NewClassID(JS_GetRuntime(ctx), &class_id);
    
    JSClassDef class_def = {
        .class_name = name,
        .finalizer = [](JSRuntime* rt, JSValue obj) {
            void* uobj = nullptr;
            if (uobj) {
                // Handle UE object lifetime if needed
            }
        }
    };
    
    JS_NewClass(JS_GetRuntime(ctx), class_id, &class_def);
    
    // 4. Create Constructor
    JSValue ctor = JS_NewCFunction2(ctx, 
        [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
            // Create or retrieve UE object
            void* uclass = nullptr;
            void* uobj = nullptr;
            
            JSValue obj = JS_NewObjectClass(ctx, 0);
            JS_SetOpaque(obj, uobj);
            return obj;
        }, 
        name, 0, JS_CFUNC_constructor, 0
    );

    // 5. Bind UFUNCTIONS
   

    // 6. Register and Cache
    uclass_registry.emplace(name, UEClassWrapper{class_id, uclass});
    JS_SetPropertyStr(ctx, obj, name, ctor);
    
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
        const Actor = UE.Actor;
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
 
 
 