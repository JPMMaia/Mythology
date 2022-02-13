add_library(MythologyAddonsExample SHARED)
add_library(Mythology::Addons::Example ALIAS MythologyAddonsExample)

target_link_libraries(
    MythologyAddonsExample
        PRIVATE
            Mythology::Addon::Interface
)

target_sources(MythologyAddonsExample PRIVATE "Entry.cpp")
