/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    traffic_types.h
 * Purpose: Small shared types used by the signal, light, and car modules.
 * Subsystem owner: Mario Ivezaj
 */
#ifndef TRAFFIC_TYPES_H
#define TRAFFIC_TYPES_H

/* The two paired directions of the intersection. */
typedef enum {
    LANE_NS = 0,      /* north/south pair */
    LANE_EW = 1,      /* east/west pair   */
    LANE_COUNT = 2
} lane_t;

/* What one signal head is showing. */
typedef enum {
    SIG_RED = 0,
    SIG_YELLOW = 1,
    SIG_GREEN = 2
} signal_t;

#endif /* TRAFFIC_TYPES_H */
