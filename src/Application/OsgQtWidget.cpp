#include "OsgQtWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>

OsgQtWidget::OsgQtWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    _root = new osg::Group();
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    connect(&_timer, &QTimer::timeout, this, [this]() { update(); });
    _timer.start(16); // 60fps-ish
}

void OsgQtWidget::initializeGL()
{
    _viewer = new osgViewer::Viewer();
    _viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    _viewer->setSceneData(_root.get());
    _viewer->setCameraManipulator(new osgGA::TrackballManipulator());

    _gw = new osgViewer::GraphicsWindowEmbedded(0, 0, width(), height());
    osg::Camera* cam = _viewer->getCamera();
    cam->setGraphicsContext(_gw.get());
    cam->setViewport(new osg::Viewport(0, 0, width(), height()));
    cam->setProjectionMatrixAsPerspective(45.0, double(width()) / double(height()), 0.1, 10000.0);

    // 常用：开启光照（你也可以后面做材质/StateSet统一管理）
    cam->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
}

void OsgQtWidget::resizeGL(int w, int h)
{
    if (_gw.valid())
        _gw->resized(0, 0, w, h);

    if (_viewer.valid())
        _viewer->getCamera()->setViewport(0, 0, w, h);
}

void OsgQtWidget::paintGL()
{
    if (_viewer.valid())
        _viewer->frame();
}

int OsgQtWidget::qtButtonToOsg(Qt::MouseButton b) const
{
    // OSG: 1=Left,2=Middle,3=Right
    if (b == Qt::LeftButton) return 1;
    if (b == Qt::MiddleButton) return 2;
    if (b == Qt::RightButton) return 3;
    return 0;
}

void OsgQtWidget::mousePressEvent(QMouseEvent* e)
{
    if (!_gw) return;
    _gw->getEventQueue()->mouseButtonPress(e->pos().x(), e->pos().y(), qtButtonToOsg(e->button()));
}
void OsgQtWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (!_gw) return;
    _gw->getEventQueue()->mouseButtonRelease(e->pos().x(), e->pos().y(), qtButtonToOsg(e->button()));
}
void OsgQtWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!_gw) return;
    _gw->getEventQueue()->mouseMotion(e->pos().x(), e->pos().y());
}
void OsgQtWidget::wheelEvent(QWheelEvent* e)
{
    if (!_gw) return;
    const int delta = e->angleDelta().y();
    _gw->getEventQueue()->mouseScroll(delta > 0 ? osgGA::GUIEventAdapter::SCROLL_UP
        : osgGA::GUIEventAdapter::SCROLL_DOWN);
}
