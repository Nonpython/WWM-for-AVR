//---------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "WMMHeader.h"
#include "WMM_SubLibrary.c"

//---------------------------------------------------------------------------

/* WMM sublibrary is used to make a command prompt program. The program prompts
the user to enter a location, performs the computations and prints the results to the
standard output. The program expects the files WMM_SubLibrary.c, WMMHEADER.H,
WMM.COF and EGM9615.BIN to be in the same directory. 

Manoj.C.Nair
Nov 23, 2009

 *
 * MODIFICATIONS
 *
 *    Date                 Version
 *    ----                 -----------
 *    Nov 15, 2009         0.1
	Jan 28, 2010	   1.0


*/

int main()
{

	WMMtype_MagneticModel *MagneticModel, *TimedMagneticModel;
	WMMtype_Ellipsoid Ellip;
	WMMtype_CoordSpherical CoordSpherical;
	WMMtype_CoordGeodetic CoordGeodetic;
	WMMtype_Date UserDate;
	WMMtype_GeoMagneticElements GeoMagneticElements;
	WMMtype_Geoid Geoid;
	char ans[20];
	char filename[] = "WMM.COF";
	int NumTerms, Flag = 1;

	/* Memory allocation */

	NumTerms = ( ( WMM_MAX_MODEL_DEGREES + 1 ) * ( WMM_MAX_MODEL_DEGREES + 2 ) / 2 );    /* WMM_MAX_MODEL_DEGREES is defined in WMM_Header.h */

	MagneticModel 	   = WMM_AllocateModelMemory(NumTerms);  /* For storing the WMM Model parameters */
	TimedMagneticModel  = WMM_AllocateModelMemory(NumTerms);  /* For storing the time modified WMM Model parameters */
	if(MagneticModel == NULL || TimedMagneticModel == NULL)
	{
		WMM_Error(2);
	}

	WMM_SetDefaults(&Ellip, MagneticModel, &Geoid); /* Set default values and constants */
	/* Check for Geographic Poles */
	//WMM_readMagneticModel_Large(filename, MagneticModel); //Uncomment this line when using the 740 model, and comment out the  WMM_readMagneticModel line.

	WMM_readMagneticModel(filename, MagneticModel);
	WMM_InitializeGeoid(&Geoid);    /* Read the Geoid file */
	WMM_GeomagIntroduction(MagneticModel);  /* Print out the WMM introduction */


	while (Flag == 1) {
		if(WMM_GetUserInput(MagneticModel, &Geoid, &CoordGeodetic, &UserDate) == 1)    	/*Get User Input */
		{

			WMM_GeodeticToSpherical(Ellip, CoordGeodetic, &CoordSpherical);    /*Convert from geodeitic to Spherical Equations: 17-18, WMM Technical report*/
			WMM_TimelyModifyMagneticModel(UserDate, MagneticModel, TimedMagneticModel); /* Time adjust the coefficients, Equation 19, WMM Technical report */
			WMM_Geomag(Ellip, CoordSpherical, CoordGeodetic, TimedMagneticModel, &GeoMagneticElements);   /* Computes the geoMagnetic field elements and their time change*/
			WMM_CalculateGridVariation(CoordGeodetic,&GeoMagneticElements);
			WMM_PrintUserData(GeoMagneticElements,CoordGeodetic, UserDate, TimedMagneticModel, &Geoid);     /* Print the results */
		}
	  printf("\n\n Do you need more point data ? (y or n) \n ");
	  fgets(ans, 20, stdin);
				switch(ans[0])
				{
					case 'Y':
					case 'y':
						Flag = 1;
						break;
					case 'N':
					case 'n':
						Flag = 0;
						break;
					default:
						Flag = 0;
						break;
				}

		}


WMM_FreeMagneticModelMemory(MagneticModel);
WMM_FreeMagneticModelMemory(TimedMagneticModel);     

if (Geoid.GeoidHeightBuffer)
	{
	free(Geoid.GeoidHeightBuffer);
	Geoid.GeoidHeightBuffer = NULL;
	}


return 0;
}




