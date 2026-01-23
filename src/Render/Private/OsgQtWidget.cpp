#include "OsgQtWidget.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>

using namespace cadutils;

OsgQtWidget::OsgQtWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    m_root = new osg::Group();
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    connect(&m_timer, &QTimer::timeout, this, [this]() { update(); });
    m_timer.start(16); // 60fps-ish
}

void cadutils::OsgQtWidget::SetUiPickCallback(UiPickCallback callFun)
{
    m_callBack = callFun;
}

void OsgQtWidget::initializeGL()
{
    m_viewer = new osgViewer::Viewer();
    m_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    m_viewer->setSceneData(m_root.get());
    m_viewer->setCameraManipulator(new osgGA::TrackballManipulator());

    m_gw = new osgViewer::GraphicsWindowEmbedded(0, 0, width(), height());
    osg::Camera* cam = m_viewer->getCamera();
    cam->setGraphicsContext(m_gw.get());
    cam->setViewport(new osg::Viewport(0, 0, width(), height()));
    cam->setProjectionMatrixAsPerspective(45.0, double(width()) / double(height()), 0.1, 10000.0);
    cam->setClearColor(osg::Vec4(1.f, 1.f, 1.f, 1.f));
    // 常用：开启光照（你也可以后面做材质/StateSet统一管理）
    cam->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
}

void OsgQtWidget::resizeGL(int w, int h)
{
    if (m_gw.valid())
        m_gw->resized(0, 0, w, h);

    if (m_viewer.valid())
        m_viewer->getCamera()->setViewport(0, 0, w, h);
}

void OsgQtWidget::paintGL()
{
    if (m_viewer.valid())
        m_viewer->frame();
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
    if (!m_gw) return;
    m_gw->getEventQueue()->mouseButtonPress(e->pos().x(), e->pos().y(), qtButtonToOsg(e->button()));
    ObjectId pickId = Pick(e->pos().x(), e->pos().y());
    if (m_callBack)
        m_callBack(pickId);
}
void OsgQtWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_gw) return;
    m_gw->getEventQueue()->mouseButtonRelease(e->pos().x(), e->pos().y(), qtButtonToOsg(e->button()));
}
void OsgQtWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_gw) return;
    m_gw->getEventQueue()->mouseMotion(e->pos().x(), e->pos().y());
}
void OsgQtWidget::wheelEvent(QWheelEvent* e)
{
    if (!m_gw) return;
    const int delta = e->angleDelta().y();
    m_gw->getEventQueue()->mouseScroll(delta > 0 ? osgGA::GUIEventAdapter::SCROLL_UP
        : osgGA::GUIEventAdapter::SCROLL_DOWN);
}

ObjectId OsgQtWidget::Pick(int x, int y)
{
    if (!m_viewer) 
        return ObjectId(-1);

    osg::Camera* cam = m_viewer->getCamera();
    osg::ref_ptr<osgUtil::LineSegmentIntersector> inter =
        new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);

    osgUtil::IntersectionVisitor iv(inter.get());
    cam->accept(iv);

    if (!inter->containsIntersections())
        return ObjectId(-1);

    const auto& hit = *inter->getIntersections().begin();

    // 从 nodePath 从下往上找绑的 ObjectId
    for (auto it = hit.nodePath.rbegin(); it != hit.nodePath.rend(); ++it)
    {
        osg::Node* n = *it;
        unsigned int id = 0;
        if (n->getUserValue("ObjectId", id))
            return static_cast<ObjectId>(id);
    }
    return ObjectId(-1);
}
