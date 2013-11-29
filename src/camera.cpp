#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "defs.h"
#include "camera.h"

CameraInterface g_CameraInterface;
class Camera;

static Camera *currCamera;

class Camera {
private:
    static int lastCamId;
private:
    CameraInterface val;
public:
    Camera *next, *prev;
    Camera() {
        memset(&val, 0, sizeof(val));
        val.id = ++lastCamId;
        prev = currCamera;
        if (prev != NULL) {
            next = prev->next;
            prev->next = this;
        } else {
            next = NULL;
        }
        if (next != NULL) {
            next->prev = this;
        }
    }

    void Select()
    {
        g_CameraInterface = val;
    }

    void Update()
    {
        int id = val.id;
        val = g_CameraInterface;
        val.id = id;
    }

};

int Camera::lastCamId = 0;

static void CAM_Select(Camera *cam)
{
    currCamera = cam;
    cam->Select();
}

void CAM_Init()
{
    currCamera = NULL;
    CAM_Clone();
}

void CAM_Clone()
{
    Camera *nc;

    CAM_Update();
    nc = new Camera();
    nc->Update();
    CAM_Select(nc);
}

void CAM_Next()
{
    if (currCamera != NULL) {
        if (currCamera->next != NULL) {
            CAM_Update();
            CAM_Select(currCamera->next);
        }
    }
}

void CAM_Previous()
{
    if (currCamera != NULL) {
        if (currCamera->prev != NULL) {
            CAM_Update();
            CAM_Select(currCamera->prev);
        }
    }
}

void CAM_Update()
{
    if (currCamera != NULL) {
        currCamera->Update();
    }
}

void CAM_Cleanup()
{
    if (currCamera != NULL) {
        while (currCamera->prev != NULL) {
            currCamera = currCamera->prev;
        }
    }
    while (currCamera != NULL) {
        Camera *next;

        next = currCamera->next;
        delete currCamera;
        currCamera = next;
    }
}
