#define EXTENSION_NAME InstantApp
#define LIB_NAME "InstantApp"
#define MODULE_NAME "instantapp"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_ANDROID)

#include <jni.h>

struct GiaState
{
    jobject                 m_GiaJNI;
    jmethodID               m_silentLogin;
};

GiaState g_Gia;

struct ThreadAttacher {
    JNIEnv *env;
    bool has_attached;
    ThreadAttacher() : env(NULL), has_attached(false) {
        if (dmGraphics::GetNativeAndroidJavaVM()->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
            dmGraphics::GetNativeAndroidJavaVM()->AttachCurrentThread(&env, NULL);
            has_attached = true;
        }
    }
    ~ThreadAttacher() {
        if (has_attached) {
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
            }
            env->ExceptionClear();
            dmGraphics::GetNativeAndroidJavaVM()->DetachCurrentThread();
        }
    }
};

struct ClassLoader {
    private:
    JNIEnv *env;
    jobject class_loader_object;
    jmethodID find_class;
    public:
    ClassLoader(JNIEnv *env) : env(env) {
        jclass activity_class = env->FindClass("android/app/NativeActivity");
        jmethodID get_class_loader = env->GetMethodID(activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
        class_loader_object = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
        jclass class_loader = env->FindClass("java/lang/ClassLoader");
        find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        env->DeleteLocalRef(activity_class);
        env->DeleteLocalRef(class_loader);
    }
    ~ClassLoader() {
        env->DeleteLocalRef(class_loader_object);
    }
    jclass load(const char *class_name) {
        jstring str_class_name = env->NewStringUTF(class_name);
        jclass loaded_class = (jclass)env->CallObjectMethod(class_loader_object, find_class, str_class_name);
        env->DeleteLocalRef(str_class_name);
        return loaded_class;
    }
};

static const luaL_reg intantapp_methods[] =
{
    {0,0}
};

static dmExtension::Result AppInitializeInstantApp(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static void LuaInit(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_register(L, MODULE_NAME, intantapp_methods);
    lua_pop(L,  1);
}

static void CreateJObject()
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;
    ClassLoader class_loader = ClassLoader(env);
    jclass cls = class_loader.load("com.agulev.instantapp.GiaJNI");

    //g_Gia.m_silentLogin = env->GetMethodID(cls, "silentLogin", "()V");

    jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;)V");
    g_Gia.m_GiaJNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, dmGraphics::GetNativeAndroidActivity()));
}

static dmExtension::Result InitializeInstantApp(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeInstantApp(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeInstantApp(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeInstantApp, AppFinalizeInstantApp, InitializeInstantApp, 0, 0, FinalizeInstantApp)

#else

dmExtension::Result AppInitializeInstantApp(dmExtension::AppParams* params)
{
    dmLogInfo("Registered extension InstantApp (null)");
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeInstantApp(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeInstantApp, AppFinalizeInstantApp, 0, 0, 0, 0)

#endif