#ifndef HITBOX_H
#define HITBOX_H

#include <array>
#include <algorithm>
#include <cmath>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils.h"

class Hitbox {
  public:
    enum class Shape {
        Box,
        Cylinder,
    };

  private:
    Shape _shape = Shape::Box;

    // Shared
    glm::vec3 _centerOffset = glm::vec3(0.0f);

    // Box (half extents in local space)
    glm::vec3 _boxHalfExtents = glm::vec3(0.0f);

    // Cylinder (local Z axis, oriented by entity rotation)
    float _cylRadius = 0.0f;
    float _cylHalfHeight = 0.0f;

    struct OBB {
        glm::vec3 center;
        glm::vec3 axis[3];
        float half[3];
    };

    struct Capsule {
        glm::vec3 a;
        glm::vec3 b;
        float r;
    };

    static float clamp01(float v) {
        if (v < 0.0f)
            return 0.0f;
        if (v > 1.0f)
            return 1.0f;
        return v;
    }

    static float pointAABBDistanceSquared(const glm::vec3 &p,
                                          const glm::vec3 &bmin,
                                          const glm::vec3 &bmax) {
        float dx = 0.0f;
        if (p.x < bmin.x)
            dx = bmin.x - p.x;
        else if (p.x > bmax.x)
            dx = p.x - bmax.x;

        float dy = 0.0f;
        if (p.y < bmin.y)
            dy = bmin.y - p.y;
        else if (p.y > bmax.y)
            dy = p.y - bmax.y;

        float dz = 0.0f;
        if (p.z < bmin.z)
            dz = bmin.z - p.z;
        else if (p.z > bmax.z)
            dz = p.z - bmax.z;

        return dx * dx + dy * dy + dz * dz;
    }

    static bool segmentIntersectsAABB(const glm::vec3 &p0, const glm::vec3 &p1,
                                      const glm::vec3 &bmin,
                                      const glm::vec3 &bmax) {
        const glm::vec3 d = p1 - p0;
        float tmin = 0.0f;
        float tmax = 1.0f;
        const float eps = 1e-8f;

        const float p0v[3] = {p0.x, p0.y, p0.z};
        const float dv[3] = {d.x, d.y, d.z};
        const float minv[3] = {bmin.x, bmin.y, bmin.z};
        const float maxv[3] = {bmax.x, bmax.y, bmax.z};

        for (int i = 0; i < 3; ++i) {
            if (std::abs(dv[i]) < eps) {
                if (p0v[i] < minv[i] || p0v[i] > maxv[i])
                    return false;
                continue;
            }
            const float invD = 1.0f / dv[i];
            float t1 = (minv[i] - p0v[i]) * invD;
            float t2 = (maxv[i] - p0v[i]) * invD;
            if (t1 > t2)
                std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax)
                return false;
        }
        return true;
    }

    static float segmentAABBDistanceSquared(const glm::vec3 &p0,
                                            const glm::vec3 &p1,
                                            const glm::vec3 &bmin,
                                            const glm::vec3 &bmax) {
        if (segmentIntersectsAABB(p0, p1, bmin, bmax))
            return 0.0f;

        const glm::vec3 d = p1 - p0;
        const float eps = 1e-8f;

        // Collect clamp-state change events in t-space.
        std::array<float, 8> ts;
        int n = 0;
        auto addT = [&](float t) {
            if (t < 0.0f || t > 1.0f)
                return;
            ts[n++] = t;
        };

        addT(0.0f);
        addT(1.0f);

        for (int i = 0; i < 3; ++i) {
            const float di = d[i];
            if (std::abs(di) < eps)
                continue;
            const float t1 = (bmin[i] - p0[i]) / di;
            const float t2 = (bmax[i] - p0[i]) / di;
            addT(t1);
            addT(t2);
        }

        std::sort(ts.begin(), ts.begin() + n);
        int m = 0;
        for (int i = 0; i < n; ++i) {
            if (m == 0 || std::abs(ts[i] - ts[m - 1]) > 1e-6f) {
                ts[m++] = ts[i];
            }
        }

        auto eval = [&](float t) {
            return pointAABBDistanceSquared(p0 + d * t, bmin, bmax);
        };

        float best = std::numeric_limits<float>::infinity();
        best = std::min(best, eval(0.0f));
        best = std::min(best, eval(1.0f));

        for (int i = 0; i < m - 1; ++i) {
            const float t0 = ts[i];
            const float t1 = ts[i + 1];
            if (t1 - t0 < 1e-7f)
                continue;

            const float tm = 0.5f * (t0 + t1);

            // Determine clamp status per axis in this interval using midpoint.
            float a = 0.0f;
            float b = 0.0f;
            float c = 0.0f;
            for (int k = 0; k < 3; ++k) {
                const float pm = p0[k] + d[k] * tm;
                if (pm < bmin[k]) {
                    const float bound = bmin[k];
                    a += d[k] * d[k];
                    b += 2.0f * d[k] * (p0[k] - bound);
                    const float e = (p0[k] - bound);
                    c += e * e;
                } else if (pm > bmax[k]) {
                    const float bound = bmax[k];
                    a += d[k] * d[k];
                    b += 2.0f * d[k] * (p0[k] - bound);
                    const float e = (p0[k] - bound);
                    c += e * e;
                }
            }

            best = std::min(best, eval(t0));
            best = std::min(best, eval(t1));

            if (a > eps) {
                const float tStar = -b / (2.0f * a);
                if (tStar > t0 && tStar < t1) {
                    best = std::min(best, eval(tStar));
                }
            } else {
                // Constant distance over this interval (all axes inside).
                best = std::min(best, c);
            }
        }

        return best;
    }

    static OBB extractOBBFromMatrix(const glm::mat4 &mat) {
        OBB obb;
        obb.center = glm::vec3(mat[3]);
        for (int i = 0; i < 3; ++i) {
            const glm::vec3 col = glm::vec3(mat[i]);
            const float len = glm::length(col);
            if (len < 1e-8f) {
                obb.axis[i] = glm::vec3(0.0f);
                obb.half[i] = 0.0f;
            } else {
                obb.axis[i] = col / len;
                obb.half[i] = 0.5f * len;
            }
        }
        return obb;
    }

    static float capsuleOBBDistanceSquared(const Capsule &cap, const OBB &obb) {
        auto toLocal = [&](const glm::vec3 &p) {
            const glm::vec3 d = p - obb.center;
            return glm::vec3(glm::dot(d, obb.axis[0]), glm::dot(d, obb.axis[1]),
                             glm::dot(d, obb.axis[2]));
        };

        const glm::vec3 lp0 = toLocal(cap.a);
        const glm::vec3 lp1 = toLocal(cap.b);
        const glm::vec3 bmin(-obb.half[0], -obb.half[1], -obb.half[2]);
        const glm::vec3 bmax(obb.half[0], obb.half[1], obb.half[2]);
        const float d2 = segmentAABBDistanceSquared(lp0, lp1, bmin, bmax);
        const float r2 = cap.r * cap.r;
        if (d2 <= r2)
            return 0.0f;
        return d2 - r2;
    }

    static float segmentSegmentDistanceSquared(const glm::vec3 &p1,
                                               const glm::vec3 &q1,
                                               const glm::vec3 &p2,
                                               const glm::vec3 &q2) {
        const glm::vec3 d1 = q1 - p1;
        const glm::vec3 d2 = q2 - p2;
        const glm::vec3 r = p1 - p2;
        const float a = glm::dot(d1, d1);
        const float e = glm::dot(d2, d2);
        const float eps = 1e-8f;

        float s = 0.0f;
        float t = 0.0f;

        if (a <= eps && e <= eps) {
            return glm::dot(r, r);
        }
        if (a <= eps) {
            s = 0.0f;
            t = clamp01(glm::dot(d2, r) / e);
        } else {
            const float c = glm::dot(d1, r);
            if (e <= eps) {
                t = 0.0f;
                s = clamp01(-c / a);
            } else {
                const float b = glm::dot(d1, d2);
                const float f = glm::dot(d2, r);
                const float denom = a * e - b * b;
                if (denom != 0.0f)
                    s = clamp01((b * f - c * e) / denom);
                else
                    s = 0.0f;

                t = (b * s + f) / e;
                if (t < 0.0f) {
                    t = 0.0f;
                    s = clamp01(-c / a);
                } else if (t > 1.0f) {
                    t = 1.0f;
                    s = clamp01((b - c) / a);
                }
            }
        }

        const glm::vec3 c1 = p1 + d1 * s;
        const glm::vec3 c2 = p2 + d2 * t;
        const glm::vec3 diff = c1 - c2;
        return glm::dot(diff, diff);
    }

    static float cylinderProjectionRadius(const glm::vec3 &axisUnit,
                                          float halfHeight, float radius,
                                          const glm::vec3 &testAxisUnit) {
        const float du = glm::dot(axisUnit, testAxisUnit);
        const float axial = halfHeight * std::abs(du);
        // radial component is scaled by how perpendicular the test axis is
        // to the cylinder axis.
        const float perp2 = std::max(0.0f, 1.0f - du * du);
        const float radial = radius * std::sqrt(perp2);
        return axial + radial;
    }

    static bool separatedOnAxis(const glm::vec3 &delta,
                                const glm::vec3 &axisUnit,
                                const glm::vec3 &aAxisUnit, float aHalfHeight,
                                float aRadius, const glm::vec3 &bAxisUnit,
                                float bHalfHeight, float bRadius) {
        const float dist = std::abs(glm::dot(delta, axisUnit));
        const float ra =
            cylinderProjectionRadius(aAxisUnit, aHalfHeight, aRadius, axisUnit);
        const float rb =
            cylinderProjectionRadius(bAxisUnit, bHalfHeight, bRadius, axisUnit);
        return dist > (ra + rb);
    }

    inline Capsule capsuleFromCylinder(const glm::vec3 &pos,
                                       const glm::vec3 &rot) const {
        const glm::mat4 R = getEulerRotationMatrix(rot);
        glm::vec3 axis = glm::vec3(R * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        const float len2 = glm::dot(axis, axis);
        if (len2 < 1e-8f)
            axis = glm::vec3(0.0f, 0.0f, 1.0f);
        else
            axis /= std::sqrt(len2);

        const glm::vec3 centerOffsetWorld =
            glm::vec3(R * glm::vec4(_centerOffset, 0.0f));
        const glm::vec3 center = pos + centerOffsetWorld;
        const glm::vec3 a = center - axis * _cylHalfHeight;
        const glm::vec3 b = center + axis * _cylHalfHeight;
        return Capsule{a, b, _cylRadius};
    }

    inline bool obbIntersectsOBB(const glm::mat4 &matA,
                                 const glm::mat4 &matB) const {
        glm::vec3 posA = glm::vec3(matA[3]);
        glm::vec3 posB = glm::vec3(matB[3]);
        glm::vec3 Delta = posB - posA;

        glm::vec3 A[3] = {glm::normalize(glm::vec3(matA[0])),
                          glm::normalize(glm::vec3(matA[1])),
                          glm::normalize(glm::vec3(matA[2]))};
        glm::vec3 B[3] = {glm::normalize(glm::vec3(matB[0])),
                          glm::normalize(glm::vec3(matB[1])),
                          glm::normalize(glm::vec3(matB[2]))};

        float eA[3] = {glm::length(glm::vec3(matA[0])) * 0.5f,
                       glm::length(glm::vec3(matA[1])) * 0.5f,
                       glm::length(glm::vec3(matA[2])) * 0.5f};
        float eB[3] = {glm::length(glm::vec3(matB[0])) * 0.5f,
                       glm::length(glm::vec3(matB[1])) * 0.5f,
                       glm::length(glm::vec3(matB[2])) * 0.5f};

        float R[3][3], AbsR[3][3];
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                R[i][j] = glm::dot(A[i], B[j]);
                AbsR[i][j] = std::abs(R[i][j]) + 1e-9f;
            }
        }

        float ra, rb;

        for (int i = 0; i < 3; i++) {
            ra = eA[i];
            rb = eB[0] * AbsR[i][0] + eB[1] * AbsR[i][1] + eB[2] * AbsR[i][2];
            if (std::abs(glm::dot(Delta, A[i])) > ra + rb)
                return false;
        }

        for (int i = 0; i < 3; i++) {
            ra = eA[0] * AbsR[0][i] + eA[1] * AbsR[1][i] + eA[2] * AbsR[2][i];
            rb = eB[i];
            if (std::abs(glm::dot(Delta, B[i])) > ra + rb)
                return false;
        }

        // Cross products
        ra = eA[1] * AbsR[2][0] + eA[2] * AbsR[1][0];
        rb = eB[1] * AbsR[0][2] + eB[2] * AbsR[0][1];
        if (std::abs(glm::dot(Delta, A[2]) * R[1][0] -
                     glm::dot(Delta, A[1]) * R[2][0]) > ra + rb)
            return false;

        ra = eA[1] * AbsR[2][1] + eA[2] * AbsR[1][1];
        rb = eB[0] * AbsR[0][2] + eB[2] * AbsR[0][0];
        if (std::abs(glm::dot(Delta, A[2]) * R[1][1] -
                     glm::dot(Delta, A[1]) * R[2][1]) > ra + rb)
            return false;

        ra = eA[1] * AbsR[2][2] + eA[2] * AbsR[1][2];
        rb = eB[0] * AbsR[0][1] + eB[1] * AbsR[0][0];
        if (std::abs(glm::dot(Delta, A[2]) * R[1][2] -
                     glm::dot(Delta, A[1]) * R[2][2]) > ra + rb)
            return false;

        ra = eA[0] * AbsR[2][0] + eA[2] * AbsR[0][0];
        rb = eB[1] * AbsR[1][2] + eB[2] * AbsR[1][1];
        if (std::abs(glm::dot(Delta, A[0]) * R[2][0] -
                     glm::dot(Delta, A[2]) * R[0][0]) > ra + rb)
            return false;

        ra = eA[0] * AbsR[2][1] + eA[2] * AbsR[0][1];
        rb = eB[0] * AbsR[1][2] + eB[2] * AbsR[1][0];
        if (std::abs(glm::dot(Delta, A[0]) * R[2][1] -
                     glm::dot(Delta, A[2]) * R[0][1]) > ra + rb)
            return false;

        ra = eA[0] * AbsR[2][2] + eA[2] * AbsR[0][2];
        rb = eB[0] * AbsR[1][1] + eB[1] * AbsR[1][0];
        if (std::abs(glm::dot(Delta, A[0]) * R[2][2] -
                     glm::dot(Delta, A[2]) * R[0][2]) > ra + rb)
            return false;

        ra = eA[0] * AbsR[1][0] + eA[1] * AbsR[0][0];
        rb = eB[1] * AbsR[2][2] + eB[2] * AbsR[2][1];
        if (std::abs(glm::dot(Delta, A[1]) * R[0][0] -
                     glm::dot(Delta, A[0]) * R[1][0]) > ra + rb)
            return false;

        ra = eA[0] * AbsR[1][1] + eA[1] * AbsR[0][1];
        rb = eB[0] * AbsR[2][2] + eB[2] * AbsR[2][0];
        if (std::abs(glm::dot(Delta, A[1]) * R[0][1] -
                     glm::dot(Delta, A[0]) * R[1][1]) > ra + rb)
            return false;

        ra = eA[0] * AbsR[1][2] + eA[1] * AbsR[0][2];
        rb = eB[0] * AbsR[2][1] + eB[1] * AbsR[2][0];
        if (std::abs(glm::dot(Delta, A[1]) * R[0][2] -
                     glm::dot(Delta, A[0]) * R[1][2]) > ra + rb)
            return false;

        return true;
    }

  public:
    Hitbox() = default;

    void setShape(Shape shape) { _shape = shape; }
    Shape shape() const { return _shape; }

    const glm::vec3 &centerOffset() const { return _centerOffset; }
    const glm::vec3 &boxHalfExtents() const { return _boxHalfExtents; }
    float cylinderRadius() const { return _cylRadius; }
    float cylinderHalfHeight() const { return _cylHalfHeight; }

    void setFromBounds(const glm::vec3 &minB, const glm::vec3 &maxB) {
        _centerOffset = (minB + maxB) * 0.5f;
        _boxHalfExtents = (maxB - minB) * 0.5f;

        // Requested formula:
        // vertical cylinder, radius = max(maxX, maxY), height = maxZ - minZ
        const float radius = std::max(maxB.x, maxB.y);
        _cylRadius = std::max(0.0f, radius);

        const float height = maxB.z - minB.z;
        _cylHalfHeight = 0.5f * std::max(0.0f, height);
    }

    inline float containingSphereRadius() const {
        if (_shape == Shape::Cylinder) {
            const float r = _cylRadius;
            const float h = _cylHalfHeight;
            return std::sqrt(r * r + h * h);
        }
        const float maxHalfExtent =
            std::max({_boxHalfExtents.x, _boxHalfExtents.y, _boxHalfExtents.z});
        return 1.7320508f * maxHalfExtent;
    }

    inline glm::mat4 boxTransformMatrix(const glm::vec3 &pos,
                                        const glm::vec3 &rot) const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 R = getEulerRotationMatrix(rot);
        glm::mat4 C = glm::translate(glm::mat4(1.0f), _centerOffset);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), _boxHalfExtents * 2.0f);
        return T * R * C * S;
    }

    inline glm::mat4 cylinderTransformMatrix(const glm::vec3 &pos,
                                             const glm::vec3 &rot) const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 R = getEulerRotationMatrix(rot);
        glm::mat4 C = glm::translate(glm::mat4(1.0f), _centerOffset);
        glm::mat4 S = glm::scale(glm::mat4(1.0f),
                                 glm::vec3(_cylRadius * 2.0f, _cylRadius * 2.0f,
                                           _cylHalfHeight * 2.0f));
        return T * R * C * S;
    }

    inline bool intersects(const Hitbox &other, const glm::vec3 &pos,
                           const glm::vec3 &rot, const glm::vec3 &otherPos,
                           const glm::vec3 &otherRot) const {
        if (_shape == Shape::Box && other._shape == Shape::Box) {
            return obbIntersectsOBB(
                boxTransformMatrix(pos, rot),
                other.boxTransformMatrix(otherPos, otherRot));
        }

        // Cylinder-cylinder: oriented capped cylinder SAT (flat end-caps), with
        // a cheap capsule rejection test up-front.
        if (_shape == Shape::Cylinder && other._shape == Shape::Cylinder) {
            const glm::mat4 RA = getEulerRotationMatrix(rot);
            const glm::mat4 RB = getEulerRotationMatrix(otherRot);

            glm::vec3 uA = glm::vec3(RA * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
            glm::vec3 uB = glm::vec3(RB * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
            const float uA2 = glm::dot(uA, uA);
            const float uB2 = glm::dot(uB, uB);
            if (uA2 < 1e-8f)
                uA = glm::vec3(0.0f, 0.0f, 1.0f);
            else
                uA /= std::sqrt(uA2);
            if (uB2 < 1e-8f)
                uB = glm::vec3(0.0f, 0.0f, 1.0f);
            else
                uB /= std::sqrt(uB2);

            const glm::vec3 cA =
                pos + glm::vec3(RA * glm::vec4(_centerOffset, 0.0f));
            const glm::vec3 cB =
                otherPos + glm::vec3(RB * glm::vec4(other._centerOffset, 0.0f));
            const glm::vec3 delta = cB - cA;

            const float hA = _cylHalfHeight;
            const float rA = _cylRadius;
            const float hB = other._cylHalfHeight;
            const float rB = other._cylRadius;

            // Fast early-out (safe rejection):
            // Each capped cylinder is a subset of a capsule with the same axis
            // segment and radius. If those capsules do NOT intersect, the
            // cylinders cannot intersect.
            const glm::vec3 segA0 = cA - uA * hA;
            const glm::vec3 segA1 = cA + uA * hA;
            const glm::vec3 segB0 = cB - uB * hB;
            const glm::vec3 segB1 = cB + uB * hB;
            const float rsum = rA + rB;
            const float capD2 =
                segmentSegmentDistanceSquared(segA0, segA1, segB0, segB1);
            if (capD2 > rsum * rsum) {
                return false;
            }

            // Parallel / nearly-parallel cylinders: exact test.
            glm::vec3 n = glm::cross(uA, uB);
            const float n2 = glm::dot(n, n);
            // n2 = sin^2(angle). When pushing plates, axes are often almost
            // parallel; treat that as parallel to avoid unstable
            // normalizations.
            if (n2 < 1e-4f) {
                const float dz = std::abs(glm::dot(delta, uA));
                if (dz > (hA + hB))
                    return false;
                const glm::vec3 perp = delta - uA * glm::dot(delta, uA);
                const float d2 = glm::dot(perp, perp);
                const float r = rA + rB;
                return d2 <= r * r;
            }

            n /= std::sqrt(n2);

            // Candidate separating axes.
            const glm::vec3 a1 = uA;
            const glm::vec3 a2 = uB;
            const glm::vec3 a3 = n;
            // These can be near-zero if n is small; guard normalization.
            const glm::vec3 c4 = glm::cross(n, uA);
            const glm::vec3 c5 = glm::cross(n, uB);
            const float c42 = glm::dot(c4, c4);
            const float c52 = glm::dot(c5, c5);
            const glm::vec3 a4 = (c42 > 1e-12f) ? (c4 / std::sqrt(c42)) : a3;
            const glm::vec3 a5 = (c52 > 1e-12f) ? (c5 / std::sqrt(c52)) : a3;

            if (separatedOnAxis(delta, a1, uA, hA, rA, uB, hB, rB))
                return false;
            if (separatedOnAxis(delta, a2, uA, hA, rA, uB, hB, rB))
                return false;
            if (separatedOnAxis(delta, a3, uA, hA, rA, uB, hB, rB))
                return false;
            if (separatedOnAxis(delta, a4, uA, hA, rA, uB, hB, rB))
                return false;
            if (separatedOnAxis(delta, a5, uA, hA, rA, uB, hB, rB))
                return false;
            return true;
        }

        // Mixed: approximate cylinder as a capsule for OBB distance test.
        // This is conservative (may report collisions slightly early).
        const Hitbox *cyl = this;
        const Hitbox *box = &other;
        glm::vec3 cylPos = pos;
        glm::vec3 cylRot = rot;
        glm::vec3 boxPos = otherPos;
        glm::vec3 boxRot = otherRot;

        if (_shape == Shape::Box && other._shape == Shape::Cylinder) {
            cyl = &other;
            box = this;
            cylPos = otherPos;
            cylRot = otherRot;
            boxPos = pos;
            boxRot = rot;
        }

        if (cyl->_shape != Shape::Cylinder || box->_shape != Shape::Box) {
            // Shouldn't happen.
            return false;
        }

        const Capsule cap = cyl->capsuleFromCylinder(cylPos, cylRot);

        const glm::mat4 boxMat = box->boxTransformMatrix(boxPos, boxRot);
        const OBB obb = extractOBBFromMatrix(boxMat);

        return capsuleOBBDistanceSquared(cap, obb) <= 0.0f;
    }
};

#endif
