CPMAddPackage(
        NAME fmt
        GIT_TAG 10.1.1
        GITHUB_REPOSITORY fmtlib/fmt
        OPTIONS "FMT_INSTALL OFF"
)

CPMAddPackage(
        NAME glm
        GITHUB_REPOSITORY g-truc/glm
        GIT_TAG 0.9.9.8
        SYSTEM ON
)

CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.3.8
        SYSTEM ON
        OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_INSTALL OFF"
)

CPMAddPackage(
        NAME VkMana
        GITHUB_REPOSITORY stuart6854/VkMana
        GIT_TAG main
        SYSTEM ON
        OPTIONS
        "VKMANA_BUILD_SAMPLES OFF"
)

CPMAddPackage(
        NAME assimp
        GITHUB_REPOSITORY assimp/assimp
        GIT_TAG v5.3.1
        SYSTEM ON
        OPTIONS
        "ASSIMP_BUILD_TESTS OFF"
        "ASSIMP_INSTALL OFF"
        "ASSIMP_NO_EXPORT ON"
        "ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF"
        "ASSIMP_BUILD_OBJ_IMPORTER ON"
        "ASSIMP_BUILD_FBX_IMPORTER ON"
        "ASSIMP_BUILD_GLTF_IMPORTER ON"
)

CPMAddPackage(
        NAME stb
        GITHUB_REPOSITORY nothings/stb
        GIT_TAG master
        DOWNLOAD_ONLY True
)
if (stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})
endif ()