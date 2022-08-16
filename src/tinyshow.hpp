#pragma once
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>
#pragma comment(lib, "opengl32")

class tinyshow
{
public:
    template <typename T>
    static void Show(const std::vector<T> &data, int width, int height)
    {
        Show(data.data(), GetType<T>(), width, height, data.size() / width / height);
    }

    template <typename T>
    static void Show(const void *data, int width, int height, int channels)
    {
        Show(data, GetType<T>(), width, height, channels);
    }

    static void Show(const void *data, GLuint type, int width, int height, int channels)
    {
        static GLFWcontext glfwContext{};

        if (!m_window)
        {
            m_window = glfwCreateWindow(width, height, "tinyshow", NULL, NULL);
            m_width = width;
            m_height = height;
            glGenTextures(1, &m_texture);
            glfwSetKeyCallback(m_window, KeyCallback);
            glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
        }

        glfwMakeContextCurrent(m_window);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_ALPHA);
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        GLuint format{};
        switch (channels)
        {
        case 1:
            format = GL_LUMINANCE;
            break;
        case 2:
            format = GL_LUMINANCE_ALPHA;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            assert(false);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, type, data);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex2f(-1, -1);
        glTexCoord2f(1, 1);
        glVertex2f(1, -1);
        glTexCoord2f(1, 0);
        glVertex2f(1, 1);
        glTexCoord2f(0, 0);
        glVertex2f(-1, 1);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_ALPHA);
        glPopMatrix();
        glPopAttrib();
    }

    static void WaitKey()
    {
        glfwMakeContextCurrent(m_window);
        glfwSwapBuffers(m_window);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        DeleteWindow();
        m_pressed = false;
    }

private:
    struct GLFWcontext
    {
        GLFWcontext() { glfwInit(); }
        ~GLFWcontext() { glfwTerminate(); }
    };

    static inline GLFWwindow *m_window = nullptr;
    static inline GLuint m_texture;
    static inline int m_width, m_height;
    static inline bool m_pressed = false;

    static void DeleteWindow()
    {
        glfwDestroyWindow(m_window);
        glDeleteTextures(1, &m_texture);
        m_window = nullptr;
    }

    template <typename T>
    static GLuint GetType()
    {
        static std::unordered_map<std::type_index, GLuint> types{
            {typeid(int8_t), GL_BYTE},
            {typeid(uint8_t), GL_UNSIGNED_BYTE},
            {typeid(int16_t), GL_SHORT},
            {typeid(uint16_t), GL_UNSIGNED_SHORT},
            {typeid(int), GL_INT},
            {typeid(int32_t), GL_INT},
            {typeid(uint32_t), GL_UNSIGNED_INT},
            {typeid(float), GL_FLOAT},
            {typeid(double), GL_DOUBLE}};
        if (!types.contains(typeid(T)))
        {
            throw std::runtime_error("Unknown image type: " + std::string{typeid(T).name()});
        }
        return types[typeid(T)];
    }

    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            m_pressed = true;
        }
    }

    static void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        glfwMakeContextCurrent(window);
        auto wScale = ((float)width) / ((float)m_width);
        auto hScale = ((float)height) / ((float)m_height);
        auto minScale = (wScale < hScale) ? wScale : hScale;
        int wShift = (int)nearbyint((width - minScale * m_width) / 2.0f);
        int hShift = (int)nearbyint((height - minScale * m_height) / 2.0f);

        glViewport(wShift, hShift, (int)nearbyint(m_width * minScale), (int)nearbyint(m_height * minScale));
    }
};
