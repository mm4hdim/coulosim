#include "vector.h"
#include "charge.h"
#include "renderarea.h"


#include <QElapsedTimer>

RenderArea::RenderArea(QWidget *parent)
    : QWidget{parent}
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    isPlay = false;
    elapsed = 0;
}


QSize RenderArea::minimumSizeHint() const
{
    return QSize(200,200);
}


QSize RenderArea::sizeHint() const
{
    return QSize(400,400);
}


// TODO add charge object into dynamic memory
void RenderArea::addCharge(int px, int py, int vx, int vy, float q, float mass, bool mobile)
{
    if (q == 0 || mass == 0)
        return;
    env.listOfCharge.push_back(Charge(env, px, py, vx, vy, q, mass, mobile));
    update();
}


void RenderArea::setEnv(float magnetic,
                        float electric_x, float electric_y,
                        float speed,
                        float deltaT,
                        float refresh
                        )
{
    env.magneticField = magnetic;
    env.electricField = Vector(electric_x, electric_y);

    this->speed = speed;
    this->refresh = refresh;
    this->repeat = speed * refresh / deltaT;

    env.deltaT = deltaT / 1000; // convert milisecond to second
}


void RenderArea::play()
{
    timer->start(refresh);
    isPlay = true;
}


void RenderArea::pause()
{
    timer->stop();
    isPlay = false;
}


void RenderArea::reset()
{
    pause();
    env.listOfCharge.clear();
    update();
}


void RenderArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);


    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), QBrush(Qt::white));

    painter.drawText(8,15, QString::number(elapsed) + " ms"); //DEBUG
    QElapsedTimer t;
    t.start();
    // Transformations based on real world cordinate system
    painter.translate(width() / 2, height() / 2);
    painter.scale(1,-1);

    drawCoordinates(painter);

    if (isPlay)
        for (unsigned long i = 0; i < repeat; i++)
            updateCharges();

    drawCharges(painter);
    drawElectricField(painter);

    elapsed = t.elapsed();
}


void RenderArea::drawCoordinates(QPainter& painter)
{
    painter.setPen(Qt::red);

    painter.drawLine(-width() / 2, 0, width() / 2, 0);
    painter.drawLine(width() / 2 - 4, -4, width() / 2, 0);
    painter.drawLine(width() / 2 - 4, 4, width() / 2, 0);
    painter.drawText(width() / 2 - 10,-10, tr("x"));

    painter.drawLine(0, -height() / 2, 0, height() / 2);
    painter.drawLine(4, height() / 2 - 4, 0, height() / 2);
    painter.drawLine(-4, height() / 2 - 4, 0, height() / 2);
    painter.drawText(-10, height() / 2 - 10, tr("y"));
}


void RenderArea::drawCharges(QPainter& painter)
{
    //TODO add these QPen to class attribute
    QPen pcharge = QPen(Qt::blue,10,Qt::SolidLine,Qt::RoundCap);
    QPen ncharge = QPen(Qt::red,10,Qt::SolidLine,Qt::RoundCap);

    for (auto& charge : env.listOfCharge) {
        if (charge.getSign())
            painter.setPen(pcharge);
        else
            painter.setPen(ncharge);

        painter.drawPoint(charge.getPosition());
    }
}


void RenderArea::drawElectricField(QPainter &painter)
{
    for (int i = -350; i <= 350; i += 20) {
        for (int j = -350; j <= 350; j += 20) {
            Vector ef = {0,0};

            for (auto& charge : env.listOfCharge) {
                ef = charge.getElectricField(Vector(i,j)) + ef;
            }
            ef = ef + env.electricField;
            ef = ef.normalize();
            ef = ef *10;
    //            painter.setPen(QPen(Qt::darkGray,3,Qt::SolidLine,Qt::RoundCap));
    //            painter.drawPoint(i, j);
            painter.setPen(Qt::darkGray);
            painter.drawLine(i,j,i+ef.getX(),j+ef.getY());
        }
    }
}


void RenderArea::updateCharges()
{
    for (auto& charge : env.listOfCharge) {
        charge.updateForce();
    }

    for (auto& charge : env.listOfCharge) {
        charge.updatePosition();
    }
}
