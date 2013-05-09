#ifndef GRID_UTILS
#define GRID_UTILS

int g_lon_dim = 360;
int g_lat_dim = 66;
int g_plev_dim = 27;

//X-axis
double g_lon[] = {-279.5, -278.5, -277.5, -276.5, -275.5, -274.5, -273.5, -272.5, 
    -271.5, -270.5, -269.5, -268.5, -267.5, -266.5, -265.5, -264.5, -263.5, 
    -262.5, -261.5, -260.5, -259.5, -258.5, -257.5, -256.5, -255.5, -254.5, 
    -253.5, -252.5, -251.5, -250.5, -249.5, -248.5, -247.5, -246.5, -245.5, 
    -244.5, -243.5, -242.5, -241.5, -240.5, -239.5, -238.5, -237.5, -236.5, 
    -235.5, -234.5, -233.5, -232.5, -231.5, -230.5, -229.5, -228.5, -227.5, 
    -226.5, -225.5, -224.5, -223.5, -222.5, -221.5, -220.5, -219.5, -218.5, 
    -217.5, -216.5, -215.5, -214.5, -213.5, -212.5, -211.5, -210.5, -209.5, 
    -208.5, -207.5, -206.5, -205.5, -204.5, -203.5, -202.5, -201.5, -200.5, 
    -199.5, -198.5, -197.5, -196.5, -195.5, -194.5, -193.5, -192.5, -191.5, 
    -190.5, -189.5, -188.5, -187.5, -186.5, -185.5, -184.5, -183.5, -182.5, 
    -181.5, -180.5, -179.5, -178.5, -177.5, -176.5, -175.5, -174.5, -173.5, 
    -172.5, -171.5, -170.5, -169.5, -168.5, -167.5, -166.5, -165.5, -164.5, 
    -163.5, -162.5, -161.5, -160.5, -159.5, -158.5, -157.5, -156.5, -155.5, 
    -154.5, -153.5, -152.5, -151.5, -150.5, -149.5, -148.5, -147.5, -146.5, 
    -145.5, -144.5, -143.5, -142.5, -141.5, -140.5, -139.5, -138.5, -137.5, 
    -136.5, -135.5, -134.5, -133.5, -132.5, -131.5, -130.5, -129.5, -128.5, 
    -127.5, -126.5, -125.5, -124.5, -123.5, -122.5, -121.5, -120.5, -119.5, 
    -118.5, -117.5, -116.5, -115.5, -114.5, -113.5, -112.5, -111.5, -110.5, 
    -109.5, -108.5, -107.5, -106.5, -105.5, -104.5, -103.5, -102.5, -101.5, 
    -100.5, -99.5, -98.5, -97.5, -96.5, -95.5, -94.5, -93.5, -92.5, -91.5, 
    -90.5, -89.5, -88.5, -87.5, -86.5, -85.5, -84.5, -83.5, -82.5, -81.5, 
    -80.5, -79.5, -78.5, -77.5, -76.5, -75.5, -74.5, -73.5, -72.5, -71.5, 
    -70.5, -69.5, -68.5, -67.5, -66.5, -65.5, -64.5, -63.5, -62.5, -61.5, 
    -60.5, -59.5, -58.5, -57.5, -56.5, -55.5, -54.5, -53.5, -52.5, -51.5, 
    -50.5, -49.5, -48.5, -47.5, -46.5, -45.5, -44.5, -43.5, -42.5, -41.5, 
    -40.5, -39.5, -38.5, -37.5, -36.5, -35.5, -34.5, -33.5, -32.5, -31.5, 
    -30.5, -29.5, -28.5, -27.5, -26.5, -25.5, -24.5, -23.5, -22.5, -21.5, 
    -20.5, -19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, 
    -10.5, -9.5, -8.5, -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 
    1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5, 13.5, 
    14.5, 15.5, 16.5, 17.5, 18.5, 19.5, 20.5, 21.5, 22.5, 23.5, 24.5, 25.5, 
    26.5, 27.5, 28.5, 29.5, 30.5, 31.5, 32.5, 33.5, 34.5, 35.5, 36.5, 37.5, 
    38.5, 39.5, 40.5, 41.5, 42.5, 43.5, 44.5, 45.5, 46.5, 47.5, 48.5, 49.5, 
    50.5, 51.5, 52.5, 53.5, 54.5, 55.5, 56.5, 57.5, 58.5, 59.5, 60.5, 61.5, 
    62.5, 63.5, 64.5, 65.5, 66.5, 67.5, 68.5, 69.5, 70.5, 71.5, 72.5, 73.5, 
	74.5, 75.5, 76.5, 77.5, 78.5, 79.5} ;

//Y-axis
double g_lat[] = {-19.14525, -18.31191, -17.50033, -16.71014, -15.94058, -15.19058, 
    -14.45871, -13.74323, -13.04212, -12.35312, -11.67377, -11.00143, 
    -10.33333, -9.666666, -9.002051, -8.343542, -7.695041, -7.060205, 
    -6.442354, -5.844389, -5.268724, -4.717221, -4.191149, -3.691149, 
    -3.217221, -2.768724, -2.344389, -1.942354, -1.560205, -1.195041, 
    -0.843542, -0.5020515, -0.1666663, 0.166667, 0.5020523, 0.8435428, 
    1.195042, 1.560206, 1.942354, 2.34439, 2.768725, 3.217222, 3.69115, 
    4.19115, 4.717222, 5.268725, 5.84439, 6.442354, 7.060205, 7.695042, 
    8.343543, 9.002052, 9.666667, 10.33333, 11.00143, 11.67377, 12.35313, 
    13.04212, 13.74323, 14.45871, 15.19058, 15.94058, 16.71014, 17.50033, 
	18.31191, 19.14525} ;


double g_plev[] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95, 105, 115, 125, 135, 145, 
    155, 165, 175, 185, 195, 205, 215, 225, 236.1228, 250.6, 270.6208, 
	298.3049} ;


#endif