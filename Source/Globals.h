#ifndef GLOBALS_H
#define GLOBALS_H

#include "Camera.h"
#include <vector>
#include "Particle.h"

// gross class (to be refactored), breaks if you include it in files you include above!
class Globals {
public:
    // defines an class operation that lets clients access its unique instance.
    static Globals& get() {
        // may be responsible for creating its own unique instance.
        if (nullptr == instance) instance = new Globals();
        return *instance;
    }
    Globals(const Globals&) = delete; // rule of three
    Globals& operator=(const Globals&) = delete;
    static void destruct() {
        delete instance;
        instance = nullptr;
    }

    int screenWidth = 1600, screenHeight = 900; // Width and height of window in pixels.
    bool cursorLocked = false; // whether to lock cursor for first person mode
    bool camLock = false;
    bool drawWires = true;
    unsigned int depthMap;
    unsigned int depthCubeMap;
    float far_plane;
    glm::mat4 lightSpaceMatrix;
    Shader shadowShader;
    Shader wireShader;
    Shader animShadowShader;
    Shader animPointShadowShader;
    Shader animCellShader;
    Shader cellShader;
    Shader pointShadowShader;
    Shader rigProgram;
    Shader lightProgram;
    Shader animProgram;
    Shader noTexAnimProgram;
    Camera * camera;
    Camera * handCam;
    double rotX;
    double rotY;

private:
    Globals()
    {
        camera = new Camera(screenWidth, screenHeight, glm::vec3(0.0f, 11.0f, 30.0f), 90.0f, 0.1f, 1000.0f);
        handCam = new Camera(screenWidth, screenHeight, glm::vec3(0.0f, 0.0f, 2.0f), 90.0f, 0.1f, 1000.0f);
    } // no public constructor
    ~Globals() = default; // no public destructor
    static Globals* instance; // declaration class variable

};
#endif
