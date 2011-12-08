/* "C" event display.
 * OpendGL (GLUT) based.
 *
 * Alexey Zhelezov, DESY/ITEP, 2005 
 *
 * July 2005, J�rgen Samson: Moved parts of the TCP/IP
 *            server to glut's timer loop to make glced 
 *            "standard glut" compliant
 *
 * June 2007, F.Gaede: - added world_size command line parameter
 *                     - added help message for "-help, -h, -?"
 *                     - replaced fixed size window geometry with geometry comand-line option
 *                     
 */




#include <iomanip>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include "GL/gl.h"
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#include <ced.h>
#include <ced_cli.h>

#include <errno.h>
#include <sys/select.h>

//hauke
#include <ctype.h> //toupper
#include <sys/time.h>
#include <time.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h> //getenv
#include <sys/stat.h>
//#include <wx>

using namespace std;


    //#include<jpeglib.h>

#define DETECTOR1               4001
#define DETECTOR2               4002
#define DETECTOR3               4003
#define DETECTOR4               4004
#define DETECTOR5               4005
#define DETECTOR6               4006
#define DETECTOR7               4007
#define DETECTOR8               4008
#define DETECTOR9               4009
#define DETECTOR10              4010
#define DETECTOR11              4011
#define DETECTOR12              4012
#define DETECTOR13               4013
#define DETECTOR14               4014
#define DETECTOR15               4015
#define DETECTOR16               4016
#define DETECTOR17               4017
#define DETECTOR18               4018
#define DETECTOR19               4019
#define DETECTOR20              4020

#define DETECTOR_ALL            4100


#define GRAFIC_HIGH             2000            
#define GRAFIC_LOW              2001
#define GRAFIC_PERSP            2002
#define GRAFIC_BUFFER           2003
#define GRAFIC_TRANS            2004
#define GRAFIC_LIGHT            2005
#define GRAFIC_ALIAS            2006
#define GRAFIC_FOG              2007


#define CUT_ANGLE0              2100
#define CUT_ANGLE30             2101
#define CUT_ANGLE90             2102
#define CUT_ANGLE135            2103
#define CUT_ANGLE180            2104
#define CUT_ANGLE270            2105
#define CUT_ANGLE360            2106

#define TRANS0                  3000
#define TRANS40                 3101

#define TRANS60                 3001
#define TRANS70                 3002
#define TRANS80                 3003
#define TRANS90                 3004
#define TRANS95                 3005
#define TRANS100                3006

#define FULLSCREEN              6001
#define AXES                    6002
#define FPS                     6003




static int available_cutangles[]={0,30,90,135, 180, 270, 360};  //for new angles add the new angle to this list and to define in top of this


#define BGCOLOR_WHITE           1000
#define BGCOLOR_SILVER          1001
#define BGCOLOR_DIMGRAY         1002
#define BGCOLOR_BLACK           1003
#define BGCOLOR_LIGHTSTEELBLUE  1004
#define BGCOLOR_STEELBLUE       1005
#define BGCOLOR_BLUE            1006
#define BGCOLOR_SEAGREEN        1007
#define BGCOLOR_ORANGE          1008
#define BGCOLOR_YELLOW          1009
#define BGCOLOR_VIOLET          1010

#define BGCOLOR_GAINSBORO       1011
#define BGCOLOR_LIGHTGREY       1012
#define BGCOLOR_DARKGRAY        1013
#define BGCOLOR_GRAY            1014

#define BGCOLOR_USER            1100


#define VIEW_FISHEYE    20
#define VIEW_FRONT      21
#define VIEW_SIDE       22
#define VIEW_ZOOM_IN    23
#define VIEW_ZOOM_OUT   24
#define VIEW_RESET      25
#define VIEW_CENTER     26

#define LAYER_0         30
#define LAYER_1         31
#define LAYER_2         32
#define LAYER_3         33
#define LAYER_4         34
#define LAYER_5         35
#define LAYER_6         36
#define LAYER_7         37
#define LAYER_8         38
#define LAYER_9         39
#define LAYER_10        40
#define LAYER_11        41
#define LAYER_12        42
#define LAYER_13        43
#define LAYER_14        44
#define LAYER_15        45
#define LAYER_16        46
#define LAYER_17        47
#define LAYER_18        48
#define LAYER_19        49
#define LAYER_ALL       60

#define HELP            100
#define SAVE1           101
#define SAVE2           102
#define SAVE3           103
#define SAVE4           104
#define SAVE5           105
#define LOAD1           131
#define LOAD2           132
#define LOAD3           133
#define LOAD4           134
#define LOAD5           135


#define SAVE_IMAGE      5555
#define SAVE_IMAGE1     5556
#define SAVE_IMAGE4     5557
#define SAVE_IMAGE10    5558
#define SAVE_IMAGE20    5559
#define SAVE_IMAGE100   5560


#define TOGGLE_PHI_PROJECTION   5000
#define TOGGLE_Z_PROJECTION     5001

//#define PHI_PROJECTION_OFF 5001


extern CEDsettings setting;

//extern int graphic[];  //= {0,0,0,0}; //light, transparence, perspective, anti aliasing
//extern double cut_angle;
//extern double trans_value;
//static double z_cutting=7000;
//static bool fixed_view=0;

//extern bool phi_projection;
//extern bool z_projection;

int ced_picking(int x,int y,GLfloat *wx,GLfloat *wy,GLfloat *wz); //from ced_srv.c, need header files!

static char layerDescription[CED_MAX_LAYER][CED_MAX_LAYER_CHAR]; 
const char layer_keys[] = {'0','1', '2','3','4','5','6','7','8','9',')', '!', '@', '#', '$', '%', '^', '&', '*', '(', 't', 'y', 'u', 'i', 'o'};

const char detec_layer_keys[] = {'t','y','u','i','o','p','[',']','\\', 'T', 'Y','U','I','O','P','{','}','|'};

static int mainWindow=-1;
static int subWindow=-1;
static int layerMenu;
static int detectorMenu;
static int subsubMenu2;

static int subscreenshot;

static int subSave;
static int subLoad;


//static int helpWindow=-1;
static int showHelp=0;

#define DEFAULT_WORLD_SIZE 1000.  //SJA:FIXED Reduce world size to give better scale

static float WORLD_SIZE;
static float FISHEYE_WORLD_SIZE;

double fisheye_alpha = 0.0;

//hauke
long int doubleClickTime=0;
static float BG_COLOR[4];

extern int SELECTED_ID ;

//fg - make axe a global to be able to rescale the world volume
static GLfloat axe[][3]={
  { 0., 0., 0., },
  { DEFAULT_WORLD_SIZE/2, 0., 0. },
  { 0., DEFAULT_WORLD_SIZE/2, 0. },
  { 0., 0., DEFAULT_WORLD_SIZE/2 }
};


void updateScreenshotMenu(void);
void screenshot(char *name, int times);

// allows to reset the visible world size
static void set_world_size( float length) {
  WORLD_SIZE = length ;
  axe[1][0] = WORLD_SIZE / 2. ;
  axe[2][1] = WORLD_SIZE / 2. ;
  axe[3][2] = WORLD_SIZE / 2. ;
};


//hauke
static void set_bg_color(float one, float two, float three, float four){
    BG_COLOR[0]=one;
    BG_COLOR[1]=two;
    BG_COLOR[2]=three;
    BG_COLOR[3]=four;

    glClearColor(BG_COLOR[0],BG_COLOR[1],BG_COLOR[2],BG_COLOR[3]);
}

typedef GLfloat color_t[4];

static color_t bgColors[] = {
  { 0.0, 0.2, 0.4, 0.0 }, //light blue
  { 0.0, 0.0, 0.0, 0.0 }, //black
  { 0.2, 0.2, 0.2, 0.0 }, //gray shades
  { 0.4, 0.4, 0.4, 0.0 },
  { 0.6, 0.6, 0.6, 0.0 },
  { 0.8, 0.8, 0.8, 0.0 },
  { 1.0, 1.0, 1.0, 0.0 }  //white
};

static float userDefinedBGColor[] = {-1.0, -1.0, -1.0, -1.0};

static unsigned int iBGcolor = 0;

/* AZ I check for TCP sockets as well,
 * function will return 0 when such "event" happenes */
extern struct __glutSocketList {
  struct __glutSocketList *next;
  int fd;
  void  (*read_func)(struct __glutSocketList *sock);
} *__glutSockets;


// from ced_srv.c
void ced_prepare_objmap(void);
int ced_get_selected(int x,int y,GLfloat *wx,GLfloat *wy,GLfloat *wz);
//SJA:FIXED set this to extern as it is a global from ced_srv.c
//extern unsigned long ced_visible_layers; 
//extern bool ced_visible_layers[100];

static struct _geoCylinder {
  GLuint obj;
  GLfloat d;       // radius
  //GLfloat ir;      
  GLuint  sides;   // poligon order
  GLfloat rotate;  // angle degree
  GLfloat z;       // 1/2 length
  GLfloat shift;   // in z
  GLfloat r;       // R 
  GLfloat g;       // G  color
  GLfloat b;       // B
} geoCylinder[] = {
  { 0,   50.0,  6,  0.0, 5658.5, -5658.5, 0.0, 0.0, 1.0 }, // beam tube
  { 0,  380.0, 24,  0.0, 2658.5, -2658.5, 0.0, 0.0, 1.0 }, // inner TPC
  { 0, 1840.0,  8, 22.5, 2700.0, -2700.0, 0.5, 0.5, 0.1 }, // inner ECAL
  { 0, 3000.0, 16,  0.0, 2658.5, -2658.5, 0.0, 0.8, 0.0 }, // outer HCAL
  { 0, 2045.7,  8, 22.5, 2700.0, -2700.0, 0.5, 0.5, 0.1 }, // outer ECAL
  { 0, 3000.0,  8, 22.5, 702.25,  2826.0, 0.0, 0.8, 0.0 }, // endcap HCAL
  { 0, 2045.7,  8, 22.5, 101.00,  2820.0, 0.5, 0.5, 0.1 }, // endcap ECAL
  { 0, 3000.0,  8, 22.5, 702.25, -4230.5, 0.0, 0.8, 0.0 }, // endcap HCAL
  { 0, 2045.7,  8, 22.5, 101.00, -3022.0, 0.5, 0.5, 0.1 }, // endcap ECAL
};

static GLuint makeCylinder(struct _geoCylinder *c){
    GLUquadricObj *q1 = gluNewQuadric();
    GLuint obj;
  
    glPushMatrix();
    obj = glGenLists(1);
    glNewList(obj, GL_COMPILE);
    glTranslatef(0.0, 0.0, c->shift);
    if(c->rotate > 0.01 )
        glRotatef(c->rotate, 0, 0, 1);
    gluQuadricNormals(q1, GL_SMOOTH);
    gluQuadricTexture(q1, GL_TRUE);
    gluCylinder(q1, c->d, c->d, c->z*2, c->sides, 1);
    glEndList();
    glPopMatrix();
    gluDeleteQuadric(q1);
    return obj;
}

static void makeGeometry(void) {
    unsigned i;

    // cylinders
    for(i=0;i<sizeof(geoCylinder)/sizeof(struct _geoCylinder);i++){
        geoCylinder[i].obj=makeCylinder(geoCylinder+i);
    }
}

////function from: http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=44286
//bool screenshot(char *fileName){
//    static unsigned char header[54] = {
//        0x42, 0x4D, 0x36, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
//        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
//        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00, 
//        0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    unsigned char *pixels = (unsigned char *) malloc(Xres * Yres * 3);
//    ((unsigned __int16 *) header)[ 9] = Xres;
//    ((unsigned __int16 *) header)[11] = Yres;
//
//    glReadPixels(0,0,Xres,Yres,GL_RGB,GL_UNSIGNED_BYTE,pixels);
//
//    unsigned char temp;
//    for (unsigned int i = 0; i < Xres * Yres * 3; i += 3){
//        temp = pixels[i];
//        pixels[i] = pixels[i + 2];
//        pixels[i + 2] = temp;
//    }
//
//    HANDLE FileHandle;
//    unsigned long Size;
//
//    if (fileName == NULL){
//        char file[256];
//        i = 0;
//        do {
//            sprintf(file,"Screenshot%d.bmp",i);
//            FileHandle = CreateFile(file,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
//            i++;
//        } while (FileHandle == INVALID_HANDLE_VALUE);
//    } else {
//        FileHandle = CreateFile(fileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
//        if (FileHandle == INVALID_HANDLE_VALUE) return false;
//    }
//
//    WriteFile(FileHandle,header,sizeof(header),&Size,NULL);
//    WriteFile(FileHandle,pixels,Xres * Yres * 3,&Size,NULL);
//    
//    CloseHandle(FileHandle);
//
//    free(pixels);
//    return true;    
//}

int isLayerVisible(int x){
    //return(ced_visible_layers[x]);

    return(setting.layer[x]);
}
static void init(void){
    //Set background color
    glClearColor(BG_COLOR[0],BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);

    //glShadeModel(GL_FLAT);
    glShadeModel(GL_SMOOTH);

    glClearDepth(1);

    glEnable(GL_DEPTH_TEST); //activate 'depth-test'

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear buffers

    //glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //default

//glEnable(GL_POLYGON_STIPPLE);


    //glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //glass
    //glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA); //locks nice, but lines diapear

    //glBlendFunc(GL_ONE, GL_ZERO);
    //glBlendFunc(GL_ONE, GL_ONE);
    //glClearColor(0,0,0,0);

    glEnableClientState(GL_VERTEX_ARRAY);
    // GL_NORMAL_ARRAY GL_COLOR_ARRAY GL_TEXTURE_COORD_ARRAY,GL_EDGE_FLAG_ARRAY

    // to make round points
    //glEnable(GL_POINT_SMOOTH);

    // to put text
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    // To enable Alpha channel (expensive !!!)
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    makeGeometry();
}

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} Point;

static struct {
    GLfloat va; // vertical angle
    GLfloat ha; // horisontal angle
    GLfloat sf; // scale factor
    Point mv; // the center
    GLfloat va_start;
    GLfloat ha_start;
    GLfloat sf_start;
    Point mv_start;
} mm = {
    30.,
    150.,
    0.1, //hauke decrease zoom, //SJA:FIXED set redraw scale a lot smaller
    { 0., 0., 0. },
    0.,
    0.,
    1.,
    { 0., 0., 0. },
}, mm_reset ;


static GLfloat window_width=0.;
static GLfloat window_height=0.;
static enum {
    NO_MOVE,
    TURN_XY,
    ZOOM,
    ORIGIN
} move_mode;
static GLfloat mouse_x=0.;
static GLfloat mouse_y=0.;




// bitmaps for X,Y and Z
static unsigned char x_bm[]={ 
    0xc3,0x42,0x66,0x24,0x24,0x18,
    0x18,0x24,0x24,0x66,0x42,0xc3
};
static unsigned char y_bm[]={
    0xc0,0x40,0x60,0x20,0x30,0x10,
    0x18,0x2c,0x24,0x66,0x42,0xc3
};
static unsigned char z_bm[]={ 
    0xff,0x40,0x60,0x20,0x30,0x10,
    0x08,0x0c,0x04,0x06,0x02,0xff
};
  

static void axe_arrow(void){
    GLfloat k=WORLD_SIZE/window_height;
    glutSolidCone(8.*k,30.*k,16,5);
}

static void display_world(void){
/*   static GLfloat axe[][3]={ */
/*     { 0., 0., 0., }, */
/*     { WORLD_SIZE/2, 0., 0. }, */
/*     { 0., WORLD_SIZE/2, 0. }, */
/*     { 0., 0., WORLD_SIZE/2 } */
/*   }; */
  //  unsigned i;
    if(setting.show_axes == false){
        return;
    }

    glColor3f(0.2,0.2,0.8);
    //glLineWidth(2.);
    glLineWidth(0.5);

    glBegin(GL_LINES);
    glVertex3fv(axe[0]);
    glVertex3fv(axe[1]);
    glEnd();
    glBegin(GL_LINES);
    glVertex3fv(axe[0]);
    glVertex3fv(axe[2]);
    glEnd();
    glBegin(GL_LINES);
    glVertex3fv(axe[0]);
    glVertex3fv(axe[3]);
    glEnd();
  
    glColor3f(0.5,0.5,0.8);
    glPushMatrix();
    glTranslatef(WORLD_SIZE/2.-WORLD_SIZE/100.,0.,0.);
    glRotatef(90.,0.0,1.0,0.0);
    axe_arrow();
    glPopMatrix();
  
    glPushMatrix();
    glTranslatef(0.,WORLD_SIZE/2.-WORLD_SIZE/100.,0.);
    glRotatef(-90.,1.0,0.,0.);
    axe_arrow();
    glPopMatrix();
  
  
    glPushMatrix();
    glTranslatef(0.,0.,WORLD_SIZE/2.-WORLD_SIZE/100.);
    axe_arrow();
    glPopMatrix();
  
    // Draw X,Y,Z ...
    //glColor3f(1.,1.,1.); //white labels
    //glColor3f(0.,0.,0.); //black labels

    glGetDoublev(GL_COLOR_CLEAR_VALUE, setting.bgcolor);
    double dark=1.0-(setting.bgcolor[0]+setting.bgcolor[1]+setting.bgcolor[2])/3.0;
    //glColor3f(1-setting.bgcolor[0], 1-setting.bgcolor[1], 1-setting.bgcolor[2]);
    glColor3f(dark,dark,dark); 


    glRasterPos3f(WORLD_SIZE/2.+WORLD_SIZE/8,0.,0.);
    glBitmap(8,12,4,6,0,0,x_bm);
    glRasterPos3f(0.,WORLD_SIZE/2.+WORLD_SIZE/8,0.);
    glBitmap(8,12,4,6,0,0,y_bm);
    glRasterPos3f(0.,0.,WORLD_SIZE/2.+WORLD_SIZE/8);
    glBitmap(8,12,4,6,0,0,z_bm);
  
    
    // cylinders
    /*
    glLineWidth(1.);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    
    for(i=0;i<sizeof(geoCylinder)/sizeof(struct _geoCylinder);i++){
      glPushMatrix();
      //  glPolygonMode(GL_FRONT_AND_BACK, (i<2)?GL_FILL:GL_LINE);    
      glColor4f(geoCylinder[i].r,geoCylinder[i].g,geoCylinder[i].b,
  	      (i>=2)?1.:0.2);
      glCallList(geoCylinder[i].obj);
      glPopMatrix();
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    */
    //buildMenuPopup(); //hauke: test
    //glutAttachMenu(GLUT_RIGHT_BUTTON);
  
}


void printFPS(void){
    //calculate fps:
    //----------------------
    static int fps=0;
    static int old_fps=0;
    static double startTime;
    struct timeval tv;
    gettimeofday(&tv, 0); 

    if(tv.tv_sec+tv.tv_usec/1000000.0-startTime < 1.0){
        fps++;
    }else{
        startTime=tv.tv_sec+tv.tv_usec/1000000.0;
        //printf("FPS: %i\n", fps);
        old_fps=fps;
        fps=1;
    }

    if(setting.fps == false){
        return;
    }
    //print on screen: 
    //----------------------

    //saves the matrices on the stack
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    //changes the matrices to be compatible with the old ced_draw_legend code:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLfloat w=glutGet(GLUT_SCREEN_WIDTH);
    GLfloat h=glutGet(GLUT_SCREEN_HEIGHT); ;

    int  WORLD_SIZE=1000; //static worldsize maybe will get problems in the future...
    glOrtho(-WORLD_SIZE*w/h,WORLD_SIZE*w/h,-WORLD_SIZE,WORLD_SIZE, -15*WORLD_SIZE,15*WORLD_SIZE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    char text[400];
    void *font=GLUT_BITMAP_TIMES_ROMAN_10; //default font

    sprintf(text, "FPS: %i", old_fps);

    glLoadIdentity();

    double dark=1.0-(setting.bgcolor[0]+setting.bgcolor[1]+setting.bgcolor[2])/3.0;
    glColor3f(dark,dark,dark);

    glRasterPos2f(-1200,-950);
    char *c;
    for (c=text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }

    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void write_world_into_front_buffer(void){
 


    glMatrixMode(GL_PROJECTION);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
  
    glRotatef(mm.va,1.,0.,0.);
    glRotatef(mm.ha,0.,1.0,0.);
    glScalef(mm.sf,mm.sf,mm.sf); //streech the world





    
    glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z);
  
      //glMatrixMode(GL_MODELVIEW); //
  
    // draw static objects

    glMatrixMode(GL_MODELVIEW);
    display_world(); 


   //glTranslatef(0,0,1000);

     const GLdouble clip_plane[]={0,0,-1,setting.z_cutting};
     if(setting.z_cutting < 6999){
          glEnable(GL_CLIP_PLANE0);
     }else{
          glDisable(GL_CLIP_PLANE0);
     }
     glClipPlane(GL_CLIP_PLANE0,clip_plane);
  
  
  
  
    // draw elements (hits + detector)
    ced_prepare_objmap();
    ced_do_draw_event();
  
    //cout << "mm.sf: " << mm.sf << "hinterer clipping plane: " << 5000*2.0*mm.sf << std::endl;
    //gluPerspective(60,window_width/window_height,100*2.0*mm.sf,5000*2.0*mm.sf);

//    std::cout  << "clipping planes: " << 200*2.0*mm.sf << " bis " << 5000*2.0*mm.sf << std::endl;
//
//    gluPerspective(60,window_width/window_height,200*2.0*mm.sf,5000*2.0*mm.sf);
//        glMatrixMode( GL_MODELVIEW );
//  
//        glLoadIdentity();
//        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
//
//


    //printFPS();
}

void drawStringBig (char *s){
    unsigned int i;
    for (i = 0; i[s]; i++){
        glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, s[i]);
    }
}

void drawHelpString (const string & str, float x,float y){ //format help strings strings: "[<key>] <description>"
    unsigned int i;
    glRasterPos2f(x,y);
  
    int monospace = 0;
    for (i = 0; str[i]; i++){
        if(str[i] == '['){ 
            monospace = 1;
            glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, '[');
            i++;
        }
        else if(str[i] == ']'){
             monospace = 0;
        }
        if(monospace){
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
        }else{
            glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, str[i]);
        }
    }
}


void printShortcuts(void){

    const unsigned int MAX_STR_LEN=30;
    int i;
    float line = 12; //height of one line
    float column = MAX_STR_LEN*5; //width of one line

    
    vector<string> shortcuts;
    shortcuts.push_back( "GENERAL SHORTCUTS:" );


    shortcuts.push_back( "[Esc] Quit CED" );
    shortcuts.push_back( "[h] Toggle shortcut frame" );
    shortcuts.push_back( "[r] Reset view" );
    shortcuts.push_back( "[f] Font view" );
    shortcuts.push_back( "[s] Side view" );
    shortcuts.push_back( "[F] Front projection" );
    shortcuts.push_back( "[S] Side projection" );
    shortcuts.push_back( "[v] Fisheye projection" );
    shortcuts.push_back( "[b] Change background color" );
    shortcuts.push_back( "[+] Zoom in" );
    shortcuts.push_back( "[-] Zoom out" );
    shortcuts.push_back( "[c] Center" );
    shortcuts.push_back( "[Z] Cut in z-axe direction" );
    shortcuts.push_back( "[z] Cut in -z-axe direction" );
    shortcuts.push_back( "[>] Increase transparency" );
    shortcuts.push_back( "[<] Decrease transparency" );
    shortcuts.push_back( "[m] Increase detector cut angle" );
    shortcuts.push_back( "[m] Decrease detector cut angle" );
    shortcuts.push_back( "[`] Toggle all data layers" );
    shortcuts.push_back( "[~] Toggle all detector layers" );


    shortcuts.push_back( "  " );
    shortcuts.push_back( "DATA LAYERS:" );


    char label[MAX_STR_LEN+1];

    for(i=0;i<NUMBER_DATA_LAYER;i++){
        snprintf(label,MAX_STR_LEN+1, "(%s) [%c] %s%i: %s", isLayerVisible(i)?"X":"_",layer_keys[i], (i<10)?"0":"", i, layerDescription[i]);
        if(strlen(label) >= MAX_STR_LEN){
            label[MAX_STR_LEN-3]='.';
            label[MAX_STR_LEN-2]='.';
            label[MAX_STR_LEN-1]='.';
            label[MAX_STR_LEN]=0;
        }
        shortcuts.push_back(label);
    }
        
//        drawHelpString(label, ((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column,(ITEMS_PER_COLUMN-(aline%ITEMS_PER_COLUMN))*line);
//        aline++;
//
//        jj=j;
//
//        for(;j<CED_MAX_LAYER_CHAR-1;j++){
//            if(layerDescription[i][j] == ',' || layerDescription[i][j] == 0){
//                tmp[j-jj]=0;
//                j++; //scrip ", "
//                jj=j+1;
//                sprintf(label,"[%c] %s%i: %s", layer_keys[i], (i<10)?"0":"", i, tmp);
//                drawHelpString(label, ((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column,-1*(ITEMS_PER_COLUMN-(aline%ITEMS_PER_COLUMN))*line);
//
//                aline++;
//                if(layerDescription[i][j] == 0){ break; }
//            }else{
//                tmp[j-jj]=layerDescription[i][j];
//            }
//        }


    shortcuts.push_back( " " );
    shortcuts.push_back( "DETECTOR LAYERS: " );

    for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
        snprintf(label,MAX_STR_LEN+1, "(%s) [%c] %s%i: %s", isLayerVisible(i)?"X":"_",detec_layer_keys[-1*NUMBER_DATA_LAYER+i], ((i)<10)?"0":"", (i), layerDescription[i]);
        if(strlen(label) >= MAX_STR_LEN){
            label[MAX_STR_LEN-3]='.';
            label[MAX_STR_LEN-2]='.';
            label[MAX_STR_LEN-1]='.';
            label[MAX_STR_LEN]=0;
        }
        shortcuts.push_back(label);
    }


    //sprintf (label, "Control keys");
    //glRasterPos2f(((int)(0/ITEMS_PER_COLUMN))*column+0.02, 0.80F);
    //drawStringBig(label);


 



//    glDisable(GL_DEPTH_TEST); //activate 'depth-test'
//
//    //    glDisable(GL_BLEND);
//    glClear(GL_DEPTH_BUFFER_BIT ); 
//    std::cout << "show help" << std::endl;
    //saves the matrices on the stack
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();


    glMatrixMode(GL_PROJECTION);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);


    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

 
    //GLfloat w=glutGet(GLUT_SCREEN_WIDTH);
    //GLfloat h=glutGet(GLUT_SCREEN_HEIGHT); ;

    GLfloat w=glutGet(GLUT_WINDOW_WIDTH);
    GLfloat h=glutGet(GLUT_WINDOW_HEIGHT); ;

    int  WORLD_SIZE=1000; //static worldsize maybe will get problems in the future...
    //glOrtho(-WORLD_SIZE*w/h,WORLD_SIZE*w/h,-WORLD_SIZE,WORLD_SIZE, -15*WORLD_SIZE,15*WORLD_SIZE);

    //glOrtho(0,w,h, 0,-15*WORLD_SIZE,15*WORLD_SIZE);

    glOrtho(0,w,h, 0,0,15*WORLD_SIZE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

    //glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH,GL_NICEST);
    //glDepthMask(GL_TRUE);



    double border_factor_line=0.005;
    double border_factor_quad=0.0052;




    double boarder_quad = 1000*border_factor_quad;
    double boarder_line = 1000*border_factor_line;


    if(int(w/column) > 1){
        //h=boarder_quad*2+(shortcuts.size()*line)/(int(w/column))*3;

        //h=(boarder_quad*2.+(shortcuts.size()*column*1./w + 1.)*line)*3.;

        h=(boarder_quad*2.+(shortcuts.size()*1./int((w-3.*boarder_quad)/column) + 1.)*line)*3.+5;
    }else{
        h*=3;
    }


    //glColor4f(1.0,1.0,1.0,0.5);

    //glColor4f(1.0,1.0,1.0,0.5);

    //glColor4f(0.0,1.0,1.0,0.5);

    glColor4f(0.5,1,1,0.8);

    const int ITEMS_PER_COLUMN=int((h/3.0-boarder_quad*2)/(line)); //how many lines per column?
    //const int ITEMS_PER_COLUMN=int((h/3.0)/(line)); //how many lines per column?

//    glBegin(GL_QUADS); 
//    glVertex3f(boarder_quad, boarder_quad,0);
//    glVertex3f(w-boarder_quad,boarder_quad,0);
//    glVertex3f(w-boarder_quad, h/3.-boarder_quad,0);
//    glVertex3f(boarder_quad, h/3.-boarder_quad,0);
//    glEnd();
    glBegin(GL_QUADS); 
    glVertex3f(boarder_quad, boarder_quad,0);
    glVertex3f(w-boarder_quad,boarder_quad,0);
    glVertex3f(w-boarder_quad, h/3.-boarder_quad,0);
    glVertex3f(boarder_quad, h/3.-boarder_quad,0);
    glEnd();


    //glColor4f(0.0,1.0,1.0,1);

    glColor4f(0.1,0.8,1.0,0.8);

    glLineWidth(3.);
    glBegin(GL_LINES); 
    glVertex3f(boarder_line, boarder_line,0);
    glVertex3f(w-boarder_line,boarder_line,0);


    glVertex3f(w-boarder_line, h/3-boarder_line,0);
    glVertex3f(boarder_line, h/3.-boarder_line,0);

    glVertex3f(boarder_line, boarder_line,0);
    glVertex3f(boarder_line, h/3. - boarder_line,0);

    glVertex3f(w-boarder_line,boarder_line,0);
    glVertex3f(w-boarder_line, h/3.-boarder_line,0);
    glEnd();




//----------------

    glColor3f(0.0,0.0,0.0);
    


//    glRasterPos2f(0+100,h/6);
//    drawStringBig("Work in process....");


//    float line = 45/(h/3.0); //height of one line
//    float column = 200/w; //width of one line


    //cout << " w = " << w << "h = " << h << endl;


//    int i;
    for(i=0;(unsigned) i<shortcuts.size();i++){
       //drawHelpString(shortcuts[i], ((int)(i/ITEMS_PER_COLUMN))*column+0.02, (ITEMS_PER_COLUMN+(i%ITEMS_PER_COLUMN))*line );
       drawHelpString(shortcuts[i],  int(i/ITEMS_PER_COLUMN)*column+boarder_quad+5, (i%ITEMS_PER_COLUMN)*line+boarder_quad+10);
       //cout << "x " << int(i/ITEMS_PER_COLUMN)*column+boarder_quad+5 << "y: " << (i%ITEMS_PER_COLUMN)*line+boarder_quad+15<< endl;
       //cout << "point: " <<  int(i/ITEMS_PER_COLUMN)*column+border_factor_quad*w+10 << ", " << (i%ITEMS_PER_COLUMN)*line+border_factor_quad*h+40 << endl;

 //      std::cout << "pos: " <<  ((int)(i/ITEMS_PER_COLUMN))*column+0.02 <<  "," <<  (ITEMS_PER_COLUMN-(i%ITEMS_PER_COLUMN))*line << endl;
    }




/*

    int i;
    for(i=0;(unsigned) i<shortcuts.size();i++){
       drawHelpString(shortcuts[i], ((int)(i/ITEMS_PER_COLUMN))*column+0.02, (ITEMS_PER_COLUMN+(i%ITEMS_PER_COLUMN))*line );

       std::cout << "pos: " <<  ((int)(i/ITEMS_PER_COLUMN))*column+0.02 <<  "," <<  (ITEMS_PER_COLUMN-(i%ITEMS_PER_COLUMN))*line << endl;
    }

    int actual_column=(int)((i-1)/ITEMS_PER_COLUMN)+1;

    int aline=0;
    int j=0;
    char tmp[CED_MAX_LAYER_CHAR];
    int jj=0;

    glColor3f(1.0, 1.0, 1.0);
    sprintf (label, "Layers");
    glRasterPos2f(((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column, 0.80F);
    drawStringBig(label);

    for(i=0;i<NUMBER_DATA_LAYER;i++){
        for(j=0;j<CED_MAX_LAYER_CHAR-1;j++){
            if(layerDescription[i][j] != ','){
                tmp[j]=layerDescription[i][j];
            }else{
                tmp[j]=0;
                j+=2;
                break;
            }
        }
        
        sprintf(label,"[%c] %s%i: %s", layer_keys[i], (i<10)?"0":"", i, tmp);
        drawHelpString(label, ((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column,(ITEMS_PER_COLUMN-(aline%ITEMS_PER_COLUMN))*line);
        aline++;

        jj=j;

        for(;j<CED_MAX_LAYER_CHAR-1;j++){
            if(layerDescription[i][j] == ',' || layerDescription[i][j] == 0){
                tmp[j-jj]=0;
                j++; //scrip ", "
                jj=j+1;
                sprintf(label,"[%c] %s%i: %s", layer_keys[i], (i<10)?"0":"", i, tmp);
                drawHelpString(label, ((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column,-1*(ITEMS_PER_COLUMN-(aline%ITEMS_PER_COLUMN))*line);

                aline++;
                if(layerDescription[i][j] == 0){ break; }
            }else{
                tmp[j-jj]=layerDescription[i][j];
            }
        }
    }
*/
//---------------


    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}
static void display(void){
 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    glPushMatrix();
  
    
  
    // TODO: fix it! 
    // in case of no rotate, in some cases it could get strange 
    // lines in fisheye view from (0,0,0) to (-inf, -inf,x)


    glRotatef(mm.va,1.,0.,0.);
    glRotatef(mm.ha,0.,1.0,0.);
    glScalef(mm.sf,mm.sf,mm.sf); //streech the world





    
    glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z);
  
      //glMatrixMode(GL_MODELVIEW); //
  
    // draw static objects
    display_world(); //only axes?
  
  
     //glTranslatef(0,0,1000);
  
     const GLdouble clip_plane[]={0,0,-1,setting.z_cutting};
     if(setting.z_cutting < 6999){
          glEnable(GL_CLIP_PLANE0);
     }else{
          glDisable(GL_CLIP_PLANE0);
     }
     glClipPlane(GL_CLIP_PLANE0,clip_plane);
  
  
  
  
    // draw elements (hits + detector)
    ced_prepare_objmap();
    ced_do_draw_event();
  
    //cout << "mm.sf: " << mm.sf << "hinterer clipping plane: " << 5000*2.0*mm.sf << std::endl;
    //gluPerspective(60,window_width/window_height,100*2.0*mm.sf,5000*2.0*mm.sf);

//    std::cout  << "clipping planes: " << 200*2.0*mm.sf << " bis " << 5000*2.0*mm.sf << std::endl;
//
//    gluPerspective(60,window_width/window_height,200*2.0*mm.sf,5000*2.0*mm.sf);
//        glMatrixMode( GL_MODELVIEW );
//  
//        glLoadIdentity();
//        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
//
//

    if(showHelp == 1){
        printShortcuts();
    }


    printFPS();
    

  
    glutSwapBuffers();

    glPopMatrix();
}

static void reshape(int w,int h){
    // printf("Reshaped: %dx%d\n",w,h);
    window_width=w;
    window_height=h;
  
  
    //if(graphic[3]){
    if(setting.antia){

        //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        //glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        //glHint(GL_POLYGON_SMOOTH,GL_FASTEST);
  
        //glEnable(GL_POINT_SMOOTH);
        //glEnable(GL_LINE_SMOOTH);
        //glEnable(GL_POLYGON_SMOOTH);
        //glShadeModel(GL_SMOOTH);
  
        //glEnable(GL_BLEND);
        //glEnable (GL_BLEND);
        //glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_LINE_SMOOTH);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    }else{
        glDisable(GL_POINT_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
    }
  
    //if(graphic[2] == 0){
    if(setting.persp == false){

        glViewport(0,0,w,h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-WORLD_SIZE*w/h,WORLD_SIZE*w/h,-WORLD_SIZE,WORLD_SIZE, -15*WORLD_SIZE,15*WORLD_SIZE);
  
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity(); 
    }else{
        glViewport(0,0,w,h);
  
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        //gluPerspective(60,window_width/window_height,100,500000);
        //double plane1, plane2;
        //plane1=100.0*mm.sf;
        //plane2=50000.0*mm.sf;
        //gluPerspective(60,window_width/window_height,plane1,plane2);
        //gluPerspective(60,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);

        gluPerspective(45,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);

        //gluPerspective(170,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);


        //std::cout  << "clipping planes: " << plane1 << " bis " << plane2<< std::endl;


        glMatrixMode( GL_MODELVIEW );
  
        glLoadIdentity();
      
        //glClearDepth(1.0);                  
        //glEnable(GL_DEPTH_TEST);            
        //glDepthFunc(GL_LEQUAL);             
        //glDepthFunc(GL_LESS);             
  
  
  
  
        //glEnable (GL_LINE_SMOOTH);
  
        //glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  
  
        //    glShadeModel(GL_SMOOTH);
   
        //glDepthMask(GL_TRUE);
  
        // //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
        // //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        //glBlendFunc(GL_ONE, GL_ZERO);
        //glEnable(GL_BLEND);
      
        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
    }
  
  
 //   //hauke
 //   if(showHelp == 1){
 //       glutSetWindow (subWindow);
 //       glutReshapeWindow (int(window_width-10),int(window_height/4));
 //   }


    updateScreenshotMenu();
}

void saveSettings(int slot){
    ofstream file;
    const char *home = getenv("HOME");
    char filename[1000];
    char dirname[1000];

    snprintf(dirname, 1000, "%s/.glced_cfg/", home);
    //if(exists){
       mkdir(dirname,700); 
    //}
    snprintf(filename, 1000, "%s/.glced_cfg/settings%i", home, slot);

    //file.open(filename, ios::out | ios::binary);
    file.open(filename);

    if(file.is_open()){ 
//        file << setting.trans << endl;
//        file << setting.persp << endl;
//        file.close();
        setting.va=mm.va;
        setting.ha=mm.ha;
        setting.win_w=(int)window_width;
        setting.win_h=(int)window_height;
        setting.zoom = mm.sf; 
        setting.fisheye_alpha=fisheye_alpha;

        setting.fisheye_world_size = FISHEYE_WORLD_SIZE; 
        setting.world_size = WORLD_SIZE;
        //double bgcolor[4];
        glGetDoublev(GL_COLOR_CLEAR_VALUE, setting.bgcolor);
        //glGetDoublev(GL_COLOR_CLEAR_VALUE, bgcolor);
        //cout << "bgcolor: " << bgcolor[0] << ", " << bgcolor[1] << ", " << bgcolor[2] << ", "  << bgcolor[3] << "\n" ;

        //file.write((char*)&setting, sizeof(setting));
        file<<"#Config version:"<<std::endl<<VERSION_CONFIG << std::endl; 
        file<<"#Transp:"<<std::endl<<setting.trans << std::endl; 
        file<<"#Persp:"<<std::endl<<setting.persp  << std::endl;
        file<<"#Anti A:"<<std::endl<<setting.antia<< std::endl;
        file<<"#Light:"<<std::endl<<setting.light<< std::endl;
        file<<"#Cut angle:"<<std::endl<<setting.cut_angle<< std::endl;
        file<<"#Trans value:"<<std::endl<<setting.trans_value<< std::endl;
        for(int i=0;i<CED_MAX_LAYER;i++){
            file<<"#Visibility Layer " << i << ":" <<std::endl<<setting.layer[i]<< std::endl;
        }
        file<<"#Phi projection:"<<std::endl<<setting.phi_projection<< std::endl;
        file<<"#Z projection:"<<std::endl<<setting.z_projection<< std::endl;
        for(int i=0;i<3;i++){
            file<<"#View setting" << i << ":" <<std::endl<<setting.view[i] << std::endl;
        }
        file<<"#Vertical angle:"<<std::endl<<setting.va<< std::endl;
        file<<"#Horiz angle:"<<std::endl<<setting.ha<< std::endl;
        file<<"#Fixed view:"<<std::endl<<setting.fixed_view<< std::endl;
        file<<"#Z cutting:"<<std::endl<<setting.z_cutting<< std::endl;
        file<<"#Window height:"<<std::endl<<setting.win_h<< std::endl;
        file<<"#Window width:"<<std::endl<<setting.win_w<< std::endl;
        file<<"#Zoom:"<<std::endl<<setting.zoom<< std::endl;
        file<<"#Fisheye_alpha:"<<std::endl<<setting.fisheye_alpha<< std::endl;
        file<<"#World size:"<<std::endl<<setting.world_size<< std::endl;
        file<<"#fisheye world size:"<<std::endl<<setting.fisheye_world_size<< std::endl;
        for(int i=0;i<4;i++){
            file<<"#Background color, value "<< i << ":" << std::endl<<setting.bgcolor[i]<< std::endl;
        }

        file<<"#Show axes:"<<std::endl<<setting.show_axes<< std::endl;
        file<<"#Show fps:"<<std::endl<<setting.fps<< std::endl;


        std::cout << "Save settings to: " << filename << std::endl;

    }else{
        std::cout << "Error open file: " << filename << std::endl;
    }
}

void defaultSettings(void){
        setting.trans=true;
        setting.persp=true;
        setting.light=false;
        setting.antia=false;
        setting.cut_angle=180;
        setting.trans_value=0.8;
        setting.z_cutting=7000;


        setting.win_w=500;
        setting.win_h=500;
        setting.show_axes=true;
        setting.fps=false;

        setting.va=mm.va;
        setting.ha=mm.ha;
    
        
        for(int i=0; i < CED_MAX_LAYER; i++){
            setting.layer[i]=true; // turn all layers on
        }
        for(int i=0;i < 4; i++){
            setting.bgcolor[i]=0; //black
        }

        std::cout << "Set to default settings" << std::endl;
}

void idle(void){
    glutPostRedisplay();
}


void loadSettings(int slot){
    ifstream file;

    const char *home = getenv("HOME");
    char filename[1000];
    snprintf(filename, 1000, "%s/.glced_cfg/settings%i",home, slot);
    //std::cout << "Read config: " << filename << std::endl;
    file.open(filename);

    if(file.is_open()){
        string line;
//        file.read((char*)&setting, sizeof(setting));
            getline(file,line);getline(file,line);
            if(VERSION_CONFIG != atoi(line.c_str())){
                std::cout << "WARNING: Cant read configfile (" << filename << ") please delete or rename it" << std::endl; 
                defaultSettings();
            }

            getline(file,line);getline(file,line);
            setting.trans=atoi(line.c_str());

            getline(file,line);getline(file,line);
            setting.persp=atoi(line.c_str());

            getline(file,line);getline(file,line);
            setting.antia=atoi(line.c_str());

            getline(file,line);getline(file,line);
            setting.light=atoi(line.c_str());

            getline(file,line);getline(file,line);
            setting.cut_angle=atof(line.c_str());

            getline(file,line);getline(file,line);
            setting.trans_value=atof(line.c_str());

            for(int i=0;i<CED_MAX_LAYER;i++){
                getline(file,line);getline(file,line);
                setting.layer[i]=atoi(line.c_str());
            }

            getline(file,line);getline(file,line);
            setting.phi_projection=atoi(line.c_str());

            getline(file,line);getline(file,line);
            setting.z_projection=atoi(line.c_str());

            for(int i=0;i<3;i++){
                getline(file,line);getline(file,line);
                setting.view[i]=atof(line.c_str());
            }

            getline(file,line);getline(file,line);
            setting.va=atof(line.c_str());
            getline(file,line);getline(file,line);
            setting.ha=atof(line.c_str());

            getline(file,line);getline(file,line);
            setting.fixed_view=atoi(line.c_str());
            
            getline(file,line);getline(file,line);
            setting.z_cutting=atof(line.c_str());

            getline(file,line);getline(file,line);
            setting.win_h=atoi(line.c_str());

            getline(file,line);getline(file,line);
            setting.win_w=atoi(line.c_str());
            if(setting.win_w == 0 || setting.win_h == 0){
                setting.win_w = setting.win_h = 500;
            }

            getline(file,line);getline(file,line);
            setting.zoom=atof(line.c_str());

            getline(file,line);getline(file,line);
            setting.fisheye_alpha=atof(line.c_str());

            getline(file,line);getline(file,line);
            setting.world_size=atof(line.c_str());

            getline(file,line);getline(file,line);
            setting.fisheye_world_size=atof(line.c_str());

            for(int i=0;i<4;i++){
                getline(file,line);getline(file,line);
                setting.bgcolor[i] = atof(line.c_str());
            }

            getline(file,line);getline(file,line);
            setting.show_axes=atoi(line.c_str());

            getline(file,line);getline(file,line);
            setting.fps=atoi(line.c_str());



        mm.va=setting.va;
        mm.ha=setting.ha;
        mm.sf = setting.zoom; 
        fisheye_alpha=setting.fisheye_alpha;


        FISHEYE_WORLD_SIZE = setting.fisheye_world_size; 
        WORLD_SIZE=setting.world_size;




        //set_bg_color(setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[3]); 



        //reshape(setting.win_w, setting.win_h);


        std::cout << "Read settings from: " << filename << std::endl;

    }else{ //set to default
        std::cout << "WARNING: Failed to read settings from: " << filename << std::endl;
        defaultSettings();
    }

}




void mouseWheel(int button, int dir, int x, int y){ //hauke
    if(dir > 0){
        selectFromMenu(VIEW_ZOOM_IN);
    }else{
        selectFromMenu(VIEW_ZOOM_OUT);
    }
}
static void mouse(int btn,int state,int x,int y){
    //hauke
    struct timeval tv;

    struct __glutSocketList *sock;

    //#ifdef __APPLE__
    //hauke
    int mouseWheelDown=9999;
    int mouseWheelUp=9999;

    #ifdef GLUT_WHEEL_UP
        mouseWheelDown = GLUT_WHEEL_DOWN;
        mouseWheelUp = GLUT_WHEEL_UP;
    #else
        if(glutDeviceGet(GLUT_HAS_MOUSE)){
            //printf("Your mouse have %i buttons\n", glutDeviceGet(GLUT_NUM_MOUSE_BUTTONS)); 
    
            mouseWheelDown= glutDeviceGet(GLUT_NUM_MOUSE_BUTTONS)+1;
            mouseWheelUp=glutDeviceGet(GLUT_NUM_MOUSE_BUTTONS);
        }
    #endif

    if(state!=GLUT_DOWN){
        move_mode=NO_MOVE;
        return;
    }
    mouse_x=x;
    mouse_y=y;
    mm.ha_start=mm.ha;
    mm.va_start=mm.va;
    mm.sf_start=mm.sf;
    mm.mv_start=mm.mv;
    switch(btn){
    case GLUT_LEFT_BUTTON:
        //hauke
        gettimeofday(&tv, 0); 
        //FIX IT: get the system double click time
        if( (tv.tv_sec*1000000+tv.tv_usec-doubleClickTime) < 300000 && (tv.tv_sec*1000000+tv.tv_usec-doubleClickTime) > 5){ //1000000=1sec
            //printf("Double Click %f\n", tv.tv_sec*1000000+tv.tv_usec-doubleClickTime);
            if(!ced_picking(x,y,&mm.mv.x,&mm.mv.y,&mm.mv.z)){
               sock=__glutSockets;
               int id = SELECTED_ID;
               //printf(" ced_get_selected : socket connected: %d", sock->fd );	
               send( sock->fd , &id , sizeof(int) , 0 );
            }
        }else{
            //printf("Single Click\n");
            if(setting.fixed_view == 0){ //dont rotate the view when in side or front projection
                move_mode=TURN_XY;
            }
        }
        doubleClickTime=tv.tv_sec*1000000+tv.tv_usec;
        return;
        case GLUT_RIGHT_BUTTON:
          move_mode=ZOOM;
          return;
        case GLUT_MIDDLE_BUTTON:
          //#ifdef __APPLE__
          //    move_mode=ZOOM;
          //#else
          //    move_mode=ORIGIN;
	      //#endif
          move_mode=ORIGIN;
          return;
        default:
          break;
    }


    //hauke
    if (btn == mouseWheelUp || btn == 3 ){ // 3 is mouse-wheel-up under ubuntu
    
        selectFromMenu(VIEW_ZOOM_IN);
      //  mm.mv.z+=150./mm.sf;
      //  glutPostRedisplay();
        return;
    }
    if (btn == mouseWheelDown || btn == 4 ){ // 4 is mouse-wheel-down under ubuntu
        selectFromMenu(VIEW_ZOOM_OUT);
        return;
    
      //  mm.mv.z-=150./mm.sf;
      //  glutPostRedisplay();
    }
    //end hauke
    
}

void printBinaer(int x){
    printf("Binaer:"); 
    int i;
    for(i=20;i>=0;--i) {
        printf("%d",((x>>i)&1));
    }
    printf("\n");
}
/*
int isLayerVisible(int x){
    if( ((1<<(x))&ced_visible_layers) > 0){
        return(1);
    }else{
        return(0);
    }
}
*/




/*
static void toggle_layer(unsigned l){
    //printf("Toggle layer %u:\n",l);
    //printBinaer(ced_visible_layers);
    ced_visible_layers^=(1<<l);
    //std::cout << "ced_visible_layers: "<<ced_visible_layers << std::endl;

    //  printf("Toggle Layer %u  and ced_visible_layers = %u \n",l,ced_visible_layers);  
    //printBinaer(ced_visible_layers);
}
*/
static void toggle_layer(unsigned l){
    if(l > CED_MAX_LAYER-1){ return; }

//    if(ced_visible_layers[l]){
//        ced_visible_layers[l]=false;
//    }else{
//        ced_visible_layers[l]=true;
//    }


    if(setting.layer[l]){
        setting.layer[l]=false;
    }else{
        setting.layer[l]=true;
    }

}

/*
static void show_all_layers(void){
  ced_visible_layers=0xffffffff;
  //  printf("show all layers  ced_visible_layers = %u \n",ced_visible_layers);
}
*/

static void keypressed(unsigned char key,int x,int y){
    //SM-H: TODO: socket list for communicating with client
    //struct __glutSocketList *sock;
  
    glutSetWindow(mainWindow); //hauke
  
    if(key=='r' || key=='R'){ 
        selectFromMenu(VIEW_RESET);
    } else if(key=='f'){     
       selectFromMenu(VIEW_FRONT);
    } else if(key == 'F'){
      selectFromMenu(TOGGLE_Z_PROJECTION); 
    } else if(key=='s'){
        selectFromMenu(VIEW_SIDE);
    } else if(key=='S'){
        selectFromMenu(TOGGLE_PHI_PROJECTION);
    } else if(key==27) { //esc
        exit(0);
    } else if(key=='c' || key=='C'){
      //selectFromMenu(VIEW_CENTER);
      if(!ced_get_selected(x,y,&mm.mv.x,&mm.mv.y,&mm.mv.z)) glutPostRedisplay();
    } else if(key=='v' || key=='V'){
          selectFromMenu(VIEW_FISHEYE);
    } else if((key>='0') && (key<='9')){
          selectFromMenu(LAYER_0+key-'0');
    } else if(key==')'){ // 0
          selectFromMenu(LAYER_10);
    } else if(key=='!'){ // 1
          selectFromMenu(LAYER_11);
    } else if(key=='@'){ // 2
          selectFromMenu(LAYER_12);
    } else if(key=='#'){ // 3
          selectFromMenu(LAYER_13);
    } else if(key=='$'){ // 4
          selectFromMenu(LAYER_14);
    } else if(key=='%'){ // 5
          selectFromMenu(LAYER_15);
    } else if(key=='^'){ // 6
          selectFromMenu(LAYER_16);
    } else if(key=='&'){ // 7
          selectFromMenu(LAYER_17);
    } else if(key=='*'){ // 8
          selectFromMenu(LAYER_18);
    } else if(key=='('){ // 9
          selectFromMenu(LAYER_19);
    } else if(key=='`'){
          selectFromMenu(LAYER_ALL);
    } else if(key=='t'){
          selectFromMenu(DETECTOR1);
    } else if(key=='y'){
          selectFromMenu(DETECTOR2);
    } else if(key=='u'){
          selectFromMenu(DETECTOR3);
    } else if(key=='i'){
          selectFromMenu(DETECTOR4);
    } else if(key=='o'){
          selectFromMenu(DETECTOR5);
    } else if(key=='p'){
          selectFromMenu(DETECTOR6);
    } else if(key=='['){
          selectFromMenu(DETECTOR7);
    } else if(key==']'){
          selectFromMenu(DETECTOR8);
    } else if(key=='\\'){
          selectFromMenu(DETECTOR9);
    } else if(key=='T'){
          selectFromMenu(DETECTOR10);
    } else if(key=='Y'){
          selectFromMenu(DETECTOR11);
    } else if(key=='U'){
          selectFromMenu(DETECTOR12);
    } else if(key=='I'){
          selectFromMenu(DETECTOR13);
    } else if(key=='O'){
          selectFromMenu(DETECTOR14);
    } else if(key=='P'){
          selectFromMenu(DETECTOR15);
    } else if(key=='{'){
          selectFromMenu(DETECTOR16);
    } else if(key=='}'){
          selectFromMenu(DETECTOR17);
    } else if(key=='|'){
          selectFromMenu(DETECTOR18);
    } else if(key=='~'){
          selectFromMenu(DETECTOR_ALL);
    } else if(key == '+'|| key == '='){
          selectFromMenu(VIEW_ZOOM_IN);
    } else if(key == '-'|| key == '_'){
          selectFromMenu(VIEW_ZOOM_OUT);
    } else if(key == 'z'){
          if(setting.z_cutting < 7000){ setting.z_cutting+=100; };
          glutPostRedisplay();
    } else if(key == 'Z'){
          if(setting.z_cutting > -7000){ setting.z_cutting-=100; };
          glutPostRedisplay();
    } else if(key=='t'){ // t - momentum at ip layer 2
      toggle_layer(20);
      glutPostRedisplay();
    } else if(key=='y'){ // y - momentum at ip layer = 3
      toggle_layer(21);
      glutPostRedisplay();
    } else if(key=='u'){ // u - momentum at ip layer = 4
      toggle_layer(22);
      glutPostRedisplay();
    } else if(key=='i'){ // i - momentum at ip layer = 5
      toggle_layer(23);
      glutPostRedisplay();
    } else if(key=='o'){ // o - momentum at ip layer = 6
      toggle_layer(24);
      glutPostRedisplay();
    } else if(key=='b'){ // toggle background color
        ++iBGcolor;
        if (iBGcolor >= sizeof(bgColors)/sizeof(color_t)){
            glClearColor(userDefinedBGColor[0],userDefinedBGColor[1],userDefinedBGColor[2],userDefinedBGColor[3]);
            iBGcolor=-1;
            printf("using color: %s\n","user defined");
            glutPostRedisplay();
            return;
        }else{
            glClearColor(bgColors[iBGcolor][0],bgColors[iBGcolor][1],bgColors[iBGcolor][2],bgColors[iBGcolor][3]);
            glutPostRedisplay();
            printf("using color %u\n",iBGcolor);
        }
    } else if(key == 'h'){
          toggleHelpWindow();
    } else if(key == '<'){
          if(setting.trans_value > 0.005){
            //std::cout << "increase trans: " << setting.trans_value << endl;
            setting.trans_value-=0.005;
            glutPostRedisplay();
          }
    }else if(key == '>'){
          if(setting.trans_value < 1-0.005){
            setting.trans_value+=0.005;
            //std::cout << "decrease trans" << setting.trans_value <<endl;
            glutPostRedisplay();
          }
    }else if(key == 'm'){
          if(setting.cut_angle > 0){
            setting.cut_angle-=0.5;
            //std::cout << "decrease trans" << setting.trans_value <<endl;
            glutPostRedisplay();
          }
    } else if(key == 'M'){
          if(setting.cut_angle < 360){
            setting.cut_angle+=0.5;
            //std::cout << "decrease trans" << setting.trans_value <<endl;
            glutPostRedisplay();
          }
    }


}

static void SpecialKey( int key, int x, int y ){
   switch (key) {
   case GLUT_KEY_UP:
       mm.mv.z+=50.;
      break;
   case GLUT_KEY_DOWN:
       mm.mv.z-=50.;
      break;
   default:
      return;
   }
   glutPostRedisplay();
}


static void motion(int x,int y){
  
    // printf("Mouse moved: %dx%d %f\n",x,y,angle_z);
    if((move_mode == NO_MOVE) || !window_width || !window_height)
      return;
  
    if(move_mode == TURN_XY){
      //    angle_y=correct_angle(start_angle_y-(x-mouse_x)*180./window_width);
      //    turn_xy((x-mouse_x)*M_PI/window_height,
      //           (y-mouse_y)*M_PI/window_width);
      mm.ha=mm.ha_start+(x-mouse_x)*180./window_width;
      mm.va=mm.va_start+(y-mouse_y)*180./window_height;
      
      //todo
    } else if (move_mode == ZOOM){
        mm.sf=mm.sf_start+(y-mouse_y)*10./window_height;
        if(mm.sf<0)
  	  mm.sf=0.001;
        else if(mm.sf>2000.)
  	  mm.sf=2000.;
     
    } else if (move_mode == ORIGIN){
        /* 
        //old code: do not work with rotate 
        mm.mv.x=mm.mv_start.x-(x-mouse_x)*WORLD_SIZE/window_width
        mm.mv.y=mm.mv_start.y+(y-mouse_y)*WORLD_SIZE/window_height
        */
  
        float grad2rad=3.141*2/360;
        float x_factor_x =  cos(mm.ha*grad2rad);
        float x_factor_y =  cos((mm.va-90)*grad2rad)*cos((mm.ha+90)*grad2rad);
        float y_factor_x =  0; 
        float y_factor_y = -cos(mm.va*grad2rad);
        float z_factor_x =  cos((mm.ha-90)*grad2rad);
        float z_factor_y = -cos(mm.ha*grad2rad)*cos((mm.va+90)*grad2rad);
  
        //float scale_factor=2200/mm.sf/exp(log(window_width*window_height)/2) ;
        float scale_factor=580/mm.sf/exp(log(window_width*window_height)/2.5) ;
  
  
        //mm.mv.x=mm.mv_start.x- (x-mouse_x)*WORLD_SIZE/window_width*10*x_factor_x - (y-mouse_y)*WORLD_SIZE/window_width*10*x_factor_y;
        //mm.mv.y=mm.mv_start.y- (x-mouse_x)*WORLD_SIZE/window_width*10*y_factor_x - (y-mouse_y)*WORLD_SIZE/window_width*10*y_factor_y;
        //mm.mv.z=mm.mv_start.z - (x-mouse_x)*WORLD_SIZE/window_width*10*z_factor_x - (y-mouse_y)*WORLD_SIZE/window_width*10*z_factor_y;
  
        mm.mv.x=mm.mv_start.x- scale_factor*(x-mouse_x)*x_factor_x - scale_factor*(y-mouse_y)*x_factor_y;
        mm.mv.y=mm.mv_start.y- scale_factor*(x-mouse_x)*y_factor_x - scale_factor*(y-mouse_y)*y_factor_y;
        mm.mv.z=mm.mv_start.z -scale_factor*(x-mouse_x)*z_factor_x - scale_factor*(y-mouse_y)*z_factor_y;
  
  
        //printf("y_factor_x = %f, y_factor_y=%f\n", y_factor_x, y_factor_y);
        //printf("mm.ha = %f, mm.va = %f\n",mm.ha, mm.va);
    }   
    glutPostRedisplay();
}

static void timer (int val)
{
    //change timer for testing to 1
    fd_set fds;
    int rc;
    struct __glutSocketList *sock;
    int max_fd=0;
  
    /* set timeout to 0 for nonblocking select call */
    struct timeval timeout={0,0};
  
    FD_ZERO(&fds);
  
    for(sock=__glutSockets;sock;sock=sock->next)
      {
        FD_SET(sock->fd,&fds);
        if(sock->fd>max_fd)
  	max_fd=sock->fd;
      }
    /* FIXME? Is this the correct way for a non blocking select call? */
    rc = select(max_fd + 1, &fds, NULL, NULL, &timeout);
    if (rc < 0) 
      {
        if (errno == EINTR) 
         {
  	  //glutTimerFunc(500,timer,01);
        //glutTimerFunc(50,timer,1);
        glutTimerFunc(1,timer,1);
  
  	  return;
  	} 
        else 
  	{
  	  perror("In glced::timer, during select.");
  	  exit(-1);
  	}
      }
    // speedup if rc==0
    else if (rc>0)
      {
        for(sock=__glutSockets;sock;sock=sock->next)
  	{
  	  if(FD_ISSET(sock->fd,&fds))
  	    {
  	      (*(sock->read_func))(sock);
            //printf("reading...\n");
  	      //glutTimerFunc(500,timer,01);
            //glutTimerFunc(50,timer,01);
          glutTimerFunc(1,timer,01);
  
  
  	      return ; /* to avoid complexity with removed sockets */
  	    }
  	}
      }
  
    //fix for old glut version
    glutSetWindow(mainWindow); 
  
    //glutTimerFunc(200,timer,01);
    glutTimerFunc(1,timer,01);
    return;
}



int glut_tcp_server(unsigned short port, void (*user_func)(void *data));

static void input_data(void *data){
    if(ced_process_input(data)>0){
        glutPostRedisplay();
    }
}


//http://www.linuxfocus.org/English/March1998/article29.html
void drawString (char *s){
    unsigned int i;
    for (i = 0; i[s]; i++){
        glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, s[i]);
      //glutBitmapCharacter (GLUT_BITMAP_9_BY_15, s[i]);
    }
}


void subDisplay(void){
    char label[CED_MAX_LAYER_CHAR];
    int i;

    glutSetWindow(subWindow);
    //glClearColor(0.5, 0.5, 0.5, 100);
    glClearColor(0.5, 0.5, 0.5, 0.5);


    //std::cout << glutGet(GLUT_WINDOW_WIDTH) << " vs " << window_width << std::endl; 
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float line = 45/window_height; //height of one line
    //float column = 200/window_width;
    float column = 200/window_width; //width of one line

    const int ITEMS_PER_COLUMN=int(window_height/60.0); //how many lines per column?
    //const int MAX_COLUMN= window_width/100;
    //border
    glColor3f(0,0.9,.9);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0.001, 0.01);
    glVertex2f(0.001, 0.99);
    glVertex2f(0.999, 0.99);
    glVertex2f(0.999, 0.01);
    glEnd();

    glColor3f(1.0, 1.0, 1.0); //white

    //printf("window_height %f\nwindow width %f\n", window_height, window_width);

    vector<string> shortcuts;
    shortcuts.push_back( "[h] Toggle shortcut frame" );
    shortcuts.push_back( "[r] Reset view" );
    shortcuts.push_back( "[f] Font view" );
    shortcuts.push_back( "[s] Side view" );
    shortcuts.push_back( "[F] Front projection" );
    shortcuts.push_back( "[S] Side projection" );
    shortcuts.push_back( "[v] Fisheye projection" );
    shortcuts.push_back( "[b] Change background color" );
    shortcuts.push_back( "[+] Zoom in" );
    shortcuts.push_back( "[-] Zoom out" );
    shortcuts.push_back( "[c] Center" );
    shortcuts.push_back( "[Z] Cut in z-axe direction" );
    shortcuts.push_back( "[z] Cut in -z-axe direction" );
    shortcuts.push_back( "[>] Increase transparency" );
    shortcuts.push_back( "[<] Decrease transparency" );
    shortcuts.push_back( "[`] Toggle all data layers" );
    shortcuts.push_back( "[~] Toggle all detector layers" );
    shortcuts.push_back( "[Esc] Quit CED" );

    glColor3f(1.0, 1.0, 1.0);
    sprintf (label, "Control keys");
    glRasterPos2f(((int)(0/ITEMS_PER_COLUMN))*column+0.02, 0.80F);
    drawStringBig(label);

    //for(i=0;(unsigned) i<sizeof(shortcuts)/sizeof(shortcuts[0]);i++){

    for(i=0;(unsigned) i<shortcuts.size();i++){
       //if((i/ITEMS_PER_COLUMN) > MAX_COLUMN) break;
       //sprintf(label,"%s", shortcuts[i]);
       //glRasterPos2f(((int)(i/ITEMS_PER_COLUMN))*column+0.02,(ITEMS_PER_COLUMN-(i%ITEMS_PER_COLUMN))*line);
       //printf("i=%i  lineposition=%i\n",i, (ITEMS_PER_COLUMN-(i%ITEMS_PER_COLUMN)));
       drawHelpString(shortcuts[i], ((int)(i/ITEMS_PER_COLUMN))*column+0.02, (ITEMS_PER_COLUMN-(i%ITEMS_PER_COLUMN))*line );
        //drawString(label);
    }

    int actual_column=(int)((i-1)/ITEMS_PER_COLUMN)+1;

    int aline=0;
    int j=0;
    char tmp[CED_MAX_LAYER_CHAR];
    int jj=0;

    glColor3f(1.0, 1.0, 1.0);
    sprintf (label, "Layers");
    glRasterPos2f(((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column, 0.80F);
    drawStringBig(label);

    for(i=0;i<NUMBER_DATA_LAYER;i++){
        for(j=0;j<CED_MAX_LAYER_CHAR-1;j++){
            if(layerDescription[i][j] != ','){
                tmp[j]=layerDescription[i][j];
            }else{
                tmp[j]=0;
                j+=2;
                break;
            }
        }
        
       //sprintf(label,"[%c] %s%i: %s", layer_keys[i], (i<10)?"0":"", i, layerDescription[i]);
        sprintf(label,"[%c] %s%i: %s", layer_keys[i], (i<10)?"0":"", i, tmp);
        drawHelpString(label, ((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column,(ITEMS_PER_COLUMN-(aline%ITEMS_PER_COLUMN))*line);
        aline++;

        jj=j;

        for(;j<CED_MAX_LAYER_CHAR-1;j++){
            if(layerDescription[i][j] == ',' || layerDescription[i][j] == 0){
                tmp[j-jj]=0;
                j++; //scrip ", "
                jj=j+1;
                //drawHelpString(tmp, ((int)(aline/ITEMS_PER_COLUMN)+actual_column+.18)*column,(ITEMS_PER_COLUMN-(aline%ITEMS_PER_COLUMN))*line);
                sprintf(label,"[%c] %s%i: %s", layer_keys[i], (i<10)?"0":"", i, tmp);
                drawHelpString(label, ((int)(aline/ITEMS_PER_COLUMN)+actual_column)*column,(ITEMS_PER_COLUMN-(aline%ITEMS_PER_COLUMN))*line);

                aline++;
                if(layerDescription[i][j] == 0){ break; }
            }else{
                tmp[j-jj]=layerDescription[i][j];
            }
        }
    }
    glutSwapBuffers ();
}
void subReshape (int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluOrtho2D (0.0F, 1.0F, 0.0F, 1.0F);
};
void writeString(char *str,int x,int y){
    int i;
    glColor3f(0, 0.0, 0.0);//print timer in red
    glRasterPos2f(x, y);

    for(i=0;str[i];i++){
       //glRasterPos2f(x+i*10,y);
       glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10,str[i]);
       glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]); 
       printf("char = %c", str[i]);
    }
    //glPopMatrix();
}
void toggleHelpWindow(void){ //hauke
    if(showHelp == 1){
        showHelp=0;
    }else{
        showHelp=1;
    }
    glutPostRedisplay();
//    mainWindow=glutGetWindow();
//    
//    if(showHelp == 1){
//        glutDestroyWindow(subWindow);
//        showHelp=0;
//    }else if(showHelp == 0){
//        subWindow=glutCreateSubWindow(mainWindow,5,5,int(window_width-10),int(window_height/4.0));
//
//        glutDisplayFunc(subDisplay);
//        glutReshapeFunc(subReshape);
//            
//        glutKeyboardFunc(keypressed);
//        glutSpecialFunc(SpecialKey);
//
//        glutPostRedisplay();
//
//        glutSetWindow(mainWindow);
//        showHelp=1;
//    }    
//    glutSetWindow(mainWindow);
}

void updateLayerEntryInPopupMenu(int id){ //id is layer id, not menu id!
    char string[200];
    char tmp[41];
    if(id < 0 || id > NUMBER_POPUP_LAYER-1){
        return;
    }
    strncpy(tmp, layerDescription[id], 40); 
    tmp[40]=0;
    
    sprintf(string,"[%s] Layer %s%i [%c]: %s%s",isLayerVisible(id)?"X":"   ", (id < 10)?"0":"" ,id, layer_keys[id], tmp, (strlen(layerDescription[id]) > 40)?"...":"");
    glutSetMenu(layerMenu);
    glutChangeToMenuEntry(id+2,string, id+LAYER_0);                     
}

void updateScreenshotMenu(void){
    char tmp[200];

    glutSetMenu(subscreenshot);
    window_width=  glutGet(GLUT_WINDOW_WIDTH);
    window_height= glutGet(GLUT_WINDOW_HEIGHT);

    sprintf(tmp,"Screenshot small (%.0f x %.0f) (~ %.2f MB)",window_width, window_height, window_width*window_height*3./1000000.);
    glutChangeToMenuEntry(1,tmp,SAVE_IMAGE1);

    sprintf(tmp,"Screenshot medium (%.0f x %.0f) (~ %.2f MB)",window_width*4, window_height*4, 4*4*window_width*window_height*3./1000000.);
    glutChangeToMenuEntry(2,tmp,SAVE_IMAGE4);


    sprintf(tmp,"Screenshot large (%.0f x %.0f) (~ %.2f MB)",window_width*10, window_height*10, 10*10*window_width*window_height*3./1000000.);
    glutChangeToMenuEntry(3,tmp,SAVE_IMAGE10);


    sprintf(tmp,"Screenshot extra large (%.0f x %.0f) (~ %.2f MB)",window_width*20, window_height*20, 20*20*window_width*window_height*3./1000000.);
    glutChangeToMenuEntry(4,tmp,SAVE_IMAGE20);


    sprintf(tmp,"Screenshot too large (%.0f x %.0f) (~ %.2f MB)",window_width*100, window_height*100, 100*100*window_width*window_height*3./1000000.);
    glutChangeToMenuEntry(5,tmp,SAVE_IMAGE100);

}
void updateSaveLoadMenu(int id){ //id is save id, not menu id!
    struct stat s;


    const char *home = getenv("HOME");
    char filename[1000];
    char menuStr[1000];
    snprintf(filename, 1000, "%s/.glced_cfg/settings%i", home, id);
    if(stat(filename,&s) == 0){
        snprintf(menuStr,1000,"Slot %i, created: %s",id,ctime(&s.st_mtime));
    }else{
        snprintf(menuStr,1000,"Slot %i, %s",id,"Empty");
    }

    glutSetMenu(subSave);
    glutChangeToMenuEntry(id,menuStr, SAVE1+id-1);                     
    


    glutSetMenu(subLoad);
    glutChangeToMenuEntry(id,menuStr, LOAD1+id-1);                     

    //std::cout << menuStr << std::endl;
}


void updateLayerEntryDetector(int id){ //id is layer id, not menu id!
    char string[200];
    char tmp[41];
    if(id < NUMBER_DATA_LAYER || id > NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER-1 || id > CED_MAX_LAYER-1 || id < 0){
        return;
    }
    strncpy(tmp, layerDescription[id], 40); 
    tmp[40]=0;
    
    //sprintf(string,"[%s] Layer %s%i [%c]: %s%s",isLayerVisible(id)?"X":"   ", (id < 10)?"0":"" ,id, layer_keys[id], tmp, (strlen(layerDescription[id]) > 40)?"...":"");
    sprintf(string,"[%s] Layer %s%i [%c]: %s%s",isLayerVisible(id)?"X":"   ", (id < 10)?"0":"" ,id,detec_layer_keys[id-NUMBER_DATA_LAYER],tmp, (strlen(layerDescription[id]) > 40)?"...":"");

    glutSetMenu(detectorMenu);
    glutChangeToMenuEntry(id-NUMBER_DATA_LAYER+2,string, id-NUMBER_DATA_LAYER+DETECTOR1);                     
}


void addLayerDescriptionToMenu(int id, char * str){
    if(id < 0 || id >= CED_MAX_LAYER){
        printf("Warning: Layer id out of range\n");
        return;
    }
    strncpy(layerDescription[id], str,CED_MAX_LAYER_CHAR-1);
    updateLayerEntryInPopupMenu(id);
    updateLayerEntryDetector(id);

}


void update_cut_angle_menu(void){
    char str[200];

    int i;

    glutSetMenu(subsubMenu2);

    for(i=0; (unsigned)i < sizeof(available_cutangles)/sizeof(available_cutangles[0]); i++){

        if(available_cutangles[i] == setting.cut_angle){
            sprintf(str,"[X] %i", available_cutangles[i]);
            glutChangeToMenuEntry(i+1, str,  CUT_ANGLE0+i);
        }else{
            sprintf(str,"[  ] %i", available_cutangles[i]);
            glutChangeToMenuEntry(i+1, str,  CUT_ANGLE0+i);
        }
    }

}


void selectFromMenu(int id){ //hauke
    int i;
    int anz;
    static float z_cutting_backup;
    static float cut_angle_backup;
    static float mm_ha_backup; 
    static float mm_va_backup;
    static int graphic_2_backup;
    //static int fullscreen=false;


    glutSetWindow(mainWindow); //hauke


    switch(id){
        case BGCOLOR_GAINSBORO:
            set_bg_color(0.862745,0.862745,0.862745,0); 
            //set_bg_color(0.862745,0.862745,0.862745,0);
            break;

        case BGCOLOR_LIGHTGREY:
            set_bg_color(0.827451,0.827451,0.827451,0);
            break;

        case BGCOLOR_DARKGRAY:
            set_bg_color(0.662745,0.662745,0.662745,0);
            break;

        case BGCOLOR_GRAY:
            set_bg_color(0.501961,0.501961,0.501961,0);
            break;

        case BGCOLOR_SILVER:
            set_bg_color(0.7529,0.7529,0.7529,0);
            break;

        case BGCOLOR_DIMGRAY:
            set_bg_color(0.4118,0.4118,0.4118,0);
            break;

        case BGCOLOR_LIGHTSTEELBLUE:
            set_bg_color(0.6902,0.7686 ,0.8706,0);
            break;

        case BGCOLOR_STEELBLUE:
            set_bg_color(0.2745,0.5098,0.70588,0);
            break;

        case BGCOLOR_SEAGREEN:
            set_bg_color(0.18039,0.54509,0.34117,0);
            break;

        case BGCOLOR_ORANGE:
            set_bg_color(1,0.647,0,0);
            break;

        case BGCOLOR_YELLOW:
            set_bg_color(1,1,0,0);
            break;

        case BGCOLOR_VIOLET:
            set_bg_color(0.9333,0.5098,0.9333,0);
            break;

        case BGCOLOR_BLACK:
            set_bg_color(0,0,0,0);
            break;

        case BGCOLOR_BLUE:
            set_bg_color(0,0.2,0.4,0);
            break;

        case BGCOLOR_WHITE:
            set_bg_color(1,1,1,0);
            break;

        case BGCOLOR_USER:
            set_bg_color(userDefinedBGColor[0],userDefinedBGColor[1], userDefinedBGColor[2], userDefinedBGColor[3]);

        case VIEW_RESET:
            //if(graphic[2] == 0){selectFromMenu(GRAFIC_PERSP); }
            if((setting.trans == true && setting.persp == false) || (setting.trans == false && setting.persp == true)){
                selectFromMenu(GRAFIC_PERSP); //switch persp on in new view, switch persp off in classic view
            }
            setting.z_cutting=7000; //no z cutting
            setting.cut_angle=0;    // no detector cutting
            setting.phi_projection = false; // no phi projection
            setting.z_projection=false; // no phi projection;
            mm=mm_reset;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;
            fisheye_alpha=0;
            setting.fixed_view=false;
            update_cut_angle_menu();
            set_world_size(DEFAULT_WORLD_SIZE ); 
            //std::cout << "DEFAULT_WORLD_SIZE "  << DEFAULT_WORLD_SIZE << "zoom: " << mm.sf << std::endl;
            break;


        case VIEW_FISHEYE:
            if(fisheye_alpha==0.0){
                mm.sf *= 8.0; //zoom in to hold the same detector size
                fisheye_alpha = 1e-3;
                FISHEYE_WORLD_SIZE = WORLD_SIZE/(WORLD_SIZE*fisheye_alpha); //<-- new
                set_world_size(WORLD_SIZE); // <-- old
            }
            else{
                mm.sf *= 1.0/8.0; //zoom out for the same look
                fisheye_alpha = 0.0;
                set_world_size(FISHEYE_WORLD_SIZE); //<-- old
            }
            break;

        case VIEW_FRONT:
            //mm=mm_reset;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;

            if(setting.fixed_view){ break;}

            mm.ha=0.;
            mm.va=0.;
            break;

        case VIEW_SIDE:
            //mm=mm_reset;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;
            if(setting.fixed_view){ break;}

                mm.ha=90.;
                mm.va=0.;

            break;

        case TOGGLE_PHI_PROJECTION:
            if(setting.phi_projection){ //turn projection off
                setting.phi_projection=false;
                setting.z_cutting=z_cutting_backup;
                setting.cut_angle=cut_angle_backup;
                //if(graphic_2_backup != graphic[2]){selectFromMenu(GRAFIC_PERSP); } //restore persp setting
                if(graphic_2_backup != setting.persp){selectFromMenu(GRAFIC_PERSP); } //restore persp setting

                mm.ha = mm_ha_backup;
                mm.va = mm_va_backup;

                setting.fixed_view=false;

            }else{ //turn projection on
                if(setting.z_projection){
                    selectFromMenu(TOGGLE_Z_PROJECTION);
                }

                z_cutting_backup=setting.z_cutting;
                cut_angle_backup=setting.cut_angle;

                setting.phi_projection=true;

                //graphic_2_backup=graphic[2];
                graphic_2_backup=setting.persp;

                //if(graphic[2]==1){selectFromMenu(GRAFIC_PERSP); }
                if(setting.persp==1){selectFromMenu(GRAFIC_PERSP); }


                setting.cut_angle=180;
                setting.z_cutting=7000;
                mm_ha_backup=mm.ha;
                mm_va_backup = mm.va;
                mm.ha=90.;
                mm.va=0.;

                setting.fixed_view=true;
            }

            update_cut_angle_menu();
            break;

        case TOGGLE_Z_PROJECTION:
            if(setting.z_projection){ //turn projection off

                setting.z_projection=false;
                //z_cutting=7000;
                //selectFromMenu(GRAFIC_PERSP);
                setting.z_cutting=z_cutting_backup;
                setting.cut_angle=cut_angle_backup;
                //if(graphic[2]==0){selectFromMenu(GRAFIC_PERSP); }
                //if(graphic_2_backup != graphic[2]){selectFromMenu(GRAFIC_PERSP); } //restore persp setting
                if(graphic_2_backup != setting.persp){selectFromMenu(GRAFIC_PERSP); } //restore persp setting


                mm.ha = mm_ha_backup;
                mm.va = mm_va_backup;

                setting.fixed_view=false;
            }else{ //turn projection on

                if(setting.phi_projection){selectFromMenu(TOGGLE_PHI_PROJECTION);}

                z_cutting_backup=setting.z_cutting;
                cut_angle_backup=setting.cut_angle;

                setting.z_projection=true;
                setting.cut_angle=0;
                setting.z_cutting=10;


                //graphic_2_backup=graphic[2];
                graphic_2_backup=setting.persp;

                //if(graphic[2]==1){selectFromMenu(GRAFIC_PERSP); }
                if(setting.persp==true){selectFromMenu(GRAFIC_PERSP); }

               
               //side view
                mm_ha_backup=mm.ha;
                mm_va_backup = mm.va;

                mm.ha=0.;
                mm.va=0.;




                setting.fixed_view=true;
            }

            update_cut_angle_menu();
            break;

        case VIEW_ZOOM_IN:
            mm.sf += mm.sf*50.0/window_height;
            //reshape((int)window_width, (int)window_height);

            //if(mm.sf>50){ mm.sf=50; }
            break;

        case VIEW_ZOOM_OUT:
            mm.sf -= mm.sf*50.0/window_height;
            //reshape((int)window_width, (int)window_height);

            //if(mm.sf<0.01){ mm.sf=0.01; }
            break;

        case VIEW_CENTER:
            //ced_get_selected(x,y,&mm.mv.x,&mm.mv.y,&mm.mv.z);
            break;

        case LAYER_ALL:
            glutSetMenu(layerMenu);
            anz=0;
            //for(i=0;i<NUMBER_POPUP_LAYER;i++){ //try to turn all layers on

            for(i=0;i<NUMBER_DATA_LAYER;i++){ //try to turn all layers on
                if(!isLayerVisible(i)){
                   //sprintf(string,"[X] Layer %s%i [%c]: %s", (i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);                     
                   toggle_layer(i);
                   updateLayerEntryInPopupMenu(i);
                   anz++;
                }
            }
            if(anz == 0){ //turn all layers off
                //for(i=0;i<NUMBER_POPUP_LAYER;i++){

                for(i=0;i<NUMBER_DATA_LAYER;i++){
                   //sprintf(string,"[   ] Layer %s%i [%c]: %s",(i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);                     
                   toggle_layer(i);
                   updateLayerEntryInPopupMenu(id);
                }
            }
            break;

        case DETECTOR_ALL:
            glutSetMenu(detectorMenu);
            anz=0;
            for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){ //try to turn all layers on
                if(!isLayerVisible(i)){
                   //sprintf(string,"[X] Layer %s%i [%c]: %s", (i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);                     
                   toggle_layer(i);
                   updateLayerEntryDetector(i);
                   anz++;
                }
            }
            if(anz == 0){ //turn all layers off
                for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
                   //sprintf(string,"[   ] Layer %s%i [%c]: %s",(i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);                     
                   toggle_layer(i);
                   updateLayerEntryDetector(id);
                }
            }
            break;


        case DETECTOR1:
        case DETECTOR2:
        case DETECTOR3:
        case DETECTOR4:
        case DETECTOR5:
        case DETECTOR6:
        case DETECTOR7:
        case DETECTOR8:
        case DETECTOR9:
        case DETECTOR10:
        case DETECTOR11:
        case DETECTOR12:
        case DETECTOR13:
        case DETECTOR14:
        case DETECTOR15:
        case DETECTOR16:
        case DETECTOR17:
        case DETECTOR18:
        case DETECTOR19:
        case DETECTOR20:

            glutSetMenu(detectorMenu);
            toggle_layer(id-DETECTOR1+NUMBER_DATA_LAYER);
            //std::cout << "toogle layer " << id-DETECTOR1 + NUMBER_DATA_LAYER<< std::endl;
            updateLayerEntryDetector(id-DETECTOR1+NUMBER_DATA_LAYER);
            break;



        case LAYER_0:
        case LAYER_1:
        case LAYER_2:
        case LAYER_3:
        case LAYER_4:
        case LAYER_5:
        case LAYER_6:
        case LAYER_7:
        case LAYER_8:
        case LAYER_9:
        case LAYER_10:
        case LAYER_11:
        case LAYER_12:
        case LAYER_13:
        case LAYER_14:
        case LAYER_15:
        case LAYER_16:
        case LAYER_17:
        case LAYER_18:
        case LAYER_19:
            glutSetMenu(layerMenu);
            toggle_layer(id-LAYER_0);
            //std::cout << "toogle layer " << id-LAYER_0 << std::endl;
            updateLayerEntryInPopupMenu(id-LAYER_0);
            break;

        case CUT_ANGLE0:
            setting.cut_angle=0; 
            update_cut_angle_menu();
            break;

        case CUT_ANGLE30:
            setting.cut_angle=30; 
            update_cut_angle_menu();
            break;

        case CUT_ANGLE90:
            setting.cut_angle=90;
            update_cut_angle_menu();
            break;

        case CUT_ANGLE135:
            setting.cut_angle=135;
            update_cut_angle_menu();
            break;

        case CUT_ANGLE180:
            setting.cut_angle=180;
            update_cut_angle_menu();
            break;

        case CUT_ANGLE270:
            setting.cut_angle=270;
            update_cut_angle_menu();
            break;

        case CUT_ANGLE360:
            setting.cut_angle=360;
            update_cut_angle_menu();
            break;

        case TRANS0:
            setting.trans_value=0;
            break;

        case TRANS40:
            setting.trans_value=0.4;
            break;

        case TRANS60:
            setting.trans_value=0.6;
            break;

        case TRANS70:
            setting.trans_value=0.7;
            break;

        case TRANS80:
            setting.trans_value=0.8;
            break;

        case TRANS90:
            setting.trans_value=0.9;
            break;

        case TRANS95:
            setting.trans_value=0.95;
            break;

        case TRANS100:
            setting.trans_value=1.0;
            break;
//        case FULLSCREEN:
//////            glutDestroyWindow(mainWindow);;
//////            glutGameModeString("1280x1024:32@60");
//////            glutEnterGameMode();
////            if(fullscreen == false){
////                glutFullScreen(); 
////                fullscreen = true;
////            }else{
////                fullscreen = false; 
////                reshape(setting.win_w, setting.win_h);
////            }
//            
        case AXES:
            if(setting.show_axes){
                setting.show_axes= false;
            }else{
                setting.show_axes= true;
            }
            break;

        case FPS:  
            if(setting.fps){
                glutIdleFunc(NULL);
                setting.fps=false;
            }else{
                glutIdleFunc(idle);
                setting.fps=true;
            }
            break;

        case GRAFIC_HIGH:
            setting.light=true;
            setting.trans=false;
            setting.persp=false;
            selectFromMenu(GRAFIC_TRANS);
            selectFromMenu(GRAFIC_LIGHT);
            selectFromMenu(GRAFIC_PERSP);
            break;
            
        case GRAFIC_LOW:
            setting.light=true;
            setting.trans=true;
            setting.persp=true;
            selectFromMenu(GRAFIC_TRANS);
            selectFromMenu(GRAFIC_LIGHT);
            selectFromMenu(GRAFIC_PERSP);
            break;

        case GRAFIC_TRANS:
/*
            if(graphic[1] == 1){
                //printf("Transparency  is now off\n");
                graphic[1] = 0;
            }else{
                //printf("Transparency  is now on\n");
                graphic[1] = 1;
            }

*/

            if(setting.trans == true){
                //printf("Transparency  is now off\n");
                setting.trans = false;
            }else{
                //printf("Transparency  is now on\n");
                setting.trans = true;
            }

            break;
            
        case GRAFIC_LIGHT:
            //if(graphic[0] == 1){
            if(setting.light == true){

                //printf("Light  is now on\n");
                //graphic[0] = 0;
                setting.light=false;
                glDisable(GL_LIGHTING); 
            }else{
                 //printf("Light is now on\n");
                 //graphic[0] = 1;
                setting.light = true;

                 break; //do nothing...

                 //TODO: CHANGE IT
                 GLfloat light0_spec[] = {1, 1, 1, 0.5};
                 GLfloat light0_pos[] = {0, 0, 8000};
                 //GLfloat light0_ambi[]= {0.5, 0.5, 0.5, 0.5};     

                 //glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_ambi);
                 //glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambi);
                 glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);


                 glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
                 
                 ////glClearColor (0.0, 0.0, 0.0, 0.0);
                 //glShadeModel (GL_SMOOTH);



                 //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
                 //glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

                 glColorMaterial ( GL_FRONT_AND_BACK, GL_EMISSION ) ;
                 glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE) ;
                 glEnable (GL_COLOR_MATERIAL) ;


                 glEnable(GL_NORMALIZE);

                 glEnable(GL_LIGHTING); 
                 glEnable(GL_LIGHT0);

                 glEnable(GL_DEPTH_TEST);

                 glMatrixMode(GL_MODELVIEW);
            }
            break;

        case GRAFIC_ALIAS:
            //if(graphic[3] == 1){
            if(setting.antia == true){
                printf("Anti aliasing is off\n");
                //graphic[3] = 0;
                setting.antia = false;
                reshape((int)window_width, (int)window_height);
            }else{
                printf("Anti aliasing is on\n");
                //graphic[3] = 1;
                setting.antia=true;
                reshape((int)window_width, (int)window_height);
            }
            break;


        case GRAFIC_FOG:
                glGetDoublev(GL_COLOR_CLEAR_VALUE, setting.bgcolor);
                //GLfloat fogcolor[4]={setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],1.0};   
                GLfloat fogcolor[4];
                fogcolor[0]=setting.bgcolor[0];
                fogcolor[1]=setting.bgcolor[1];
                fogcolor[2]=setting.bgcolor[2];
                fogcolor[3]=0.5;


                glFogfv(GL_FOG_COLOR,fogcolor);          
                glFogf(GL_FOG_DENSITY,0.5);                 
                //glFogi(GL_FOG_MODE,GL_EXP);             

                glFogi(GL_FOG_MODE,GL_LINEAR);            
                glFogf(GL_FOG_START,500.0);              
                glFogf(GL_FOG_END,3000.0);                
                glHint(GL_FOG_HINT, GL_FASTEST);          
                glEnable(GL_FOG);
                break;


        case GRAFIC_PERSP:
            if(setting.persp == true){
                //printf("Perspective is now flat\n");
                setting.persp = false;

                reshape((int)window_width, (int)window_height); //hack, call resize function to overwrite perspectivic settings
            }else{
                //printf("Perspective is now 3d\n");
                setting.persp = true;
                reshape((int)window_width, (int)window_height); //hack, call resize function to overwrite perspectivic settings
            }
            break;
        case HELP:
            toggleHelpWindow();
            break;
        case SAVE1:
        case SAVE2:
        case SAVE3:
        case SAVE4:
        case SAVE5:
            saveSettings(id-SAVE1+1); 
            updateSaveLoadMenu(id-SAVE1+1);
            break;

        case LOAD1:
        case LOAD2:
        case LOAD3:
        case LOAD4:
        case LOAD5:
            loadSettings(id-LOAD1+1); 
            set_bg_color(setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[3]); 
            break;

        case SAVE_IMAGE1:
            screenshot("/tmp/glced.tga",1);
            break;
        case SAVE_IMAGE4:
            screenshot("/tmp/glced.tga",4);
            break;
        case SAVE_IMAGE10:
            screenshot("/tmp/glced.tga",10);
            break;
        case SAVE_IMAGE20:
            screenshot("/tmp/glced.tga",20);
            break;
        case SAVE_IMAGE100:
            screenshot("/tmp/glced.tga",100);
            break;
    }

    reshape((int)window_width, (int)window_height);
    glutPostRedisplay();
    //printf("bgcolor = %f %f %f %f\n",setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[2]); 

}

int buildMenuPopup(void){ //hauke
    //int menu;
    //int subMenu1;
    //int subMenu2;
    int subMenu3;
    int subMenu4;
    int subsubMenu1;
    //int subsubMenu3;
    int DetectorComponents;

    int bgColorMenu = glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("White",BGCOLOR_WHITE);
    glutAddMenuEntry("Gainsboro", BGCOLOR_GAINSBORO);
    glutAddMenuEntry("Lightgrey", BGCOLOR_LIGHTGREY);
    glutAddMenuEntry("Silver", BGCOLOR_SILVER);
    glutAddMenuEntry("Darkgray", BGCOLOR_DARKGRAY);
    glutAddMenuEntry("Gray", BGCOLOR_GRAY);
    glutAddMenuEntry("Dimgray", BGCOLOR_DIMGRAY);
    glutAddMenuEntry("Black",BGCOLOR_BLACK);
    glutAddMenuEntry("Lightsteelblue",BGCOLOR_LIGHTSTEELBLUE);
    glutAddMenuEntry("Steelblue",BGCOLOR_STEELBLUE);
    glutAddMenuEntry("Blue",BGCOLOR_BLUE);
    glutAddMenuEntry("Seagreen",BGCOLOR_SEAGREEN);
    glutAddMenuEntry("Orange",BGCOLOR_ORANGE);
    glutAddMenuEntry("Yellow",BGCOLOR_YELLOW);
    glutAddMenuEntry("Violet",BGCOLOR_VIOLET);
    if(userDefinedBGColor[0] >= 0){ //is set
        glutAddMenuEntry("User defined",BGCOLOR_USER);
    }

    int cameraMenu = glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Reset view [r]", VIEW_RESET);
    glutAddMenuEntry("Front view [f]", VIEW_FRONT);
    glutAddMenuEntry("Side view [s]", VIEW_SIDE);
    glutAddMenuEntry("Toggle side view projection [S]", TOGGLE_PHI_PROJECTION);
    glutAddMenuEntry("Toggle front view projection [F]", TOGGLE_Z_PROJECTION);
    glutAddMenuEntry("Toggle fisheye projection [v]",VIEW_FISHEYE);
    glutAddMenuEntry("Zoom in [+]", VIEW_ZOOM_IN);
    glutAddMenuEntry("Zoom out [-]", VIEW_ZOOM_OUT);
    //glutAddMenuEntry("Center [c]", VIEW_CENTER);


    //set up detector components and data layer menu
    int i;
    subMenu3 = glutCreateMenu(selectFromMenu);
    layerMenu=subMenu3;
    glutAddMenuEntry("Show/Hide all data Layers [`]", LAYER_ALL);
    for(i=0;i<NUMBER_POPUP_LAYER;i++){
        //sprintf(string,"[%s] Layer %s%i [%c]: %s",isLayerVisible(i)?"X":"   ", (i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
        glutAddMenuEntry(" ",LAYER_0+i);
        //updateLayerEntryInPopupMenu(LAYER_0+i);
    }




    DetectorComponents = glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Show/Hide all detector components", DETECTOR_ALL);
    for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
        glutAddMenuEntry(" ",DETECTOR1+i-NUMBER_DATA_LAYER);
    }
    detectorMenu=DetectorComponents;



    subsubMenu1 = glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Perspective",GRAFIC_PERSP);
    //glutAddMenuEntry("Deepbuffer", GRAFIC_BUFFER);
    glutAddMenuEntry("Transparency/mesh", GRAFIC_TRANS);
    //glutAddMenuEntry("Light", GRAFIC_LIGHT);
    //glutAddMenuEntry("Anti Aliasing", GRAFIC_ALIAS);
    glutAddMenuEntry("Fade far objects to current background color", GRAFIC_FOG);
    glutAddMenuEntry("Toggle visible of axes", AXES);
    #ifndef __APPLE__
        glutAddMenuEntry("Show FPS", FPS);
    #endif


    subsubMenu2 = glutCreateMenu(selectFromMenu);
    for(i=0; (unsigned) i < sizeof(available_cutangles) / sizeof(available_cutangles[0]); i++){
        glutAddMenuEntry(" ",  CUT_ANGLE0+i);
    }

    update_cut_angle_menu();


    int transMenu=glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("  0%",TRANS0);
    glutAddMenuEntry("40%",TRANS40);
    glutAddMenuEntry("60%",TRANS60);
    glutAddMenuEntry("70%",TRANS70);
    glutAddMenuEntry("80%",TRANS80);
    glutAddMenuEntry("90%",TRANS90);
    glutAddMenuEntry("95%",TRANS95);
    glutAddMenuEntry("100%",TRANS100);




    subMenu4 = glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Classic View",GRAFIC_LOW);
    glutAddMenuEntry("New View", GRAFIC_HIGH);
//    glutAddMenuEntry("Full Screen mode", FULLSCREEN);

    glutAddSubMenu("Graphic details", subsubMenu1);
    glutAddSubMenu("Transparency value", transMenu);



    subSave=glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Slot 1",SAVE1);
    glutAddMenuEntry("Slot 2",SAVE2);
    glutAddMenuEntry("Slot 3",SAVE3);
    glutAddMenuEntry("Slot 4",SAVE4);
    glutAddMenuEntry("Slot 5",SAVE5);


    subLoad=glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Slot 1",LOAD1);
    glutAddMenuEntry("Slot 2",LOAD2);
    glutAddMenuEntry("Slot 3",LOAD3);
    glutAddMenuEntry("Slot 4",LOAD4);
    glutAddMenuEntry("Slot 5",LOAD5);

    for(int i=1;i<=5;i++){
        updateSaveLoadMenu(i);
    }


    subscreenshot=glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("...",SAVE_IMAGE1);
    glutAddMenuEntry("...",SAVE_IMAGE4);
    glutAddMenuEntry("...",SAVE_IMAGE10);
    glutAddMenuEntry("...",SAVE_IMAGE20);
    glutAddMenuEntry("...",SAVE_IMAGE100);
    updateScreenshotMenu();


    int graphicDetailsMenu=glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Toggle perspective",GRAFIC_PERSP);
    glutAddMenuEntry("Toggle wireframe",GRAFIC_TRANS);
    glutAddMenuEntry("Fade far objects into background color",GRAFIC_FOG);



    int graphicMenu=glutCreateMenu(selectFromMenu);
    glutAddMenuEntry("Graphic low",GRAFIC_LOW);
    glutAddMenuEntry("Graphic high",GRAFIC_HIGH);
    glutAddSubMenu("Details",graphicDetailsMenu);
    glutAddSubMenu("Change background color",bgColorMenu);



    int toolMenu=glutCreateMenu(selectFromMenu);
    glutAddSubMenu("Screenshot",subscreenshot);
    glutAddMenuEntry("Show FPS",FPS);




    int menu=glutCreateMenu(selectFromMenu);

    //int visiMenu=glutCreateMenu(selectFromMenu);
    glutAddSubMenu("Data layer",subMenu3);
    glutAddSubMenu("Detector components",detectorMenu);
    glutAddSubMenu("Detector cuts",subsubMenu2);
    glutAddSubMenu("Detector transparency",transMenu);
    glutAddMenuEntry("Toggle axes",AXES);
    glutAddSubMenu("Graphic settings",graphicMenu);
    glutAddSubMenu("Save current settings",subSave);
    glutAddSubMenu("Load settings",subLoad);
    glutAddSubMenu("Camera",cameraMenu);
    glutAddMenuEntry("Show Keybinding [h]", HELP);
    glutAddSubMenu("Tools",toolMenu);

//    glutAddSubMenu("View", subMenu2);
//    glutAddSubMenu("Data layers", subMenu3);
//    glutAddSubMenu("Detector components", DetectorComponents);
//    glutAddSubMenu("Detector cuts", subsubMenu2);
//    glutAddSubMenu("Background Color", subMenu1);
//    glutAddSubMenu("Graphic options", subMenu4);
//
//    glutAddSubMenu("Save settings",subSave);
//
//    glutAddSubMenu("Load saved settings",subLoad);
//
//    glutAddSubMenu("Save screenshot",subscreenshot);
//    glutAddMenuEntry("Toggle help [h]",HELP);


    return menu;
}


int main(int argc,char *argv[]){
    bool geometry = false;

    mm_reset=mm;
    WORLD_SIZE = DEFAULT_WORLD_SIZE ;

    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH|GLUT_ALPHA);
    //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    //glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    //  glutInitWindowSize(600,600); // change to smaller window size */
    /*   glutInitWindowPosition(500,0); */
  
    //glutInitWindowSize(500,500);
    //cout << setting.win_w << " x " << setting.win_h << std::endl;


            //glutGameModeString("1280x1024:32@60");
            //glutEnterGameMode();

    loadSettings(1);  
    setting.screenshot_sections=1;
    //set_bg_color(setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[2]); //set to default (black)=0;

    //set_bg_color(0.0,0.0,0.0,0.0); //set to default (black)
    //set_bg_color(bgColors[0][0],bgColors[0][1],bgColors[0][2],bgColors[0][3]); //set to default (light blue [0.0, 0.2, 0.4, 0.0])
  
    //graphic[1]=1; //transp
    //graphic[2]=1; //persp
    //cut_angle=0; //degrees
    //phi_projection=false;
    //projection=false;
  
    //trans_value=0.8;



    char hex[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    int tmp[6];
  
    int i;
    for(i=1;i<argc ; i++){
  
      if(!strcmp( argv[i] , "-world_size" ) ) {
        float w_size = atof(  argv[++i] )  ;
        printf( "  setting world size to  %f " , w_size ) ;
        //set_world_size( w_size ) ;
        mm.sf = 205.0/w_size;
      } else if(!strcmp(argv[i], "-bgcolor") && i < argc-1){
        i++;
        if (!strcmp(argv[i],"Black") || !strcmp(argv[i],"black")){
          printf("Set background color to black.\n");
          set_bg_color(0.0,0.0,0.0,0.0); //Black
        } else if (!strcmp(argv[i],"Blue") || !strcmp(argv[i],"blue")){
          printf("Set background color to blue.\n");
          set_bg_color(0.0,0.2,0.4,0.0); //Dark blue
        }else if (!strcmp(argv[i],"White") || !strcmp(argv[i],"white")){
          printf("Set background color to white.\n");
          set_bg_color(1.0,1.0,1.0,0.0); //White
        }else if((strlen(argv[i]) == 8 && argv[i][0] == '0' && toupper(argv[i][1]) == 'X') || strlen(argv[i]) == 6){
          printf("Set background to user defined color.\n");
          int n=0;
          if(strlen(argv[i]) == 8){
              n=2;
          }
          int k;
          for(k=0;k<6;k++){
              int j;
              tmp[k]=999;
              for(j=0;j<16;j++){
                  if(toupper(argv[i][k+n]) == hex[j]){
                      tmp[k]=j;
                  }
  
              }
              if(tmp[k]==999){
                  printf("Unknown digit '%c'!\nSet background color to default value.\n",argv[i+1][k+n]);
                  break;
              }
              if(k==5){
                  userDefinedBGColor[0] = (tmp[0]*16 + tmp[1])/255.0;
                  userDefinedBGColor[1] = (tmp[2]*16 + tmp[3])/255.0;
                  userDefinedBGColor[2] = (tmp[4]*16 + tmp[5])/255.0;
                  userDefinedBGColor[3] = 0.0;
              
                  printf("set color to: %f/%f/%f\n",(tmp[0]*16 + tmp[1])/255.0, (tmp[2]*16 + tmp[3])/255.0, (tmp[4]*16 + tmp[5])/255.0); 
                  set_bg_color((tmp[0]*16 + tmp[1])/255.0,(tmp[2]*16 + tmp[3])/255.0,(tmp[4]*16 + tmp[5])/255.0,0.0);
              }
          }
        } else {
          printf("Unknown background color.\nPlease choose black/blue/white or a hexadecimal number with 6 digits!\nSet background color to default value.\n");
        }
      
      } else if(!strcmp( argv[i] , "-h" ) || 
         !strcmp( argv[i] , "--help" )|| 
         !strcmp( argv[i] , "-?" )
         ) {


      printf( "\n  CED event display server: \n\n"
          "   Usage:  glced [-bgcolor COLOR] [-world_size LENGTH] [-geometry X_GEOMETRY] [-trust TRUSTED_HOST]\n\n" 
          "        options:  \n"
          "              COLOR:        Background color (values: black, white, blue or hexadecimal number)\n"
          "              LENGTH:       Visible world-cube size in mm (default: 6000) \n"
          "              X_GEOMETRY:   Window position and size in the form WxH+X+Y \n"
          "                              (W:width, H: height, X: x-offset, Y: y-offset) \n"
          "              TRUSTED_HOST: Ip or name of the host who is allowed to connect to CED\n\n"
          "   Example: \n\n"
          "     ./bin/glced -bgcolor 4C4C66 -world_size 1000. -geometry 600x600+500+0  -trust 192.168.11.22 > /tmp/glced.log 2>&1 & \n\n" 
          "    "
          "   Change port (before starting glced):"
              "         export CED_PORT=<portnumber>\n\n\n"
          "   To connect Marlin from a remote machine set variables CED_HOST=<this_host> and CED_PORT=<this_CED_PORT> on the machine where Marlin is started from\n\n"
          "   On this machine start glced with option: -trust <host_where_Marlin_is_started_from> to accept the connection from the remote host"
          "\n\n"
          ) ;

        exit(0) ;
      } else if(!strcmp(argv[i], "-trust")){
          i++;
          if(i >= argc){
              printf("wrong syntax!\n");
              exit(0);
          }

          //printf("test: %s %s\n",argv[i], argv[i+1]);
          struct hostent *host = gethostbyname(argv[i]);
          if (host != NULL){
  	    extern char trusted_hosts[50];

              snprintf(trusted_hosts, 50, "%u.%u.%u.%u",(unsigned char)host->h_addr[0] ,(unsigned char)host->h_addr[1] ,(unsigned char)host->h_addr[2] ,(unsigned char)host->h_addr[3]);
  
             printf("Trust ip: %s\n", trusted_hosts);
          } else{
              printf("ERROR: Host %s is unknown!\n", argv[i+1]);  
          }
      } else if(!strcmp(argv[i], "-geometry")){
        geometry = true;
      }else {
          //printf("ERROR: Unknown parameter %s\n Try %s -h for help\n", argv[i], argv[0]);
          //exit(1);
      }
    }
  
    ced_register_elements();
  
    char *p;
    p = getenv ( "CED_PORT" );
    if(p != NULL){
      printf("Try to use user defined port %s.\n", p);
      glut_tcp_server(atoi(p),input_data);
    }else{
      glut_tcp_server(7286,input_data);
    }
  
 

    if(geometry == false){
        glutInitWindowSize(setting.win_w,setting.win_h);
    }

    mainWindow=glutCreateWindow("C Event Display (CED)");


  
    //glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    //glHint(GL_POLYGON_SMOOTH,GL_FASTEST); 
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //glEnable(GL_POLYGON_SMOOTH);
    glShadeModel(GL_SMOOTH);
  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);



    set_bg_color(setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[2]); //set to default (black)
    //glClearColor(BG_COLOR[0],BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);
    init();


   

    #ifndef __APPLE__
    //glutMouseWheelFunc(mouseWheel); //dont works under mac os!
    #endif
  

  
    glutMouseFunc(mouse);
    glutDisplayFunc(display);
    if(setting.fps){
        glutIdleFunc(idle); //to show fps
    }

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keypressed);
    glutSpecialFunc(SpecialKey);
    glutMotionFunc(motion);

  
    //glutTimerFunc(2000,time,23);
    //glutTimerFunc(500,timer,23);
  

    buildMenuPopup(); //hauke
    glutAttachMenu(GLUT_RIGHT_BUTTON); 

  
    for(i=0;i<NUMBER_POPUP_LAYER;i++){ //fill the layer section
      updateLayerEntryInPopupMenu(i);
    }


    for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){ //fill the layer section
      updateLayerEntryDetector(i);
    }



  

    glutTimerFunc(500,timer,1);
  

//    glDisable(GL_BLEND);




    glutMainLoop();
    return 0;
}

int save_pixmap_as_tga(unsigned char *buffer_all,char *name,int wi, int hi){
    //based on: http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=44286

    int header_size=24;
    int mem_size = wi*hi*3;
    //unsigned char tmp;
    FILE *out_file;
    unsigned char *header;

    if (!(header = (unsigned char *) calloc(1, header_size))) { return(-1); }

    //write header
    header[2] = 2;  // uncompressed
    header[12] = wi & 255;
    header[13] = wi >> 8;
    header[14] = hi & 255;
    header[15] = hi >> 8;
    header[16] = 24;    // 24 bits per pix

//    //RGB -> BGR
//    for(int j=0;j<mem_size;j+=3){
//        tmp = buffer_all[j];
//        buffer_all[j] = buffer_all[j+2];
//        buffer_all[j+2] = tmp;
//    }

    if (!(out_file = fopen(name, "wb"))) { return(-2); }

    fwrite(header, sizeof(unsigned char), header_size, out_file);
    fwrite(buffer_all, sizeof(unsigned char), mem_size, out_file);

    fclose(out_file);
    return(0);
}


int save_pixmap_as_bmp(unsigned char *buffer_all,char *name,unsigned int wi, unsigned int hi){
    unsigned int mem_size = wi*hi*3;
    FILE *out_file;
    unsigned char *header;

    unsigned int header_size=26;

    cout << endl << "               bmp screenshot width: " << wi << " hight: " << hi << endl;
    if (!(header = (unsigned char *) calloc(1, header_size))) { return(-1); }

    header[0]  = 'B';
    header[1]  = 'M';
    header[2]  = (mem_size+header_size)          & 255;
    header[3]  = ((mem_size+header_size)  >> 8)  & 255;
    header[4]  = ((mem_size+header_size)  >> 16) & 255;
    header[5]  = ((mem_size+header_size)  >> 24) & 255;
    header[6]  = 0;
    header[7]  = 0;
    header[8]  = 0;
    header[9]  = 0;
    header[10] = header_size;
    header[11] = 0;
    header[12] = 0;
    header[13] = 0;
    header[14] = 12;
    header[15] = 0;
    header[16] = 0;
    header[17] = 0;
    header[18] = (unsigned char)(wi & 255);      
    header[19] = (unsigned char)((wi >> 8) & 255);
    header[20] = (unsigned char)(hi & 255);      
    header[21] = (unsigned char)((hi >> 8) & 255);
    header[22] = 1;
    header[23] = 0;
    header[24] = 24;
    header[25] = 0;

    std::cout << "hight: " << int((header[21] << 8) + header[20]) << endl;
    std::cout << "wight: " << int((header[19] << 8)+  header[18]) << endl;





//    unsigned int header_size=54;
//    //header
//    header[0] = 'B';  
//    header[1] = 'M';  
//    header[2] = (mem_size+header_size) & 255;
//    header[3] = ((mem_size+header_size) >> 8) & 255;
//    header[4] = ((mem_size+header_size)  >> 16) & 255;
//    header[5] = ((mem_size+header_size)  >> 24) & 255;
//    header[6] = 0;
//    header[7] = 0;
//    header[8] = 0;
//    header[9] = 0;
//    header[10] = header_size & 255;
//    header[11] = (header_size >>8) & 255;
//    header[12] = (header_size >>16) & 255;
//    header[13] = (header_size >>24) & 255;
//    //info header:
//    header[14] = 40;
//    header[15] = 0;
//    header[16] = 0;
//    header[17] = 0;
//
//    header[18] = wi & 255;
//    header[19] = (wi >> 8) & 255;
//    header[20] = (wi >> 16) & 255;
//    header[21] = (wi >> 24) & 255;
//    header[22] =  hi & 255;
//
//    header[23] = (hi >> 8) & 255;
//    header[24] = (hi >> 16) & 255;
//    header[25] = (hi >> 24) & 255;
//    header[26] = 1;
//    header[27] = 0;
//    header[28] = 24;
//    header[29] = 0;
//    header[30] = 0;
//    header[34] =  ( mem_size) & 255;
//    header[35] =  ((mem_size) >> 8) & 255;
//    header[36] =  ((mem_size)  >> 16) & 255;
//    header[37] =  ((mem_size)  >> 24) & 255;
//    header[38] = 0;
//    header[39] = 0;
//    header[40] = 0;
//    header[41] = 0;
//    header[42] = 0;
//    header[43] = 0;
//    header[44] = 0;
//    header[45] = 0;
//    header[46] = 0;
//    header[47] = 0;
//    header[48] = 0;
//    header[49] = 0;
//    header[50] = 0;
//    header[51] = 0;
//    header[52] = 0;
//    header[53] = 0;

    //cout << "size" << (header_size & 255) << endl;

    if (!(out_file = fopen(name, "wb"))) { return(-2); }

    //char tmp;
//    //RGB BGR
//    for(int i=0;i<mem_size;i+=3){
//        tmp = buffer_all[i+2];
//        buffer_all[i+2] = buffer_all[i];
//        buffer_all[i] = tmp;
//    }

    fwrite(header, sizeof(unsigned char), header_size, out_file);
    fwrite(buffer_all, sizeof(unsigned char), mem_size, out_file);

    fclose(out_file);
    return(0);
}

void screenshot(char *name, int times)
{
    if(times > 100){
        std::cout << "Sorry 100x100 are the max value" << std::cout;
        return;
    }

    setting.screenshot_sections=times;


    //int HEADER_SIZE=24;
    unsigned char *buffer_all;
    unsigned char *buffer[100*100];

    //char filename[100];

    int w=glutGet(GLUT_WINDOW_WIDTH);
    int h=glutGet(GLUT_WINDOW_HEIGHT);

    window_width=w;
    window_height=h;

    int buf_size = (w*h*3);

    std::cout << "Generating screenshot (" << w*times << "x" << h*times << "):" << std::endl;
    

    //int buf_size_all = HEADER_SIZE + w*h*3 *times*times;

    int buf_size_all = w*h*3 *times*times;


    std::cout << "    Requesting memory ";
    for(int i=0;i<times*times;i++){
        std::cout << ".";
        std::cout.flush();
        if (!(buffer[i] = (unsigned char *) calloc(1, buf_size)))
        {
            return;
        }
    }
    if (!(buffer_all = (unsigned char *) calloc(1, buf_size_all)))
    {
        return;
    }
    std::cout << " Done" << std::endl;
    

    std::cout << "    Generating image ";
    if(setting.persp == true){
        glTranslatef(0.0, 0.0, +2000); //HOTFIX!!! TODO: find the place where this translation is made 
        double near_plane=200.;
        for(int i=0;i<times;i++){
            for(int j=0;j<times;j++){

                std::cout << ".";
                std::cout.flush();

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();

                glViewport(0,0,w,h);

                if(w >= h){
                glFrustum((-1*near_plane/2.      + 2*i*(near_plane/2.)/times)*w*1.0/h,
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times)*w*1.0/h,
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times), 
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times),
                          near_plane ,50000.0*mm.sf*2+50000/(mm.sf*2));
                }else{
                glFrustum((-1*near_plane/2.      + 2*i*(near_plane/2.)/times),
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times),
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times)*h*1.0/w, 
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times)*h*1.0/w,
                          near_plane*h*1./w ,50000.0*mm.sf*2+50000/(mm.sf*2));

                }

                glViewport(0,0,w,h);
                gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
                glViewport(0,0,w,h);
 
                glMatrixMode(GL_MODELVIEW);
                write_world_into_front_buffer();

                glViewport(0,0,w,h);

                glPixelStorei( GL_PACK_ALIGNMENT, 1 );
                glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer[i+j*times]);

                glMatrixMode(GL_MODELVIEW);
            }
        }
     }else{


        double near_plane=200./(WORLD_SIZE/10);

        for(int i=0;i<times;i++){
            for(int j=0;j<times;j++){

                std::cout << ".";
                std::cout.flush();

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();

                glViewport(0,0,w,h);

                if(w >= h){


                glOrtho((-1*near_plane/2.      + 2*i*(near_plane/2.)/times)*w*1.0/h*WORLD_SIZE,
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times)*w*1.0/h*WORLD_SIZE,
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times)*WORLD_SIZE, 
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times)*WORLD_SIZE,
                          near_plane ,(50000.0*mm.sf*2+50000/(mm.sf*2)));
                }else{

                //near_plane*=1./(h*1./w);
                double tmp2=WORLD_SIZE;
                WORLD_SIZE*=w*1./h;
                glOrtho((-1*near_plane/2.      + 2*i*(near_plane/2.)/times)*WORLD_SIZE,
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times)*WORLD_SIZE,
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times)*h*1.0/w*WORLD_SIZE, 
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times)*h*1.0/w*WORLD_SIZE,
                          near_plane,(50000.0*mm.sf*2+50000/(mm.sf*2)));

                WORLD_SIZE=tmp2;

                }

                glViewport(0,0,w,h);
                gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
                glViewport(0,0,w,h);
 
                glMatrixMode(GL_MODELVIEW);
                write_world_into_front_buffer();

                glViewport(0,0,w,h);

                glPixelStorei( GL_PACK_ALIGNMENT, 1 );
                glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer[i+j*times]);

                glMatrixMode(GL_MODELVIEW);
            }
        }
    }
    std::cout << " Done" << std::endl;





        //// RGB to BGR
        //for(int j=0;j<times*times;j++){
        //    for (int i=0; i<buf_size; i+=3)
        //    {
        //        tmp = buffer[j][i];
        //        buffer[j][i] = buffer[j][i + 2];
        //        buffer[j][i + 2] = tmp;
        //    }
        //}



    for(int k=0;k<times;k++){
        for(int l=0;l<h;l++){
            for(int j=0;j<times;j++){
                for(int i=0; i < w*3; i++){
                    buffer_all[i+w*3*j+l*w*3*times+k*times*w*h*3]=buffer[j+times*k][i+l*w*3];
                }
            }
        }
    }

    //RGB -> BGR
    char tmp;
    for(int j=0;j<buf_size_all;j+=3){
        tmp = buffer_all[j];
        buffer_all[j] = buffer_all[j+2];
        buffer_all[j+2] = tmp;
    }




    std::cout << "    Save screenshot as: " << name ;
    cout.flush();
    //save_pixmap_as_tga(buffer_all, name, w*times, h*times);

//    save_pixmap_as_bmp(buffer_all, "/tmp/glced.bmp", w*times, h*times);

    save_pixmap_as_tga(buffer_all, "/tmp/glced.tga", w*times, h*times);
    std::cout << " Done" << endl ;

    std::cout << "    Clean memory ";
    for(int i=0;i<times*times;i++){
        std::cout << ".";
        std::cout.flush();
        free(buffer[i]);
    }
    free(buffer_all);
    std::cout << " Done" << std::endl;

    setting.screenshot_sections=1;
}





////from: http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=44286
//void screenshot_old(char *name)
//{
//    //int HEADER_SIZE=24;
//
//    int HEADER_SIZE=24;
//    unsigned char *buffer_all;
//
//    unsigned char *buffer1;
//
//    unsigned char *buffer2;
//
//    unsigned char *buffer3;
//
//    unsigned char *buffer4;
//    char filename[50];
//    int w=window_width;
//    int h=window_height;
//    int buf_size = (w * h * 3);
//
//    int buf_size_all = HEADER_SIZE + buf_size * 4;
//    //int i;
//    unsigned char temp;
//    FILE *out_file;
//
//
//     //mm.mv.y=
//     //mm.mv.z=
//  
//  
//    float grad2rad=3.141*2/360;
//    float x_factor_x =  cos(mm.ha*grad2rad);
//    float x_factor_y =  cos((mm.va-90)*grad2rad)*cos((mm.ha+90)*grad2rad);
//    float y_factor_x =  0; 
//    float y_factor_y = -cos(mm.va*grad2rad);
//    float z_factor_x =  cos((mm.ha-90)*grad2rad);
//    float z_factor_y = -cos(mm.ha*grad2rad)*cos((mm.va+90)*grad2rad);
//  
//    //float scale_factor=580/mm.sf/exp(log(window_width*window_height)/2.5) ;
//
//    float scale_factor=0.5*580/mm.sf/exp(log(window_width*window_height)/2.5);
//    float move_x=0;
//    float move_y=0;
//
//
//
//    // allocate mem to read from frame buf
//    if (!(buffer1 = (unsigned char *) calloc(1, buf_size)))
//    {
//        return;
//    }
//    if (!(buffer2 = (unsigned char *) calloc(1, buf_size)))
//    {
//        return;
//    }
//
//
//    if (!(buffer3 = (unsigned char *) calloc(1, buf_size)))
//    {
//        return;
//    }
//    if (!(buffer4 = (unsigned char *) calloc(1, buf_size)))
//    {
//        return;
//    }
//
//    if (!(buffer_all = (unsigned char *) calloc(1, buf_size_all)))
//    {
//        return;
//    }
//
//
//    if(setting.persp== false){
//        move_x=-w;
//        move_y=-h;
//    //    x_factor_x =  cos(mm.ha*grad2rad);
//    //    x_factor_y =  cos((mm.va-90)*grad2rad)*cos((mm.ha+90)*grad2rad);
//    //    y_factor_x =  0; 
//    //    y_factor_y = -cos(mm.va*grad2rad);
//    //    z_factor_x =  cos((mm.ha-90)*grad2rad);
//    //    z_factor_y = -cos(mm.ha*grad2rad)*cos((mm.va+90)*grad2rad);
//    
//        mm.mv.x-= -1*scale_factor*(move_x)*x_factor_x - scale_factor*(move_y)*x_factor_y;
//        mm.mv.y-= -1*scale_factor*(move_x)*y_factor_x - scale_factor*(move_y)*y_factor_y;
//        mm.mv.z-= -1*scale_factor*(move_x)*z_factor_x - scale_factor*(move_y)*z_factor_y;
//      
//        glutPostRedisplay();
//        reshape(w,h);
//        display();
//        reshape(w,h);
//    
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer3);
//    
//        //mm.mv.x+=1000;
//    
//    
//    
//        move_x=+w*2;
//        move_y=0;
//    
//    //    x_factor_x =  cos(mm.ha*grad2rad);
//    //    x_factor_y =  cos((mm.va-90)*grad2rad)*cos((mm.ha+90)*grad2rad);
//    //    y_factor_x =  0; 
//    //    y_factor_y = -cos(mm.va*grad2rad);
//    //    z_factor_x =  cos((mm.ha-90)*grad2rad);
//    //    z_factor_y = -cos(mm.ha*grad2rad)*cos((mm.va+90)*grad2rad);
//    
//        mm.mv.x-=-1* scale_factor*(move_x)*x_factor_x - scale_factor*(move_y)*x_factor_y;
//        mm.mv.y-=-1* scale_factor*(move_x)*y_factor_x - scale_factor*(move_y)*y_factor_y;
//        mm.mv.z-=- 1*scale_factor*(move_x)*z_factor_x - scale_factor*(move_y)*z_factor_y;
//      
//    
//        glutPostRedisplay();
//        display();
//        reshape(w,h);
//    
//    
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer4);
//    
//    
//        move_x=0;
//        move_y=2*h;
//    //    x_factor_x =  cos(mm.ha*grad2rad);
//    //    x_factor_y =  cos((mm.va-90)*grad2rad)*cos((mm.ha+90)*grad2rad);
//    //    y_factor_x =  0; 
//    //    y_factor_y = -cos(mm.va*grad2rad);
//    //    z_factor_x =  cos((mm.ha-90)*grad2rad);
//    //    z_factor_y = -cos(mm.ha*grad2rad)*cos((mm.va+90)*grad2rad);
//    
//        mm.mv.x-=- 1*scale_factor*(move_x)*x_factor_x - scale_factor*(move_y)*x_factor_y;
//        mm.mv.y-=- 1*scale_factor*(move_x)*y_factor_x - scale_factor*(move_y)*y_factor_y;
//        mm.mv.z-=- 1*scale_factor*(move_x)*z_factor_x - scale_factor*(move_y)*z_factor_y;
//      
//    
//        glutPostRedisplay();
//    
//        display();
//        reshape(w,h);
//    
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer2);
//    
//    
//    
//        move_x=-2*w;
//        move_y=0;
//    //    x_factor_x =  cos(mm.ha*grad2rad);
//    //    x_factor_y =  cos((mm.va-90)*grad2rad)*cos((mm.ha+90)*grad2rad);
//    //    y_factor_x =  0; 
//    //    y_factor_y = -cos(mm.va*grad2rad);
//    //    z_factor_x =  cos((mm.ha-90)*grad2rad);
//    //    z_factor_y = -cos(mm.ha*grad2rad)*cos((mm.va+90)*grad2rad);
//    
//        mm.mv.x-=-1* scale_factor*(move_x)*x_factor_x - scale_factor*(move_y)*x_factor_y;
//        mm.mv.y-=-1* scale_factor*(move_x)*y_factor_x - scale_factor*(move_y)*y_factor_y;
//        mm.mv.z-=-1* scale_factor*(move_x)*z_factor_x - scale_factor*(move_y)*z_factor_y;
//      
//    
//        glutPostRedisplay();
//        display();
//        reshape(w,h);
//    
//    
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer1);
//    
//    
//    
//        move_x=+w;
//        move_y=-h;
//        mm.mv.x-=-1* scale_factor*(move_x)*x_factor_x - scale_factor*(move_y)*x_factor_y;
//        mm.mv.y-=-1* scale_factor*(move_x)*y_factor_x - scale_factor*(move_y)*y_factor_y;
//        mm.mv.z-=-1* scale_factor*(move_x)*z_factor_x - scale_factor*(move_y)*z_factor_y;
//        glutPostRedisplay();
//        display();
//        reshape(w,h);
//    
//      
//    
//    }else if(setting.persp == true){
//        //reshape(500,500);
//        int w=window_width;
//        int h=window_height;
//
//
//        std::cout << "w" << w << "h: " << h << std::endl;
//
//    //    double x=cos((90.-45./4.)*3.141*2./360.)*2000;
//    //    double y=cos((90.-45./4.)*3.141*2./360.)*2000;
//    //    double z=2000.-pow(pow(2000,2)-pow(x,2)-pow(y,2),0.5);
//    //    z=0;
//    //    std::cout << "(" << x << "," << y << "," << z << ") |x| = " << pow(pow(x,2)+pow(y,2)+pow(z,2),0.5) << endl;
//
//
//
////        glMatrixMode(GL_PROJECTION);
////        glLoadIdentity();
////        glViewport(0,0,w,h);
////        gluPerspective(45./2.,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);
////        gluLookAt  (0,0,2000,    -1.*x,y,z,    0,1,0);
////        glMatrixMode(GL_MODELVIEW);
////        write_world_into_front_buffer();
////        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer3);
////        glMatrixMode(GL_MODELVIEW);
////        glutPostRedisplay();
////        display();
////        reshape(w,h);
////
////
////
////        glMatrixMode(GL_PROJECTION);
////        glLoadIdentity();
////        glViewport(0,0,w,h);
////        gluPerspective(45./2.,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);
////        gluLookAt  (0,0,2000,    -1.*x,-1.*y,z,    0,1,0);
////        glMatrixMode(GL_MODELVIEW);
////        write_world_into_front_buffer();
////        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer1);
////        glMatrixMode(GL_MODELVIEW);
////        glutPostRedisplay();
////        display();
////        reshape(w,h);
////
////
////
////
////        glMatrixMode(GL_PROJECTION);
////        glLoadIdentity();
////        glViewport(0,0,w,h);
////        gluPerspective(45./2.,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);
////        gluLookAt  (0,0,2000,  x,y,z,    0,1,0);
////        glMatrixMode(GL_MODELVIEW);
////        write_world_into_front_buffer();
////        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer4);
////        glMatrixMode(GL_MODELVIEW);
////        glutPostRedisplay();
////        display();
////        reshape(w,h);
////
////        glMatrixMode(GL_PROJECTION);
////        glLoadIdentity();
////        glViewport(0,0,w,h);
////        gluPerspective(45./2.,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);
////        gluLookAt  (0,0,2000,    x,-1*y,z,    0,1,0);
////        glMatrixMode(GL_MODELVIEW);
////        write_world_into_front_buffer();
////        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer2);
////        glMatrixMode(GL_MODELVIEW);
////        glutPostRedisplay();
////        display();
////        reshape(w,h);
//
//
/////////////////////////
//        
//
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        glFrustum(-100,0,-100,0,200.,50000.0*mm.sf*2+50000/(mm.sf*2));
//        glViewport(0,0,w,h);
//        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
//        glMatrixMode(GL_MODELVIEW);
//        write_world_into_front_buffer();
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer1);
//        glMatrixMode(GL_MODELVIEW);
//        glutPostRedisplay();
//        display();
//        reshape(w,h);
//
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        glFrustum(0,100,-100,0,200.0,50000.0*mm.sf*2+50000/(mm.sf*2));
//        glViewport(0,0,w,h);
//        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
//        glMatrixMode(GL_MODELVIEW);
//        write_world_into_front_buffer();
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer2);
//        glMatrixMode(GL_MODELVIEW);
//        glutPostRedisplay();
//        display();
//        reshape(w,h);
//
//
//
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        glFrustum(-100,0,0,100,200.0,50000.0*mm.sf*2+50000/(mm.sf*2));
//        glViewport(0,0,w,h);
//        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
//        glMatrixMode(GL_MODELVIEW);
//        write_world_into_front_buffer();
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer3);
//        glMatrixMode(GL_MODELVIEW);
//        glutPostRedisplay();
//        display();
//        reshape(w,h);
//
//
//
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        glFrustum(0,100,0,100,200.0,50000.0*mm.sf*2+50000/(mm.sf*2));
//        glViewport(0,0,w,h);
//        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
//        glMatrixMode(GL_MODELVIEW);
//        write_world_into_front_buffer();
//        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer4);
//        glMatrixMode(GL_MODELVIEW);
//        glutPostRedisplay();
//        display();
//        reshape(w,h);
//
//
//
//     }
//
//
//    // open file for output 
//    if (!(out_file = fopen(name, "w")))
//    {
//        return;
//    }
//
//
//
//
//    
//    // set header info
//    buffer_all[2] = 2;  // uncompressed
//    buffer_all[12] = (w*2) & 255;
//    buffer_all[13] = (w*2) >> 8;
//    buffer_all[14] = (h*2) & 255;
//    buffer_all[15] = (h*2) >> 8;
//    buffer_all[16] = 24;    // 24 bits per pix
//
//
//    // RGB to BGR
//    for (int i = 0; i < buf_size; i += 3)
//    {
//        temp = buffer1[i];
//        buffer1[i] = buffer1[i + 2];
//        buffer1[i + 2] = temp;
//    }
//
//    for (int i = 0; i < buf_size; i += 3)
//    {
//        temp = buffer2[i];
//        buffer2[i] = buffer2[i + 2];
//        buffer2[i + 2] = temp;
//    }
//
//    for (int i = 0; i < buf_size; i += 3)
//    {
//        temp = buffer3[i];
//        buffer3[i] = buffer3[i + 2];
//        buffer3[i + 2] = temp;
//    }
//
//    for (int i = 0; i < buf_size; i += 3)
//    {
//        temp = buffer4[i];
//        buffer4[i] = buffer4[i + 2];
//        buffer4[i + 2] = temp;
//    }
//
//
//
//
//    for(int l=0; l < h; l++){
//        for(int c=0; c < w*3; c++){
//                buffer_all[c+2*w*l*3+HEADER_SIZE] = buffer1[c+w*l*3];
//        }
//
//        for(int c=0; c < w*3; c++){
//                buffer_all[w*3+c+2*w*l*3+HEADER_SIZE] = buffer2[c+w*l*3];
//        }
//     }
//
//    for(int l=0; l < h; l++){
//        for(int c=0; c < w*3; c++){
//                buffer_all[c+2*w*(l+h)*3+HEADER_SIZE] = buffer3[c+w*l*3];
//        }
//
//        for(int c=0; c < w*3; c++){
//                buffer_all[w*3+c+2*w*(l+h)*3+HEADER_SIZE] = buffer4[c+w*l*3];
//        }
//     }
//
//
//
//    // write header + color buf to file
//    fwrite(buffer_all, sizeof(unsigned char), buf_size_all, out_file);
//
//    // cleanup
//    fclose(out_file);
//    free(buffer_all);
//
//    free(buffer1);
//
//    free(buffer2);
//
//    free(buffer3);
//
//    free(buffer4);
//    std::cout << "screenshot saved" << std::endl;
//}
