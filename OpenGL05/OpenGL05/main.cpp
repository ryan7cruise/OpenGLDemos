#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager		shaderManager;			// 着色器管理器
GLMatrixStack		modelViewMatrix;		// 模型视图矩阵
GLMatrixStack		projectionMatrix;		// 投影矩阵
GLFrustum			viewFrustum;			// 视景体
GLGeometryTransform	transformPipeline;		// 几何图形变换管道

GLTriangleBatch		lSphereBatch;           //大球
GLTriangleBatch     sSphereBatch;           //小球
GLBatch             floorBatch;             //地板

//角色帧 照相机角色帧
GLFrame             cameraFrame;

//随机位置的球
#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

void SetupRC()
{
    //设置clear color
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //初始化固定管线
    shaderManager.InitializeStockShaders();
    //设置管线的模型视图矩阵和投影矩阵引用
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    //开启深度测试
    glEnable(GL_DEPTH_TEST);
    //按一个方块一个方块的添加地板顶点
    floorBatch.Begin(GL_LINES, 324);
    for (GLfloat idx = -20.0; idx <= 20.0; idx+=0.5) {
        floorBatch.Vertex3f(idx, -0.55, 20.0);
        floorBatch.Vertex3f(idx, -0.55, -20.0);
        floorBatch.Vertex3f(20, -0.55, idx);
        floorBatch.Vertex3f(-20, -0.55, idx);
    }
    floorBatch.End();
    //初始化大球顶点数据
    gltMakeSphere(lSphereBatch, 0.4f, 40, 80);
    //初始化小球顶点数据
    gltMakeSphere(sSphereBatch, 0.1f, 26, 13);
    //初始化小球随机位置
    for (int idx = 0; idx < NUM_SPHERES; idx++) {
        GLfloat x = ((GLfloat)((rand() % 400) - 200 ) * 0.1f);
        GLfloat z = ((GLfloat)((rand() % 400) - 200 ) * 0.1f);
        spheres[idx].SetOrigin(x, 0.0, z);
    }
}

//进行调用以绘制场景
void RenderScene(void)
{
    //地板、大球、小球颜色
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
    static GLfloat vLargeSphereColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    static GLfloat vSmallSphereColor[] = { 0.0f, 0.0f, 1.0f, 1.0f};
    //点光源位置
    M3DVector4f vLightPos = {0.0f, 10.0f, 5.0f, 1.0f};
    
    //利用一个计时器获取时间的增量
    static CStopWatch rotTimer;
    GLfloat yRot = rotTimer.GetElapsedSeconds() * 60.0;
    
    //清空颜色和深度缓冲
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //入栈相机矩阵
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.PushMatrix(mCamera);
    
    //绘制地板
    shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 vFloorColor);
    floorBatch.Draw();
    
    //模型视图矩阵向z轴负方向移动3.0
    modelViewMatrix.Translate(0.0, 0.0, -3.0);
    
    //绘制小球
    for (int idx = 0; idx < NUM_SPHERES; idx++) {
        //模型视图矩阵入栈
        modelViewMatrix.PushMatrix();
        //移动小球到设定的位置
        modelViewMatrix.MultMatrix(spheres[idx]);
        //绘制小球
        shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
                                     transformPipeline.GetModelViewMatrix(),
                                     transformPipeline.GetProjectionMatrix(),
                                     vLightPos,
                                     vSmallSphereColor);
        sSphereBatch.Draw();
        //模型视图矩阵出栈
        modelViewMatrix.PopMatrix();
    }
    
    //模型视图矩阵入栈
    modelViewMatrix.PushMatrix();
    //模型视图矩阵入阵绕y旋转yRot度，完成自转
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    //绘制大球
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
                                 transformPipeline.GetModelViewMatrix(),
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightPos,
                                 vLargeSphereColor);
    lSphereBatch.Draw();
    //模型视图矩阵出栈
    modelViewMatrix.PopMatrix();
    
    //模型视图矩阵入阵绕y旋转yRot * -1.5度
    modelViewMatrix.Rotate(yRot * -1.5, 0.0, 1.0, 0.0);
    //模型视图矩阵入阵向x轴正方向移动0.8
    modelViewMatrix.Translate(0.8, 0.0, 0.0);
    //用GLT_SHADER_FLAT绘制会像一个圆圈没有立体感
//    shaderManager.UseStockShader(GLT_SHADER_FLAT,
//                                 transformPipeline.GetModelViewProjectionMatrix(),
//                                 vSphereColor);
    //还是利用点光源绘制公转的小球
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
                                 transformPipeline.GetModelViewMatrix(),
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightPos,
                                 vSmallSphereColor);
    sSphereBatch.Draw();
    //模型视图矩阵出栈
    modelViewMatrix.PopMatrix();
    
    //交换缓冲帧
    glutSwapBuffers();
    //绘制下一帧
    glutPostRedisplay();
}

//屏幕更改大小或已初始化
void ChangeSize(int nWidth, int nHeight)
{
    //设置视口
    glViewport(0, 0, nWidth, nHeight);
    //辅助计算投影矩阵
    viewFrustum.SetPerspective(35.0, float(nWidth) / float(nHeight), 1.0, 100.0);
    //将计算出的投影矩阵，加载到投影矩阵堆栈
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    //模型矩阵还原为单位阵
    modelViewMatrix.LoadIdentity();
}

//上下左右按键处理
void SpeacialKeys(int key,int x,int y){
    //前后移动步长
    float linear = 0.1;
    //左右旋转角度
    float angular = float(m3dDegToRad(3.0));
    switch (key) {
        case GLUT_KEY_UP:
            //相机前进
            cameraFrame.MoveForward(linear);
            break;
        case GLUT_KEY_DOWN:
            //相机后退
            cameraFrame.MoveForward(-linear);
            break;
        case GLUT_KEY_LEFT:
            //相机绕z轴旋转angular度
            cameraFrame.RotateWorld(angular, 0.0, 1.0, 0.0);
            break;
        case GLUT_KEY_RIGHT:
            //相机绕z轴旋转-angular度
            cameraFrame.RotateWorld(-angular, 0.0, 1.0, 0.0);
            break;
        default:
            break;
    }
}

int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    //初始化配置
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    //初始化窗口
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL SphereWorld");
    //设置回调函数
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpeacialKeys);
    glutDisplayFunc(RenderScene);
    //设置GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    //初始化场景信息
    SetupRC();
    //开启loop
    glutMainLoop();
    return 0;
}
