#include "Angel.h"

#include <string>

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
const float TIMER_UPDATE_VALUE = 10.0;
const float ANGLE_INCREMENT_VALUE = 1;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points[NumVertices];
color4 colors[NumVertices];
const int QuadSphereVertices = 342; // 8 rows of 18 quads
point4 QuadptsSphere[QuadSphereVertices];
color4 QuadptsSpherecolor[QuadSphereVertices];
const int FanSphereVertices = 40;
point4 sphereptsFan[FanSphereVertices];
color4 spherefancolors[FanSphereVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

// Shader transformation matrices
mat4  model_view;
GLuint program, vao, spherefanvao, spherequadvao, vPosition, vColor, ModelView, Projection;

// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };
enum ballAnimation{ OldBallPosition, AttachedtoArm, Newballposition, BallatEnd};
ballAnimation currentBallanimation;
point4 oldpos, newpos;
GLfloat aspect;
vec3 finalRotation; 
enum  CamerAngle {side, top};
CamerAngle camerAngle = CamerAngle::side;

// Menu option values
const int  Quit = 4;
const int CameraAngle = 5;



//----------------------------------------------------------------------------

int Index = 0;

void quad( int a, int b, int c, int d )
{
    colors[Index] = vertex_colors[a];points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a];points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a];points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a];points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a];points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a];points[Index] = vertices[d]; Index++;
}

void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}


void computesphere(){
    const float DegreesToRadians = M_PI / 180.0; // M_PI = 3.14159...
    int i = 0;

    for(float phi = -80.0; phi <= 80.0; phi += 20.0)
    {
        float phiradians = phi*DegreesToRadians;
        float phiradians20 = (phi + 20.0)*DegreesToRadians;
        for(float theta = -180.0; theta <= 180.0; theta += 20.0)
        {
            float thetaradians = theta*DegreesToRadians;
            QuadptsSphere[i] = vec3(sin(thetaradians)*cos(phiradians),
            cos(thetaradians)*cos(phiradians), sin(phiradians));
            i++;
            QuadptsSphere[i] = vec3(sin(thetaradians)*cos(phiradians20),
            cos(thetaradians)*cos(phiradians20), sin(phiradians20));
            i++;
        }
    }
    i = 0;
    sphereptsFan[i] = vec3(0.0, 0.0, 1.0);
    i++;

    float sintimes80 = sin(80.0*DegreesToRadians);
    float costimes80 = cos(80.0*DegreesToRadians);

    for(float theta = -180.0; theta <= 180.0; theta += 20.0)
    {
        float thetaradians = theta*DegreesToRadians;
        sphereptsFan[i] = vec3(sin(thetaradians)*costimes80,
        cos(thetaradians)*costimes80, sintimes80);
        i++;
    }
    sphereptsFan[i] = vec3(0.0, 0.0, -1.0);
    i++;
    for(float theta = -180.0; theta <= 180.0; theta += 20.0)
    {
        float thetaradians = theta*DegreesToRadians;
        sphereptsFan[i] = vec3(sin(thetaradians)*costimes80,
        cos(thetaradians)*costimes80, -sintimes80);
        i++;
    }
    for(auto& x: spherefancolors) {
        x = vertex_colors[1];
    }
    for(auto& x: QuadptsSpherecolor) {
        x = vertex_colors[1];
    }
}

vec3 robotRotation(point4 p){
    const float RadiansToDegrees = 180.0 / M_PI;
    vec3 result;

    vec2 bi(-1, 0);
    vec2 bf(p.x, p.z);
    float bf_mag = bf.x*bf.x + bf.y*bf.y;
    if(bf_mag != 0.0){
        float angle = acos(dot(bi, bf)/(sqrt(bi.x*bi.x + bi.y*bi.y)*sqrt(bf_mag)));
        float cross = bf.x * bi.y - bf.y * bi.x;
        if(cross<0.0){
            angle = -angle;
        }
        result.x = angle * RadiansToDegrees;
    }
    else result.x = 0;
    float aside = LOWER_ARM_HEIGHT;
    float bside = UPPER_ARM_HEIGHT + UPPER_ARM_WIDTH/2;
    float diffY = p.y - BASE_HEIGHT;
    float cside = sqrt(bf_mag + diffY*diffY);
    float oppositeC = acos((aside*aside + bside*bside - cside*cside)/(2*aside*bside)) * RadiansToDegrees;
    float oppositeB = acos((aside*aside + cside*cside - bside*bside)/(2*aside*cside)) * RadiansToDegrees;
    float bottomAngle = asin(diffY/cside) * RadiansToDegrees;

    result.y = 90 - (bottomAngle + oppositeB);
    result.z = 180 - oppositeC;
    return result;
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void base()
{
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
         Scale( BASE_WIDTH,
            BASE_HEIGHT,
            BASE_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glBindVertexArray( vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void upper_arm()
{
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
              Scale( UPPER_ARM_WIDTH,
                 UPPER_ARM_HEIGHT,
                 UPPER_ARM_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glBindVertexArray( vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

}

//----------------------------------------------------------------------------

void lower_arm()
{
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
              Scale( LOWER_ARM_WIDTH,
                 LOWER_ARM_HEIGHT,
                 LOWER_ARM_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glBindVertexArray( vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

}

//----------------------------------------------------------------------------

void draw_sphere(mat4 reqmat = Translate( 0.0, 0.5 * UPPER_ARM_WIDTH, 0.0 ))
{
    mat4 instance = ( reqmat *
              Scale( UPPER_ARM_WIDTH *0.5,
                 UPPER_ARM_WIDTH *0.5,
                 UPPER_ARM_WIDTH *0.5 ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glBindVertexArray( spherequadvao );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, QuadSphereVertices );
    glBindVertexArray( spherefanvao );
    glDrawArrays( GL_TRIANGLE_FAN, 0, FanSphereVertices/2 );
    glDrawArrays( GL_TRIANGLE_FAN, FanSphereVertices/2, FanSphereVertices/2 );
}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if(camerAngle == CamerAngle::top){
        model_view = RotateX(90);
    }
    else{
        model_view = mat4(1.0);
    }
    if(currentBallanimation == OldBallPosition){
        draw_sphere(Translate(oldpos));
    }
    else if(currentBallanimation == Newballposition || currentBallanimation == BallatEnd){
        draw_sphere(Translate(newpos));
    }

    // Accumulate ModelView Matrix as we traverse the tree
    model_view *= RotateY(Theta[Base] );
    base();

    model_view *= ( Translate(0.0, BASE_HEIGHT, 0.0) *
            RotateZ(Theta[LowerArm]) );
    lower_arm();

    model_view *= ( Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
            RotateZ(Theta[UpperArm]) );
    upper_arm();

    if(currentBallanimation == AttachedtoArm){
        model_view *= Translate(0.0, UPPER_ARM_HEIGHT, 0.0);
        draw_sphere();
    }

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void init_cube()
{
    colorcube();
    
    // Create a vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(points)) );
}


void init_sphere_quad()
{    
    // Create a vertex array object
    glGenVertexArrays( 1, &spherequadvao );
    glBindVertexArray( spherequadvao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    glBufferData( GL_ARRAY_BUFFER, 
        sizeof(QuadptsSphere) + sizeof(QuadptsSpherecolor),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(QuadptsSphere), QuadptsSphere );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(QuadptsSphere), sizeof(QuadptsSpherecolor), QuadptsSpherecolor );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(QuadptsSphere)) );
}

void init_sphere_fan()
{
    // Create a vertex array object
    glGenVertexArrays( 1, &spherefanvao );
    glBindVertexArray( spherefanvao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    glBufferData( GL_ARRAY_BUFFER, 
        sizeof(sphereptsFan) + sizeof(spherefancolors),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(sphereptsFan), sphereptsFan );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(sphereptsFan), sizeof(spherefancolors), spherefancolors );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(sphereptsFan)) );
}

void init(){
    init_cube();
    computesphere();
    init_sphere_quad();
    init_sphere_fan();
    glEnable( GL_DEPTH_TEST );
    glDepthFunc(GL_LESS);
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
}

//----------------------------------------------------------------------------

// void mouse( int button, int state, int x, int y )
// {

//     if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
//      // Incrase the joint angle
//      Theta[Axis] += 5.0;
//      if ( Theta[Axis] > 360.0 ) { Theta[Axis] -= 360.0; }
//     }

//     if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {
//      // Decrase the joint angle
//      Theta[Axis] -= 5.0;
//      if ( Theta[Axis] < 0.0 ) { Theta[Axis] += 360.0; }
//     }
//     glutPostRedisplay();
//     for(auto x: Theta) std::cout<<x<<'\t'; std::cout<<std::endl;
// }

//----------------------------------------------------------------------------

void setProjectionMatrix(){

    GLfloat  left = -10.0, right = 10.0;
    GLfloat  bottom = -5, top = 15.0;
    GLfloat  zNear = -10.0, zFar = 10.0;
    if ( camerAngle == CamerAngle::top ) {
        bottom = -10.0;
        top    =  10.0;
    }
    if ( aspect > 1.0 ) {
        left *= aspect;
        right *= aspect;
    }
    else {
        bottom /= aspect;
        top /= aspect;
    }
    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );

    model_view = mat4( 1.0 );  // An Identity matrix
}

void menu( int option )
{
    if ( option == Quit ) {
        exit( EXIT_SUCCESS );
    }
    else if(option == CameraAngle){
        if(camerAngle == CamerAngle::side){
            camerAngle = CamerAngle::top;
            setProjectionMatrix();
            glutPostRedisplay();
        }
        else{
            camerAngle = CamerAngle::side;
        }
    }
    else {
        printf("%i\n",option);
    Axis = option;
    }
}

//----------------------------------------------------------------------------

void reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspect = GLfloat(width)/height;
    setProjectionMatrix();
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
    case 033: // Escape Key
    case 'q': case 'Q':
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------

bool checkAxis(int axis, float val){
    float oldaxis = Theta[axis];
    float difference = val - oldaxis;

    if(abs(difference) < ANGLE_INCREMENT_VALUE){
        Theta[axis] = val;
        return true;
    }
    if(difference > 0.0){
        Theta[axis] += ANGLE_INCREMENT_VALUE;
    }
    else{
        Theta[axis] -= ANGLE_INCREMENT_VALUE;
    }
    return false;
}

void update(int){
    int result = checkAxis(Base, finalRotation.x) & checkAxis(LowerArm, finalRotation.y) & checkAxis(UpperArm, finalRotation.z);
    glutPostRedisplay();

    if (result!=0){
        if(currentBallanimation == OldBallPosition){
            finalRotation = robotRotation(newpos);
            currentBallanimation = AttachedtoArm;
        }else if(currentBallanimation == AttachedtoArm){
            finalRotation = vec3(0,0,0);
            currentBallanimation = Newballposition;
        }else if(currentBallanimation == Newballposition){
            currentBallanimation = BallatEnd;
        }
    }
    glutTimerFunc(TIMER_UPDATE_VALUE, update, 0);
}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    std::string CameraView = argv[7];
    if ( argc < 8 ){
        printf("Missing arguements\n");
        return -1;   
    }
    else if(argc > 8){
        printf("Too many arguement, Please check again");
        return -1;
    } 
    else {
        oldpos = point4(atof(argv[1]), atof(argv[2]), atof(argv[3]), 1);
        newpos = point4(atof(argv[4]), atof(argv[5]), atof(argv[6]), 1);

        if (CameraView == "-tv"){
            camerAngle = CamerAngle::top;
        }
    }

    currentBallanimation = OldBallPosition;
    finalRotation = robotRotation(oldpos);

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutInitContextVersion( 3, 2 );
    glutInitContextProfile( GLUT_CORE_PROFILE );
    glutCreateWindow( "myrobot" );

    // Iff you get a segmentation error at line 34, please uncomment the line below
    glewExperimental = GL_TRUE; 
    glewInit();

    // Load shaders and use the resulting shader program
    program = InitShader( "vshader81.glsl", "fshader81.glsl" );
    glUseProgram( program );
    vPosition = glGetAttribLocation( program, "vPosition" );
    vColor = glGetAttribLocation( program, "vColor" );
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    init();

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    // glutMouseFunc( mouse );

    glutCreateMenu( menu );
    // Set the menu values to the relevant rotation axis values (or Quit)

    glutAddMenuEntry( "Change View", CameraAngle );
    glutAddMenuEntry( "base", Base );
    glutAddMenuEntry( "lower arm", LowerArm );
    glutAddMenuEntry( "upper arm", UpperArm );
    glutAddMenuEntry( "quit", Quit );
    glutAttachMenu( GLUT_MIDDLE_BUTTON );

    glutTimerFunc(TIMER_UPDATE_VALUE, update, 0);

    glutMainLoop();
    return 0;
}
