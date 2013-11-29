#ifndef INC_CAMERA_H
#define INC_CAMERA_H

class CameraInterface {
public:
    int id;
    glm::vec3 pos;
    glm::vec3 rot;
};

extern CameraInterface g_CameraInterface;

void CAM_Init();
void CAM_Clone();
void CAM_Next();
void CAM_Previous();
void CAM_Update();
void CAM_Cleanup();

#endif

