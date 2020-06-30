空格键切换使用不同顶点连接方式。

```c
switch(nStep)
{
    case 0:
        glutSetWindowTitle("GL_POINTS");
        break;
    case 1:
        glutSetWindowTitle("GL_LINES");
        break;
    case 2:
        glutSetWindowTitle("GL_LINE_STRIP");
        break;
    case 3:
        glutSetWindowTitle("GL_LINE_LOOP");
        break;
    case 4:
        glutSetWindowTitle("GL_TRIANGLES");
        break;
    case 5:
        glutSetWindowTitle("GL_TRIANGLE_STRIP");
        break;
    case 6:
        glutSetWindowTitle("GL_TRIANGLE_FAN");
        break;
}
```

