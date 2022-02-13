#if _WIN32
#ifdef MYTHOLOGY_ADDON_IMPLEMENTATION
#define MYTHOLOGY_ADDON_API __declspec(dllexport)
#else
#define MYTHOLOGY_ADDON_API __declspec(dllimport)
#endif // MYTHOLOGY_ADDON_IMPLEMENTATION
#else
#define MYTHOLOGY_ADDON_API
#endif // _WIN32

class Addon_interface;

extern "C"
{
    MYTHOLOGY_ADDON_API Addon_interface* create_addon_interface();
    using Create_addon_interface_function_pointer = Addon_interface * (*)();

    MYTHOLOGY_ADDON_API void destroy_addon_interface(Addon_interface* addon_interface);
    using Destroy_addon_interface_function_pointer = void(*)(Addon_interface*);
}
