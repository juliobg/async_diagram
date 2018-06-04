#ifndef STATESWIDGET_H
#define STATESWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <Qt>
#include <map>
#include <vector>
#include <memory>
#include <QColor>
#include <QPushButton>
#include <QScrollBar>
#include <QGridLayout>
#include <QMouseEvent>
#include <QTimer>
#include <iostream>
#include "cevent.h"

class Action
{
    enum Type
    {
        Zoom
    };
};

class ZoomAction
{

};

class CGraphElement;
typedef std::shared_ptr<CGraphElement> TPGraphElement;

class CColorProvider
{
public:
    QColor GetColor(int aIndex)
    {
        return QColor(IndexColors[aIndex].c_str());
    }
    QColor GetColorBW(int aIndex)
    {
        return QColor(IndexColorsBW[aIndex%2].c_str());
    }

private:
    static std::string IndexColors[];
    static std::string IndexColorsBW[];
};


class CGraphElement
{
public:
    CGraphElement(const std::string& aName, const QColor& aColor)
        : iName(aName), iColor(aColor) {}
    virtual QRectF GetBox() = 0;
    virtual QRectF GetDataBox()
    {
        return GetBox();
    }

    virtual void AddElement(TPGraphElement aElement)
    {
        iSubElements.push_back(aElement);
    }
    virtual void AddEvent(const CEvent &aEvent) {}

    std::string GetName()
    {
        return iName;
    }
    bool Intersects(const QRectF& aRect)
    {
        return GetBox().intersects(aRect);
    }

    //TODO. Optimize!
    virtual std::vector<TPGraphElement> GetVisibleElements(const QRectF& aWorldWindow)
    {
        std::vector<TPGraphElement> r;
        for (const auto& element : iSubElements)
            if (element->Intersects(aWorldWindow))
                r.push_back(element);

        return r;

    }
    virtual void Draw (QPainter &aPainter, const QRectF& aWorldWindow, const QRect& aViewPort)
    {
        for (auto& element : GetVisibleElements(aWorldWindow))
            element->Draw(aPainter, aWorldWindow, aViewPort);
    }
    QPoint WorldWindowPointToViewPort(const QPointF& aPoint, const QRectF& aWorldWindow, const QRect& aViewPort) const
    {
        int x;

        if (aPoint.x()==-std::numeric_limits<double>::infinity())
            x=aViewPort.topLeft().x();
        else if (aPoint.x()==std::numeric_limits<double>::infinity() || aPoint.x()!=aPoint.x())
            x=aViewPort.bottomRight().x();
        else
            x= (aViewPort.bottomRight().x()-aViewPort.topLeft().x())/(aWorldWindow.bottomRight().x()-aWorldWindow.topLeft().x())*(aPoint.x()-aWorldWindow.topLeft().x());


        return QPoint(x, (aViewPort.bottomRight().y()-aViewPort.topLeft().y())/(aWorldWindow.bottomRight().y()-aWorldWindow.topLeft().y())*(aPoint.y()-aWorldWindow.topLeft().y()));
    }

    QRect WorldWindowRectToViewPort(const QRectF& aRect, const QRectF& aWorldWindow, const QRect& aViewPort) const
    {
        return QRect(WorldWindowPointToViewPort(aRect.topLeft(), aWorldWindow, aViewPort),
                     WorldWindowPointToViewPort(aRect.bottomRight(), aWorldWindow, aViewPort));
    }

    QRect WorldWindowRectToViewPort(const std::pair<QPointF, QPointF>& aRect, const QRectF& aWorldWindow, const QRect& aViewPort) const
    {
        return QRect(WorldWindowPointToViewPort(aRect.first, aWorldWindow, aViewPort),
                     WorldWindowPointToViewPort(aRect.second, aWorldWindow, aViewPort));
    }

    std::pair<QPoint, QPoint> WorldWindowPairToViewPort(const std::pair<QPointF, QPointF>& aRect, const QRectF& aWorldWindow, const QRect& aViewPort) const
    {
        return std::make_pair(WorldWindowPointToViewPort(aRect.first, aWorldWindow, aViewPort),
                              WorldWindowPointToViewPort(aRect.second, aWorldWindow, aViewPort));
    }

    virtual QColor AssignColor(const std::string aName)
    {
        if (iColorMap.find(aName) == iColorMap.end())
            iColorMap[aName] = iColorMap.size();

        return iColorProvider.GetColor(iColorMap[aName]);
    }

    virtual void UpdateTimeEnd(double aTimeEnd)
    {
    }

protected:
    CColorProvider iColorProvider;
    std::string iName;
    QColor iColor;
    std::vector<TPGraphElement> iSubElements;
    std::map<std::string, int> iColorMap;
};

class CGraphState : public CGraphElement
{

public:
    CGraphState (const QRectF& aParent, double aTime, const std::string aName, const QColor& aColor) :
        iTime (aTime), CGraphElement(aName, aColor)
    {
        iY0 = aParent.topLeft().y()*0.9+aParent.bottomRight().y()*0.1;
        iY1 = aParent.topLeft().y()*0.1+aParent.bottomRight().y()*0.9;
    }

    virtual QRectF GetBox()
    {
        return QRectF(QPointF(iTime, iY0), QPointF(iTime+1, iY1));
    }

    virtual std::pair<QPointF, QPointF> GetBoxPair()
    {
        return std::make_pair(QPointF(iTime, iY0), QPointF(iTime, iY1));
    }

    virtual QRectF GetDataBox()
    {
        return QRectF(QPointF(iTime, iY0),
                      QPointF(iTime+1, iY1));
    }

    virtual void Draw (QPainter &aPainter, const QRectF& aWorldWindow, const QRect& aViewPort)
    {
        aPainter.setRenderHint(QPainter::Antialiasing, false);
        aPainter.setBrush(Qt::NoBrush);
        QPen penBText(QColor("#FF0000"));
        aPainter.setPen(penBText);
        auto line = WorldWindowPairToViewPort(GetBoxPair(), aWorldWindow, aViewPort);
        aPainter.drawLine (line.first, line.second);
        aPainter.setBrush(QColor("#FFFFFF"));
        aPainter.drawEllipse(line.second, 5, 5);
    }

    virtual void UpdateTimeEnd(double aTimeEnd)
    {
    }

private:
    double iTime;
    double iY0;
    double iY1;

};

class CGraphEvent : public CGraphElement
{
public:
    CGraphEvent (const QRectF& aParent, double aTimeStart, double aTimeEnd, const std::string aName, const QColor& aColor) :
        iTimeStart (aTimeStart), iTimeEnd(aTimeEnd), CGraphElement(aName, aColor)
    {
        iY0 = aParent.topLeft().y()*0.75+aParent.bottomRight().y()*0.25;
        iY1 = aParent.topLeft().y()*0.25+aParent.bottomRight().y()*0.75;
    }

    virtual QRectF GetBox()
    {
        return QRectF(QPointF(iTimeStart, iY0), QPointF(iTimeEnd, iY1));
    }

    virtual std::pair<QPointF, QPointF> GetBoxPair()
    {
        return std::make_pair(QPointF(iTimeStart, iY0), QPointF(iTimeEnd, iY1));
    }

    virtual QRectF GetDataBox()
    {
        return QRectF(QPointF(iTimeStart == -std::numeric_limits<double>::infinity()? iTimeEnd : iTimeStart, iY0),
                      QPointF(iTimeEnd == std::numeric_limits<double>::infinity()? iTimeStart : iTimeEnd, iY1));
    }

    virtual void Draw(QPainter &aPainter, const QRectF& aWorldWindow, const QRect& aViewPort)
    {

        if (iTimeStart != -std::numeric_limits<double>::infinity()) {

            aPainter.setRenderHint(QPainter::Antialiasing, false);
            aPainter.setBrush(Qt::NoBrush);
            aPainter.fillRect(WorldWindowRectToViewPort(GetBoxPair(), aWorldWindow, aViewPort), iColor);
            QPen penBText(QColor("#000000"));
            aPainter.setPen(penBText);
            aPainter.drawRect(WorldWindowRectToViewPort(GetBoxPair(), aWorldWindow, aViewPort));

            if (iTimeEnd == std::numeric_limits<double>::infinity())
                aPainter.drawText(WorldWindowRectToViewPort(GetBoxPair(), aWorldWindow, aViewPort).adjusted(30, 0, 0, 0),
                                        Qt::AlignLeft | Qt::AlignVCenter, QString(GetName().c_str()));
            else
                aPainter.drawText(WorldWindowRectToViewPort(GetBoxPair(), aWorldWindow, aViewPort),
                              Qt::AlignCenter | Qt::AlignVCenter, QString(GetName().c_str()));
        }
        else {
            aPainter.setRenderHint(QPainter::Antialiasing, false);
            aPainter.setBrush(Qt::NoBrush);
            QPen penBText(QColor("#000000"));
            aPainter.setPen(penBText);
            aPainter.fillRect(WorldWindowRectToViewPort(GetBoxPair(), aWorldWindow, aViewPort), QColor("#eeeeee"));
            aPainter.drawRect(WorldWindowRectToViewPort(GetBoxPair(), aWorldWindow, aViewPort));
        }
    }

    virtual void UpdateTimeEnd(double aTimeEnd)
    {
        iTimeEnd = aTimeEnd;
    }

private:
    double iTimeEnd;
    double iTimeStart;
    double iY0;
    double iY1;
};

class CGraphObject : public CGraphElement
{
public:
    CGraphObject (int aIndex, const std::string aName, const QColor& aColor) :
        iY0 (aIndex*iSize), iY1((aIndex+1)*iSize), CGraphElement(aName, aColor) {}

    virtual QRectF GetBox()
    {
        if (iSubElements.size() == 0)
            return QRectF(QPointF(-std::numeric_limits<double>::infinity(), iY0), QPointF(std::numeric_limits<double>::infinity(), iY1));
        QPointF tl = iSubElements.front()->GetBox().topLeft();
        QPointF br = iSubElements.back()->GetBox().bottomRight();
        return QRectF(QPointF(tl.x(), iY0), QPointF(br.x(), iY1));
    }
    virtual QRectF GetDataBox()
    {
        QPointF tl = iSubElements.front()->GetDataBox().topLeft();
        QPointF br = iSubElements.back()->GetDataBox().bottomRight();
        return QRectF(QPointF(tl.x(), iY0), QPointF(br.x(), iY1));
    }

    virtual void AddEvent(const CEvent &aEvent)
    {
        if (aEvent.iType == CEvent::EStateChange) {
            if (iLastState)
                iLastState->UpdateTimeEnd(aEvent.iBegin);

            AddElement (std::make_shared<CGraphEvent>(GetBox(), aEvent.iBegin, std::numeric_limits<double>::infinity(), aEvent.iName, AssignColor(aEvent.iName)));
            iLastState = iSubElements.back();
        }
        else
            AddElement (std::make_shared<CGraphState>(GetBox(), aEvent.iBegin, aEvent.iName, QColor(255,255,255)));
    }

    virtual void Draw (QPainter &aPainter, const QRectF& aWorldWindow, const QRect& aViewPort)
    {
        aPainter.setRenderHint(QPainter::Antialiasing, false);
        aPainter.setBrush(Qt::NoBrush);
        aPainter.fillRect(WorldWindowRectToViewPort(GetBox(), aWorldWindow, aViewPort), iColor);
        CGraphElement::Draw(aPainter, aWorldWindow, aViewPort);
        DrawCaption(aPainter, aWorldWindow, aViewPort);
    }

    void DrawCaption(QPainter &aPainter, const QRectF& aWorldWindow, const QRect& aViewPort)
    {
        auto r = WorldWindowRectToViewPort(GetBox(), aWorldWindow, aViewPort);
        int width=GetCaptionWidth();
        auto r2 = QRect(r.topLeft().x(), r.topLeft().y(), width, r.height());
        QPen penHText(QColor("#ffffff"));
        aPainter.setPen(penHText);
        aPainter.fillRect(r2, QColor(31, 64, 97, 230));
        aPainter.drawText(r2, Qt::AlignCenter | Qt::AlignVCenter, QString(GetName().c_str()));
        QPen penBText(QColor("#000000"));
        aPainter.setPen(penBText);
    }

    static int GetCaptionWidth()
    {
        QFont f;
        QFontMetrics fm(f);
        return fm.width(MaxObjectName);
    }

    static constexpr double iSize = 10.0;
private:

    TPGraphElement iLastState;
    double iY0;
    double iY1;
    static const char MaxObjectName[];
};

class CGraphObjectGroup : public CGraphElement
{
public:
    CGraphObjectGroup (const std::string aName, const QColor& aColor) :
        CGraphElement(aName, aColor) {}
    virtual QRectF GetBox()
    {
        if (iSubElements.size() == 0)
            return QRectF(QPointF(0, 0), QPointF(0, 0));
        QPointF tl = iSubElements.front()->GetBox().topLeft();
        QPointF br = iSubElements.back()->GetBox().bottomRight();
        return QRectF(QPointF(tl.x(), tl.y()), QPointF(br.x(), br.y()));
    }
    virtual QRectF GetDataBox()
    {
        if (iSubElements.size() == 0)
            return QRectF(QPointF(0, 0), QPointF(0, 0));

        auto min = std::min_element(iSubElements.begin(), iSubElements.end(),
                                    [] (const TPGraphElement& aFirst, const TPGraphElement& aSecond)
        {return aFirst->GetDataBox().topLeft().x() < aSecond->GetDataBox().topLeft().x();});

        auto max = std::max_element(iSubElements.begin(),iSubElements.end(),
                                    [] (TPGraphElement& aFirst, TPGraphElement& aSecond)
        {return aFirst->GetDataBox().bottomRight().x() < aSecond->GetDataBox().bottomRight().x();});

        QPointF tl = iSubElements.front()->GetDataBox().topLeft();
        QPointF br = iSubElements.back()->GetDataBox().bottomRight();
        return QRectF(QPointF((*min)->GetDataBox().topLeft().x(), tl.y()), QPointF((*max)->GetDataBox().bottomRight().x(), br.y()));
    }

    virtual void AddEvent(const CEvent &aEvent)
    {
        if (iMapObjects.find(aEvent.iObject) == iMapObjects.end())
            AddObject(aEvent.iObject);

        iMapObjects[aEvent.iObject]->AddEvent(aEvent);
    }

    void AddObject(std::string aName)
    {
        auto newobject = std::make_shared<CGraphObject>(iMapObjects.size(), aName, AssignColor(aName));
        newobject->AddEvent(CEvent(-std::numeric_limits<double>::infinity(), "UNKNOWN", aName, CEvent::EStateChange));
        AddElement (newobject);
        iMapObjects[aName] = newobject;
    }

    QColor AssignColor(const std::string aName)
    {
        if (iColorMap.find(aName) == iColorMap.end())
            iColorMap[aName] = iColorMap.size();

        return iColorProvider.GetColorBW(iColorMap[aName]);
    }


private:
    std::map<std::string, TPGraphElement> iMapObjects;
};


class StatesWidget : public QWidget, public IEventCollectionObserver
{
    Q_OBJECT
public:
    void NewElementAdded(const CEvent* aEvent)
    {
        objectgroup.AddEvent(*aEvent);
        if (!iRepaintScheduled)
        {
            QTimer::singleShot(500, this, SLOT(Update()));
            iRepaintScheduled = true;
        }
    }

    void SetEventList(CEventCollection* aEventCollection)
    {
        iEventCollection = aEventCollection;
        aEventCollection->AddObserver(this);
    }

    StatesWidget(QWidget *parent) : QWidget(parent),
        objectgroup("group", QColor()), iWWTopLeft(0, 0), iWWWidth(0),
        iHFactor(4)
    {
        QGridLayout* tmplayout = new QGridLayout(this);
        tmplayout->setContentsMargins(0, 0, 0, 0);

        iVerticalBar = new QScrollBar(Qt::Orientation::Vertical, this);
        iHorizontalBar = new QScrollBar(Qt::Orientation::Horizontal, this);
        tmplayout->addWidget(iVerticalBar, 0, 1, Qt::AlignRight);
        tmplayout->addWidget(iHorizontalBar, 1, 0, Qt::AlignBottom);
        tmplayout->setRowStretch (0, 100);
        tmplayout->setColumnStretch (0, 100);


        QObject::connect(iVerticalBar, SIGNAL(valueChanged(int)),
                         this, SLOT(UpdateWorldWindowPosition(int)));

        QObject::connect(iHorizontalBar, SIGNAL(valueChanged(int)),
                         this, SLOT(UpdateWorldWindowPosition(int)));
    }

    QPushButton *button;
    QScrollBar *iVerticalBar;
    QScrollBar *iHorizontalBar;
    CGraphObjectGroup objectgroup;
    QRectF iWorldWindow;
    const CEventCollection* iEventCollection;

    int GetVScrollLength()
    {
        return GetScrollBox().height()/GetWorldWindowHeight()*height();
    }

    int GetHScrollLength()
    {
        return GetScrollBox().width()/iWWWidth*width();
    }

signals:

public slots:
    void Update()
    {
        UpdateBarsRange();

        if (iWWWidth == 0) {
            iWWWidth = GetScrollBox().width();
            UpdateBarsRange();
            UpdateWorldWindowPosition(0);
        }
        iRepaintScheduled = false;
    }

    void UpdateBarsRange()
    {
        iVerticalBar->setRange(0, GetVScrollLength());
        iVerticalBar->setPageStep(height());

        iHorizontalBar->setRange(0, GetHScrollLength());
        iHorizontalBar->setPageStep(width());
    }

    void UpdateWorldWindowPosition(int pos)
    {
        double h = (double)iVerticalBar->value()/iVerticalBar->maximum();
        double w = (double)iHorizontalBar->value()/iHorizontalBar->maximum();

        iWWTopLeft=QPointF(GetScrollBox().x()+GetScrollBox().width()*w, (GetScrollBox().height()-GetWorldWindowHeight())*h);

        repaint();
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        QPainter painter(this);

        objectgroup.Draw(painter, QRectF(iWWTopLeft.x(), iWWTopLeft.y(), iWWWidth, GetWorldWindowHeight()), QRect(0, 0, width() - 1, height() - 1));

        painter.fillRect(QRect(point1.x(), 0, point2.x()-point1.x(), height()-1), QColor(240, 240, 20, 100));
    }

    void mousePressEvent(QMouseEvent *e)
    {
        point1 = e->pos();
    }

    void mouseReleaseEvent(QMouseEvent *e)
    {
        iWWTopLeft.setX(iWWTopLeft.x()+point1.x()/(double)width()*iWWWidth);
        iWWWidth = GetZoomWidth();
        point2=point1;

        repaint();
    }

    void mouseMoveEvent(QMouseEvent *e)
    {
        point2 = e->pos();
        repaint();
    }

    void keyPressEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key_H)
        {
            iWorldWindow.setHeight(1.1*iWorldWindow.height());
            repaint();
        }
    }

private slots:


private:
    QPainter iPainter;
    double iHFactor;
    QPointF iWWTopLeft;
    QPoint point1, point2;
    double iWWWidth;
    bool iRepaintScheduled;

    double GetWorldWindowHeight()
    {
        QFont f;
        QFontMetrics fm(f);
        int h=fm.height();
        return height()/((double)h*iHFactor)*CGraphObject::iSize;
    }

    QRectF GetScrollBox()
    {
        return objectgroup.GetDataBox().adjusted(-iWWWidth*0.5, 0, iWWWidth*0.5, 0);
    }

    double GetZoomWidth()
    {
        return (point2.x()-point1.x())/(double)width()*iWWWidth;
    }
};

#endif // STATESWIDGET_H
