
/* World Magnetic Model (WMM) File processing program. The program can accept
the input parameters from a user specified file. Then WMM program is called
to compute the magnetic fields. The results are then printed to the specified
output file.

WMM sublibrary is used in this program. The program expects the files
WMM.COF and EGM9615.BIN to be in the same directory.

The program uses the user interface developed for geomag61.c
Note the option for geocentric height (C) is not supported in this version
The height entered is considered as height above mean sea level

Manoj.C.Nair@Noaa.Gov
November 15, 2009

 *
 *
 * MODIFICATIONS
 *
 *    Date                 Version
 *    ----                 -----------
 *    Nov 15, 2009         0.1
	Jan 28, 2010	   1.0




*/
/****************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* The following include file must define a function 'isnan' */
/* This function, which returns '1' if the number is NaN and 0*/
/* otherwise, could be hand-written if not available. */
/* Comment out one of the two following lines, as applicable */
#include <math.h>               /* for gcc */


#include "WMMHeader.h"
#include "WMM_SubLibrary.c"

#define NaN log(-1.0)
/* constants */
#define RECL 81

#define MAXINBUFF RECL+14

/** Max size of in buffer **/

#define MAXREAD MAXINBUFF-2
/** Max to read 2 less than total size (just to be safe) **/


#define PATH MAXREAD




/****************************************************************************/
/*                                                                          */
/*      Some variables used in this program                                 */
/*                                                                          */
/*    Name         Type                    Usage                            */
/* ------------------------------------------------------------------------ */

/*                                                                          */
/*   minalt     Float array of MAXMOD  Minimum height of model.             */
/*                                                                          */
/*   altmin     Float                  Minimum height of selected model.    */
/*                                                                          */
/*   altmax     Float array of MAXMOD  Maximum height of model.             */
/*                                                                          */
/*   maxalt     Float                  Maximum height of selected model.    */
/*                                                                          */
/*   sdate  Scalar Float           start date inputted                      */
/*                                                                          */
/*   alt        Scalar Float           altitude above WGS84 Ellipsoid       */
/*                                                                          */
/*   latitude   Scalar Float           Latitude.                            */
/*                                                                          */
/*   longitude  Scalar Float           Longitude.                           */
/*                                                                          */
/*   inbuff     Char a of MAXINBUF     Input buffer.                        */
/*                                                                          */
/*                                                                          */
/*   minyr      Float                  Min year of all models               */
/*                                                                          */
/*   maxyr      Float                  Max year of all models               */
/*                                                                          */
/*   yrmax      Float array of MAXMOD  Max year of model.                   */
/*                                                                          */
/*   yrmin      Float array of MAXMOD  Min year of model.                   */
/*                                                                          */
/****************************************************************************/


int main(int argv, char**argc)
{
#ifdef MAC
  ccommand(argv, argc);
#endif
  /*  WMM Variable declaration  */

	WMMtype_MagneticModel *MagneticModel, *TimedMagneticModel;
	WMMtype_Ellipsoid Ellip;
	WMMtype_CoordSpherical CoordSpherical;
	WMMtype_CoordGeodetic CoordGeodetic;
	WMMtype_Date UserDate;
	WMMtype_GeoMagneticElements GeoMagneticElements;
	WMMtype_Geoid Geoid;
	double b;
	char a;
	char ans[20];
	char filename[] = "WMM.COF";
	int NumTerms, Flag = 1;


  /* Control variables */
  int   again = 1;
  int   decyears = 3;
  int   units = 4;
  int   decdeg = 3;
  int   range = -1;
  int   counter = 0;
  int   nmax;
  int   igdgc=3;
  int   isyear=-1;
  int   ismonth=-1;
  int   isday=-1;
  int   ieyear=-1;
  int   iemonth=-1;
  int   ieday=-1;
  int   ilat_deg=200;
  int   ilat_min=200;
  int   ilat_sec=200;
  int   ilon_deg=200;
  int   ilon_min=200;
  int   ilon_sec=200;
  int   fileline;
  int   ddeg, ideg;


  int  coords_from_file = 0;
  int arg_err = 0;


  char  mdfile[PATH];
  char  inbuff[MAXINBUFF];

  char *begin;
  char *rest;
  char args[7][MAXREAD];
  int iarg;

  char coord_fname[PATH];
  char out_fname[PATH];
  FILE *coordfile,*outfile;
  int iline=0;
  int read_flag;

  float minyr;
  float maxyr;
  float minalt;
  float maxalt;
  float alt=-999999;
  float sdate=-1;
  float step=-1;
  float syr;
  float edate=-1;
  float latitude=200;
  float longitude=200;
  float dmin, imin;



  /*  Subroutines used  */
  
  void print_result_file(FILE *outf, double d, double i, double h, double x, double y, double z, double f,
						 double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot);
  float degrees_to_decimal();
  float julday();
  int   safegets(char *buffer,int n);
  int getshc();


  /* Initializations. */

  inbuff[MAXREAD+1]='\0';  /* Just to protect mem. */
  inbuff[MAXINBUFF-1]='\0';  /* Just to protect mem. */


 /* Memory allocation */
	NumTerms = ( ( WMM_MAX_MODEL_DEGREES + 1 ) * ( WMM_MAX_MODEL_DEGREES + 2 ) / 2 );    /* WMM_MAX_MODEL_DEGREES is defined in WMM_Header.h */

	MagneticModel 	   = 	WMM_AllocateModelMemory(NumTerms);  /* For storing the WMM Model parameters */
	TimedMagneticModel  = 	WMM_AllocateModelMemory(NumTerms);  /* For storing the time modified WMM Model parameters */
	if(MagneticModel == 	NULL || TimedMagneticModel == NULL)
	{
		WMM_Error(2);
	}

	WMM_SetDefaults(&Ellip, MagneticModel, &Geoid); /* Set default values and constants */
	/* Check for Geographic Poles */
	//WMM_readMagneticModel_Large(filename, MagneticModel); //Uncomment this line when using the 740 model, and comment out the  WMM_readMagneticModel line.

	WMM_readMagneticModel(filename, MagneticModel);
	WMM_InitializeGeoid(&Geoid);    /* Read the Geoid file */

  maxyr = MagneticModel->epoch + 5.0;
  minyr = MagneticModel->epoch;

  for (iarg=0; iarg<argv; iarg++)
	if (argc[iarg] != NULL)
	  strncpy(args[iarg],argc[iarg],MAXREAD);

  //printing out version number and header
  printf("\n\n WMM_FileProc File processing program November 23, 2009 ");

  if (argv==1 || ((argv==2)&&(*(args[1])=='h')))
	{
	  printf("\n\nWorld Magnetic Model - File Processing Utility : USAGE:\n");
	  printf("coordinate file: wmm_file f input_file output_file\n");
	  printf("or for help:     wmm_file h \n");
	  printf("\n");
	  printf("The input file may have any number of entries but they must follow\n");
	  printf("the following format\n");
	  printf("Date and location Formats: \n");
	  printf("   Date: xxxx.xxx for decimal  (2013.7)\n");
	  printf("   Altitude: M - Above mean sea level: E above WGS84 Ellipsoid \n");
	  printf("   Altitude: Kxxxxxx.xxx for kilometers  (K1000.13)\n");
      printf("             Mxxxxxx.xxx for meters  (m1389.24)\n");
	  printf("             Fxxxxxx.xxx for feet  (F192133.73)\n");
	  printf("   Lat/Lon: xxx.xxx in decimal  (-76.53)\n");
	printf("            (Lat and Lon must be specified in the same format.)\n");
	  printf("   Date and altitude must fit model.\n");
	  printf("   Lat: -90 to 90 (Use - to denote Southern latitude.)\n");
	  printf("   Lon: -180 to 180 (Use - to denote Western longitude.)\n");
	  printf("   Date: 2010.0 to 2015.0\n");
	  printf("   An example of an entry in input file\n");
	  printf("   2013.7 E F30000 -70.3 -30.8 \n");
		printf("\n Press enter to exit.");
		printf ("\n >");
		fgets(ans, 20, stdin);
	  exit(2);
	} /* help */
  
  if ((argv==4)&&(*(args[1])=='f'))
    {
	  printf("\n\n 'f' switch: converting file with multiple locations.\n");
	  printf("     The first five output columns repeat the input coordinates.\n");
	  printf("     Then follows D, I, H, X, Y, Z, and F.\n");
      printf("     Finally the SV: dD, dI, dH, dX, dY, dZ,  and dF\n");
	  printf("     The units are the same as when the program is\n");
      printf("     run in command line or interactive mode.\n\n");
	  coords_from_file = 1;
	  strncpy(coord_fname,args[2],MAXREAD);
	  coordfile=fopen(coord_fname, "rt");
      strncpy(out_fname,args[3],MAXREAD);
      outfile=fopen(out_fname, "w");
      fprintf(outfile,"Date Coord-System Altitude Latitude Longitude D_deg D_min I_deg I_min H_nT X_nT Y_nT Z_nT F_nT dD_min dI_min dH_nT dX_nT dY_nT dZ_nT dF_nT\n");
	} /* file option */

  if (argv>=2 && argv !=4)
	{
	  printf("\n\nERROR in 'f' switch option: wrong number of arguments\n");
	  exit(2);
	}

  if (argv==1)
	{
	  printf("\n\nERROR in switch option: wrong number of arguments\n");
	  exit(2);
	}



   while (again == 1)
	{
      if (coords_from_file) 
		{
		  argv = 6;
		  read_flag = fscanf(coordfile,"%s%s%s%s%s%*[^\n]",args[1],args[2],args[3],args[4],args[5]);
		  if (read_flag == EOF) goto reached_EOF;
          fprintf(outfile,"%s %s %s %s %s ",args[1],args[2],args[3],args[4],args[5]);fflush(outfile);
		  iline++;
        } /* coords_from_file */
      
      /* Switch on how many arguments are supplied. */
      /* Note that there are no 'breaks' after the cases, so these are entry points */
      switch(argv)
		{
		case 6 : strncpy(inbuff, args[5], MAXREAD);
          if ((rest=strchr(inbuff, ',')))     /* If it contains a comma */
            {
			  decdeg=2;                        /* Then not decimal degrees */
			  begin=inbuff;
              rest[0]='\0';                    /* Chop into sub string */
              rest++;                          /* Move to next substring */
			  ilon_deg=atoi(begin);
              begin=rest;
              if ((rest=strchr(begin, ',')))
                {
				  rest[0]='\0';
				  rest++;
                  ilon_min=atoi(begin);
                  ilon_sec=atoi(rest);
                } 
              else 
                {
                  ilon_min=0;
                  ilon_sec=0;
                }
            } 
          else 
            {
              decdeg=1;                        /* Else it's decimal */
			  longitude=atof(args[5]);
			}

        case 5 : strncpy(inbuff, args[4], MAXREAD);
          if ((rest=strchr(inbuff, ',')))
            {
              decdeg=2;
			  begin=inbuff;
              rest[0]='\0';
              rest++;
              ilat_deg=atoi(begin);
              begin=rest;
              if ((rest=strchr(begin, ',')))
				{
                  rest[0]='\0';
                  rest++;
                  ilat_min=atoi(begin);
                  ilat_sec=atoi(rest);
				}
              else 
				{
                  ilat_min=0;
                  ilat_sec=0;
                }
            }
          else 
            {
              decdeg=1;
			  latitude=atof(args[4]);
            }
          
        case 4 : strncpy(inbuff, args[3], MAXREAD);
          inbuff[0]=toupper(inbuff[0]);
          if (inbuff[0]=='K') units=1;
          else if (inbuff[0]=='M') units=2;
          else if (inbuff[0]=='F') units=3;
          if (strlen(inbuff)>1)
			{
              inbuff[0]='\0';
              begin=inbuff+1;
              alt=atof(begin);
            }

        case 3 : strncpy(inbuff, args[2], MAXREAD);
          inbuff[0]=toupper(inbuff[0]);
		  if (inbuff[0]=='M') igdgc=1;    /* height is above  mean sea level*/
		  else if (inbuff[0]=='E') igdgc=2; /* height is above  WGS 84 ellepsoid */

          
        case 2 : strncpy(inbuff, args[1], MAXREAD);
          if ((rest=strchr(inbuff, '-')))   /* If it contains a dash */
            {
			  range = 2;                     /* They want a range */
              rest[0]='\0';                  /* Sep dates */
              rest++;
              begin=rest;
              if ((rest=strchr(begin, '-')))    /* If it contains 2 dashs */
                {
                  rest[0]='\0';                  /* Sep step */
				  rest++;
				  step=atof(rest);               /* Get step size */
                }
			  if ((rest=strchr(begin, ',')))    /* If it contains a comma */
				{
                  decyears=2;                    /* It's not decimal years */
                  rest[0]='\0';
                  rest++;
                  ieyear=atoi(begin);
                  begin=rest;
                  if ((rest=strchr(begin, ',')))
                    {
                      rest[0]='\0';
                      rest++;
                      iemonth=atoi(begin);
                      ieday=atoi(rest);
                    } 
                  else 
                    {
					  iemonth=0;
                      ieday=0;
                    }
                  if ((rest=strchr(inbuff, ',')))
                    {
                      begin=inbuff;
                      rest[0]='\0';
                      rest++;
                      isyear=atoi(begin);
                      begin=rest;
					  if ((rest=strchr(begin, ',')))
                        {
                          rest[0]='\0';
						  rest++;
                          ismonth=atoi(begin);
                          isday=atoi(rest);
                        } 
                      else 
                        {
						  ismonth=0;
                          isday=0;
                        }
                    } 
                  else 
					{
                      sdate=atof(inbuff);
                    }
                } 
              else 
                {
                  decyears=1;                    /* Else it's decimal years */
                  sdate=atof(inbuff);
                  edate=atof(begin);
                }
            } 
          else 
            {
              range = 1;
              if ((rest=strchr(inbuff, ',')))   /* If it contains a comma */
                {
                  decyears=2;                    /* It's not decimal years */
                  begin=inbuff;
                  rest[0]='\0';
                  rest++;
                  isyear=atoi(begin);
                  begin=rest;
                  if ((rest=strchr(begin, ',')))
                    {
                      rest[0]='\0';
                      rest++;
                      ismonth=atoi(begin);
                      isday=atoi(rest);
                    } 
                  else 
                    {
                      ismonth=0;
                      isday=0;
					}
                } 
              else 
				{
                  decyears=1;                    /* Else it's decimal years */
				  sdate=atof(args[1]);
                }
            }
          if (sdate==0)
            {                        /* If date not valid */
              decyears=-1;
              range=-1;
			}
			break;

		}

	  if (range == 2 && coords_from_file)
		{
		  printf("Error in line %1d, date = %s: date ranges not allowed for file option\n\n",iline,args[1]);
		  exit(2);
		}

	  /*  Obtain the desired model file and read the data  */

	 /* if date specified in command line then warn if past end of validity */

      /*  Take in field data  */
      
      /* Get date */
      
	  if (coords_from_file && !arg_err && (decyears != 1 && decyears != 2))
        {printf("\nError: unrecognized date %s in coordinate file line %1d\n\n",args[1],iline); arg_err = 1;}

	  if (coords_from_file && !arg_err && range != 1)
		{printf("\nError: unrecognized date %s in coordinate file line %1d\n\n",args[1],iline); arg_err = 1;}

	  if (coords_from_file && !arg_err && (sdate < minyr || sdate >maxyr))
		{printf("\nWarning:  date out of range in coordinate file line %1d\n\n",iline);
		printf("\nExpected range = %6.1lf - %6.1lf, entered %6.1lf\n",minyr,maxyr,sdate);}


	   /* Get altitude min and max for selected model. */
	  minalt= -10; /* To be defined */
	  maxalt= 1000;

	  /* Get Coordinate prefs */

	  if (coords_from_file && !arg_err && (igdgc != 1 && igdgc != 2))
		  {printf("\nError: Unrecognized height reference. %1d\n\n",args[1],iline); arg_err = 1;}

	   /* If needed modify height referencing */
	  if (igdgc==2)
		{
		  Geoid.UseGeoid = 0;     /* height above WGS-84 Ellipsoid */
		}else if (igdgc==1)
		{
		  Geoid.UseGeoid = 1;    /* height above MSL */
		}


	  /* Do unit conversions if neccessary */
	  if (units==2)
		{
		  minalt*=1000.0;
		  maxalt*=1000.0;
		}
	  else if (units==3)
		{
		  minalt*=3280.0839895;
		  maxalt*=3280.0839895;
		}

	  /* Get altitude */

	  if (coords_from_file && !arg_err && (alt < minalt || alt > maxalt))
		{printf("\nError: unrecognized altitude %s in coordinate file line %1d\n\n",args[3],iline); arg_err = 1;}

	  /* Convert altitude to km */
	  if (units==2)
		{
		  alt *= 0.001;
		}
	  else if (units==3)
		{
		  alt /= 3280.0839895;
		}



	  /* Get lat/long prefs */

	  if (coords_from_file && !arg_err && decdeg != 1 )
		{printf("\nError: unrecognized lat %s or lon %s in coordinate file line %1d\n\n",args[4],args[5],iline); arg_err = 1;}



	  /* Get lat/lon */


	  /** This will compute everything needed for 1 point in time. **/

			CoordGeodetic.lambda = longitude;
			CoordGeodetic.phi	 = latitude;
			CoordGeodetic.HeightAboveGeoid = alt;
			UserDate.DecimalYear = sdate;


			WMM_ConvertGeoidToEllipsoidHeight(&CoordGeodetic, &Geoid);   /*This converts the height above mean sea level to height above the WGS-84 ellipsoid*/
			WMM_GeodeticToSpherical(Ellip, CoordGeodetic, &CoordSpherical);    /*Convert from geodeitic to Spherical Equations: 17-18, WMM Technical report*/
			WMM_TimelyModifyMagneticModel(UserDate, MagneticModel, TimedMagneticModel); /* Time adjust the coefficients, Equation 19, WMM Technical report */
			WMM_Geomag(Ellip, CoordSpherical, CoordGeodetic, TimedMagneticModel, &GeoMagneticElements);   /* Computes the geoMagnetic field elements and their time change*/
			WMM_CalculateGridVariation(CoordGeodetic,&GeoMagneticElements);




	  /** Above will compute everything for 1 point in time.  **/


	  /*  Output the final results. */


    if (coords_from_file)
		{
	print_result_file(outfile,
			GeoMagneticElements.Decl,
			GeoMagneticElements.Incl,
			GeoMagneticElements.H,
			GeoMagneticElements.X,
			GeoMagneticElements.Y,
			GeoMagneticElements.Z,
			GeoMagneticElements.F,
			60 * GeoMagneticElements.Decldot,
			60 * GeoMagneticElements.Incldot,
			GeoMagneticElements.Hdot,
			GeoMagneticElements.Xdot,
			GeoMagneticElements.Ydot,
			GeoMagneticElements.Zdot,
			GeoMagneticElements.Fdot);
		}

	  if (coords_from_file)
		again = !feof(coordfile) && !arg_err;

	  if (again == 1)
		{
		  /* Reset defaults to catch on all while loops */
		  igdgc=decyears=units=decdeg=-1;
		  ismonth=isday=isyear=sdate=edate=range=step=-1;
		  latitude=ilat_deg=ilat_min=ilat_sec=200;
		  longitude=ilon_deg=ilon_min=ilon_sec=200;
		  alt=-9999999;
		  argv = 1;
		}
	} /* while (again == 1) */

 reached_EOF:

  if (coords_from_file) printf("\n Processed %1d lines\n\n",iline);

  if (coords_from_file && !feof(coordfile) && arg_err) printf("Terminated prematurely due to argument error in coordinate file\n\n");


fclose(coordfile);
fclose(outfile);

WMM_FreeMagneticModelMemory(MagneticModel);
WMM_FreeMagneticModelMemory(TimedMagneticModel);     

if (Geoid.GeoidHeightBuffer)
	{
	free(Geoid.GeoidHeightBuffer);
	Geoid.GeoidHeightBuffer = NULL;
	}
  return 0;
}



void print_result_file(FILE *outf, double d, double i, double h, double x, double y, double z, double f,
					   double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot)
{
  int   ddeg,ideg;
  float dmin,imin;

  /* Change d and i to deg and min */

  ddeg=(int)d;
  dmin=(d-(float)ddeg)*60;
  if (ddeg!=0) dmin=fabs(dmin);
  ideg=(int)i;
  imin=(i-(float)ideg)*60;
  if (ideg!=0) imin=fabs(imin);

  if (isnan(d))
	{
	  if (isnan(x))
		fprintf(outf," NaN        %4dd %2.0fm  %8.1f      NaN      NaN %8.1f %8.1f",ideg,imin,h,z,f);
	  else
		fprintf(outf," NaN        %4dd %2.0fm  %8.1f %8.1f %8.1f %8.1f %8.1f",ideg,imin,h,x,y,z,f);
	}
  else
	fprintf(outf," %4dd %2.0fm  %4dd %2.0fm  %8.1f %8.1f %8.1f %8.1f %8.1f",ddeg,dmin,ideg,imin,h,x,y,z,f);

  if (isnan(ddot))
	{
	  if (isnan(xdot))
		fprintf(outf,"      NaN  %7.1f     %8.1f      NaN      NaN %8.1f %8.1f\n",idot,hdot,zdot,fdot);
	  else
		fprintf(outf,"      NaN  %7.1f     %8.1f %8.1f %8.1f %8.1f %8.1f\n",idot,hdot,xdot,ydot,zdot,fdot);
	}
  else
	fprintf(outf," %7.1f   %7.1f     %8.1f %8.1f %8.1f %8.1f %8.1f\n",ddot,idot,hdot,xdot,ydot,zdot,fdot);
  return;
} /* print_result_file */


/****************************************************************************/
/*                                                                          */
/*                       Subroutine safegets                                */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  Gets characters from stdin untill it has reached n characters or \n,    */
/*     whichever comes first.  \n is converted to \0.                       */
/*                                                                          */
/*  Input: n - Integer number of chars                                      */
/*         *buffer - Character array ptr which can contain n+1 characters   */
/*                                                                          */
/*  Output: size - integer size of sting in buffer                          */
/*                                                                          */
/*  Note: All strings will be null terminated.                              */
/*                                                                          */
/*  By: David Owens                                                         */
/*      dio@ngdc.noaa.gov                                                   */
/****************************************************************************/

int safegets(char *buffer,int n){
   char *ptr;                    /** ptr used for finding '\n' **/

   fgets(buffer,n,stdin);        /** Get n chars **/
   buffer[n+1]='\0';             /** Set last char to null **/
   ptr=strchr(buffer,'\n');      /** If string contains '\n' **/
   if (ptr!=NULL){                /** If string contains '\n' **/
      ptr[0]='\0';               /** Change char to '\0' **/
   }

   return strlen(buffer);        /** Return the length **/
}


/****************************************************************************/
/*                                                                          */
/*                       Subroutine degrees_to_decimal                      */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Converts degrees,minutes, seconds to decimal degrees.                */
/*                                                                          */
/*     Input:                                                               */
/*            degrees - Integer degrees                                     */
/*            minutes - Integer minutes                                     */
/*            seconds - Integer seconds                                     */
/*                                                                          */
/*     Output:                                                              */
/*            decimal - degrees in decimal degrees                          */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 12, 1988                                                */
/*                                                                          */
/****************************************************************************/

float degrees_to_decimal(int degrees,int minutes,int seconds)
{
   float deg;
   float min;
   float sec;
   float decimal;

   deg = degrees;
   min = minutes/60.0;
   sec = seconds/3600.0;

   decimal = fabs(sec) + fabs(min) + fabs(deg);

   if (deg < 0) {
      decimal = -decimal;
   } else if (deg == 0){
      if (min < 0){
         decimal = -decimal;
      } else if (min == 0){
         if (sec<0){
            decimal = -decimal;
         }
      }
   }

   return(decimal);
}

/****************************************************************************/
/*                                                                          */
/*                           Subroutine julday                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Computes the decimal day of year from month, day, year.              */
/*     Leap years accounted for 1900 and 2000 are not leap years.           */
/*                                                                          */
/*     Input:                                                               */
/*           year - Integer year of interest                                */
/*           month - Integer month of interest                              */
/*           day - Integer day of interest                                  */
/*                                                                          */
/*     Output:                                                              */
/*           date - Julian date to thousandth of year                       */
/*                                                                          */
/*     FORTRAN                                                              */
/*           S. McLean                                                      */
/*           NGDC, NOAA egc1, 325 Broadway, Boulder CO.  80301              */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 12, 1988                                                */
/*                                                                          */
/*     Julday Bug Fix                                                       */
/*           Thanks to Rob Raper                                            */
/****************************************************************************/


float julday(i_month, i_day, i_year)
int i_month;
int i_day;
int i_year;
{
   int   aggregate_first_day_of_month[13];
   int   leap_year = 0;
   int   truncated_dividend;
   float year;
   float day;
   float decimal_date;
   float remainder = 0.0;
   float divisor = 4.0;
   float dividend;
   float left_over;

   aggregate_first_day_of_month[1] = 1;
   aggregate_first_day_of_month[2] = 32;
   aggregate_first_day_of_month[3] = 60;
   aggregate_first_day_of_month[4] = 91;
   aggregate_first_day_of_month[5] = 121;
   aggregate_first_day_of_month[6] = 152;
   aggregate_first_day_of_month[7] = 182;
   aggregate_first_day_of_month[8] = 213;
   aggregate_first_day_of_month[9] = 244;
   aggregate_first_day_of_month[10] = 274;
   aggregate_first_day_of_month[11] = 305;
   aggregate_first_day_of_month[12] = 335;

   /* Test for leap year.  If true add one to day. */

   year = i_year;                                 /*    Century Years not   */
   if ((i_year != 1900) && (i_year != 2100))      /*  divisible by 400 are  */
   {                                              /*      NOT leap years    */
	  dividend = year/divisor;
	  truncated_dividend = dividend;
	  left_over = dividend - truncated_dividend;
	  remainder = left_over*divisor;
	  if ((remainder > 0.0) && (i_month > 2))
	  {
	 leap_year = 1;
	  }
	  else
	  {
	 leap_year = 0;
	  }
   }
   day = aggregate_first_day_of_month[i_month] + i_day - 1 + leap_year;
   if (leap_year)
   {
	  decimal_date = year + (day/366.0);  /*In version 3.0 this was incorrect*/
   }
   else
   {
	  decimal_date = year + (day/365.0);  /*In version 3.0 this was incorrect*/
   }
   return(decimal_date);
}


int isnan (double d)

{

   return d != d;

}


