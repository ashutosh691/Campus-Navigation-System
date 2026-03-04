/*
 * Utility Functions - Reduced to core geographic calculations.
 */

 #ifndef UTILS_H
 #define UTILS_H
 
 #define PI 3.14159265358979323846
 #define EARTH_RADIUS_KM 6371.0
 
 double haversine_distance(double lat1, double lon1, double lat2, double lon2);
 
 #endif // UTILS_H