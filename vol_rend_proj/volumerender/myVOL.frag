uniform bool grid;
varying vec3  textureCoords;

uniform sampler3D volumeTex;
//uniform sampler3D volumeIndTex;
uniform sampler3D volumeClusTex; //SHOW_CLUSTER
uniform sampler1D tfTex;
uniform sampler3D mapCorr;

uniform sampler1D gridTexX;
uniform sampler1D gridTexY;
uniform sampler1D gridTexZ;

uniform int pearsonVariable;
uniform int pearsonCorrelationActive;

uniform float referenceX; // [0, 1]
uniform float referenceY; // [0, 1]
uniform float referenceZ; // [0, 1]

uniform float offsetX;

uniform float sampleSpacing;
uniform vec3  centralDifferenceSpacing;
uniform vec4  lightPar;
uniform vec3  eyePos;
uniform vec3  eyeDir;
uniform int   bOrtho;
uniform int   drawSlice;

//SHOW_CLUSTER
uniform int clusterId;
uniform int xblock;
uniform int yblock;
uniform int zblock;

struct Light
{
	float ambient_diffuse;
	float specular;
};

float lookUpValue ( float value )
{
	float nval;
	if ( value==0.0 ) return -10000000000.0; // dummy value
	else
	{
		nval = -1.0/9.0 + 10.0 * value / 9.0; // [0.1, 1] -> [0, 1]
		return nval;
		//return (dataMin + (dataMax - dataMin) * nval);
	}
}

vec3 getCorrelColor ( float p, bool smooth, bool vors )
{
	float r, g, b;
	vec4 rgba;

	// P in the range of [-1 and 1]
	// 0.75 0.375 1.0 violet
	// 0.0 0.0 1.0 blue
	// 0.0 1.0 1.0 cyan
	// 0.0 1.0 0.0 green
	// 1.0 1.0 0.0 yellow
	// 1.0 0.5 0.0 orange
	// 1.0 0.0 0.0 red

	//if (p<-1.0 || p>1.0)
	//  {r = 0.005; g = 0.005; b = 0.005;} // gray

	if ( p<-1.0 || p>1.0 ) return vec3 ( 0.0, 0.0, 0.0 );

	if ( !smooth )
	{
		if ( -1.0<=p && p<-0.8 )
			//{r = 0.75; g = 0.375; b = 1.0;} // violet
			{r = 0.8; g = 0.0; b = 1.0;} // violet
		else if ( -0.8<=p && p<-0.6 )
			{r = 0.0; g = 0.0; b = 0.75;} // dark blue
		else if ( -0.6<=p && p<-0.4 )
			{r = 0.0; g = 0.0; b = 1.0;} // blue
		else if ( -0.4<=p && p<-0.2 )
			{r = 0.0; g = 1.0; b = 1.0;} // cyan
		else if ( -0.2<=p && p<0.2 )
			{r = 1.0; g = 1.0; b = 1.0;} // white
		else if ( 0.2<=p && p<0.4 )
			{r = 1.0; g = 1.0; b = 0.0;} // yellow
		else if ( 0.4<=p && p<0.6 )
			{r = 1.0; g = 0.75; b = 0.0;} // light orange
		else if ( 0.6<=p && p<0.8 )
			{r = 1.0; g = 0.5; b = 0.0;} // orange
		else if ( 0.8<=p && p<1.0 )
			{r = 1.0; g = 0.0; b = 0.0;} // red
		else
			//{r = 0.5; g = 0.5; b = 0.5;} // gray
			{r = 0.0; g = 0.0; b = 0.0;} // black
		return vec3 ( r * 0.02, g * 0.02, b * 0.02 );
	}
	else
	{
		rgba = texture1D ( tfTex, 0.5 * p + 0.5 ); // [-1, 1] -> [0, 1]
		// for volume
		if ( vors )
			return vec3 ( rgba.r * 0.25, rgba.g * 0.25, rgba.b * 0.25 );
		// for slice
		else
			return vec3 ( rgba.r * 0.125, rgba.g * 0.125, rgba.b * 0.125 );
	}

}


vec4 getPearsonCoefColorLocation ( vec3 texPosition, bool vors, bool bval)
{
	//referenceY = 1 - referenceY;
	texPosition.x = texPosition.x + offsetX;
	if ( texPosition.x<0.0 ) texPosition.x = texPosition.x + 1.0;
	if ( texPosition.x>1.0 ) texPosition.x = texPosition.x - 1.0;

	int i, size = 6;//72
	float oneoversize = 1.0/6.0;//72
	float A[6];//72];
	float B[6];//72]

	float x, y, z;
	float a, zpos, refzpos;
	float zposcp;
	vec3 col;

	if ( grid )
	{
		x = texture1D ( gridTexX, texPosition.x ).r;
		y = texture1D ( gridTexY, texPosition.y ).r;
		z = texPosition.z;
		//z = texture1D(gridTexZ, texPosition.z).r;
	}
	else
	{
		x = texPosition.x;
		y = texPosition.y;
		z = texPosition.z;
	}

	//a = texture3D(volumeTexPearson, vec3(x,y,z)).r;
	//a = 0.05;
	a = 0.01;


	// a faster version?
	float sum_sq_x, sum_sq_y, sum_coproduct, sweep, iflt, delta_x, delta_y, mean_x, mean_y;
	float pop_sd_x, pop_sd_y, cov_x_y, p;

	sum_sq_x = 0.0;
	sum_sq_y = 0.0;
	sum_coproduct = 0.0;

	zpos = z * oneoversize;
	zposcp = zpos;
	refzpos = referenceZ * oneoversize;

	A[0] = lookUpValue ( texture3D ( volumeTex, vec3 ( x,y,zpos ) ).r );
	B[0] = lookUpValue ( texture3D ( volumeTex, vec3 ( referenceX, referenceY, refzpos ) ).r );

	mean_x = A[0];
	mean_y = B[0];
	zpos += oneoversize;
	refzpos += oneoversize;
	iflt = 2.0;


	for ( i=1; i<size; i++ )
	{
		sweep = ( iflt - 1.0 ) / iflt;
		A[i] = lookUpValue ( texture3D ( volumeTex, vec3 ( x,y,zpos ) ).r );
		B[i] = lookUpValue ( texture3D ( volumeTex, vec3 ( referenceX, referenceY, refzpos ) ).r );
		delta_x = A[i] - mean_x;
		delta_y = B[i] - mean_y;
		sum_sq_x += delta_x * delta_x * sweep;
		sum_sq_y += delta_y * delta_y * sweep;
		sum_coproduct += delta_x * delta_y * sweep;
		mean_x += delta_x / iflt;
		mean_y += delta_y / iflt;
		zpos += oneoversize;
		refzpos += oneoversize;
		iflt += 1.0;
	}

	pop_sd_x = sqrt ( sum_sq_x * oneoversize );
	pop_sd_y = sqrt ( sum_sq_y * oneoversize );
	cov_x_y = sum_coproduct * oneoversize;
	if ( pop_sd_x * pop_sd_y == 0.0 )
	{
		return vec4 ( 0.0, 0.0, 0.0, 0.0 );
	}
	else
	{
		p = cov_x_y / ( pop_sd_x * pop_sd_y );
		if(bval){
			return vec4(p, 0.0, 0.0, 0.0);
		}
		col = getCorrelColor ( p, false, vors );
		return vec4 ( col, a );
	}

}


vec4 getPearsonCoefColorVariable ( vec3 texPosition, bool vors, bool bval )
{
	float refzpos;
	//referenceY = 1 - referenceY;
	texPosition.x = texPosition.x + offsetX;

	if ( texPosition.x<0.0 ) texPosition.x = texPosition.x + 1.0;
	if ( texPosition.x>1.0 ) texPosition.x = texPosition.x - 1.0;

	int i, size = 3;//6
	float oneoversize = 1.0/3.0; //36.0;
	float oneoversizehf = oneoversize * 0.5;
	float A[3];//36];
	float B[3];//36];

	float x, y, z;
	float a, zpos;
	vec3 col;

	if ( grid )
	{
		x = texture1D ( gridTexX, texPosition.x ).r;
		y = texture1D ( gridTexY, texPosition.y ).r;
		//z = texture1D(gridTexZ, texPosition.z).r;
		z = texPosition.z;
	}
	else
	{
		x = texPosition.x;
		y = texPosition.y;
		z = texPosition.z;
	}

	a = 0.01;


	float sum_sq_x, sum_sq_y, sum_coproduct, sweep, iflt, delta_x, delta_y, mean_x, mean_y;
	float pop_sd_x, pop_sd_y, cov_x_y, p;

	sum_sq_x = 0.0;
	sum_sq_y = 0.0;
	sum_coproduct = 0.0;



	zpos = z * oneoversizehf;
	A[0] = texture3D ( volumeTex, vec3 ( x,y,zpos ) ).r;
	B[0] = texture3D ( volumeTex, vec3 ( x,y,zpos+0.5 ) ).r;
	mean_x = A[0];
	mean_y = B[0];
	zpos += oneoversizehf;
	iflt = 2.0;

	for ( i=1; i<size; i++ )
	{
		sweep = ( iflt - 1.0 ) / iflt;
		A[i] = texture3D ( volumeTex, vec3 ( x,y,zpos ) ).r;
		B[i] = texture3D ( volumeTex, vec3 ( x,y,zpos+0.5 ) ).r;
		delta_x = A[i] - mean_x;
		delta_y = B[i] - mean_y;
		sum_sq_x += delta_x * delta_x * sweep;
		sum_sq_y += delta_y * delta_y * sweep;
		sum_coproduct += delta_x * delta_y * sweep;
		mean_x += delta_x / iflt;
		mean_y += delta_y / iflt;
		zpos += oneoversizehf;
		iflt += 1.0;
	}

	pop_sd_x = sqrt ( sum_sq_x * oneoversize );
	pop_sd_y = sqrt ( sum_sq_y * oneoversize );
	cov_x_y = sum_coproduct * oneoversize;
	
	if ( pop_sd_x * pop_sd_y == 0.0 )
	{
		return vec4 ( 0.0, 0.0, 0.0, 0.0 );
	}
	else
	{
		p = cov_x_y / ( pop_sd_x * pop_sd_y );
		if(bval){
			return vec4(p, 0.0, 0.0, 0.0);
		}
		col = getCorrelColor ( p, false, vors );
		//return vec4(refzpos+0.5, 0.0, 0.0, 0.0);
		return vec4 ( col, a );
	}

}


vec3 getNormal ( vec3 texPosition )
{
	vec3 gradient = vec3( 0 );
	vec4 cd100 = vec4 ( 0 );
	vec4 cdn100 = vec4 ( 0 );
	vec4 cd010 = vec4 ( 0 );
	vec4 cdn010 = vec4 ( 0 );
	vec4 cd001 = vec4 ( 0 );
	vec4 cdn001 = vec4 ( 0 );
	float nx = 0.0;
	float ny = 0.0;
	float nz = 0.0;
	texPosition.x = texPosition.x + offsetX;
	if ( texPosition.x<0.0 ) texPosition.x = texPosition.x + 1.0;
	if ( texPosition.x>1.0 ) texPosition.x = texPosition.x - 1.0;

// new code
	if ( pearsonCorrelationActive == 1 )
	{
		if ( pearsonVariable == 1 )
		{
			cd100 = getPearsonCoefColorVariable ( texPosition.xyz+vec3 ( centralDifferenceSpacing.r,0,0 ), false, true);
			cdn100 = getPearsonCoefColorVariable ( texPosition.xyz+vec3 ( -centralDifferenceSpacing.r,0,0 ), false, true);
			cd010 = getPearsonCoefColorVariable ( texPosition.xyz+vec3 ( 0,centralDifferenceSpacing.g,0 ), false, true);
			cdn010 = getPearsonCoefColorVariable ( texPosition.xyz+vec3 ( 0,-centralDifferenceSpacing.g,0 ), false, true);
			cd001 = getPearsonCoefColorVariable ( texPosition.xyz+vec3 ( 0,0,centralDifferenceSpacing.b ), false, true);
			cdn001 = getPearsonCoefColorVariable ( texPosition.xyz+vec3 ( 0,0,-centralDifferenceSpacing.b ), false, true);
			nx = cd100.x- cdn100.x;
			ny=cd010.x- cdn010.x;
			nz=cd001.x- cdn001.x;
			gradient.x = nx;
			gradient.y = ny;
			gradient.z = nz;
		}
		else
		{
			cd100 = getPearsonCoefColorLocation ( texPosition.xyz+vec3 ( centralDifferenceSpacing.r,0,0 ), false, true);
			cdn100 = getPearsonCoefColorLocation ( texPosition.xyz+vec3 ( -centralDifferenceSpacing.r,0,0 ), false, true);
			cd010 = getPearsonCoefColorLocation ( texPosition.xyz+vec3 ( 0,centralDifferenceSpacing.g,0 ), false, true);
			cdn010 = getPearsonCoefColorLocation ( texPosition.xyz+vec3 ( 0,-centralDifferenceSpacing.g,0 ), false, true);
			cd001 = getPearsonCoefColorLocation ( texPosition.xyz+vec3 ( 0,0,centralDifferenceSpacing.b ), false, true);
			cdn001 = getPearsonCoefColorLocation ( texPosition.xyz+vec3 ( 0,0,-centralDifferenceSpacing.b ), false, true);
			nx = cd100.x- cdn100.x;
			ny=cd010.x- cdn010.x;
			nz=cd001.x- cdn001.x;
			gradient.x = nx;
			gradient.y = ny;
			gradient.z = nz;
		}
		
		
	}
	else{
		gradient.x=texture3D ( volumeTex,texPosition.xyz+vec3 ( centralDifferenceSpacing.r,0,0 ) ).r
	           -texture3D ( volumeTex,texPosition.xyz+vec3 ( -centralDifferenceSpacing.r,0,0 ) ).r;
		gradient.y=texture3D ( volumeTex,texPosition.xyz+vec3 ( 0,centralDifferenceSpacing.g,0 ) ).r
	           -texture3D ( volumeTex,texPosition.xyz+vec3 ( 0,-centralDifferenceSpacing.g,0 ) ).r;
		gradient.z=texture3D ( volumeTex,texPosition.xyz+vec3 ( 0,0,centralDifferenceSpacing.b ) ).r
	           -texture3D ( volumeTex,texPosition.xyz+vec3 ( 0,0,-centralDifferenceSpacing.b ) ).r;
    }

	
	gradient=gradient * 10.0;

	if ( length ( gradient ) > 0.0 )
		gradient = normalize ( gradient );

	return gradient;
}

vec3 RGB2HSV ( vec3 color )
{
	int maxVal=0;
	float h = 0.0, s = 0.0, v = 0.0;
	float r = color.r, g = color.g, b = color.b;
	float mn=r, mx=r;
	vec3 hsv = vec3 ( 0.0, 0.0, 0.0 );

	if ( g > mx ) {mx=g; maxVal=1;}
	if ( b > mx ) {mx=b; maxVal=2;}
	if ( g < mn ) mn=g;
	if ( b < mn ) mn=b;

	float  delta = mx - mn;

	v = mx;

	if ( mx == 0.0 )
	{
		s = 0.0;
		h = 0.0;
		hsv.r = h; hsv.g = s; hsv.b = v;
	}
	else
	{
		s = delta / mx;
		if ( s == 0.0 )
		{
			h=-1.0;
			hsv.r = h; hsv.g = s; hsv.b = v;
		}
		else
		{
			if ( maxVal==0 ) {h = ( g - b ) / delta;}      // yel < h < mag
			else if ( maxVal==1 ) {h = 2.0 + ( b - r ) / delta;}  // cyan < h < yel
			else if ( maxVal==2 ) {h = 4.0 + ( r - g ) / delta;}  // mag < h < cyan
			h *= 60.0;
			if ( h < 0.0 ) h += 360.0;
			hsv.r = h; hsv.g = s; hsv.b = v;
		}
	}
	return hsv;
}

vec3 HSV2RGB ( vec3 color )
{
	int i;
	float r = 0.0, g = 0.0, b = 0.0;
	float h = color.r, s = color.g, v = color.b;
	float f, p, q, t, hTemp;
	vec3 rgb = vec3 ( 0.0, 0.0, 0.0 );

	if ( s == 0.0 || h == -1.0 ) // s==0? Totally unsaturated = grey so R,G and B all equal value
	{
		rgb.r = rgb.g = rgb.b = v;
	}
	else
	{
		hTemp = h/60.0;
		i = int ( floor ( hTemp ) );        // which sector
		f = hTemp - float ( i );            // how far through sector
		p = v * ( 1.0 - s );
		q = v * ( 1.0 - s * f );
		t = v * ( 1.0 - s * ( 1.0 - f ) );

		if ( i==0 )
			{r = v; g = t; b = p;}
		else if ( i==1 )
			{r = q; g = v; b = p;}
		else if ( i==2 )
			{r = p; g = v; b = t;}
		else if ( i==3 )
			{r = p; g = q; b = v;}
		else if ( i==4 )
			{r = t; g = p; b = v;}
		else if ( i==5 )
			{r = v; g = p; b = q;}
		rgb.r = r; rgb.g = g; rgb.b = b;
	}
	return rgb;
}

vec4 getColor ( vec3 texPosition )
{
	texPosition.x = texPosition.x + offsetX;;
	if ( texPosition.x<0.0 ) texPosition.x = texPosition.x + 1.0;
	if ( texPosition.x>1.0 ) texPosition.x = texPosition.x - 1.0;

	float x = texture1D ( gridTexX, texPosition.x ).r;
	float y = texture1D ( gridTexY, texPosition.y ).r;
	//float z = texture1D(gridTexZ, texPosition.z).r;
	float z = texPosition.z;

	float scalar = texture3D ( volumeTex, texPosition.xyz ).r;

	//float scalar = texture3D(volumeTex, vec3(x,y,z)).r;
	//float scalar = texture3D(volumeTex, vec3(x,y,z)).r;

	return texture1D ( tfTex, scalar );
	//return vec4(scalar,scalar,scalar,1);
}

vec4 getBlackWhite ( vec3 texPosition )
{
	texPosition.x = texPosition.x + offsetX;
	if ( texPosition.x<0.0 ) texPosition.x = texPosition.x + 1.0;
	if ( texPosition.x>1.0 ) texPosition.x = texPosition.x - 1.0;

	float x = texture1D ( gridTexX, texPosition.x ).r;
	float y = texture1D ( gridTexY, texPosition.y ).r;
	float z = texture1D ( gridTexZ, texPosition.z ).r;
	//z = texPosition.z;

	float scalar = texture3D ( volumeTex, texPosition.xyz ).r;

	//float scalar = texture3D(volumeTex, vec3(x,y,z)).r;
	//float scalar = texture3D(volumeTex, vec3(x,y,z)).r;


	//return texture1D(tfTex, scalar);
	return vec4 ( scalar,scalar,scalar,1 );
}


float getClusterVal ( vec3 texPosition )
{
	texPosition.x = texPosition.x + offsetX;
	if ( texPosition.x<0.0 ) texPosition.x = texPosition.x + 1.0;
	if ( texPosition.x>1.0 ) texPosition.x = texPosition.x - 1.0;

	//float x = texPosition.x; // / float(xblock);
	//float y = texPosition.y; // / float(yblock);
	//float z = texPosition.z; // / float(zblock);

	float scalar = texture3D ( volumeClusTex, texPosition.xyz ).r;

	//float scalar = texture3D(volumeClusTex, vec3(x,y,z)).r;
	return scalar;
}

/* two side lighting*/
Light getLight ( vec3 normal, vec3 lightDir, vec4 lightPar, vec3 rayDir )
{
	Light light;

	float ambient = lightPar.r;

	float diffuse = lightPar.g * max ( dot ( lightDir, normal ), dot ( lightDir, -normal ) );

	vec3 H = normalize ( -rayDir + lightDir );

	float DotHV = max ( dot ( H, normal ), dot ( H, -normal ) );

//     H = normalize(-rayDir - lightDir);
//     DotHV = max(DotHV, max(dot(H, normal), dot(H, -normal)));

	float specular = 0.0;

	if ( DotHV > 0.0 )
		specular = lightPar.b * pow ( DotHV, lightPar.a );


	light.ambient_diffuse = ambient + diffuse;
	light.specular = specular;

	return light;
}

void DrawGreyScaleSlice()
{
	//gl_FragColor = getColor(textureCoords);
	gl_FragColor = getBlackWhite ( textureCoords );
	//gl_FragColor /= gl_FragColor.a;

}

void DrawSlice()
{

	gl_FragColor = getColor ( textureCoords );

	if ( clusterId <  100 )
	{
		float clusterval = getClusterVal ( textureCoords );
		if ( clusterval != 1.0 )
		{
			//accumulatedColor.rgb = vec3{1,0,0};
			//accumulatedColor.a = 0.0;//accumulatedColor.a;//10.0;

			gl_FragColor = vec4 ( 0,0,1,0.000000001 );
		}
		else
			gl_FragColor /= gl_FragColor.a;

		return;
	}

	if ( pearsonCorrelationActive == 0 )
	{
		gl_FragColor /= gl_FragColor.a;
		return;
	}

	if ( clusterId >=  100 )
	{
/*
		if ( pearsonCorrelationActive == 1 )
		{
			if ( pearsonVariable == 1 )
			{
				gl_FragColor = getPearsonCoefColorVariable ( textureCoords, false, false);
			}
			else
			{
				gl_FragColor = getPearsonCoefColorLocation ( textureCoords, false, false);
			}
			//gl_FragColor = vec4(0,0,1,0.000000001);
		}
*/
		gl_FragColor /= gl_FragColor.a;
	}
}

void DrawVolume()
{
	vec3 rayStart = textureCoords;

	vec3 rayDir;

	if ( bOrtho==1 )
		rayDir = normalize ( vec3 ( 0.5 ) - eyePos );
	else
		rayDir = normalize ( rayStart - eyePos );

	vec3 samplePos = vec3 ( 0.0 );
	float sampleLen = sampleSpacing;

	vec4 tfColor;

	vec4 sampleColor;

	vec4 accumulatedColor = vec4 ( 0 );

	vec3 normal;

	vec3 lightDir[2];

	lightDir[0] = normalize ( vec3 ( 1.0, 1.0, 1.0 ) );
	//lightDir[0] = normalize(rayDir * -1.0);
	lightDir[1] = normalize ( vec3 ( 1.0, 0.0, 1.0 ) );
	Light oneLight;

	float ambient_diffuse = 0.0;
	float specular = 0.0;
	float dis = 0.0;

	while ( samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 &&
	        samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 &&
	        accumulatedColor.a < 1.0 )
	{
		while ( samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 &&
		        samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 &&
		        accumulatedColor.a < 1.0 )
		{

			samplePos = rayStart + rayDir * sampleLen;

			sampleLen += sampleSpacing;

			//tfColor = vec4(1.0,0.0,0.0,1.0);
			tfColor = getColor ( samplePos );
			sampleColor = tfColor;

			normal = getNormal ( samplePos );

			ambient_diffuse = 0.0;
			specular = 0.0;

			//for ( int i = 0; i < 1; i ++) {
			oneLight = getLight ( normal, lightDir[0], lightPar, rayDir );
			ambient_diffuse += oneLight.ambient_diffuse;
			specular += oneLight.specular;
			//}

			sampleColor.rgb = tfColor.rgb * ambient_diffuse + tfColor.a * specular;

			accumulatedColor.rgb = ( 1.0-accumulatedColor.a ) *sampleColor.rgb+accumulatedColor.rgb;

			accumulatedColor.a   = ( 1.0-accumulatedColor.a ) *sampleColor.a + accumulatedColor.a;

		}
	}

	gl_FragColor = min ( accumulatedColor, vec4 ( 1.0 ) );
}

void main ( void )
{
	//if(clusterId < 100){
	//    float clusterval = getClusterVal(textureCoords);
	//    float clusterval_d = abs(clusterval-1.0);
	//    if(clusterval < 0.001)
	//        gl_FragColor = vec4(1,0,0,1);
	//    else if(clusterval_d < 0.001)
	//        gl_FragColor = vec4(0,1,0,1);
	//    else if(clusterval > 1.0)
	//        gl_FragColor = vec4(0,0,1,1);
	//    else
	//        gl_FragColor = vec4(0,0,0,1);
	//}
	//else
	//    gl_FragColor = vec4(0,0,0,1);
	//return;

	if ( drawSlice == 1 )
		DrawSlice();
	else if ( drawSlice == 2 )
		DrawGreyScaleSlice();
	else
	{
		//DrawVolume();

		vec3 rayStart = textureCoords;

		vec3 rayDir;

		if ( bOrtho==1 )
			rayDir = normalize ( vec3 ( 0.5 ) - eyePos );
		else
			rayDir = normalize ( rayStart - eyePos );

		vec3 samplePos = vec3 ( 0.0 );
		float sampleLen = sampleSpacing;

		vec4 tfColor;

		vec4 sampleColor;

		vec4 accumulatedColor = vec4 ( 0 );

		vec3 normal;

		vec3 lightDir[2];

		lightDir[0] = normalize ( vec3 ( 1.0, 1.0, 1.0 ) );
		//lightDir[0] = normalize(rayDir * -1.0);
		lightDir[1] = normalize ( vec3 ( 1.0, 0.0, 1.0 ) );
		Light oneLight;

		float ambient_diffuse = 0.0;
		float specular = 0.0;
		float dis = 0.0;

		float minval = 0.1;
		while ( samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 &&
		        samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 &&
		        accumulatedColor.a < 1.0 )
		{
			while ( samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 &&
			        samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 &&
			        accumulatedColor.a < 1.0 )
			{

				samplePos = rayStart + rayDir * sampleLen;
				sampleLen += sampleSpacing;
				/*
				if ( pearsonCorrelationActive==1 )
				{
					if ( pearsonVariable==1 )
						tfColor = getPearsonCoefColorVariable ( samplePos, true, false );
					else
						tfColor = getPearsonCoefColorLocation ( samplePos, true, false );
				}
				else
				{
					//tfColor = vec4(1.0,0.0,0.0,1.0);
				*/
					tfColor = getColor ( samplePos );
				//}

				sampleColor = tfColor;

				if ( clusterId <  100 )
				{
					float clusterval = getClusterVal ( samplePos );
					if ( clusterval < 1.0 )  //if(clusterval != 1.0){
					{
						//gl_FragColor = vec4(0,0,tfColor.b,0.000000001);

						vec3 hsv = RGB2HSV ( tfColor.rgb );
						hsv.g = hsv.g * minval;
						tfColor.rgb = HSV2RGB ( hsv.rgb );
					}
				}
				normal = getNormal ( samplePos );

				ambient_diffuse = 0.0;
				specular = 0.0;

				//for ( int i = 0; i < 1; i ++) {
				oneLight = getLight ( normal, lightDir[0], lightPar, rayDir );
				ambient_diffuse += oneLight.ambient_diffuse;
				specular += oneLight.specular;
				//}

				sampleColor.rgb = tfColor.rgb * ambient_diffuse + tfColor.a * specular;

				accumulatedColor.rgb = ( 1.0-accumulatedColor.a ) *sampleColor.rgb+accumulatedColor.rgb;

				accumulatedColor.a   = ( 1.0-accumulatedColor.a ) *sampleColor.a + accumulatedColor.a;



			}
		}

		gl_FragColor = min ( accumulatedColor, vec4 ( 1.0 ) );
	}

}