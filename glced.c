/* "C" event display.
 * OpendGL (GLUT) based.
 *
 * Alexey Zhelezov, DESY/ITEP, 2005 
 *
 * July 2005, J�rgen Samson: Moved parts of the TCP/IP
 *            server to glut's timer loop to make glced 
 *            "standard glut" compliant
 *
 */

#include <GL/gl.h>
#include <GL/glu.h>

//#include <X11/Xlib.h>
//#include <GL/glx.h>

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <ced.h>
#include <ced_cli.h>

#include <errno.h>
#include <sys/select.h>



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
unsigned ced_visible_layers;


// The size of initialy visible world (+-)
#define WORLD_SIZE 6000.

static struct _geoCylinder {
  GLuint obj;
  GLfloat d;       // radius
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
  for(i=0;i<sizeof(geoCylinder)/sizeof(struct _geoCylinder);i++)
    geoCylinder[i].obj=makeCylinder(geoCylinder+i);
}


static void init(void){

  glClearColor(0.0,0.2,0.4,0.0);
  glShadeModel(GL_FLAT);

  glEnableClientState(GL_VERTEX_ARRAY);
  // GL_NORMAL_ARRAY GL_COLOR_ARRAY GL_TEXTURE_COORD_ARRAY,GL_EDGE_FLAG_ARRAY

  // to make round points
  glEnable(GL_POINT_SMOOTH);

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
  -20.,
  20.,
  1.,
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
static char x_bm[]={ 
  0xc3,0x42,0x66,0x24,0x24,0x18,
  0x18,0x24,0x24,0x66,0x42,0xc3
};
static char y_bm[]={
  0xc0,0x40,0x60,0x20,0x30,0x10,
  0x18,0x2c,0x24,0x66,0x42,0xc3
};
static char z_bm[]={ 
  0xff,0x40,0x60,0x20,0x30,0x10,
  0x08,0x0c,0x04,0x06,0x02,0xff
};
  

static void axe_arrow(void){
  GLfloat k=WORLD_SIZE/window_height;
  glutSolidCone(8.*k,30.*k,16,5);
}

static void display_world(void){
  static GLfloat axe[][3]={
    { 0., 0., 0., },
    { WORLD_SIZE/2, 0., 0. },
    { 0., WORLD_SIZE/2, 0. },
    { 0., 0., WORLD_SIZE/2 }
  };
  //  unsigned i;

  glColor3f(0.2,0.2,0.8);
  glLineWidth(2.);
  glBegin(GL_LINES);
  glVertex3fv(axe[0]);
  glVertex3fv(axe[1]);
  glVertex3fv(axe[0]);
  glVertex3fv(axe[2]);
  glVertex3fv(axe[0]);
  glVertex3fv(axe[3]);
  glEnd();

  glColor3f(0.5,0.5,0.8);
  glPushMatrix();
  glTranslatef(WORLD_SIZE/2.-WORLD_SIZE/100.,0.,0.);
  glRotatef(90.,.0,1.0,0.0);
  axe_arrow();
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.,WORLD_SIZE/2.-WORLD_SIZE/100.,0.);
  glRotatef(180.,.0,1.0,1.0);
  axe_arrow();
  glPopMatrix();


  glPushMatrix();
  glTranslatef(0.,0.,WORLD_SIZE/2.-WORLD_SIZE/100.);
  axe_arrow();
  glPopMatrix();

  // Draw X,Y,Z ...
  glColor3f(1.,1.,1.);
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
}

static void display(void){

  glClear(GL_COLOR_BUFFER_BIT);

  glPushMatrix();

  glRotatef(mm.va,1.,0.,0.);
  glRotatef(mm.ha,0.,1.0,0.);
  glScalef(mm.sf,mm.sf,mm.sf);
  glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z);

  // draw static objects
  display_world();

  // draw elements
  ced_prepare_objmap();
  ced_do_draw_event();

  glPopMatrix();

  glutSwapBuffers();
}


static void reshape(int w,int h){
  // printf("Reshaped: %dx%d\n",w,h);
  window_width=w;
  window_height=h;
  glViewport(0,0,w,h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-WORLD_SIZE*w/h,WORLD_SIZE*w/h,-WORLD_SIZE,WORLD_SIZE,
	  -15*WORLD_SIZE,15*WORLD_SIZE);
  glMatrixMode(GL_MODELVIEW);
  // glMatrixMode(GL_PROJECTION);
  glLoadIdentity(); 
}

static void mouse(int btn,int state,int x,int y){

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
    move_mode=TURN_XY;
    return;
  case GLUT_RIGHT_BUTTON:
    move_mode=ZOOM;
    return;
  case GLUT_MIDDLE_BUTTON:
    move_mode=ORIGIN;
    return;
  default:
    break;
  }
}

static void toggle_layer(unsigned l){
  ced_visible_layers^=(1<<l);
}

static void show_all_layers(void){
  ced_visible_layers=0xffffffff;
}

static void keypressed(unsigned char key,int x,int y){
  printf("Key at %dx%d: %u('%c')\n",x,y,key,key);
  if(key=='r' || key=='R'){
      mm=mm_reset;
      glutPostRedisplay();
  } else if(key=='f' || key=='F'){
      mm=mm_reset;
      mm.ha=mm.va=0.;
      glutPostRedisplay();
  } else if(key=='s' || key=='S'){
      mm=mm_reset;
      mm.ha=90.;
      mm.va=0.;
      glutPostRedisplay();
  } else if(key==27)
      exit(0);
  else if(key=='c' || key=='C'){
    if(!ced_get_selected(x,y,&mm.mv.x,&mm.mv.y,&mm.mv.z))
      glutPostRedisplay();
  } else if((key>='0') && (key<='9')){
    toggle_layer(key-'0');
    glutPostRedisplay();
  } else if(key==')'){ // 0
    toggle_layer(10);
    glutPostRedisplay();
  } else if(key=='!'){ // 1
    toggle_layer(11);
    glutPostRedisplay();
  } else if(key=='@'){ // 2
    toggle_layer(12);
    glutPostRedisplay();
  } else if(key=='#'){ // 3
    toggle_layer(13);
    glutPostRedisplay();
  } else if(key=='$'){ // 4
    toggle_layer(14);
    glutPostRedisplay();
  } else if(key=='%'){ // 5
    toggle_layer(15);
    glutPostRedisplay();
  } else if(key=='^'){ // 6
    toggle_layer(16);
    glutPostRedisplay();
  } else if(key=='&'){ // 7
    toggle_layer(17);
    glutPostRedisplay();
  } else if(key=='*'){ // 8
    toggle_layer(18);
    glutPostRedisplay();
  } else if(key=='('){ // 9
    toggle_layer(19);
    glutPostRedisplay();
  } else if(key=='`'){
    show_all_layers();
    glutPostRedisplay();
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


//static void ppoint(Point *p){
//  printf("= %f %f %f\n",p->x,p->y,p->z);
//}

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
  } else if (move_mode == ZOOM){
      mm.sf=mm.sf_start+(y-mouse_y)*10./window_height;
      if(mm.sf<0.2)
	  mm.sf=0.2;
      else if(mm.sf>20.)
	  mm.sf=20.;
  } else if (move_mode == ORIGIN){
      mm.mv.x=mm.mv_start.x-(x-mouse_x)*WORLD_SIZE/window_width;
      mm.mv.y=mm.mv_start.y+(y-mouse_y)*WORLD_SIZE/window_height;
  }   
  glutPostRedisplay();
}





static void timer (int val)
{
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
  /* Fixme? Is this the correct way for a non blocking select call? */
  rc = select(max_fd + 1, &fds, NULL, NULL, &timeout);
  if (rc < 0) 
    {
      if (errno == EINTR) 
	{
	  glutTimerFunc(500,timer,01);
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
	      glutTimerFunc(100,timer,01);
	      return ; /* to avoid complexity with removed sockets */
	    }
	}
    }


  glutTimerFunc(500,timer,01);
  return;

}



int glut_tcp_server(unsigned short port,
		    void (*user_func)(void *data));

static void input_data(void *data){
  if(ced_process_input(data)>0)
    glutPostRedisplay();
}


int main(int argc,char *argv[]){

  ced_register_elements();

  glut_tcp_server(7286,input_data);

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
  glutInitWindowSize(900,900);
  glutInitWindowPosition(500,0);
  glutCreateWindow("C Event Display (CED)");

  init();
  mm_reset=mm;

  glutMouseFunc(mouse);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keypressed);
  glutSpecialFunc(SpecialKey);
  glutMotionFunc(motion);

  //    glutTimerFunc(2000,time,23);
  glutTimerFunc(500,timer,23);

  glutMainLoop();

  return 0;
}
