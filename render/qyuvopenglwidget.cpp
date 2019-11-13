#include <QCoreApplication>
#include <QOpenGLTexture>

#include "qyuvopenglwidget.h"

// 存储顶点坐标和纹理坐标
// 存在一起缓存在vbo
// 使用glVertexAttribPointer指定访问方式即可
static const GLfloat coordinate[] = {
    // 顶点坐标，存储4个xyz坐标
    // 坐标范围为[-1,1],中心点为 0,0
    // 二维图像z始终为0
    // GL_TRIANGLE_STRIP的绘制方式：
    // 使用前3个坐标绘制一个三角形，使用后三个坐标绘制一个三角形，正好为一个矩形
    // x     y     z
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,

    // 纹理坐标，存储4个xy坐标
    // 坐标范围为[0,1],左下角为 0,0
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f
};

// 顶点着色器
static const QString s_vertShader = R"(
    attribute vec3 vertexIn;    // xyz顶点坐标
    attribute vec2 textureIn;   // xy纹理坐标
    varying vec2 textureOut;    // 传递给片段着色器的纹理坐标
    void main(void)
    {
        gl_Position = vec4(vertexIn, 1.0);  // 1.0表示vertexIn是一个顶点位置
        textureOut = textureIn; // 纹理坐标直接传递给片段着色器
    }
)";

// 片段着色器
static QString s_fragShader = R"(
    varying vec2 textureOut;        // 由顶点着色器传递过来的纹理坐标
    uniform sampler2D textureY;     // uniform 纹理单元，利用纹理单元可以使用多个纹理
    uniform sampler2D textureU;     // sampler2D是2D采样器
    uniform sampler2D textureV;     // 声明yuv三个纹理单元
    void main(void)
    {
        vec3 yuv;
        vec3 rgb;
        // 根据指定的纹理textureY和坐标textureOut来采样
        yuv.x = texture2D(textureY, textureOut).r;
        yuv.y = texture2D(textureU, textureOut).r - 0.5;
        yuv.z = texture2D(textureV, textureOut).r - 0.5;
        // 采样完转为rgb
        rgb = mat3(1.0, 1.0, 1.0,
                    0.0, -0.39465, 2.03211,
                    1.13983, -0.58060, 0.0) * yuv;
        // 输出颜色值
        gl_FragColor = vec4(rgb, 1.0);
    }
)";

QYUVOpenGLWidget::QYUVOpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{

}

QYUVOpenGLWidget::~QYUVOpenGLWidget()
{
    // opengl函数要在opengl上下文中执行
    makeCurrent();
    // 销毁定点缓冲对象
    m_vbo.destroy();
    // 清理纹理
    deInitTextures();
    doneCurrent();
}

QSize QYUVOpenGLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize QYUVOpenGLWidget::sizeHint() const
{
    return size();
}

void QYUVOpenGLWidget::setFrameSize(const QSize &frameSize)
{
    if (m_frameSize != frameSize) {
        m_frameSize = frameSize;
        m_needUpdate = true;
        // inittexture immediately
        repaint();
    }
}

const QSize& QYUVOpenGLWidget::frameSize()
{
    return m_frameSize;
}

void QYUVOpenGLWidget::updateTextures(quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV)
{
    if (m_textureInited) {
        // 使用yuv视频帧更新yuv纹理
        updateTexture(m_texture[0], 0, dataY, linesizeY);
        updateTexture(m_texture[1], 1, dataU, linesizeU);
        updateTexture(m_texture[2], 2, dataV, linesizeV);
        // 更新ui
        update();
    }
}

void QYUVOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    // 关闭深度测试功能GL_DEPTH_TEST
    glDisable(GL_DEPTH_TEST);

    // 顶点缓冲对象初始化

    // 创建顶点缓冲对象
    m_vbo.create();
    // 绑定顶点缓冲对象
    m_vbo.bind();
    // 顶点数组复制到顶点缓冲对象
    m_vbo.allocate(coordinate, sizeof(coordinate));

    // 初始化着色器
    initShader();

    // 设置背景清理色为黑色
    glClearColor(0.0,0.0,0.0,0.0);
    // 清理颜色背景
    glClear(GL_COLOR_BUFFER_BIT);
}

void QYUVOpenGLWidget::paintGL()
{    
    if (m_needUpdate) {
        deInitTextures();
        initTextures();
        m_needUpdate = false;
    }

    if (m_textureInited) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texture[1]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_texture[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void QYUVOpenGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    repaint();
}

void QYUVOpenGLWidget::initShader()
{
    // opengles的float、int等要手动指定精度
    if (QCoreApplication::testAttribute(Qt::AA_UseOpenGLES)) {
        s_fragShader.prepend(R"(
                             precision mediump int;
                             precision mediump float;
                             )");
    }
    // 初始化顶点着色器（创建，编译）
    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, s_vertShader);
    // 初始化片段着色器（创建，编译）
    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, s_fragShader);
    // 链接着色器
    m_shaderProgram.link();
    // 绑定着色器
    m_shaderProgram.bind();

    // 指定顶点坐标在vbo中的访问方式
    // 参数解释：顶点坐标在shader中的参数名称，顶点坐标为float，起始偏移为0，顶点坐标类型为vec3，步幅为3个float
    m_shaderProgram.setAttributeBuffer("vertexIn", GL_FLOAT, 0, 3, 3 * sizeof(float));
    // 启用顶点属性
    m_shaderProgram.enableAttributeArray("vertexIn");

    // 指定纹理坐标在vbo中的访问方式
    // 参数解释：纹理坐标在shader中的参数名称，纹理坐标为float，起始偏移为12个float（跳过前面存储的12个顶点坐标），纹理坐标类型为vec2，步幅为2个float
    m_shaderProgram.setAttributeBuffer("textureIn", GL_FLOAT, 12 * sizeof(float), 2, 2 * sizeof(float));
    m_shaderProgram.enableAttributeArray("textureIn");

    // 关联片段着色器中的纹理单元和opengl中的纹理单元（opengl一般提供16个纹理单元）
    m_shaderProgram.setUniformValue("textureY", 0);
    m_shaderProgram.setUniformValue("textureU", 1);
    m_shaderProgram.setUniformValue("textureV", 2);
}

void QYUVOpenGLWidget::initTextures()
{    
    // 创建纹理
    glGenTextures(1, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    // 设置纹理缩放时的策略
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置st方向上纹理超出坐标时的显示策略
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_frameSize.width(), m_frameSize.height(), 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_texture[1]);
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_frameSize.width()/2, m_frameSize.height()/2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_texture[2]);
    glBindTexture(GL_TEXTURE_2D, m_texture[2]);
    // 设置纹理缩放时的策略
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置st方向上纹理超出坐标时的显示策略
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_frameSize.width()/2, m_frameSize.height()/2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    m_textureInited = true;
}

void QYUVOpenGLWidget::deInitTextures()
{
    glDeleteTextures(3, m_texture);
    memset(m_texture, 0, 3);
    m_textureInited = false;
}

void QYUVOpenGLWidget::updateTexture(GLuint texture, quint32 textureType, quint8 *pixels, quint32 stride)
{
    if (!pixels)
        return;
    // Y分量m_frameSize， UV分量m_frameSize/2
    QSize size = 0 == textureType ? m_frameSize : m_frameSize/2;

    makeCurrent();
    glBindTexture(GL_TEXTURE_2D, texture);
    // 指定yuv步长为yuv响应的linesize
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), GL_RED, GL_UNSIGNED_BYTE, pixels);
    doneCurrent();
}
