#include <math.h>
#include <stdbool.h>

#include "nanogui.h"

typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

typedef struct {
    vec3_t origin;
    vec3_t direction;
} ray_t;

typedef struct {
    vec3_t center;
    float radius;
} sphere_t;

static float vec3_length(vec3_t v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static vec3_t vec3_normalized(vec3_t v) {
    float l = vec3_length(v);
    return (vec3_t) {v.x / l, v.y / l, v.z / l};
}

static vec3_t vec3_scale(vec3_t v, float scale) {
    return (vec3_t) {v.x * scale, v.y * scale, v.z * scale};
}

static vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t) {a.x + b.x, a.y + b.y, a.z + b.z};
}

static vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t) {a.x - b.x, a.y - b.y, a.z - b.z};
}

static float clampf(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static bool ray_sphere_intersection(ray_t r, sphere_t s, float *t) {
    vec3_t oc = {r.origin.x - s.center.x, r.origin.y - s.center.y, r.origin.z - s.center.z};
    float a = r.direction.x * r.direction.x + r.direction.y * r.direction.y + r.direction.z * r.direction.z;
    float b = 2.0f * (oc.x * r.direction.x + oc.y * r.direction.y + oc.z * r.direction.z);
    float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - s.radius * s.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return false;
    } else {
        *t = (-b - sqrtf(discriminant)) / (2.0f * a);
        return true;
    }
}

static bool ray_sphere_intersection_with_normal(ray_t r, sphere_t s, float *t, vec3_t *normal) {
    if (ray_sphere_intersection(r, s, t)) {
        vec3_t pos = vec3_add(r.origin, vec3_scale(r.direction, *t));
        *normal = vec3_scale(vec3_sub(pos, s.center), 1.0f / s.radius);
        return true;
    } else {
        return false;
    }
}

int main() {
    int width = 640, height = 480;
    nano_gui_create_fixed_size_window(width, height);

    // Main loop
    while (nano_gui_process_events()) {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                float x = (float) i / (float) width;
                float y = (float) j / (float) height;
                // Transform to NDC and correct aspect ratio
                x = 2 * x - 1;
                y = 2 * y - 1;
                x *= (float) width / (float) height;
                ray_t ray;
                ray.direction = vec3_normalized((vec3_t) {x, y, -1.0f});
                ray.origin = (vec3_t) {0, 0, 0};
                sphere_t sphere = {{0, 0, -2}, 1.0f};

                float depth;
                vec3_t normal;
                if (ray_sphere_intersection_with_normal(ray, sphere, &depth, &normal)) {
//                    uint8_t depth_clamped = (uint8_t) clampf(depth * 255, 0, 255);
                    uint8_t r = (uint8_t) clampf(normal.x * 255, 0, 255);
                    uint8_t g = (uint8_t) clampf(normal.y * 255, 0, 255);
                    uint8_t b = (uint8_t) clampf(normal.z * 255, 0, 255);
                    nano_gui_draw_pixel(i, j, r, g, b);
                } else {
                    nano_gui_draw_pixel(i, j, 0, 0, 0);
                }
            }
        }
    }
    return 0;
}