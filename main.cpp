#include "graphics.h"

using namespace std;
using namespace mssm;

typedef Vec4base<uint8_t> Vec4uc;


enum RaycastHit {
    SKIP,
    MISS,
    HIT
};

enum RaycastHit testLine(Vec2d testRay, Vec2d p0, Vec2d p1) {
    if(abs(p0.y - p1.y) < 0.0001) {
        return SKIP;
    }
    if(p0.x < testRay.x && p1.x < testRay.x) {
        return SKIP;
    }
    if((p0.y > testRay.y && p1.y > testRay.y) || (p0.y < testRay.y && p1.y < testRay.y)) {
        return SKIP;
    }
    double ky = (testRay.y - p0.y) / (p1.y - p0.y);
    double ix = (p1.x - p0.x) * ky + p0.x;
    if(ix < testRay.x) {
        return MISS;
    }
    return HIT;
}


class PointInPolygonApp {
private:
    enum Mode {
        DRAWING,
        TESTING,
        TEST_ALL,
        TEST_ALL_DEBUG
    };

    struct PointColor {
        Vec2d point;
        Vec4uc color;
    };

    Graphics g;
    Array<Vec2d> polygonPoints;
    Array<enum RaycastHit> intersections;
    enum Mode mode;

    Array<Vec2d> pointsInside;
    Array<PointColor> pointsWDebugInfo;
    bool recalculatePointsInside;

public:
    PointInPolygonApp() : g{"Hello Graphics!", 1024, 768} { }

    void run() {
        init();
        main();
        cleanup();
    }
private:
    void tickDrawing() {
        Vec2d mousePos = g.mousePos();
        if(g.onMousePress(MouseButton::Left)) {
            polygonPoints.push_back(mousePos);
        }

        if(g.onKeyPress(Key(' '))) {
            for(size_t i = 0;i < polygonPoints.size();i++) {
                polygonPoints[i] = {g.randomDouble(0, g.width()), g.randomDouble(0, g.height())};
            }
        }
    }

    void tickTesting() {
        Vec2d mousePos = g.mousePos();

        for(size_t i = 0;i < polygonPoints.size();i++) {
            enum RaycastHit raycastHit = testLine(mousePos, polygonPoints[i], polygonPoints[(i + 1) % polygonPoints.size()]);
            intersections[i] = raycastHit;
        }
    }

    void tickTestAll() {
        if(!recalculatePointsInside) {
            return;
        }

        pointsInside.clear();
        pointsWDebugInfo.clear();

        for(size_t ix = 0;ix < g.width();ix++) {
            for(size_t iy = 0;iy < g.height();iy++) {
                Vec2d point{ix, iy};
                // PointColor debugPoint{point, {0, 0, 0, 0}};
                uint32_t intersections = 0;
                for(size_t i = 0;i < polygonPoints.size();i++) {
                    Vec2d point_i = polygonPoints[i];
                    Vec2d point_ip1 = polygonPoints[(i + 1) % polygonPoints.size()];
                    size_t im1 = i == 0 ? polygonPoints.size() - 1 : i - 1;
                    Vec2d point_im1 = polygonPoints[im1];

                    RaycastHit testCLine = testLine(point, point_i, point_ip1);
                    if(testCLine == HIT) {
                        intersections++;

                        if(abs(point_i.y - point.y) < 0.00001) {
                            // debugPoint.color.w = 255;
                            // debugPoint.color.x = 255;

                            double difProd = (point_ip1.y - point_i.y) * (point_im1.y - point_i.y);

                            // if(difProd > 0) {
                            //     debugPoint.color.y = 255;
                            // }
                            if(difProd < 0) {
                                // debugPoint.color.x = 127;

                                intersections--;
                            }
                        }
                    }
                }
                if(intersections % 2 == 1) {
                    pointsInside.push_back(point);
                    // debugPoint.color.w = 255;
                    // debugPoint.color.z = 255;
                }
                // pointsWDebugInfo.push_back(debugPoint);
            }
        }
    }

    void tick() {
        switch(mode) {
        case DRAWING:
            tickDrawing();
            break;
        case TESTING:
            tickTesting();
            break;
        case TEST_ALL:
        case TEST_ALL_DEBUG:
            tickTestAll();
            break;
        }
    }

    void drawPoints() {
        if(polygonPoints.size() > 1) {
            for(size_t i = 0;i < polygonPoints.size();i++) {
                g.line(polygonPoints[i], polygonPoints[(i + 1) % polygonPoints.size()]);
            }
        } else if(polygonPoints.size() > 0) {
            g.point(polygonPoints[0], WHITE);
        }
    }

    void drawDrawing() {
        drawPoints();
    }

    void drawTesting() {
        // drawPoints();

        if(polygonPoints.size() > 1) {
            for(size_t i = 0;i < polygonPoints.size();i++) {
                Color color;
                switch(intersections[i]) {
                case SKIP:
                    color = GREY;
                    break;
                case MISS:
                    color = RED;
                    break;
                case HIT:
                    color = GREEN;
                    break;
                }

                g.line(polygonPoints[i], polygonPoints[(i + 1) % polygonPoints.size()], color);
            }
        } else if(polygonPoints.size() > 0) {
            g.point(polygonPoints[0], WHITE);
        }

        Vec2d mousePos = g.mousePos();
        g.line(mousePos, {g.width(), mousePos.y}, YELLOW);
    }

    void drawTestAll() {
        g.points(pointsInside);
    }

    void drawTestAllDebug() {
        for(PointColor& pc : pointsWDebugInfo) {
            if(pc.color.w == 0) continue;
            Color color = {pc.color.x, pc.color.y, pc.color.z, pc.color.w};
            g.rect(pc.point, 1, 1, TRANSPARENT, color);
        }
    }

    void draw() {
        switch(mode) {
        case DRAWING:
            drawDrawing();
            break;
        case TESTING:
            drawTesting();
            break;
        case TEST_ALL:
            drawTestAll();
            break;
        case TEST_ALL_DEBUG:
            drawTestAllDebug();
            break;
        }
    }


    void init() {
        mode = DRAWING;

        polygonPoints.clear();
        intersections.clear();

        pointsInside.clear();

        pointsInside.push_back({100, 100});
        pointsInside.push_back({130, 100});
        pointsInside.push_back({131, 101});
    }

    void main() {
        while(g.draw()) {
            if(!g.isDrawable()) {
                continue;
            }

            if(g.onKeyRelease(Key('1'))) {
                mode = DRAWING;
            }
            if(g.onKeyRelease(Key('2'))) {
                mode = TESTING;
                intersections.resize(polygonPoints.size(), SKIP);
            }
            if(g.onKeyRelease(Key('3'))) {
                mode = TEST_ALL;
                recalculatePointsInside = true;
            }
            if(g.onKeyRelease(Key('4'))) {
                mode = TEST_ALL_DEBUG;
                recalculatePointsInside = true;
            }

            tick();
            draw();
        }
    }

    void cleanup() {
        // nothing to do here (yet)
    }
};

int main() {
    PointInPolygonApp app{};

    app.run();

    return 0;
}


