/*
 * Utility Functions Implementation
 */

 #include "utils.h"
 #include <math.h>
 
 double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
     double rad_lat1 = lat1 * (PI / 180.0);
     double rad_lon1 = lon1 * (PI / 180.0);
     double rad_lat2 = lat2 * (PI / 180.0);
     double rad_lon2 = lon2 * (PI / 180.0);
     double d_lon = rad_lon2 - rad_lon1;
     double d_lat = rad_lat2 - rad_lat1;
     double a = sin(d_lat / 2) * sin(d_lat / 2) +
                cos(rad_lat1) * cos(rad_lat2) *
                sin(d_lon / 2) * sin(d_lon / 2);
     double c = 2 * asin(sqrt(a));
     return EARTH_RADIUS_KM * c;
 }