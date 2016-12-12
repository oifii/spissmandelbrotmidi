// spissmandelbrotmidi.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/*
int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}
*/

//greensquare.cpp is an open-source example
//screensaver by Rachel Grey, lemming@alum.mit.edu.
//Paste into an IDE to compile if desired.
//I haven't chosen to include the resource file,
//so you'd need to provide a description string
//and so forth.


#include <windows.h>
#include  <scrnsave.h>
#include  <GL/gl.h>
#include <GL/glu.h>
//#include "glut.h" // include GLUT library header
#include <math.h>
#include <ctime>
#include "resource.h"

//get rid of these warnings:
//truncation from const double to float
//conversion from double to float
#pragma warning(disable: 4305 4244) 
                        

//Define a Windows timer

#define TIMER 1



//These forward declarations are just for readability,
//so the big three functions can come first 

void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void GetConfig();               
void WriteConfig(HWND hDlg);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();
void OnTimer(HDC hDC);


int Width, Height; //globals for size of screen



int windowID;

GLfloat minX = -2.2f, maxX = 0.8f, minY = -1.5f, maxY = 1.5; // complex plane boundaries
GLfloat stepX = (maxX - minX)/(GLfloat)Width;
GLfloat stepY = (maxY - minY)/(GLfloat)Height;

GLfloat xCenter = (minX + maxX)/2.0;
GLfloat yCenter = (minY + maxY)/2.0;

//GLfloat[] black = {0.0f, 0.0f, 0.0f}; // black color
GLfloat black[] = {0.0f, 0.0f, 0.0f}; // black color
const int paletteSize = 128;
GLfloat palette[paletteSize][3];

const GLfloat radius = 5.0f;
bool fullScreen=false;


void mandelbrotzoom()
{
	/*
	GLfloat xCenter = (minX + maxX)/2.0;
	GLfloat yCenter = (minY + maxY)/2.0;
	*/
	GLfloat xWidth = abs(maxX - minX);
	GLfloat yHeight = abs(maxY - minY);
	//approach center in the mandelbrot antena along y axis
	int random_integer;
	int lowest=1, highest=1000;
	int range=(highest-lowest)+1;
	random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
	
	GLfloat xCenterNew = minX/2.0 + (xCenter-minX)*((GLfloat)random_integer)/(2.0*((GLfloat)highest));
	GLfloat yCenterNew = yCenter; //same
	//GLfloat xWidthNew = xWidth/2.0; //new
	GLfloat xWidthNew = xWidth; //same
	//GLfloat yHeightNew = yHeight/2.0; //new
	GLfloat yHeightNew = yHeight; //same
	minX = xCenterNew - xWidthNew/2.0;
	maxX = xCenterNew + xWidthNew/2.0;
	minY = yCenterNew - yHeightNew/2.0;
	maxY = yCenterNew + yHeightNew/2.0;

	//same as before
	stepX = (maxX - minX)/(GLfloat)Width;
	stepY = (maxY - minY)/(GLfloat)Height;

}

//****************************************
GLfloat* calculateColor(GLfloat u, GLfloat v)
{
	GLfloat re = u;
	GLfloat im = v;
	GLfloat tempRe=0.0;
	for(int i=0; i < paletteSize; i++)
	{
		tempRe = re*re - im*im + u;
		im = re * im * 2 + v;
		re = tempRe;
		if( (re*re + im*im) > radius )
		{
			return palette[i];
		}
	}
	return black;
}

/*
//****************************************
void repaint() 
{// function called to repaint the window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen buffer
	glBegin(GL_POINTS); // start drawing in single pixel mode
	for(GLfloat y = maxY; y >= minY; y -= stepY)
	{
		for(GLfloat x = minX; x <= maxX; x += stepX)
		{
			glColor3fv(calculateColor(x,y)); // set color
			glVertex3f(x, y, 0.0f); // put pixel on screen (buffer) - [ 1 ]
		}
	}
	glEnd(); // end drawing
	glutSwapBuffers(); // swap the buffers - [ 2 ]
}
*/

/*
//****************************************
void reshape (int w, int h)
{ // function called when window size is changed
	stepX = (maxX-minX)/(GLfloat)w; // calculate new value of step along X axis
	stepY = (maxY-minY)/(GLfloat)h; // calculate new value of step along Y axis
	glViewport (0, 0, (GLsizei)w, (GLsizei)h); // set new dimension of viewable screen
	glutPostRedisplay(); // repaint the window
}
*/

//****************************************
void createPalette()
{
	for(int i=0; i < 32; i++)
	{
		palette[i][0] = (8*i)/(GLfloat)255;
		palette[i][1] = (128-4*i)/(GLfloat)255;
		palette[i][2] = (255-8*i)/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[32+i][0] = (GLfloat)1;
		palette[32+i][1] = (8*i)/(GLfloat)255;
		palette[32+i][2] = (GLfloat)0;
	}
	for(int i=0; i < 32; i++)
	{
		palette[64+i][0] = (128-4*i)/(GLfloat)255;
		palette[64+i][1] = (GLfloat)1;
		palette[64+i][2] = (8*i)/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[96+i][0] = (GLfloat)0;
		palette[96+i][1] = (255-8*i)/(GLfloat)255;
		palette[96+i][2] = (8*i)/(GLfloat)255; 
	}
}




//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////


// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{
  static HDC hDC;
  static HGLRC hRC;
  static RECT rect;


	//////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));

  switch ( message ) 
  {

  case WM_CREATE: 
		// get window dimensions
		GetClientRect( hWnd, &rect );
		Width = rect.right;         
		Height = rect.bottom;
    
        //get configuration from registry
        GetConfig();

        // setup OpenGL, then animation
		InitGL( hWnd, hDC, hRC );
		SetupAnimation(Width, Height);

		//set timer to tick every 10 ms
		//SetTimer( hWnd, TIMER, 10, NULL );
		//SetTimer( hWnd, TIMER, 10000, NULL );
		SetTimer( hWnd, TIMER, 5000, NULL );
		//SetTimer( hWnd, TIMER, 2500, NULL );
    return 0;
 
  case WM_DESTROY:
		KillTimer( hWnd, TIMER );
        CleanupAnimation();
		CloseGL( hWnd, hDC, hRC );
    return 0;

  case WM_TIMER:
    OnTimer(hDC);       //animate!      
    return 0;                           

  }

  return DefScreenSaverProc(hWnd, message, wParam, lParam );

}

bool bTumble = true;


BOOL WINAPI
ScreenSaverConfigureDialog(HWND hDlg, UINT message, 
                           WPARAM wParam, LPARAM lParam)
{

  //InitCommonControls();  
  //would need this for slider bars or other common controls

  HWND aCheck;

  switch ( message ) 
  {

        case WM_INITDIALOG:
                LoadString(hMainInstance, IDS_DESCRIPTION, szAppName, 40);

                GetConfig();
				/*
                aCheck = GetDlgItem( hDlg, IDC_TUMBLE );
                SendMessage( aCheck, BM_SETCHECK, 
                bTumble ? BST_CHECKED : BST_UNCHECKED, 0 );
				*/

    return TRUE;

  case WM_COMMAND:
    switch( LOWORD( wParam ) ) 
                { 
			/*
            case IDC_TUMBLE:
                        bTumble = (IsDlgButtonChecked( hDlg, IDC_TUMBLE ) == BST_CHECKED);
                        return TRUE;
			*/
                //cases for other controls would go here

            case IDOK:
                        WriteConfig(hDlg);      //get info from controls
                        EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
                        return TRUE; 

                case IDCANCEL: 
                        EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
                        return TRUE;   
                }

  }     //end command switch

  return FALSE; 
}



// needed for SCRNSAVE.LIB
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
  return TRUE;
}


/////////////////////////////////////////////////
////   INFRASTRUCTURE ENDS, SPECIFICS BEGIN   ///
////                                          ///
////    In a more complex scr, I'd put all    ///
////     the following into other files.      ///
/////////////////////////////////////////////////


// Initialize OpenGL
static void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC)
{
  
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory( &pfd, sizeof pfd );
  pfd.nSize = sizeof pfd;
  pfd.nVersion = 1;
  //pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  
  hDC = GetDC( hWnd );
  
  int i = ChoosePixelFormat( hDC, &pfd );  
  SetPixelFormat( hDC, i, &pfd );

  hRC = wglCreateContext( hDC );
  wglMakeCurrent( hDC, hRC );

}

// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );

  ReleaseDC( hWnd, hDC );
}


void SetupAnimation(int Width, int Height)
{
		//spi, begin
		createPalette();
		stepX = (maxX-minX)/(GLfloat)Width; // calculate new value of step along X axis
		stepY = (maxY-minY)/(GLfloat)Height; // calculate new value of step along Y axis
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		//spi, end
        //window resizing stuff
        glViewport(0, 0, (GLsizei) Width, (GLsizei) Height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
		//spi, begin
        //glOrtho(-300, 300, -240, 240, 25, 75); //original
        //glOrtho(-300, 300, -8, 8, 25, 75); //spi, last
		glOrtho(minX, maxX, minY, maxY, ((GLfloat)-1), (GLfloat)1); //spi
        //glOrtho(-150, 150, -120, 120, 25, 75); //spi
		//spi, end
        
		/*
		glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();
        gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //original
                //camera xyz, the xyz to look at, and the up vector (+y is up)
        */

        //background
        glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black


        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
        glShadeModel(GL_SMOOTH); 

        //no need to initialize any objects
        //but this is where I'd do it

        glColor3f(0.1, 1.0, 0.3); //green

}

static GLfloat spin=0;   //a global to keep track of the square's spinning


void OnTimer(HDC hDC) //increment and display
{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* //no rotation
        spin = spin + 1; 
		*/
        glPushMatrix();
        /*
		glRotatef(spin, 0.0, 0.0, 1.0);
		*/
        glPushMatrix();
		

		/*
        glTranslatef(150, 0, 0); //original
		*/

		/* //no rotation
        if(bTumble)
                glRotatef(spin * -3.0, 0.0, 0.0, 1.0); 
        else
                glRotatef(spin * -1.0, 0.0, 0.0, 1.0);  
		*/

		//spi, begin
		/*
        //draw the square (rotated to be a diamond)

        float xvals[] = {-30.0, 0.0, 30.0, 0.0};
        float yvals[] = {0.0, -30.0, 0.0, 30.0};

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
                for (int i=0; i < 4; i++)
                        glVertex2f(xvals[i], yvals[i]);
        glEnd();
		*/
		mandelbrotzoom();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
		glOrtho(minX, maxX, minY, maxY, ((GLfloat)-1), (GLfloat)1);
 
		glBegin(GL_POINTS); // start drawing in single pixel mode
		for(GLfloat y = maxY; y >= minY; y -= stepY)
		{
			for(GLfloat x = minX; x <= maxX; x += stepX)
			{
				glColor3fv(calculateColor(x,y)); // set color
				glVertex3f(x, y, 0.0f); // put pixel on screen (buffer) - [ 1 ]
			}
		}
		glEnd(); // end drawing
		//spi, end

        glPopMatrix();

        glFlush();
        SwapBuffers(hDC);
        glPopMatrix();
}

void CleanupAnimation()
{
        //didn't create any objects, so no need to clean them up
}



/////////   REGISTRY ACCESS FUNCTIONS     ///////////

void GetConfig()
{

        HKEY key;
        //DWORD lpdw;

        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                //"Software\\GreenSquare", //lpctstr
                L"Software\\Oifii\\Screensaver", //lpctstr
                0,                      //reserved
                KEY_QUERY_VALUE,
                &key) == ERROR_SUCCESS) 
        {
                DWORD dsize = sizeof(bTumble);
                DWORD dwtype =  0;

                //RegQueryValueEx(key,"Tumble", NULL, &dwtype, (BYTE*)&bTumble, &dsize);
                RegQueryValueEx(key,L"Tumble", NULL, &dwtype, (BYTE*)&bTumble, &dsize);


                //Finished with key
                RegCloseKey(key);
        }
        else //key isn't there yet--set defaults
        {
                bTumble = true;
        }
        
}

void WriteConfig(HWND hDlg)
{

        HKEY key;
        DWORD lpdw;

        if (RegCreateKeyEx( HKEY_CURRENT_USER,
                //"Software\\GreenSquare", //lpctstr
                L"Software\\Oifii\Screensaver", //lpctstr
                0,                      //reserved
                L"",                     //ptr to null-term string specifying the object type of this key
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,
                &key,
                &lpdw) == ERROR_SUCCESS)
                
        {
                //RegSetValueEx(key,"Tumble", 0, REG_DWORD, 
                RegSetValueEx(key,L"Tumble", 0, REG_DWORD, 
                        (BYTE*)&bTumble, sizeof(bTumble));

                //Finished with keys
                RegCloseKey(key);
        }

}