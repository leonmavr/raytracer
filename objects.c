#include "objects.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define DEG_TO_RAD(degrees) ((degrees) * M_PI / 180.0)


vec3_i32_t ray_at(ray_t* ray, float t) {
    vec3_f_t v = vec3_f_scalmul(&ray->dir, t);
    vec3_i32_t t_times_dir = (vec3_i32_t) {(i32_t)v.x, (i32_t)v.y, (i32_t)v.z};
    return vec3_i32_add(&ray->origin, &t_times_dir);
}

vec3_i32_t ray_sphere_inters(ray_t* ray, sphere_t* sph) {
    // see https://raytracing.github.io/books/RayTracingInOneWeekend.html#addingasphere/ray-sphereintersection
    // for derivation and notation
    // ray(t) = A + tB, C = (Cx, Cy, Cz) centre of the sphere
    vec3_i32_t A = ray->origin;
    vec3_f_t B = ray->dir;
    vec3_i32_t C = sph->origin;
    i32_t r = sph->rad;
    const float a = vec3_f_dot(&B, &B);
    // b = 2B (C - A)
    vec3_i32_t utmp = vec3_i32_sub(&C, &A);
    vec3_f_t ftmp = (vec3_f_t){utmp.x, utmp.y, utmp.z};
    const float b = 2*vec3_f_dot(&B, &ftmp);
    const float c = vec3_i32_dot(&utmp, &utmp) - r*r;
    // the solutions of the 2nd order equation for t
    float discr = sqrt(b*b - 4*a*c);
    if (discr > 0) {
        const float sqrt_discr = sqrt(discr);
        const float t1 = (-b + sqrt_discr)/(2*a);
        const float t2 = (-b - sqrt_discr)/(2*a);
        // keep the smallest (t0)
        const float t0 = (t1 < t2) ? t1 : t2;
        // coordinates of intersection (A+t0*B)
        vec3_i32_t inters = (vec3_i32_t) {A.x + t0*B.x, A.y + t0*B.y,
                                          A.z + t0*B.z};
        return inters;
    } else // TODO: better way to describe intersection
        return (vec3_i32_t) {0, 0, 0};
}

/* get outward normal positioned at origin given a point on a sphere */
vec3_f_t sphere_unit_normal(sphere_t* sph, vec3_i32_t* where) {
    vec3_i32_t normal = vec3_i32_sub(&sph->origin, where);
    const float r = sph->rad;
    return (vec3_f_t) {(1.0*normal.x)/r, (1.0*normal.y)/r, (1.0*normal.z)/r};
}

static vec3_f_t vec3_f_unit_random() {
    // acosθsinϕ,asinθsinϕ,acosϕ)
    float phi = (float)rand() / RAND_MAX * M_PI;
    float theta = (float)rand() / RAND_MAX * M_2_PI;
    float x = cos(theta) * sin(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(phi);
    return (vec3_f_t) {x, y, z};
}

void cam_set(camera_t* cam, i32_t cx, i32_t cy, i32_t f, float fov_deg) {
    cam->cx = cx;
    cam->cy = cy;
    cam->f = f;
    cam->fovy_rad = DEG_TO_RAD(fov_deg);
    cam->fovx_rad = (float)WIDTH/HEIGHT*DEG_TO_RAD(fov_deg);
}

void light_add(light_t** lights, light_type_t type, float intensity, vec3_i32_t* dir) {
    if (lights == NULL) {
        lights = (light_t**) malloc(sizeof(light_t*));
        lights[0] = malloc(sizeof(light_t));
        // set the first light source
        lights[0]->type = type;
        lights[0]->intensity = 1.0;
        if (type == LIGHT_DIR)
            lights[0]->geometry.dir = *dir;
        else if (type == LIGHT_POINT)
            lights[0]->geometry.point = *dir;
    } else{
        const size_t n = sizeof(lights) / sizeof(lights[0]);
        lights = (light_t**) realloc(lights, sizeof(light_t*) * (n+1));
        lights[n] = (light_t*) malloc(sizeof(light_t));
        lights[n]->type = type;
        lights[n]->intensity = intensity;
        if (type == LIGHT_DIR)
            lights[n]->geometry.dir = *dir;
        else if (type == LIGHT_POINT)
            lights[n]->geometry.point = *dir;
        // normalise intensities
        float sum_intty = .0;
        for (size_t i = 0; i < n+1; ++i)
            sum_intty += lights[i]->intensity;
        for (size_t i = 0; i < n+1; ++i)
            lights[i]->intensity /= sum_intty;
    }
}

bool cam_is_visible(camera_t* cam, vec3_i32_t* p) {
    // height and width of fov pyramid
    i32_t dy = HEIGHT/tan(cam->fovy_rad/2);
    i32_t dx = WIDTH/HEIGHT*dy;
    return (abs(p->x) <= dx + cam->cx) && (abs(p->y) <= dy + cam->cy);
}

void ray_set(ray_t* ray, vec3_i32_t* start, vec3_i32_t* end) {
    ray->origin.x = start->x; 
    ray->origin.y = start->y; 
    ray->origin.z = start->z; 
    vec3_i32_t diff = vec3_i32_sub(start, end);
    vec3_f_t fdiff = (vec3_f_t) {diff.x, diff.y, diff.z};
    ray->dir = vec3_f_get_unit(&fdiff);
}
