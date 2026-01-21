#pragma once


#include <QOpenGLWidget>
#include <QTimer>
#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osgGA/TrackballManipulator>
#include <osg/Group>

class OsgQtWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit OsgQtWidget(QWidget* parent = nullptr);

    osg::Group* root() const { return _root.get(); }
    osgViewer::Viewer* viewer() const { return _viewer.get(); }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    //  ‰»Î ¬º˛
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    osg::ref_ptr<osgViewer::Viewer> _viewer;
    osg::ref_ptr<osg::Group> _root;
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;
    QTimer _timer;

    int qtButtonToOsg(Qt::MouseButton b) const;
};