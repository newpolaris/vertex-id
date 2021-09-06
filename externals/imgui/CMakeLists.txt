project(imgui)

add_definitions(
	-DUSE_IM_GUI_OTHER_FONT
	)
add_library(
        imgui STATIC
        imconfig.h
        imgui.cpp
        imgui.h
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_impl_glfw.cpp
        imgui_impl_glfw.h
        imgui_impl_opengl3.cpp
        imgui_impl_opengl3.h
        imgui_widgets.cpp
        imgui_internal.h
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        )

target_include_directories(imgui PRIVATE ${glad_SOURCE_DIR}/include)
target_link_libraries(imgui PRIVATE glad)

target_include_directories(imgui PRIVATE ${GLFW_SOURCE_DIR}/deps)
target_include_directories(imgui PRIVATE ${GLFW_SOURCE_DIR}/include)
target_link_libraries(imgui PRIVATE glfw)
