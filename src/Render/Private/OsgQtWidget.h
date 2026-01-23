#pragma once

#include "NameDefine.h"

#include <QOpenGLWidget>
#include <QTimer>
#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osgGA/TrackballManipulator>
#include <osg/Group>

namespace cadutils
{
    using UiPickCallback = std::function<void(ObjectId)>;

    class OsgQtWidget : public QOpenGLWidget
    {
        Q_OBJECT
    public:
        explicit OsgQtWidget(QWidget* parent = nullptr);
        void SetUiPickCallback(UiPickCallback callFun);

        osg::Group* root() const { return m_root.get(); }
        osgViewer::Viewer* viewer() const { return m_viewer.get(); }

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
        ObjectId Pick(int x, int y);
    private:
        osg::ref_ptr<osgViewer::Viewer> m_viewer;
        osg::ref_ptr<osg::Group> m_root;
        osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_gw;
        QTimer m_timer;
        UiPickCallback m_callBack;
        int qtButtonToOsg(Qt::MouseButton b) const;
    };
}