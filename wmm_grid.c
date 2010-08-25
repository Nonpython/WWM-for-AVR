
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "WMMHeader.h"
#include "WMM_SubLibrary.c"



/*
 *
 * MODIFICATIONS
 *
 *    Date                 Version
 *    ----                 -----------
 *    Jul 15, 2009         0.1
 *    Nov 15, 2009         0.2
 *	  Jan 28, 2010         1.0

Manoj.C.Nair@NOAA.Gov

*/



int main()
{
	WMMtype_MagneticModel *MagneticModel;
	WMMtype_Ellipsoid Ellip;
	WMMtype_CoordGeodetic minimum, maximum;
	WMMtype_Geoid Geoid;
	WMMtype_Date startdate, enddate;
	int NumTerms, ElementOption, PrintOption,  swabtype;
	double cord_step_size, altitude_step_size, time_step_size;
	char filename[] = "WMM.COF";
	char OutputFilename[20];


	NumTerms = ( ( WMM_MAX_MODEL_DEGREES + 1 ) * ( WMM_MAX_MODEL_DEGREES + 2) / 2 );
	MagneticModel = WMM_AllocateModelMemory(NumTerms);

	if(MagneticModel == NULL )
	{
		WMM_Error(2);
	}

	swabtype = 	WMM_swab_type();

	WMM_SetDefaults(&Ellip, MagneticModel, &Geoid);
	WMM_readMagneticModel(filename, MagneticModel);
   	WMM_InitializeGeoid(&Geoid);
	WMM_GeomagIntroduction(MagneticModel);  /* Print out the WMM introduction */
	printf("\n\n This program may be used to generate a grid / volume of magnetic field values over\nlatitude, longitude, altitude and time axes. To skip an axis, keep the start and end values the same\nand enter zero for the step size.\n");
  //	This program

	printf("\n\n                     Enter grid parameters \n\n");


	/* Get the Lat/Long, Altitude, Time limits from a user interface and print the grid to screen */

  	WMM_GetUserGrid(&minimum, &maximum, &cord_step_size, &altitude_step_size, &time_step_size, &startdate, &enddate,&ElementOption, &PrintOption, OutputFilename, &Geoid);
  	WMM_Grid(minimum, maximum, cord_step_size, altitude_step_size, time_step_size, MagneticModel, &Geoid, Ellip, startdate, enddate, ElementOption, PrintOption, OutputFilename);

	
	/* Examples */

 // 	ElementOption = 4 ; /* Declination */
//	PrintOption = 1; /*Print to File */

	/* Define the step sizes and run WMM Grid function */

	//WMM_Comparison(MagneticModel, Ellip, LegendreFunction, &Geoid);

//	minimum.lambda 			= 0;    /* Minimum Longitude in degrees */
//	maximum.lambda 			= 359.0;    /* Maximum Longitude in degrees */

// 	minimum.phi   			= -89.0;    /* Minimum Latitude in degrees */
// 	maximum.phi   			= 89.0;    /* Maximum Latitude in degrees */

// 	cord_step_size 			= 1;     /* Lat/Long interval in degrees */

//	minimum.HeightAboveGeoid 	= 0;    /* Minimum height above MSL in kilometers */
//	maximum.HeightAboveGeoid 	= 0;    /* Maximum height above MSL in kilometers */

//	altitude_step_size 		= 100;     /* Height interval in kilometers */


//	startdate.DecimalYear 			= 2009.9; /* Start date in decimal Years */
//	enddate.DecimalYear 				= 2009.9; /* End date in decimal Years */

//	time_step_size 			= 0.0;      /* Time interval in decimal Years */

//	WMM_Grid(minimum, maximum, cord_step_size, altitude_step_size, time_step_size, MagneticModel, &Geoid, Ellip, startdate, enddate, ElementOption, PrintOption);

	

// 	/* Make a profile of geoMagnetic field elements along a longitude */
// 
// 	minimum.lambda 			= 10;    /* Minimum Longitude in degrees */
// 	maximum.lambda 			= 10;    /* Maximum Longitude in degrees */
// 
// 	minimum.phi   			= -20;    /* Minimum Latitude in degrees */
// 	maximum.phi   			= 40;    /* Maximum Latitude in degrees */
// 
// 	cord_step_size 			= 0.5;     /* Lat/Long interval in degrees */
// 
// 	minimum.HeightAboveGeoid 	= 10;    /* Minimum height above MSL in kilometers */
// 	maximum.HeightAboveGeoid 	= 10;    /* Maximum height above MSL in kilometers */
// 
// 	altitude_step_size 		= 0;     /* Height interval in kilometers */
// 
//
// 	startdate.DecimalYear 			= 2007.0; /* Start date in decimal Years */
// 	enddate.DecimalYear 				= 2007.0; /* End date in decimal Years */
// 
// 	time_step_size 			= 0;      /* Time interval in decimal Years */
// 
// 	WMM_Grid(minimum, maximum, cord_step_size, altitude_step_size, time_step_size, &output, MagneticModel, &Geoid, Ellip, startdate, enddate, ElementOption, PrintOption);
// 
// 
// 
// 
// 
// 	/* Print a time series of geoMagnetic field elements at a location */
// 
// 	minimum.lambda 			= 10;    /* Minimum Longitude in degrees */
// 	maximum.lambda 			= 20;    /* Maximum Longitude in degrees */
// 
// 	minimum.phi   			= 10;    /* Minimum Latitude in degrees */
// 	maximum.phi   			= 20;    /* Maximum Latitude in degrees */
// 
// 	cord_step_size 			= 0;     /* Lat/Long interval in degrees */
// 
// 	minimum.HeightAboveGeoid 	= 0;    /* Minimum height above MSL in kilometers */
// 	maximum.HeightAboveGeoid 	= 0;    /* Maximum height above MSL in kilometers */
// 
// 	altitude_step_size 		= 0;     /* Height interval in kilometers */
// 
// 
// 	startdate.DecimalYear 			= 2005.0; /* Start date in decimal Years */
// 	enddate.DecimalYear 				= 2007.0; /* End date in decimal Years */
// 
// 	time_step_size 			= 0.1;      /* Time interval in decimal Years */
// 
// 	WMM_Grid(minimum, maximum, cord_step_size, altitude_step_size, time_step_size, &output, MagneticModel, &Geoid, Ellip, startdate, enddate, ElementOption, PrintOption);
// 

WMM_FreeMagneticModelMemory(MagneticModel);

if (Geoid.GeoidHeightBuffer)
	{
	free(Geoid.GeoidHeightBuffer);
	Geoid.GeoidHeightBuffer = NULL;
	}



 return TRUE;
}

