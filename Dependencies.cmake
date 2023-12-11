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