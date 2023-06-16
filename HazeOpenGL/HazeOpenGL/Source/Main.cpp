#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "HazeLibraryDefine.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int Add(int a, int b)
{
    return a + b;
}

int ShowOpenGLWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

void ExecuteFunction(const wchar_t* FunctionName, char* ParamStartAddress, char* RetStartAddress)
{
    if (wcscmp(FunctionName,L"显示窗口") == 0)
    {
        ShowOpenGLWindow();
    }
    else if (wcscmp(FunctionName, L"OpenGLAdd") == 0)
    {
        int a, b;
        GET_PARAM_START();
        GET_PARAM(a, ParamStartAddress);
        GET_PARAM(b, ParamStartAddress);
        //memcpy(&a, ParamStartAddress - sizeof(a), sizeof(a));
        //memcpy(&b, ParamStartAddress - sizeof(a) - sizeof(b), sizeof(b));
        int c = Add(a, b);
        //memcpy(RetStartAddress, &c, sizeof(c));
        SET_RET(c, RetStartAddress);
    }
    else if (wcscmp(FunctionName, L"OpenGL减法") == 0)
    {
        int a, b;
        memcpy(&a, ParamStartAddress - sizeof(a), sizeof(a));
        memcpy(&b, ParamStartAddress - sizeof(a) - sizeof(b), sizeof(b));
        int c = a - b;
        memcpy(RetStartAddress, &c, sizeof(c));
    }
    else if (wcscmp(FunctionName, L"乘法") == 0)
    {
        int a, b;
        memcpy(&a, ParamStartAddress - sizeof(a), sizeof(a));
        memcpy(&b, ParamStartAddress - sizeof(a) - sizeof(b), sizeof(b));
        int c = a * b;
        memcpy(RetStartAddress, &c, sizeof(c));
    }
}


//int main() {
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
//    if (window == NULL) {
//        cout << "Failed to create GLFW window" << endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//
//    glViewport(0, 0, 800, 600);
//
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    while (!glfwWindowShouldClose(window)) {
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glfwTerminate();
//    return 0;
//}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}