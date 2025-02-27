#include "Camera.hpp"

#include <math.h>


Camera::Camera(Vector4f position, float fovX, float aspectRatio, float near,  float far): 
               position(position), worldUp(0.f, 1.f, 0.f, 1.f), pitch(0.f), yaw(0.f), 
               sensitivity(.7f), horizontalFOV(fovX), aspectRatio(aspectRatio),
               frustumNear(near), frustumFar(far), orientation(0.f, 0.f, 0.f)
{
    this->updateBasisVectors();
    this->frustumRight = tan(fovX / 2) * this->frustumNear;
    this->frustumLeft = -this->frustumRight;
    
    this->frustumTop = this->frustumRight / this->aspectRatio;
    this->frustumBottom = -this->frustumTop;
}

Camera::~Camera()
{

}

void Camera::update(FrameInput& input)
{
    float speed = 0.5f;
    if (input.forward)
    {
        this->position = this->position + this->forward * speed;
    }
    if (input.left)
    {
        this->position = this->position - this->right * speed;
    }
    if (input.backward)
    {
        this->position = this->position - this->forward * speed;
    }
    if (input.right)
    {
        this->position = this->position + this->right * speed;
    }

    if (input.relativeX != 0 || input.relativeY != 0)
    {
        this->orientation.x -= this->sensitivity * (float)(input.relativeY) * PI_OVER_180;
        this->orientation.y -= this->sensitivity * (float)(input.relativeX) * PI_OVER_180;
        if (this->orientation.x > MAX_PITCH)
        {
            this->orientation.x = (float)MAX_PITCH;
        }
        else if (this->orientation.x < -MAX_PITCH)
        {
            this->orientation.x = -(float)MAX_PITCH;
        }

        this->updateBasisVectors();
    }
}

Matrix4 Camera::getViewTransform() const
{
    Matrix4 result;
    result.set(0, 0, this->right.x);
    result.set(0, 1, this->right.y);
    result.set(0, 2, this->right.z);
    result.set(0, 3,-this->right.dot(this->position));

    result.set(1, 0, this->up.x);
    result.set(1, 1, this->up.y);
    result.set(1, 2, this->up.z);
    result.set(1, 3,-this->up.dot(this->position));

    result.set(2, 0, -this->forward.x);
    result.set(2, 1, -this->forward.y);
    result.set(2, 2, -this->forward.z);
    result.set(2, 3,  this->forward.dot(this->position));

    result.set(3, 3, 1.f);
    return result;
}

Matrix4 Camera::getProjectionTransform() const
{
    Matrix4 result;
    
    result.set(0, 0, (2 * this->frustumNear)/(this->frustumRight - this->frustumLeft));
    result.set(0, 2, (this->frustumRight + this->frustumLeft)/(this->frustumRight - this->frustumLeft));

    result.set(1, 1, (2 * this->frustumNear)/(this->frustumTop - this->frustumBottom));
    result.set(1, 2, (this->frustumTop + this->frustumBottom)/(this->frustumTop - this->frustumBottom));

    result.set(2, 2, -(this->frustumFar + this->frustumNear)/(this->frustumFar - this->frustumNear));
    result.set(2, 3, -(2 * this->frustumNear * this->frustumFar)/(this->frustumFar - this->frustumNear));

    result.set(3, 2, -1.f);
    result.set(3, 3, 0.f);
    return result;
}

Vector4f Camera::getPosition() const
{
    return this->position;
}

void Camera::updateBasisVectors()
{
    this->forward = this->orientation.getRotationTransform() * Vector4f(0.f, 0.f, -1.f, 1.f);
    this->forward.normalize();
    this->right = this->forward.cross(this->worldUp);
    this->right.normalize();
    this->up = this->right.cross(this->forward);
    this->up.normalize();

    this->updateFrustumPlanes();
}

void Camera::updateFrustumPlanes()
{
    float halfHorizontalSide = std::tan(this->horizontalFOV / 2.f) * this->frustumFar;
    float halfVerticalSide = halfHorizontalSide / this->aspectRatio;
    Vector4f vecToFarPlane = this->forward * this->frustumFar;

    this->nearPlane.normal = this->forward;
    this->nearPlane.point = this->position + this->forward * this->frustumNear;

    this->farPlane.normal = -this->forward;
    this->farPlane.point = this->position + vecToFarPlane;

    Vector4f leftPlaneVec = vecToFarPlane - this->right * halfHorizontalSide;
    leftPlaneVec.normalize();
    this->leftPlane.normal = leftPlaneVec.cross(this->up);
    this->leftPlane.point = this->position;

    Vector4f rightPlaneVec = vecToFarPlane + this->right * halfHorizontalSide;
    rightPlaneVec.normalize();
    this->rightPlane.normal = this->up.cross(rightPlaneVec);
    this->rightPlane.point = this->position;

    Vector4f topPlaneVec = vecToFarPlane + this->up * halfVerticalSide;
    topPlaneVec.normalize();
    this->topPlane.normal = topPlaneVec.cross(this->right);
    this->topPlane.point = this->position;

    Vector4f bottomPlaneVec = vecToFarPlane - this->up * halfVerticalSide;
    bottomPlaneVec.normalize();
    this->bottomPlane.normal = this->right.cross(bottomPlaneVec);
    this->bottomPlane.point = this->position;
}

bool Camera::isVisible(Model* model)
{
    // check every point against every plane
    AABB box = model->getBoundingBox();
    Vector4f min = box.min;
    Vector4f max = box.max;
    std::vector<Vector4f> points{
        Vector4f(min.x, min.y, min.z, 1.f),
        Vector4f(min.x, min.y, max.z, 1.f),
        Vector4f(min.x, max.y, min.z, 1.f),
        Vector4f(min.x, max.y, max.z, 1.f),

        Vector4f(max.x, max.y, max.z, 1.f),
        Vector4f(max.x, max.y, min.z, 1.f),
        Vector4f(max.x, min.y, max.z, 1.f),
        Vector4f(max.x, min.y, min.z, 1.f)
    };

    if (this->isModelBehindPlane(this->nearPlane, points))
    {
        return false;
    }
    if (this->isModelBehindPlane(this->farPlane, points))
    {
        return false;
    }
    if (this->isModelBehindPlane(this->topPlane, points))
    {
        return false;
    }
    if (this->isModelBehindPlane(this->bottomPlane, points))
    {
        return false;
    }
    if (this->isModelBehindPlane(this->leftPlane, points))
    {
        return false;
    }
    if (this->isModelBehindPlane(this->rightPlane, points))
    {
        return false;
    }
    return true;
}

bool Camera::isModelBehindPlane(const Plane& plane, std::vector<Vector4f>& points)
{
    for (int i = 0; i < points.size(); i++)
    {
        Vector4f plane2model = points[i] - plane.point;
        plane2model.normalize();
        if (plane.normal.dot(plane2model) >= 0.f)
        {
            return false;
        }
    }
    return true;
}