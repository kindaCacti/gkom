#ifndef ENTITIES_ENTITY_H
#define ENTITIES_ENTITY_H

#include <algorithm>

#include "../shaders/shader_s.h"
#include "../utils.h"
#include "../shapes/shape.h"
#include "../hitbox.h"

class Entity {
  protected:
    // entity ma to trzymać, bo zmiana tego zwiększa nie tylko czas potrzebny do
    // odczytu danych (co jest ważne), a także zwiększa szansę na błędy przy
    // dzieleniu przez zero z klasy transform (proszę nie zmieniać)
    glm::vec3 _pos;   // center point of an entity (in x y and z)
    glm::vec3 _rot;   // rotation of an entity (int x y and z)
    glm::vec3 _scale; // scale of an entity (in x y and z)

  public:
    Entity()
        : _pos(0.f, 0.f, 0.f), _rot(0.f, 0.f, 0.f), _scale(1.f, 1.f, 1.f) {}

    virtual void move(const float dx = 0.f, const float dy = 0.f,
                      const float dz = 0.f) {
        _pos.x += dx;
        _pos.y += dy;
        _pos.z += dz;
    }

    virtual void setPosition(const float x = 0.f, const float y = 0.f,
                             const float z = 0.f) {
        _pos.x = x;
        _pos.y = y;
        _pos.z = z;
    }

    virtual void rotate(const float dx_rot = 0.0f, const float dy_rot = 0.0f,
                        const float dz_rot = 0.0f) {
        _rot.x += dx_rot;
        _rot.y += dy_rot;
        _rot.z += dz_rot;
    }

    virtual void setRotation(const float x_rot = 0.0f, const float y_rot = 0.0f,
                             const float z_rot = 0.0f) {
        _rot.x = x_rot;
        _rot.y = y_rot;
        _rot.z = z_rot;
    }

    virtual void scale(const float dx = 0.f, const float dy = 0.f,
                       const float dz = 0.f) {
        _scale.x += dx;
        _scale.y += dy;
        _scale.z += dz;
    }

    virtual void setScale(const float x = 0.f, const float y = 0.f,
                          const float z = 0.f) {
        _scale.x = x;
        _scale.y = y;
        _scale.z = z;
    }

    virtual void setScale(glm::vec3 &&scale) { _scale = std::move(scale); }
    virtual void setPosition(glm::vec3 &&pos) { _pos = std::move(pos); }
    virtual void setRotation(glm::vec3 &&rot) { _rot = std::move(rot); }
    virtual void setScale(glm::vec3 &scale) { _scale = glm::vec3(scale); }
    virtual void setPosition(glm::vec3 &pos) { _pos = glm::vec3(pos); }
    virtual void setRotation(glm::vec3 &rot) { _rot = glm::vec3(rot); }
    virtual void rotate(glm::vec3 &delta_rot) { _rot += delta_rot; }
    virtual void move(glm::vec3 &delta_pos) { _pos += delta_pos; }
    virtual void scale(glm::vec3 &delta_scale) { _scale += delta_scale; }
    virtual glm::vec3 rotation() { return _rot; }
    virtual glm::vec3 get_pos() const { return _pos; }
    virtual glm::vec3 get_rot() const { return _rot; }
};

class DrawableEntity : public virtual Entity {
  protected:
    std::unique_ptr<Shape> _shape;

  public:
    DrawableEntity(std::unique_ptr<Shape> &&shape)
        : Entity(), _shape(std::move(shape)) {}
    DrawableEntity(DrawableEntity &&) = default;

    DrawableEntity &operator=(DrawableEntity &&) = default;

    virtual bool draw(Shader &shader, bool isInstanced = false) const {
        _shape->draw(shader, getTransformMatrix());
        return true;
    }

    virtual glm::mat4 getTransformMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), _pos);
        glm::mat4 R = getEulerRotationMatrix(_rot);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), _scale);
        return T * R * S;
    }

    virtual glm::mat4 getTransformMatrixWithShapeTransform() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), _pos);
        glm::mat4 R = getEulerRotationMatrix(_rot);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), _scale);
        return T * R * S * _shape->transform.getMatrix();
    }

    virtual void setShape(std::unique_ptr<Shape> &&shape) {
        _shape = std::move(shape);
    }

    virtual ~DrawableEntity() = default;
};

class HitboxedDrawableEntity : public virtual DrawableEntity {
  protected:
    Hitbox _hitbox;

    glm::vec3 cylinderAxisWorld() const {
        const glm::mat4 R = getEulerRotationMatrix(_rot);
        const glm::vec3 localAxis = _hitbox.cylinderLocalAxis();
        glm::vec3 axis = glm::vec3(R * glm::vec4(localAxis, 0.0f));
        const float len2 = glm::dot(axis, axis);
        if (len2 < 1e-8f)
            return localAxis;
        return axis / std::sqrt(len2);
    }

    float cylinderHalfExtentAlongWorldAxis(int axisIndex) const {
        const glm::vec3 axis = cylinderAxisWorld();
        const float a = std::abs(axis[axisIndex]);
        const float h = _hitbox.cylinderHalfHeight();
        const float r = _hitbox.cylinderRadius();
        // Projection of a radius circle onto an axis perpendicular component.
        const float radial = r * std::sqrt(std::max(0.0f, 1.0f - a * a));
        return a * h + radial;
    }

    void updateHitboxFromShape() {
        if (!_shape) {
            _hitbox.setFromBounds(glm::vec3(0.0f), glm::vec3(0.0f));
            return;
        }
        const glm::vec3 minB(_shape->minX(), _shape->minY(), _shape->minZ());
        const glm::vec3 maxB(_shape->maxX(), _shape->maxY(), _shape->maxZ());
        _hitbox.setFromBounds(minB, maxB);
    }

    void attachShapeBoundsCallback() {
        if (!_shape) {
            return;
        }
        _shape->setOnBoundsUpdated([this]() { updateHitboxFromShape(); });
    }

  public:
    int expensiveCollisionChecks = 0;
    int cheapCollisionChecks = 0;

    HitboxedDrawableEntity(std::unique_ptr<Shape> &&shape,
                           glm::vec3 /*hitbox_size*/)
        : DrawableEntity(std::move(shape)) {
        attachShapeBoundsCallback();
        updateHitboxFromShape();
    }
    HitboxedDrawableEntity(std::unique_ptr<Shape> &&shape)
        : DrawableEntity(std::move(shape)) {
        attachShapeBoundsCallback();
        updateHitboxFromShape();
    }

    void setShape(std::unique_ptr<Shape> &&shape) override {
        DrawableEntity::setShape(std::move(shape));
        attachShapeBoundsCallback();
        updateHitboxFromShape();
    }
    HitboxedDrawableEntity(HitboxedDrawableEntity &&) = default;

    HitboxedDrawableEntity &operator=(HitboxedDrawableEntity &&other) noexcept {
        if (this != &other) {
            _hitbox = std::move(other._hitbox);
        }
        return *this;
    }

    ~HitboxedDrawableEntity() = default;

    // Fast broad-phase radius (no sqrt).
    // Conservative: the farthest corner distance is <= sqrt(3)*maxHalfExtent.
    inline float containingSphereRadius() const {
        return _hitbox.containingSphereRadius();
    }

    glm::vec3 hitboxCenterWorld() const {
        // Hitbox center is always at T * R * C, independent of scale.
        const glm::vec3 off = _hitbox.centerOffset();
        const float off2 = glm::dot(off, off);
        if (off2 < 1e-12f)
            return _pos;

        if (_rot.x == 0.0f && _rot.y == 0.0f && _rot.z == 0.0f)
            return _pos + off;

        const glm::mat4 R = getEulerRotationMatrix(_rot);
        return _pos + glm::vec3(R * glm::vec4(off, 0.0f));
    }

    float bottom_x() {
        const glm::mat4 R = getEulerRotationMatrix(_rot);
        const glm::vec3 centerOffsetWorld =
            glm::vec3(R * glm::vec4(_hitbox.centerOffset(), 0.0f));
        const glm::vec3 c = _pos + centerOffsetWorld;
        if (_hitbox.shape() == Hitbox::Shape::Cylinder)
            return c.x - cylinderHalfExtentAlongWorldAxis(0);
        return c.x - _hitbox.boxHalfExtents().x;
    }
    float bottom_y() {
        const glm::mat4 R = getEulerRotationMatrix(_rot);
        const glm::vec3 centerOffsetWorld =
            glm::vec3(R * glm::vec4(_hitbox.centerOffset(), 0.0f));
        const glm::vec3 c = _pos + centerOffsetWorld;
        if (_hitbox.shape() == Hitbox::Shape::Cylinder)
            return c.y - cylinderHalfExtentAlongWorldAxis(1);
        return c.y - _hitbox.boxHalfExtents().y;
    }
    float bottom_z() {
        const glm::mat4 R = getEulerRotationMatrix(_rot);
        const glm::vec3 centerOffsetWorld =
            glm::vec3(R * glm::vec4(_hitbox.centerOffset(), 0.0f));
        const glm::vec3 c = _pos + centerOffsetWorld;
        if (_hitbox.shape() == Hitbox::Shape::Cylinder)
            return c.z - cylinderHalfExtentAlongWorldAxis(2);
        return c.z - _hitbox.boxHalfExtents().z;
    }
    float top_x() {
        const glm::mat4 R = getEulerRotationMatrix(_rot);
        const glm::vec3 centerOffsetWorld =
            glm::vec3(R * glm::vec4(_hitbox.centerOffset(), 0.0f));
        const glm::vec3 c = _pos + centerOffsetWorld;
        if (_hitbox.shape() == Hitbox::Shape::Cylinder)
            return c.x + cylinderHalfExtentAlongWorldAxis(0);
        return c.x + _hitbox.boxHalfExtents().x;
    }
    float top_y() {
        const glm::mat4 R = getEulerRotationMatrix(_rot);
        const glm::vec3 centerOffsetWorld =
            glm::vec3(R * glm::vec4(_hitbox.centerOffset(), 0.0f));
        const glm::vec3 c = _pos + centerOffsetWorld;
        if (_hitbox.shape() == Hitbox::Shape::Cylinder)
            return c.y + cylinderHalfExtentAlongWorldAxis(1);
        return c.y + _hitbox.boxHalfExtents().y;
    }
    float top_z() {
        const glm::mat4 R = getEulerRotationMatrix(_rot);
        const glm::vec3 centerOffsetWorld =
            glm::vec3(R * glm::vec4(_hitbox.centerOffset(), 0.0f));
        const glm::vec3 c = _pos + centerOffsetWorld;
        if (_hitbox.shape() == Hitbox::Shape::Cylinder)
            return c.z + cylinderHalfExtentAlongWorldAxis(2);
        return c.z + _hitbox.boxHalfExtents().z;
    }

    bool fastCollisionCheck(HitboxedDrawableEntity *other) {
        if (!other)
            return false;
        if (!broadPhaseIntersects(other))
            return false;
        return intersects(other);
    }

    inline bool intersects(HitboxedDrawableEntity *other) {
        if (!other)
            return false;
        ++expensiveCollisionChecks;
        return _hitbox.intersects(other->_hitbox, _pos, _rot, other->_pos,
                                  other->_rot);
    }

    inline bool broadPhaseIntersects(const HitboxedDrawableEntity *other) {
        if (!other)
            return false;
        ++cheapCollisionChecks;
        const glm::vec3 a = hitboxCenterWorld();
        const glm::vec3 b = other->hitboxCenterWorld();
        const glm::vec3 d = b - a;
        const float dist2 = d.x * d.x + d.y * d.y + d.z * d.z;
        const float r =
            containingSphereRadius() + other->containingSphereRadius();
        return dist2 <= r * r;
    }

    inline void resetCollisionCounters() {
        expensiveCollisionChecks = 0;
        cheapCollisionChecks = 0;
    }

    glm::mat4 getHitboxTransformMatrix() const {
        return _hitbox.boxTransformMatrix(_pos, _rot);
    }

    glm::mat4 getCylinderHitboxTransformMatrix() const {
        return _hitbox.cylinderTransformMatrix(_pos, _rot);
    }

    virtual void drawHitbox(Shader &shader, const Shape &boxHitboxShape,
                            const Shape &cylinderHitboxShape) const {
        if (_hitbox.shape() == Hitbox::Shape::Cylinder) {
            cylinderHitboxShape.draw(shader, getCylinderHitboxTransformMatrix(),
                                     GL_LINES);
        } else {
            boxHitboxShape.draw(shader, getHitboxTransformMatrix(), GL_LINES);
        }
    }

    virtual void drawHitbox(Shader &shader, const Shape &hitboxShape) {
        drawHitbox(shader, hitboxShape, hitboxShape);
    }

    void setHitboxShape(Hitbox::Shape shape) { _hitbox.setShape(shape); }
    Hitbox::Shape hitboxShape() const { return _hitbox.shape(); }

    void setCylinderAxis(Hitbox::CylinderAxis axis) {
        _hitbox.setCylinderAxis(axis);
    }
    Hitbox::CylinderAxis cylinderAxis() const { return _hitbox.cylinderAxis(); }
};

#endif